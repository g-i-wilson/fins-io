import paddle.*;

public class FINSMachine extends Loop {

	private String connectionData;
	private static final String commandString = "./fins-io ";
	
	private int timeout;
	
	private boolean connected;
	
	private Map<String,Integer> memoryMap;
		
	
	public Machine (
		
		localFinsAddress, // <net>,<node>,<unit>
		localNetAddress,
		
		remoteFinsAddress, // <net>,<node>,<unit>
		remoteNetAddress,
		remoteNetPort,
		
		timeout
	) {
		connectionData = localFinsAddress+" "+remoteNetAddress+" "+remoteNetPort+" "+remoteFinsAddress;
		
		this.timeout = timeout;
		
		memoryMap = new HashMap<>();
	}
	
	public Machine address ( String address ) {
		memoryMap.put( address, null );
	}
	
	public Map<String,Integer> addresses ( String[] addresses ) {
		Map<String,Integer> addresses = new Map<>();
		for ( String a : addresses ) address(a);
	}
	
	public Machine refresh () throws Exception {
		String readString = "";
		for (String addr : memoryMap) {
			readString += " R "+addr+" 1";
		}
		SystemCommand cmd = new SystemCommand(
			commandString + connectionData + readString+" Q",
			getName(),
			timeout
		);
		cmd.run();
		if (cmd.exitValue() > 0) {
			throw new Exception( this.getClass().getName()+": write failed with exit code "+cmd.exitValue()+"\n"+cmd.stderr().text() );
		}
		for (String tuple : cmd.stdout().text().split("\n")) {
			String[] keyValue = tuple.split(",");
			memoryMap.put( keyValue[0], keyValue[1] );
		}
	}
	
	public void write ( String address, Integer value ) throws Exception {
		SystemCommand cmd = new SystemCommand(
			commandString + connectionData +" W "+address+" 1 "+value+" Q",
			getName(),
			timeout
		);
		cmd.run();
		if (cmd.exitValue() > 0) {
			throw new Exception( this.getClass().getName()+": write failed with exit code "+cmd.exitValue()+"\n"+cmd.stderr().text() );
		}
	}
	
	public Integer read ( String address ) {
		if ( !memoryMap.containsKey(address) ) address( address );
		return memoryMap.get(address); // could be null!
	}
	
	public void init () {
	
	}
	
}
