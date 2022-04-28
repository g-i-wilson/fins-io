import java.util.*;
import paddle.*;

public class FINSRead extends SystemCommand {

	private Map<String,String> memoryMap;
	boolean verbose;

	public static String readString ( String[] addresses ) {
		String str = "";
		for (String addr : addresses) {
			str += " "+addr;
		}
		return str;
	}	

	public FINSRead (
		String localFinsAddress, // <net>,<node>,<unit>
		
		String remoteNetAddress,
		String remoteNetPort,
		String remoteFinsAddress, // <net>,<node>,<unit>
		
		String[] addresses,
		Map<String,String> memoryMap,
		int timeout,
		boolean verbose
	) {
		super(
			"./fins-read "+localFinsAddress+" "+remoteNetAddress+" "+remoteNetPort+" "+remoteFinsAddress + readString(addresses),
			remoteNetAddress+":"+remoteNetPort,
			timeout,
			false
		);
		this.memoryMap = memoryMap;
		this.verbose = verbose;
	}
	
	public void postExec () {
		// System.out.println( "stdout:\n"+ stdout().text() );
		try {
			for (String tuple : stdout().text().split("\n")) {
				String[] keyValue = tuple.split(",");
				memoryMap.put( keyValue[0], keyValue[1] );
			}
		} catch (Exception e) {
			System.out.println( this.getClass().getName()+" '"+getName()+"' ERROR:" );
			e.printStackTrace();
		}
		if (verbose) System.out.println( this.getClass().getName()+" '"+getName()+"': "+memoryMap );
	}
	
	
	
	
	public static void main (String[] args) {
	
		Map<String,String> testMap = new HashMap<>();
		
		int firstArgs = 4;
		String[] addresses = new String[args.length-firstArgs];
		for (int i=firstArgs; i<args.length; i++) {
			addresses[i-firstArgs] = args[i];
		}
		
		FINSRead fr = new FINSRead(
			args[0],
			args[1],
			args[2],
			args[3],
			addresses,
			testMap,
			2000,
			true
		);
		
		(new Timer()).scheduleAtFixedRate( fr, 0, 2000 );
	}
}
