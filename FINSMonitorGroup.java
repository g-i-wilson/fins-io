import java.util.*;
import paddle.*;
import creek.*;

public class FINSMonitorGroup {
	
	private CSVLookup machines;
	private List<Timer> timers;
	private Map<String,Map<String,String>> memoryMaps;


	public FINSMonitorGroup (
		FINSAddress localFinsAddress,
		
		String dbAddress,
		int dbPort,
		
		String machinesPath,
		String eventsPath
		
	) throws Exception
	{

		machines = new CSVLookup(
			(new OutboundTCP( dbAddress, dbPort, "GET /"+machinesPath+" HTTP/1.1\r\n\r\n" ))
			.receive()
			.text()
		);
		
		System.out.println( "machines: "+machines );
		
		timers = new ArrayList<>();
		memoryMaps = new HashMap<>();
					
		for (int i=3; i<machines.length(); i++) {
		
			Map<String,String> memoryMap = new HashMap<>();
			memoryMaps.put( machines.rowLookup(2,i,"plc.NetworkAddress"), memoryMap );
			
			try {
				FINSMonitor fm = new FINSMonitor(
					localFinsAddress,
					
					machines.rowLookup(2,i,"plc.NetworkAddress"),
					Integer.parseInt(machines.rowLookup(2,i,"plc.NetworkPort")),
					
					new FINSAddress(
						machines.rowLookup(2,i,"plc.FINSNetwork")+","+
						machines.rowLookup(2,i,"plc.FINSNode")+","+
						machines.rowLookup(2,i,"plc.FINSUnit")
					),
					
					new String[]{
						machines.rowLookup(2,i,"machine-data.OnAddress"),
						machines.rowLookup(2,i,"machine-data.CycleAddress"),
						machines.rowLookup(2,i,"machine-data.AlarmAddress")
					},
					memoryMap,
					
					2500,
					
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
			new FINSAddress( args[0] ), // local FINS
			
			args[1], // database network address
			Integer.parseInt(args[2]), // database network port
			
			args[3], // database machines REST path
			args[4] // database events REST path
		);
		
		while(true) {
			Thread.sleep(2000);
			System.out.println( fmg );
		}
	}


}
