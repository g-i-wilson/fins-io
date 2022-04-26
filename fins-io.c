/*
 * Based on code by John Finlay
 * http://www.venturii.net/blog/2019/02/19/omron-fins-protocol/
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

#include <stdio.h>
#include "libfins/include/fins.h"
// git repo:	https://github.com/lammertb/libfins
// compile:	gcc -o fins-io fins-io.c -L /home/giw/fins-io/libfins/lib/ -l fins


// command line arguments
#define LOCAL_FINS_ADDR 1

#define REMOTE_IP_ADDR 2
#define REMOTE_IP_PORT 3

#define REMOTE_FINS_ADDR 4


int hex_char_to_int( char c ) {
	if (c>=48 && c<=57) {
		return ( (int)c - 48 );
	} else if (c>=65 && c<=70) {
		return ( (int)c - 55 );
	} else {
		return 0;
	}
}

int hex_str_to_int( char str[], int start, int length ) {
	int val;
	if ( (hex_char_to_int(str[start]) & 0x8) == 0x8 ) { // negative value
		val = -1; // all 1s
	} else {
		val = 0; // all 0s
	}
	for (int i=start; i<start+length; i++) {
		val = val << 4; // create 8 0s
		val |= ( hex_char_to_int(str[i]) & 0xf );
	}
	return val;
}


int main(int argc, char* argv[]) {

	/* Init */

	// need at least an IP address and read address
	if (argc < 4) {
		fprintf( stderr, "Arguments: <LOCAL_FINS> <REMOTE_IP> <REMOTE_FINS>\n");
		fprintf( stderr, "example execute: 1,2,0 10.0.0.3 9600 1,3,0\n" );
		fprintf( stderr, "example stdin: w DM500 2 ffff 1212 r DM501 1 q\n" );
		return 1;
	}
	
	// read/write
	char command[2] = "R";

	// memory address string
	char memAddress[20] = "DM0";
	
	// operation length
	int length = 1;
	
	
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
	
	// verify:
	fprintf( stderr, "local FINS network: %d\n",localFinsNet );
	fprintf( stderr, "local FINS node: %d\n", localFinsNode );
	fprintf( stderr, "local FINS unit: %d\n", localFinsUnit );
	
	fprintf( stderr, "remote IP address: %s\n", argv[REMOTE_IP_ADDR] );
	fprintf( stderr, "remote IP port: %d\n", remotePort );
	
	fprintf( stderr, "remote FINS network: %d\n", remoteFinsNet );
	fprintf( stderr, "remote FINS node: %d\n", remoteFinsNode );
	fprintf( stderr, "remote FINS unit: %d\n", remoteFinsUnit );
	
	// libfins variables
	int err = 0, err_max = 10;
	char err_msg[64];
	struct fins_sys_tp *c = NULL;


	/* Connect to PLC */
	
	struct fins_sys_tp *sys = finslib_tcp_connect(
		c,			// null pointer to context structure
		argv[REMOTE_IP_ADDR],
		remotePort,		// TCP port
		localFinsNet,		// local FINS network number
		localFinsNode,		// local FINS node number
		localFinsUnit,		// local FINS unit number
		remoteFinsNet,		// remote FINS network number
		remoteFinsNode,		// remote FINS node number
		remoteFinsUnit,		// remote FINS unit number
		&err,			// error code, if an error occured
		err_max			// maximum error code
	);
	
	if (err == 0) {
		fprintf( stderr,"Connected to %s:%d...\n", argv[REMOTE_IP_ADDR], remotePort );
	} else {
		finslib_errmsg(err, err_msg, 64);
		fprintf( stderr,"Error while connecting: [%u] [%s]\n", err, err_msg);
		finslib_disconnect(sys);
		return err;
	}

	 

	// Get PLC details

	struct fins_cpudata_tp cpudata;
	int plc_commandl = finslib_cpu_unit_data_read(
		sys,
		&cpudata
	);
	finslib_errmsg(
		plc_commandl,
		err_msg,
		64
	);
	if (plc_commandl > 0) {
		fprintf( stderr,
			"Error reading PLC specs: [%d] [%s] - sys->error_count = [%u] sockfd: [%u]\n",
			plc_commandl,
			err_msg,
			sys->error_count,
			sys->sockfd
		);
		finslib_disconnect(sys);
		return plc_commandl;
	} else {
		fprintf( stderr, "Model: %s\n", cpudata.model );
	}


	// Print CPU Status

	struct fins_cpustatus_tp cpustat;
	int cpustat_ret = finslib_cpu_unit_status_read(sys, &cpustat);

	if (cpustat_ret > 0) {
		finslib_errmsg(cpustat_ret, err_msg, 64);
		fprintf( stderr,"Error reading CPU status: [%d] [%s]\n", cpustat_ret, err_msg);
		fprintf( stderr, "Error Code: %u\n", cpustat.error_code );
		fprintf( stderr, "Error Message: %s\n", cpustat.error_message );
	} else {
		fprintf( stderr, "Running: %s\n", cpustat.running ? "true" : "false" );
		fprintf( stderr, "Run Mode: %u\n", cpustat.run_mode );
	}

		
	/* Continuous R/W operations from stdin */

	while (1) {
	
		scanf( "%1s", command );
		
		if (command[0] == 'R' || command[0] == 'r') {
			scanf( "%19s %d", memAddress, &length );
			fprintf( stderr, "READ %s (%d)\n", memAddress, length );		
			/* Read */
			uint16_t readBuf[length];
			int read_ret = finslib_memory_area_read_uint16(
				sys,
				memAddress,
				readBuf,
				length
			);
			if (read_ret > 0) {
				finslib_errmsg(read_ret, err_msg, 64);
				fprintf( stderr, "READ error: [%d] [%s]\n", read_ret, err_msg);
				finslib_disconnect(sys);
				return read_ret;
			}

			printf( "%s", memAddress );
			for (int i=0; i<length; i++) {
				printf( ",%04x", readBuf[i] );
			}
			printf( "\n" );

			finslib_errmsg(read_ret, err_msg, 64);
			fprintf( stderr, "READ: [%s]\n", err_msg);
		
		} else if (command[0] == 'W' || command[0] == 'w') {
			scanf( "%19s %d", memAddress, &length );
			fprintf( stderr, "WRITE %s (%d)\n", memAddress, length );		
			/* Write */
			uint16_t writeBuf[length];
			fprintf( stderr, "%s", memAddress );
			for (int i=0; i<length; i++) {
				unsigned int writeBufTemp;
				scanf( "%04x", &writeBufTemp );
				writeBuf[i] = (uint16_t)writeBufTemp;
				fprintf( stderr, ",%x", writeBuf[i] );
			}
			fprintf( stderr, "\n" );
			int write_ret = finslib_memory_area_write_uint16(
				sys,
				memAddress,
				writeBuf,
				length
			);
			if (write_ret > 0) {
				finslib_errmsg(write_ret, err_msg, 64);
				fprintf( stderr, "WRITE error: [%d] [%s]\n", write_ret, err_msg);
				finslib_disconnect(sys);
				return write_ret;
			}
			
			finslib_errmsg(write_ret, err_msg, 64);
			fprintf( stderr, "WRITE: [%s]\n", err_msg);
		
		} else if (command[0] == 'Q' || command[0] == 'q') {
			break;
		}

	}
	

	finslib_disconnect(sys);
	fprintf( stderr,"Connection closed.\n");
	return 0;
}
