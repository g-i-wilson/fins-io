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
// compile:	gcc -o fins_test fins_test.c -L /home/giw/libfins/lib/ -l fins

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

	/* Initialize */

	int err = 0, err_max = 10;
	struct fins_sys_tp *c = NULL;

	// memset(&c, 0, sizeof(struct fins_sys_tp)); // Not needed - finslib_tcp_connect() only sets up default variables if it is passed a NULL pointer.
	// init_system(&c, err_max);

	/* Connect to the PLC */
	printf("Writing to PLC memory: %s %s\n", argv[1], argv[2]);

	struct fins_sys_tp *sys = finslib_tcp_connect(
		c,		// null pointer to context structure
		//"10.10.0.2",	// address
		argv[1],
		9600,		// TCP port
		1,		// local FINS network number
		10,		// local FINS node number
		0,		// local FINS unit number
		1,		// remote FINS network number
		2,		// remote FINS node number
		0,		// remote FINS unit number
		&err,		// error code, if an error occured
		err_max		// maximum error code
	);
	printf("Connection Call:   Error Code was [%u]\n", err);

	char err_msg[64];
	finslib_errmsg(err, err_msg, 64);
	printf("Connection Call:   Error Message Was: [%s]\n", err_msg);

	/* Find out what kind of PLC we are talking to */

	struct fins_cpudata_tp cpudata;

	int cuer = finslib_cpu_unit_data_read(sys, &cpudata);

	finslib_errmsg(cuer, err_msg, 64);
	printf(
			"Read CPU Data:     Error Message Was: [%d] [%s] - sys->error_count = [%u] sockfd: [%u]\n",
			cuer, err_msg, sys->error_count, sys->sockfd);

	/* Read CPU Status */

	struct fins_cpustatus_tp cpustat;
	int cpustat_ret = finslib_cpu_unit_status_read(sys, &cpustat);
	finslib_errmsg(cpustat_ret, err_msg, 64);
	printf("CPU Unit Stat Read Error Message Was: [%s]\n", err_msg);

	/* Read Memory Area */

	uint16_t arr[2048];
	int i;
	for (i = 0; i < 2048; i++) {
		arr[i] = 0;
	} // Could use memset...

	int num = 16;
	//int read_ret = finslib_memory_area_read_uint16(sys, "CIO100.0", arr, num);
	int read_ret = finslib_memory_area_read_uint16(sys, argv[2], arr, num);
	
	finslib_errmsg(read_ret, err_msg, 64);
	printf("Memory Area Read Error Message Was: [%s]\n", err_msg);

	for (i = 0; i < num; i++) {
		printf("arr[%u] = [%u]\n", i, arr[i]);
	}

	/* Write Memory Area */

	for (i = 0; i < 2048; i++) {
		arr[i] = 0;
	} // Again, could use memset...
	for (i = 0; i < argc-3; i++) {
		arr[i] = (uint16_t)hex_str_to_int( argv[i+3], 0, 4 );
	}

	num = 16;
	//int write_ret = finslib_memory_area_write_uint16(sys, "CIO100.0", arr, num);
	int write_ret = finslib_memory_area_write_uint16(sys, argv[2], arr, num);
	finslib_errmsg(write_ret, err_msg, 64);
	printf("Memory Area Write Error Message Was: [%s]\n", err_msg);

	for (i = 0; i < num; i++) {
		printf("arr[%u] = [%u]\n", i, arr[i]);
	}

	/* Read Error Log */

	struct fins_errordata_tp errordat;
	size_t num_to_read = 1;
	size_t num_read = 0;
	int err_ret = finslib_error_log_read(sys, &errordat, 0, &num_to_read,
			&num_read);

	finslib_errmsg(err_ret, err_msg, 64);
	printf(
			"Read Error Log:    Error Message Was: [%s] - Requested: [%zu] Records That Were Read: [%zu]\n",
			err_msg, num_to_read, num_read);

	/* Read CPU Data */

	struct fins_cpudata_tp cpuinfo;

	int cpu_ret = finslib_cpu_unit_data_read(sys, &cpuinfo);

	finslib_errmsg(cpu_ret, err_msg, 64);
	printf("CPU Unit Data Read Error Message Was: [%s]\n", err_msg);

	/* Disconnect */

	finslib_disconnect(sys);
	printf("Connection closed.\n");
	return 0;
}
