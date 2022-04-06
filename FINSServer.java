import paddle.*;

public class FINSServer extends ServerState {

	private TemplateFile spa;
	
	public FINSServer () {
		spa = new TemplateFile( "index.html", "////" );
	}
	
	public FINSCommand finsCmd ( QueryString qs ) {
		try {
			return new FINSCommand(
				qs.get		( "localAddr" 			),
				qs.get		( "remoteAddr"			),
				qs.getInt	( "remotePort", 	9600	),
			
				qs.get		( "memAddr",	   "CIO0000" 	),
				qs.getInt	( "readLength", 	1 	),
				qs.get		( "writeData" 			).split("\\%2C")
			
			);
		} catch (Exception e) {
			e.printStackTrace();
			return null;
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
				.replace( "localAddr", qs.get("localAddr") )
				.replace( "remoteAddr", qs.get("remoteAddr") )
				.replace( "memAddr", qs.get("memAddr") )
				.replace( "readLength", qs.get("readLength") )
				.replace( "readData", finsCmd( qs ).toString() )
			;
			session.response().setBody( spa.toString() );
			
		}
	}
	
	public static void main ( String[] args ) {
		ServerState state = new FINSServer();
		new ServerHTTP( state, 8080, "FINS Server" );
	}

}
