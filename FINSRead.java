import java.util.*;
import paddle.*;

public class FINSRead extends SystemCommand {

	private Map<String,Integer> memoryMap;

	public static String readString ( String[] addresses ) {
		String str = "";
		for (String addr : addresses) {
			str += " R "+addr+" 1";
		}
		return str;
	}	

	public FINSRead (
		String localFinsAddress, // <net>,<node>,<unit>
		
		String remoteNetAddress,
		String remoteNetPort,
		String remoteFinsAddress, // <net>,<node>,<unit>
		
		String[] addresses,
		Map<String,Integer> memoryMap,
		int timeout
	) {
		super(
			"echo \""+readString(addresses)+" Q\" | ./fins-io "+localFinsAddress+" "+remoteNetAddress+" "+remoteNetPort+" "+remoteFinsAddress,
			remoteNetAddress+":"+remoteNetPort,
			timeout
		);
		this.memoryMap = memoryMap;
	}
	
	public void postExec () {
		System.out.println( "stdout: "+ stdout().text() );
		try {
			for (String tuple : stdout().text().split("\n")) {
				System.out.println( "tuple: "+tuple );
				String[] keyValue = tuple.split(",");
				memoryMap.put( keyValue[0], Integer.valueOf(keyValue[1],16) );
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		System.out.println( memoryMap );
	}
	
	
	
	
	public static void main (String[] args) {
	
		Map<String,Integer> testMap = new HashMap<>();
		
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
			2000
		);
		
		(new Timer()).scheduleAtFixedRate( fr, 0, 3000 );
	}
}
