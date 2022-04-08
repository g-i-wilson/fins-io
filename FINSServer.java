import paddle.*;

public class FINSServer extends ServerState {

	private TemplateFile formHTML;
	private TemplateFile spaHTML;
	private TemplateFile plotlyJS;
	private TemplateFile machinesJSON;
	private String localAddress;
	
	public FINSServer ( String localAddress ) {
		this.localAddress = localAddress;
		plotlyJS = new TemplateFile( "plotly-latest.min.js" );
		formHTML = new TemplateFile( "index.html", "////" );
		machinesJSON = new TemplateFile( "../machines.json" );
		spaHTML = new TemplateFile( "spa.html", "////" );
		System.out.println( "FINS Server using local address: "+localAddress );
	}
	
	public String[] finsCmd ( QueryString qs ) {
		System.out.println( "QueryString: "+qs );
		try {
			FINSCommand fc = new FINSCommand(
				localAddress,
				qs.get		( "remoteAddr"			),
				qs.getInt	( "remotePort", 	9600	),
			
				qs.get		( "memAddr",	   "CIO0000" 	),
				qs.getInt	( "readLength", 	1 	),
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
		return "0x" + String.join( ",0x", hexValues );
	}

	public String jsonHex ( String[] hexValues ) {
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
			session.response().setMIME( "application/json" );
			session.response().setBody( machinesJSON.toString() );
			
		} else {
			QueryString qs = new QueryString( session.request().body() );
			formHTML
				.replace( "remoteAddr", qs.get("remoteAddr") )
				.replace( "memAddr", qs.get("memAddr") )
				.replace( "readLength", qs.get("readLength") )
				.replace( "readData", jsHex( finsCmd( qs ) ) )
			;
			session.response().setBody( formHTML.toString() );
			
		}
	}
	
	public static void main ( String[] args ) {
		ServerState state = new FINSServer( args[0] );
		new ServerHTTP( state, 8080, "FINS Server" );
	}

}
