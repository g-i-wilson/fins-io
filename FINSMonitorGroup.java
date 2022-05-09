import java.util.*;
import paddle.*;

public class FINSMonitorGroup {

	private List<Timer> timers;
	private Map<String,Map<String,String>> memoryMaps;


	public FINSMonitorGroup (
		String localFinsAddress,
		String dbAddress,
		int dbPort,
		String machinesPath,
		String eventsPath
	) throws Exception {
		timers = new ArrayList<>();
		memoryMaps = new HashMap<>();
		
		String machinesStr =
			(new OutboundTCP( dbAddress, dbPort, "GET /"+machinesPath+" HTTP/1.1\r\n\r\n" ))
			.receive()
			.text();
			
		//System.out.println( machinesStr );
		
		String[] machines = machinesStr.split("\n");
		
		//for (String s : machines ) {
		//	System.out.println( "line: "+s );
		//}
			
		for (int i=4; i<machines.length-1; i++) {
		
			//System.out.println( "processing line: "+machines[i] );
			String[] machineData = machines[i].split(",");
			Map memoryMap = new HashMap<String,String>();
			memoryMaps.put( machineData[0], memoryMap );
			
			if (machineData.length > 8) {
				FINSMonitor fm = new FINSMonitor(
					localFinsAddress,
					
					machineData[0],
					Integer.parseInt(machineData[1]),
					machineData[2],
					
					Arrays.copyOfRange(machineData, 5, machineData.length-1),
					memoryMap,
					
					3000,
					
					dbAddress,
					dbPort,
					eventsPath,
					
					true
				);
				Timer t = new Timer();
				t.scheduleAtFixedRate( fm, 0, 3000 );
				timers.add( t );
			}
		}
	}
	
	public String toString () {
		return memoryMaps.toString();
	}
	
	
	public static void main ( String[] args ) throws Exception {
		FINSMonitorGroup fmg = new FINSMonitorGroup(
			args[0],
			args[1],
			Integer.parseInt(args[2]),
			args[3],
			args[4]
		);
		
		while(true) {
			Thread.sleep(2000);
			System.out.println( fmg );
		}
	}


}
