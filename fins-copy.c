/*
 * Based on code by John Finlay
 * http://www.venturii.net/blog/2019/02/19/omron-fins-probcol/
 *
 */
 
#include <signal.h>
#include <unistd.h>

#include <stdio.h>
#include "libfins/include/fins.h"
#include "fins-io.h"
// git repo:	https://github.com/lammertb/libfins
// compile:	gcc -o fins-copy fins-copy.c -L /home/giw/fins-io/libfins/lib/ -l fins


// command line arguments

#define LOCAL_FINS	1

#define A_ADDR		2
#define A_PORT		3
#define A_FINS		4

#define B_ADDR		5
#define B_PORT		6
#define B_FINS		7

#define FIRST_MEM_ADDR	8



static bool running = true;
static int signalValue;

void signalHandler(int s) {
	signalValue = s;
	running = false;
}




int main(int argc, char* argv[]) {

	signal(SIGINT, signalHandler);

	/* Init */

	// need at least an IP address and read address
	if (argc < 7) {
		fprintf( stderr, "Arguments: <LOCAL_FINS>  <A_ADDR> <A_PORT> <A_FINS>  <B_ADDR> <B_PORT> <B_FINS>  <ADDR> .. <ADDR>\n");
		fprintf( stderr, "example execute: 1,2,0 10.0.0.3 9600 1,3,0 10.0.0.4 9600 1,4,0 DM100 DM500 DM501\n" );
		return 1;
	}
	
	// local FINS
	// typical: 1,[last_IP_byte],0
	int localFinsNet, localFinsNode, localFinsUnit;
	sscanf( argv[LOCAL_FINS], "%d,%d,%d", &localFinsNet, &localFinsNode, &localFinsUnit );

	// A IP
	// typical: [IP_addr 9600]
	int aPort;
	sscanf( argv[A_PORT], "%d", &aPort );
	// A FINS
	// typical: 1,[last_IP_byte],0
	int aFinsNet, aFinsNode, aFinsUnit;
	sscanf( argv[A_FINS], "%d,%d,%d", &aFinsNet, &aFinsNode, &aFinsUnit );
	
	// B IP
	// typical: [IP_addr 9600]
	int bPort;
	sscanf( argv[B_PORT], "%d", &bPort );
	// B FINS
	// typical: 1,[last_IP_byte],0
	int bFinsNet, bFinsNode, bFinsUnit;
	sscanf( argv[B_FINS], "%d,%d,%d", &bFinsNet, &bFinsNode, &bFinsUnit );
	

	struct fins_sys_tp *aNull = NULL;
	struct fins_sys_tp *aPLC = connection(
		aNull,			// null pointer to context structure
		localFinsNet,		// local FINS network number
		localFinsNode,		// local FINS node number
		localFinsUnit,		// local FINS unit number
		argv[A_ADDR],		// IP address of remote PLC
		aPort,			// TCP port on remote PLC
		aFinsNet,		// remote FINS network number
		aFinsNode,		// remote FINS node number
		aFinsUnit		// remote FINS unit number
	);

	struct fins_sys_tp *bNull = NULL;
	struct fins_sys_tp *bPLC = connection(
		bNull,			// null pointer to context structure
		localFinsNet,		// local FINS network number
		localFinsNode,		// local FINS node number
		localFinsUnit,		// local FINS unit number
		argv[B_ADDR],		// IP address of remote PLC
		bPort,			// TCP port on remote PLC
		bFinsNet,		// remote FINS network number
		bFinsNode,		// remote FINS node number
		bFinsUnit		// remote FINS unit number
	);
	
	
	char errMsg[64];
	

	/* READ & WRITE Loop */
	
	while (1) {
	
		if (!running) {
			finslib_disconnect(aPLC);
			finslib_disconnect(bPLC);
			fprintf( stderr, "Signal: %d\n", signalValue);
			fprintf( stderr, "Connections closed: %s:%d, %s:%d\n", argv[A_ADDR], aPort, argv[B_ADDR], bPort );
			return 0;
		}

		for (int addr=FIRST_MEM_ADDR; addr<argc; addr++) {

			uint16_t rwBuf[1];
			
			int readRet = finslib_memory_area_read_uint16(
				aPLC,		// fins struct
				argv[addr],	// memory address
				rwBuf,		// values array (length 1)
				1		// length
			);
			
			if (readRet > 0) {
				finslib_errmsg(readRet, errMsg, 64);
				fprintf( stderr, "READ error: %s\n", errMsg);
				running = false;
				continue;
			}

			int writeRet = finslib_memory_area_write_uint16(
				bPLC,		// fins struct
				argv[addr], 	// memory address
				rwBuf,		// values array (in this case just length 1)
				1		// length
			);
			
			if (writeRet > 0) {
				finslib_errmsg(writeRet, errMsg, 64);
				fprintf( stderr, "WRITE error: %s\n", errMsg);
				running = false;
				continue;
			}

			fprintf( stderr, "Copied %s=%04x %s:%d --> %s:%d\n", argv[addr], rwBuf[0], argv[A_ADDR], aPort, argv[B_ADDR], bPort );

		}
		
//		sleep(1);		

	}

}
