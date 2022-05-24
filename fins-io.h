/*
 * Based on code by John Finlay
 * http://www.venturii.net/blog/2019/02/19/omron-fins-probcol/
 *
 */
 
#ifndef FINS_IO_HEADER
#define FINS_IO_HEADER

#include <stdio.h>
#include "libfins/include/fins.h"
// git repo:	https://github.com/lammertb/libfins
// compile:	gcc -o fins-xxxx fins-xxxx.c -L /home/<username>/fins-io/libfins/lib/ -l fins


// error variables
int errNum = 0;
int err_max = 10;
char errNumMsg[64];


void print_error ( char* pre_msg, int ret_value ) {
	finslib_errmsg(ret_value, errNumMsg, 64);
	fprintf( stderr, "Error %s: [%d] [%s]\n", pre_msg, ret_value, errNumMsg);
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

	// verify:
	fprintf( stderr, "Connecting to %s:%d...\n",	remoteAddress, remotePort );
	
	fprintf( stderr, "local FINS network: %d\n",	localFinsNet );
	fprintf( stderr, "local FINS node: %d\n", 	localFinsNode );
	fprintf( stderr, "local FINS unit: %d\n", 	localFinsUnit );
	
	//fprintf( stderr, "remote IP address: %s\n", 	remoteAddress );
	//fprintf( stderr, "remote IP port: %d\n", 	remotePort );
	
	fprintf( stderr, "remote FINS network: %d\n", 	remoteFinsNet );
	fprintf( stderr, "remote FINS node: %d\n", 	remoteFinsNode );
	fprintf( stderr, "remote FINS unit: %d\n", 	remoteFinsUnit );


	// Connect to PLC
	
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
		fprintf( stderr,"Connected established with %s:%d\n", remoteAddress, remotePort );
	} else {
		print_error( "while connecting", errNum ), 
		finslib_disconnect(thisConnection);
		return NULL;
	}

	// Print specs

	struct fins_cpudata_tp plcSpec;
	int plcSpecRet = finslib_cpu_unit_data_read( thisConnection, &plcSpec );
	if (plcSpecRet == 0) {
		fprintf( stderr, "Model: %s\n", plcSpec.model );
	} else {
		print_error( "reading PLC specs", plcSpecRet );
		fprintf( stderr, "error_count: %u", thisConnection->error_count );
		fprintf( stderr, "sockfd: %u\n", thisConnection->sockfd );
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
		print_error( "reading CPU status:", plcStatRet );
		fprintf( stderr, "error_code: %u\n", plcStat.error_code );
		fprintf( stderr, "error_message: %s\n", plcStat.error_message );
		return NULL;
	}

	fprintf( stderr, "\n" );
	
	return thisConnection;

}


#endif

