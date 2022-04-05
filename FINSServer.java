import java.net.Inet4Address;
import paddle.*;

public class FINSServer extends ServerState {

	private TemplateFile spa;
	
	public FINSServer () {
		spa = new TemplateFile( "index.html", "////" );
	}

	public void respond ( InboundHTTP session ) {
	
		if (session.path("/")) {
			QueryString qs = new QueryString( session.request().body() );
			
			String localAddr	= qs.get	( "localAddr" 			);
			String remoteAddr 	= qs.get	( "remoteAddr"			);
			int remotePort		= qs.getInt	( "remotePort", 	9600	);
			
			String memAddr	 	= qs.get	( "memAddr",	   "CIO0000" 	);
			int readLength 		= qs.getInt	( "readLength", 	1 	);
			String[] writeData	= qs.getCSV	( "writeData" 			);
			
			String readData = "";
			int localIpLast = 1;
			int remoteIpLast = 1;
			try {
				System.out.println( "Connection: "+localAddr+" -> "+remoteAddr );
				localIpLast	= Integer.parseInt( localAddr.split("\\.")[3] );
				remoteIpLast	= Integer.parseInt( remoteAddr.split("\\.")[3] );
			
				int localFinsNet	= qs.getInt	( "localFinsNet", 	1	);
				int localFinsNode	= qs.getInt	( "localFinsNode", localIpLast	);
				int localFinsUnit	= qs.getInt	( "localFinsUnit", 	0	);		
				
				int remoteFinsNet	= qs.getInt	( "remoteFinsNet", 	1	);
				int remoteFinsNode	= qs.getInt	( "remoteFinsNet", remoteIpLast	);
				int remoteFinsUnit	= qs.getInt	( "remoteFinsNet", 	0	);		
				
				SystemCommand sc = new SystemCommand(
					"./fins-io-cmd "+
					localFinsNet+","+localFinsNode+","+localFinsUnit+" "+
					remoteAddr+" "+
					remotePort+" "+
					remoteFinsNet+","+remoteFinsNode+","+remoteFinsUnit+" "+
					memAddr+" "+readLength+" "+
					String.join( " ", writeData ),
					
					"FINS command",
					
					2000
				);
				sc.run();
				readData = sc.stdout().text();
				//session.response().setBody( "application/json" );
				//session.response().setBody( "{\"readData\":["+sc+"]}" );
			
			} catch (Exception e) {
				e.printStackTrace();
			}

			spa
				.replace( "localAddr", localAddr )
				.replace( "remoteAddr", remoteAddr )
				.replace( "memAddr", memAddr )
				.replace( "readLength", readLength+"" )
				.replace( "readData", readData )
			;
			session.response().setBody( spa.toString() );
		}
	}
	
	public static void main ( String[] args ) {
		ServerState state = new FINSServer();
		new ServerHTTP( state, 8080, "FINS Server" );
	}

}
