import paddle.*;

public class FINSServer extends ServerState {

	public void respond ( InboundHTTP session ) {	
		QueryString qs = new QueryString( session.request().body() );
		
		String localAddr	= qs.get	( "localAddr"		);
		String remoteAddr 	= qs.get	( "remoteAddr"		);
		int remotePort		= qs.getInt	( "remotePort", 9600	);
		
		int 
		
		String mode 		= qs.get	( "mode", 	"R" 	);
		String memSpace 	= qs.get	( "memSpace", 	"CIO" 	);
		int memAddr 		= qs.get	( "memAddr", 	"0000" 	);
		int length 		= qs.getInt	( "length", 	1 	);
		String data 		= qs.get	( "data" 		);
		
		
	}
	
	public static void main ( String[] args ) {
		new HTTPServer(      );
		new FINSServer();
	}

}
