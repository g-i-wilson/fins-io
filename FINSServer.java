import paddle.*;
import canoedb.*;

public class FINSServer extends ServerState {

	private Database database;

	private TemplateFile formHTML;
	private TemplateFile spaHTML;
	private TemplateFile plotlyJS;
	private String localAddress;
	
	public FINSServer ( String localAddress ) {
		this.localAddress = localAddress;
		
		database = new Database( "../sanoh-machines-db" );
		
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
			
		} else if (session.path("/machines")) {
			Query machineList = database.query( session.connectionId() )
				.input( "machines", "Customer", "" )
				.output( "machines", "Customer" )
				.output( "machines", "Type" )
				.output( "machines", "Part Number" )
				.output( "machines", "Network Address" )
				.output( "machines", "Map X" )
				.output( "machines", "Map Y" )
				.execute();
			;
			System.out.println( machineList.toString() );
			session.response().setMIME( "application/json" );
			session.response().setBody( machineList.rows().toJSON() );
			
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
		new ServerHTTP(
			new FINSServer( args[0] ), // args[0]: local IP address to use
			Integer.parseInt( args[1] ), // args[1]: local port to use
			"FINS Server"
		);
	}

}
