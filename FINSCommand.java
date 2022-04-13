import paddle.*;

public class FINSCommand extends SystemCommand {

	private static final String execName = "fins-io-cmd";
	private String[] readData;

	private static int lastOctet ( String ipv4Address ) throws Exception
	{
		System.out.println( "IPv4 address: "+ipv4Address );
		return Integer.parseInt( ipv4Address.split("\\.")[3] );
	}


	public FINSCommand (
		String 	localAddress,
		String 	remoteAddress,
		int	remotePort,
		
		String	memAddr,
		int	readLength,
		String[] writeData
	) throws Exception
	{
		this(
			1, lastOctet( localAddress ), 0,
			localAddress, remoteAddress, remotePort,
			1, lastOctet( remoteAddress ), 0,
			memAddr, readLength, writeData
		);
	}
	
	public FINSCommand (
		int 	localFinsNet,
		int	localFinsNode,
		int	localFinsUnit,
		
		String 	localAddress,
		String 	remoteAddress,
		int 	remotePort,
		
		int	remoteFinsNet,
		int	remoteFinsNode,
		int 	remoteFinsUnit,
		
		String	memAddr,
		int	readLength,
		String[] writeData
	) throws Exception
	{
		super(
			"./"+execName+" "+
			localFinsNet+","+localFinsNode+","+localFinsUnit+" "+
			remoteAddress+" "+
			remotePort+" "+
			remoteFinsNet+","+remoteFinsNode+","+remoteFinsUnit+" "+
			memAddr+" "+readLength+" "+
			String.join( " ", writeData ),
			
			execName,
			
			5000 // waits this long before destroying or forciblyDestroying process
		);
		run();
		if (exitValue() > 0) {
			throw new Exception( this.getClass().getName()+": "+execName+" exit code "+exitValue()+"\n"+stderr().text() );
		} else {
			readData = stdout().text().trim().split(",");
		}
		//try {
		//	readData = stdout().text().trim().split(",");
		//} catch (Exception e) {
		//	readData = new String[]{};
		//	e.printStackTrace();
		//}
	}
	
	public String[] hexValues () {
		return readData;
	}
	
	public String toString () {
		return "0x"+String.join(",0x", readData);
	}
	
	
	// testing
	public static void main (String[] args) throws Exception {
	
		FINSCommand fc = new FINSCommand(
			args[0],
			args[1],
			9600,
			args[2],
			Integer.parseInt( args[3] ),
			args[4].split(",")
		);
		
		System.out.println( "\nData: "+fc );
	
	}

}
