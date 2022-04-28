/*
 * Based on code by John Finlay
 * http://www.venturii.net/blog/2019/02/19/omron-fins-probcol/
 *
 * This file is licensed under the MIT License as stated below
 *
 * Copyright (c) 2019 John Finlay
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include <signal.h>
#include <unistd.h>

#include <stdio.h>
#include "libfins/include/fins.h"
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


struct fins_sys_tp* connection(

	struct fins_sys_tp	*nullConnection,

	int			localFinsNet,
	int			localFinsNode,
	int			localFinsUnit,

	char 			*remoteAddress,
	int			remotePort,

	int			remoteFinsNet,
	int			remoteFinsNode,
	int			remoteFinsUnit
	
) {

	// error variables
	int errNum = 0;
	int err_max = 10;
	char errNumMsg[64];


	/* Connect to PLC */
	
	struct fins_sys_tp *thisConnection = finslib_tcp_connect(
		nullConnection,		// null pointer to context structure
		remoteAddress,		// IP address of remote PLC
		remotePort,		// TCP port on remote PLC
		localFinsNet,		// local FINS network number
		localFinsNode,		// local FINS node number
		localFinsUnit,		// local FINS unit number
		remoteFinsNet,		// remote FINS network number
		remoteFinsNode,		// remote FINS node number
		remoteFinsUnit,		// remote FINS unit number
		&errNum,		// error code, if an error occured
		err_max			// maximum error code
	);
	if (errNum == 0) {
		fprintf( stderr,"Connected to \n%s:%d\n", remoteAddress, remotePort );
	} else {
		finslib_errmsg(errNum, errNumMsg, 64);
		fprintf( stderr,"Error while connecting to %s:%d [%u] [%s]\n", remoteAddress, remotePort, errNum, errNumMsg );
		finslib_disconnect(thisConnection);
		return NULL;
	}

	// Print specs

	struct fins_cpudata_tp plcSpec;
	int plcSpecRet = finslib_cpu_unit_data_read( thisConnection, &plcSpec );
	if (plcSpecRet == 0) {
		fprintf( stderr, "Model: %s\n", plcSpec.model );
	} else {
		finslib_errmsg( plcSpecRet, errNumMsg, 64 );
		fprintf( stderr,
			"Error reading PLC specs: %s, error_count: %u, sockfd: %u\n",
			errNumMsg,
			thisConnection->error_count,
			thisConnection->sockfd
		);
		finslib_disconnect(thisConnection);
		return NULL;
	}


	// Print status

	struct fins_cpustatus_tp plcStat;
	int plcStatRet = finslib_cpu_unit_status_read(thisConnection, &plcStat);
	if (plcStatRet == 0) {
		fprintf( stderr, "Running: %s\n", plcStat.running ? "true" : "false" );
		fprintf( stderr, "Run Mode: %u\n", plcStat.run_mode );
	} else {
		finslib_errmsg(plcStatRet, errNumMsg, 64);
		fprintf( stderr,"Error reading CPU status: %s\n", errNumMsg);
		fprintf( stderr, "Error Code: %u\n", plcStat.error_code );
		fprintf( stderr, "Error Message: %s\n", plcStat.error_message );
		return NULL;
	}
	
	return thisConnection;

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
	sscanf( argv[B_FINS], "%d,%d,%d", &aFinsNet, &aFinsNode, &aFinsUnit );
	
	// verify:
	fprintf( stderr, "local FINS network: %d\n",	localFinsNet );
	fprintf( stderr, "local FINS node: %d\n", 	localFinsNode );
	fprintf( stderr, "local FINS unit: %d\n", 	localFinsUnit );
	
	fprintf( stderr, "a IP address: %s\n", 		argv[A_ADDR] );
	fprintf( stderr, "a IP port: %d\n", 		aPort );
	fprintf( stderr, "a FINS network: %d\n", 	aFinsNet );
	fprintf( stderr, "a FINS node: %d\n", 		aFinsNode );
	fprintf( stderr, "a FINS unit: %d\n", 		aFinsUnit );
	
	fprintf( stderr, "b IP address: %s\n", 		argv[B_ADDR] );
	fprintf( stderr, "b IP port: %d\n", 		bPort );
	fprintf( stderr, "b FINS network: %d\n", 	bFinsNet );
	fprintf( stderr, "b FINS node: %d\n", 		bFinsNode );
	fprintf( stderr, "b FINS unit: %d\n", 		bFinsUnit );
	

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
