import java.util.*;
import paddle.*;

public class FINSMonitor extends FINSRead {

	private String 			dbNetAddress;
	private int 			dbNetPort;
	private String 			dbNetPath;
	private String			remoteNetAddress;
	private int			remoteNetPort;
	private FINSAddress		remoteFinsAddress;
	private boolean			connected = false;
	
	public FINSMonitor (
		
		FINSAddress	localFinsAddress, // <net>,<node>,<unit>
		
		String		remoteNetAddress,
		int		remoteNetPort,
		FINSAddress	remoteFinsAddress, // <net>,<node>,<unit>
		
		String[]	plcMemoryAddresses,
		Map<String,String> plcMemory,
		
		int		period,
		
		String		dbNetAddress,
		int		dbNetPort,
		String		dbNetPath,
		
		boolean		verbose
		
	) {
		super(
			localFinsAddress, // <net>,<node>,<unit>
			remoteNetAddress,
			remoteNetPort,
			remoteFinsAddress, // <net>,<node>,<unit>
			plcMemoryAddresses,
			plcMemory,
			period,
			verbose
		);
		this.dbNetAddress = dbNetAddress;
		this.dbNetPort = dbNetPort;
		this.dbNetPath = dbNetPath;
		this.remoteNetAddress = remoteNetAddress;
		this.remoteNetPort = remoteNetPort;
		this.remoteFinsAddress = remoteFinsAddress;
		plcMemory = new HashMap<>();
		(new Timer()).scheduleAtFixedRate( this, 0, period );
	}
	
	protected void changeEvent ( Map<String,String> changes ) throws Exception {
		String changesStr = 
			"NetworkAddress="+remoteNetAddress+
			"&NetworkPort="+remoteNetPort+
			"&FINSNetwork="+remoteFinsAddress.network()+
			"&FINSNode="+remoteFinsAddress.node()+
			"&FINSUnit="+remoteFinsAddress.unit()
		;
		for (String key : changes.keySet()) {
			changesStr += "&"+key+"="+changes.get(key);
		}
		System.out.println(
			this.getClass().getName()+": "+
			(new OutboundTCP( dbNetAddress, dbNetPort, "GET /"+dbNetPath+"?"+changesStr+" HTTP/1.1\r\n\r\n" ))
				.receive()
				.text()
		);
	}
	
	public void postExec () {
		super.postExec();
		connected = (exitValue()==0);
	}
	
	public boolean connected () {
		return connected;
	}
	
	
	public static void main ( String[] args ) throws Exception {
		new ServerHTTP(
			new ServerState(),
			9090,
			"Test HTTP Server"
		);
		Thread.sleep(3);
		FINSMonitor fm = new FINSMonitor(
			new FINSAddress( "1,10,0" ),
			
			"10.10.0.2",
			9600,
			new FINSAddress( "1,2,0" ),
			
			new String[]{ "DM500", "DM503", "DM507" },
			new HashMap<>(),
			
			3000,
			
			"localhost",
			9090,
			"",
			
			true
		);
		
		while (true) {
			System.out.println( "connected: "+fm.connected() );
			Thread.sleep(1000);
		}
	}

}



