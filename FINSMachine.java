import java.time.ZonedDateTime;
import paddle.*;

public class FINSMachine extends Loop {

	private FINSRead reader;
	//private FINSWrite writer;
	
	private boolean connected;
	
	private Map<String,String> previousMemoryMap;
	private Map<String,String> currentMemoryMap;
	
	private Map<ZonedDateTime,Map<String,String>> events;
	
	
	public Machine (
		
		String localFinsAddress, // <net>,<node>,<unit>
		String localNetAddress,
		
		String remoteNetAddress,
		int remoteNetPort,
		String remoteFinsAddress, // <net>,<node>,<unit>
		
		String[] addresses,
		int timeout,
		int period
	) {
		previousMemoryMap = new LinkedHashMap<>();
		currentMemoryMap = new LinkedHashMap<>();
		reader = new FINSRead(
			localFinsAddress, // <net>,<node>,<unit>
			remoteNetAddress,
			remoteNetPort,
			remoteFinsAddress, // <net>,<node>,<unit>
			addresses,
			memoryMap,
			false
		);
		(new Timer()).scheduleAtFixedRate( reader, period, period );
	}
		
	public String read ( String address ) {
		return currentMemoryMap.get(address); // could be null!
	}
	
//	public void write ( String address, String hexValue ) throws Exception {
//	}

	public boolean connected () {
		return (reader.exitValue() == 0);
	}
	
	public Map<ZonedDateTime,Map<String,String>> events () {
		return events;
	}
	
	public init () {
	
	}
	
	public loop () {
		for (String address : currentMemoryMap) {
			
		}
	}
		
}



