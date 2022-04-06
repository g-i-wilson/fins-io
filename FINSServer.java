import paddle.*;

public class FINSServer extends ServerState {

	private TemplateFile spa;
	private String localAddress;
	
	public FINSServer ( String localAddress ) {
		this.localAddress = localAddress;
		spa = new TemplateFile( "index.html", "////" );
		System.out.println( "FINS Server using local address: "+localAddress );
	}
	
	public String finsCmd ( QueryString qs ) {
		try {
			return new FINSCommand(
				localAddress,
				qs.get		( "remoteAddr"			),
				qs.getInt	( "remotePort", 	9600	),
			
				qs.get		( "memAddr",	   "CIO0000" 	),
				qs.getInt	( "readLength", 	1 	),
				qs.get		( "writeData" 			).split("\\%2C")
			
			).toString();
		} catch (Exception e) {
			e.printStackTrace();
			return "";
		}
	}

	public void respond ( InboundHTTP session ) {
	
		if (session.path("/json")) {
			session.response().setBody( "application/json" );
			session.response().setBody(
				"{\"readData\":["+
				finsCmd( new QueryString( session.request().body() ) )+
				"]}"
			);
		} else if (session.path("/")) {
			QueryString qs = new QueryString( session.request().body() );
			spa
				.replace( "remoteAddr", qs.get("remoteAddr") )
				.replace( "memAddr", qs.get("memAddr") )
				.replace( "readLength", qs.get("readLength") )
				.replace( "readData", finsCmd( qs ) )
			;
			session.response().setBody( spa.toString() );
			
		}
	}
	
	public static void main ( String[] args ) {
		ServerState state = new FINSServer( args[0] );
		new ServerHTTP( state, 8080, "FINS Server" );
	}

}
