/*
 * obscmd.c
 *
 * Bridge to obs scripts to allow SUID bit set
 *
 * Copyright (c) 2018 Terence Kelleher. All rights reserved.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char getArgList[] = "sSoc";

int
main( int argc, char **argv )
{	
	opterr = 0;
	int c;
	
	c = getopt(argc, argv, getArgList );
	switch (c)
	{
		case 's': // start
			printf("Start\n");
			execlp("sudo", "sudo", "-i", "-u", "vitals", "obs_start.sh", NULL );
			break;
		case 'S': // stop
			printf("Stop\n" );
			execlp("sudo", "sudo", "-i", "-u", "vitals", "obs_stop.sh", NULL );
			break;
		case 'o': // open
			printf("Open\n");
			execlp("sudo", "sudo", "-i", "-u", "vitals", "obs_open.sh", NULL );
			break;
		case 'c': // close
			printf("Close\n");
			execlp("sudo", "sudo", "-i", "-u", "vitals", "obs_close.sh", NULL );
			break;
		default:
			printf("Not found : '%c'\n", c );
			break;
	}

	return ( 0 );
}
