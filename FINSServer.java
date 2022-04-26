import paddle.*;
import canoedb.*;

public class FINSServer extends ServerState {

	private Database machines;

	private TemplateFile formHTML;
	private TemplateFile spaHTML;
	private TemplateFile plotlyJS;
	private String localAddress;
	
	private StringMap3D<String> currentMachineInfo;
	private StringMap3D<String> nextMachineInfo;
	
	public FINSServer ( String localAddress, String machinesDir ) {
		this.localAddress = localAddress;
		
		machines = new Database( machinesDir );
		
		plotlyJS = new TemplateFile( "plotly-latest.min.js" );
		formHTML = new TemplateFile( "index.html", "////" );
		spaHTML = new TemplateFile( "spa.html", "////" );
		
		System.out.println( "FINS Server using local address: "+localAddress );
	}
	
	public String[] finsCmd ( QueryString qs ) {
		System.out.println( "QueryString ("+localAddress+"): "+qs );
		try {
			FINSCommand fc = new FINSCommand(
				localAddress,
				qs.getInt	( "remoteFinsNet"	, 0	),
				qs.getInt	( "remoteFinsUnit"	, 0	),
				qs.get		( "remoteAddr"			),
				qs.getInt	( "remotePort" 		, 9600	),
			
				qs.get		( "memAddr"		, "DM0" ),
				qs.getInt	( "readLength"	 	, 1 	),
				qs.get		( "writeData" 			).split("\\%2C")
			
			);
			System.out.println( fc.stderr().text() );
			return fc.hexValues();
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}
	
	public String jsHex ( String[] hexValues ) {
		if (hexValues == null) return "";
		return "0x" + String.join( ",0x", hexValues );
	}

	public String jsonHex ( String[] hexValues ) {
		if (hexValues == null) return "";
		return "\"0x" + String.join( "\",\"0x", hexValues ) + "\"";
	}

	public void respond ( InboundHTTP session ) {
	
		if (session.path("/json")) {
			session.response().setMIME( "application/json" );
			session.response().setBody(
				"{\"readData\":["+
				jsonHex( finsCmd( new QueryString( session.request().body() ) ) )+
				"]}"
			);
		} else if (session.path("/plotly")) {
			session.response().setMIME( "text/javascript" );
			session.response().setBody( plotlyJS.toString() );
		
		} else if (session.path("/spa")) {
			session.response().setBody( spaHTML.toString() );
			
		} else if (
			session.path("/read") ||
			session.path("/write")
		) {
			boolean writeMode = session.path("/write");
			session.response().setMIME( "application/json" );
			session.response().setBody(
				(new Query( session.request().body(), writeMode ))
				.rows()
				.toJSON()
			);
			
		} else if (session.path("/machines")) {
			nextMachineInfo =
				machines
				.query(session.connectionId())
				.output("machines", "Network Address")
				.output("machines", "On Address")
				.output("machines", "On Bit")
				.output("machines", "Cycle Address")
				.output("machines", "Cycle Bit")
				.output("machines", "Alarm Address")
				.output("machines", "Alarm Bit")
				.output("machines", "Accept Count Address")
				.output("machines", "Reject Count Address")
				.execute( "", false, "or", true, true )
				.rows();
			System.out.println( "machineInfo: "+machineInfo );
			for (String machine : machineInfo.keys("machines") ) {
				nextMachineInfo.write( "machines", machine, "On Status", 
				nextMachineInfo.read("machines", machine, "
			}
			session.response().setMIME( "application/json" );
			session.response().setBody( );
			
		} else if (session.path("/")) {
			QueryString qs = new QueryString( session.request().body() );
			formHTML
				.replace( "remoteFinsNet", qs.get("remoteFinsNet") )
				.replace( "remoteFinsUnit", qs.get("remoteFinsUnit") )
				.replace( "remoteAddr", qs.get("remoteAddr") )
				.replace( "remotePort", qs.get("remotePort") )
				.replace( "memAddr", qs.get("memAddr") )
				.replace( "readLength", qs.get("readLength") )
				.replace( "readData", jsHex( finsCmd( qs ) ) )
			;
			session.response().setBody( formHTML.toString() );
			
		}
	}
	
	public static void main ( String[] args ) throws Exception {
		new ServerHTTP( // <local_IP> <local_port> <machines_dir>
			new FINSServer( args[0], args[2] ),
			Integer.parseInt( args[1] ),
			"FINS Server"
		);
	}

}
