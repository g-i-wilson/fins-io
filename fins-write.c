/*
 * Based on code by John Finlay
 * http://www.venturii.net/blog/2019/02/19/omron-fins-protocol/
 *
 */

#include <stdio.h>
#include "libfins/include/fins.h"
#include "fins-io.h"
// git repo:	https://github.com/lammertb/libfins
// compile:	gcc -o fins-write fins-write.c -L /home/giw/fins-io/libfins/lib/ -l fins


// command line arguments
#define LOCAL_FINS_ADDR 1

#define REMOTE_IP_ADDR 2
#define REMOTE_IP_PORT 3

#define REMOTE_FINS_ADDR 4

#define FIRST_MEM_ADDR 5


int main(int argc, char* argv[]) {

	// Init

	// need at least an IP address and read address
	if (argc < 4) {
		fprintf( stderr, "Arguments: <LOCAL_FINS> <REMOTE_IP> <REMOTE_PORT> <REMOTE_FINS> <ADDR> <VAL> ... <ADDR> <VAL>\n");
		fprintf( stderr, "example execute: 1,2,0 10.0.0.3 9600 1,3,0 CIO0 00FF DM10 12 DM500 abcd\n" );
		return 1;
	}
	
	// local FINS
	// typical: 1,[last_IP_byte],0
	int localFinsNet, localFinsNode, localFinsUnit;
	sscanf( argv[LOCAL_FINS_ADDR], "%d,%d,%d", &localFinsNet, &localFinsNode, &localFinsUnit );

	// remote IP
	// typical: [IP_addr 9600]
	char remoteAddress[40];
	int remotePort;
	sscanf( argv[REMOTE_IP_ADDR], "%19s", remoteAddress );
	sscanf( argv[REMOTE_IP_PORT], "%d", &remotePort );
	
	// remote FINS
	// typical: 1,[last_IP_byte],0
	int remoteFinsNet, remoteFinsNode, remoteFinsUnit;
	sscanf( argv[REMOTE_FINS_ADDR], "%d,%d,%d", &remoteFinsNet, &remoteFinsNode, &remoteFinsUnit );
	
		
	// libfins variables
	int err = 0, err_max = 10;
	char err_msg[64];
	struct fins_sys_tp *thisPLC = NULL;

	
	thisPLC = connection(
		thisPLC,		// null pointer to context structure
		localFinsNet,		// local FINS network number
		localFinsNode,		// local FINS node number
		localFinsUnit,		// local FINS unit number
		argv[REMOTE_IP_ADDR],	// IP address of remote PLC
		remotePort,		// TCP port on remote PLC
		remoteFinsNet,		// remote FINS network number
		remoteFinsNode,		// remote FINS node number
		remoteFinsUnit		// remote FINS unit number
	);
	
	if (thisPLC == NULL) {
		return 1;
	}
	

		
	/* WRITE */

	for (int i=FIRST_MEM_ADDR; i<argc; i+=2) {

		if (i+1 >= argc) {
			finslib_disconnect(thisPLC);
			fprintf( stderr, "Note: uneven number of address-value pairs: <ADDR> <VAL> [ .. <ADDR> <VAL>].\n");
			fprintf( stderr, "      Nothing was written to address '%s'.\n", argv[i] );
			return 1;
		}
		
		uint16_t writeBuf[1];
		unsigned int writeBufTemp;
		
		sscanf( argv[i+1], "%04x", &writeBufTemp );
		writeBuf[0] = (uint16_t)writeBufTemp;

		fprintf( stderr, "%s,%x\n", argv[i], writeBuf[0] );

		int write_ret = finslib_memory_area_write_uint16(
			thisPLC,	// fins struct
			argv[i], 	// memory address
			writeBuf,	// values array (in this case just length 1)
			1		// length
		);
		
		if (write_ret > 0) {
			print_error( "during write", write_ret );
			finslib_disconnect(thisPLC);
			return write_ret;
		}
		
	}
	

	finslib_disconnect(thisPLC);
	fprintf( stderr,"Connection closed.\n");
	return 0;
}
