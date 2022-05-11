import java.util.*;
import paddle.*;
import creek.*;

public class FINSMonitorGroup {
	
	private CSVFile machines;
	private List<Timer> timers;
	private Map<String,Map<String,String>> memoryMaps;


	public FINSMonitorGroup (
		String machinesFilePath,
		String localFinsAddress,
		String dbAddress,
		int dbPort,
		String eventsPath
	) throws Exception
	{
		machines = new CSVFile( machinesFilePath );
		timers = new ArrayList<>();
		memoryMaps = new HashMap<>();
					
		for (List<String> machineData : machines.rows()) {
		
			Map memoryMap = new HashMap<String,String>();
			memoryMaps.put( machineData.get(0), memoryMap );
			
			
			
			try {
				FINSMonitor fm = new FINSMonitor(
					localFinsAddress,
					
					machineData.get(0),
					Integer.parseInt(machineData.get(1)),
					machineData.get(2),
					
					String.split( machineData.get(5), "," ),
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
			} catch (Exception e) {
				e.printStackTrace();
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
