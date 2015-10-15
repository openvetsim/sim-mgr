/*
 * simmgr.cpp
 *
 * SimMgr deamon.
 *
 * Copyright 2015 Terence Kelleher. All rights reserved.
 *
 */
 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <sys/timeb.h> 

#include <iostream>
#include <vector>  
#include <string>  
#include <cstdlib>
#include <sstream>

#include "../include/simmgr.h" 

using namespace std;

int
main(int argc, char *argv[] )
{
	//int mypid;
	//struct sigaction new_action;
	int sts;
	//int opt;
	//int arg;
	char *ptr;
	char buffer[256];
	struct timeb ms_time;
	
	daemonize();
	
	sts = initSHM(OPEN_WITH_CREATE );
	if ( sts < 0 )
	{
		perror("initSHM" );
		exit ( -1 );
	}

	printf("initSHM succeeds: ptr is %p\n", simmgr_shm );
	
	// Zero out the shared memory and reinit the values
	memset(simmgr_shm, 0, sizeof(struct simmgr_shm) );
	printf("memset succeeds\n" );
	simmgr_shm->hdr.version = SIMMGR_VERSION;
	printf("version succeeds\n" );
	simmgr_shm->hdr.size = sizeof(struct simmgr_shm);
	do_command_read("/bin/hostname", simmgr_shm->server.name, sizeof(simmgr_shm->server.name)-1 );
	
	ptr = getETH0_IP();
	sprintf(simmgr_shm->server.ip_addr, "%s", ptr );
	
	
	
	
	while ( 1 )
	{
		ptr = do_command_read("/bin/date", buffer, sizeof(buffer)-1 );
		sprintf(simmgr_shm->server.server_time, "%s", buffer );
		
		ftime(&ms_time);
		simmgr_shm->server.msec_time = ms_time.millitm;
	
		sleep(1);
	}
}
