public class FINSAddress {
	
	private int[] finsId;
	
	private void defaultFinsId ( String finsStr ) throws Exception {
		finsId[0] = 0;
		finsId[1] = Integer.parseInt( finsStr );
		finsId[2] = 0;
	}
	
	public FINSAddress ( String finsStr ) throws Exception {
		finsId = new int[3];
		if (finsStr.indexOf(",")>-1) {
			String[] finsStrArr = finsStr.split(",");
			for (int i=0; i<3; i++) finsId[i] = Integer.parseInt(finsStrArr[i]);
		} else if (finsStr.indexOf(".")>-1) {
			String[] finsStrArr = finsStr.split("\\.");
			defaultFinsId( finsStrArr[3] );
		} else {
			defaultFinsId( finsStr );
		}
	}
	
	public int network () {
		return finsId[0];
	}
	
	public int node () {
		return finsId[1];
	}
	
	public int unit () {
		return finsId[2];
	}
	
	public String toString () {
		return finsId[0]+","+finsId[1]+","+finsId[2];
	}
	
	public static void main ( String[] args ) throws Exception {
		System.out.println( new FINSAddress("1,2,3") );
		System.out.println( "node: "+(new FINSAddress("10,20,30")).node() );
		System.out.println( new FINSAddress("10.10.10.100") );
	}
	
}
