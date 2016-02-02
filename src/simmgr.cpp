/*
 * simmgr.cpp
 *
 * SimMgr deamon.
 *
 * Copyright 2016 Terence Kelleher. All rights reserved.
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

int start_scenario(const char *name );
int scenario_run(void );
void comm_check(void );

/* str_thdata
	structure to hold data to be passed to a thread
*/
typedef struct str_thdata
{
    int thread_no;
    char message[100];
} thdata;


/* prototype for thread routine */
void heart_thread ( void *ptr );

int
main(int argc, char *argv[] )
{
	int sts;
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
	
	// Zero out the shared memory and reinit the values
	memset(simmgr_shm, 0, sizeof(struct simmgr_shm) );

	// hdr
	simmgr_shm->hdr.version = SIMMGR_VERSION;
	simmgr_shm->hdr.size = sizeof(struct simmgr_shm);

	// server
	do_command_read("/bin/hostname", simmgr_shm->server.name, sizeof(simmgr_shm->server.name)-1 );
	ptr = getETH0_IP();
	sprintf(simmgr_shm->server.ip_addr, "%s", ptr );
	// server_time and msec_time are updated in the loop
	
	// status/cardiac
	sprintf(simmgr_shm->status.cardiac.rhythm, "%s", "normal" );
	simmgr_shm->status.cardiac.rate = 80;
	sprintf(simmgr_shm->status.cardiac.pwave, "%s", "none" );
	simmgr_shm->status.cardiac.pr_interval = 140; // Good definition at http://lifeinthefastlane.com/ecg-library/basics/pr-interval/
	simmgr_shm->status.cardiac.qrs_interval = 85;
	simmgr_shm->status.cardiac.bps_sys = 105;
	simmgr_shm->status.cardiac.bps_dia = 70;
	
	// status/respiration
	sprintf(simmgr_shm->status.respiration.left_sound_in, "%s", "normal" );
	sprintf(simmgr_shm->status.respiration.left_sound_out, "%s", "normal" );
	sprintf(simmgr_shm->status.respiration.left_sound_back, "%s", "normal" );
	sprintf(simmgr_shm->status.respiration.right_sound_in, "%s", "normal" );
	sprintf(simmgr_shm->status.respiration.right_sound_out, "%s", "normal" );
	sprintf(simmgr_shm->status.respiration.right_sound_back, "%s", "normal" );
	simmgr_shm->status.respiration.inhalation_duration = 1350;
	simmgr_shm->status.respiration.exhalation_duration = 1050;
	simmgr_shm->status.respiration.rate = 20;
	
	// status/scenario
	(void)start_scenario("default" );
	
	// status/auscultation
	simmgr_shm->status.auscultation.side = 0;
	simmgr_shm->status.auscultation.row = 0;
	simmgr_shm->status.auscultation.col = 0;
	
	// status/pulse
	simmgr_shm->status.pulse.position = 0;
	
	// status/cpr
	simmgr_shm->status.cpr.last = 0;
	simmgr_shm->status.cpr.compression = 0;
	simmgr_shm->status.cpr.release = 0;
	
	// status/defibrillation
	simmgr_shm->status.defibrillation.last = 0;
	simmgr_shm->status.defibrillation.energy = 0;
	
	
	// instructor/cardiac
	sprintf(simmgr_shm->instructor.cardiac.rhythm, "%s", "" );
	simmgr_shm->instructor.cardiac.rate = -1;
	sprintf(simmgr_shm->instructor.cardiac.pwave, "%s", "" );
	simmgr_shm->instructor.cardiac.pr_interval = -1;
	simmgr_shm->instructor.cardiac.qrs_interval = -1;
	simmgr_shm->instructor.cardiac.bps_sys = -1;
	simmgr_shm->instructor.cardiac.bps_dia = -1;
	
	// instructor/scenario
	sprintf(simmgr_shm->instructor.scenario.active, "%s", "" );
	// The start times is ignored.
	
	// instructor/respiration
	sprintf(simmgr_shm->instructor.respiration.left_sound_in, "%s", "" );
	sprintf(simmgr_shm->instructor.respiration.left_sound_out, "%s", "" );
	sprintf(simmgr_shm->instructor.respiration.left_sound_back, "%s", "" );
	sprintf(simmgr_shm->instructor.respiration.right_sound_in, "%s", "" );
	sprintf(simmgr_shm->instructor.respiration.right_sound_out, "%s", "" );
	sprintf(simmgr_shm->instructor.respiration.right_sound_back, "%s", "" );
	simmgr_shm->instructor.respiration.inhalation_duration = -1;
	simmgr_shm->instructor.respiration.exhalation_duration = -1;
	simmgr_shm->instructor.respiration.rate = -1;
	
	
	while ( 1 )
	{
		ptr = do_command_read("/bin/date", buffer, sizeof(buffer)-1 );
		sprintf(simmgr_shm->server.server_time, "%s", buffer );
		
		ftime(&ms_time);
		simmgr_shm->server.msec_time = ms_time.millitm;
		
		comm_check();
		
		(void)scenario_run();
	
		usleep(10000);	// Sleep for 10 msec
	}
}

/*
 * comm_check
 *
 * verify that the comminucations path to the SimCtl is open and ok.
 * If not, try to reestablish.
 */
void
comm_check(void )
{
	// TBD
}

/*
 * default scenario
 *
 * Needs to issue various syncs (TBD)
 *
 * Reads II commands and changes operating parameters
 */
int
scenario_run(void )
{
	int sts;
	
	// Check for instructor commands
	// Scenario
	if ( strlen(simmgr_shm->instructor.scenario.active) > 0 )
	{
		sts = start_scenario(simmgr_shm->instructor.scenario.active );
		if ( sts )
		{
			// Add error action
		}
	}
	
	// Cardiac
	if ( strlen(simmgr_shm->instructor.cardiac.rhythm ) > 0 )
	{
		sprintf(simmgr_shm->status.cardiac.rhythm, "%s", simmgr_shm->instructor.cardiac.rhythm );
		sprintf(simmgr_shm->instructor.cardiac.rhythm, "%s", "" );
	}
	if ( simmgr_shm->instructor.cardiac.rate >= 0 )
	{
		simmgr_shm->status.cardiac.rate = simmgr_shm->instructor.cardiac.rate;
		simmgr_shm->instructor.cardiac.rate = -1;
	}
	if ( strlen(simmgr_shm->instructor.cardiac.pwave ) > 0 )
	{
		sprintf(simmgr_shm->status.cardiac.pwave, "%s", simmgr_shm->instructor.cardiac.pwave );
		sprintf(simmgr_shm->instructor.cardiac.pwave, "%s", "" );
	}
	if ( simmgr_shm->instructor.cardiac.pr_interval >= 0 )
	{
		simmgr_shm->status.cardiac.pr_interval = simmgr_shm->instructor.cardiac.pr_interval;
		simmgr_shm->instructor.cardiac.pr_interval = -1;
	}
	if ( simmgr_shm->instructor.cardiac.qrs_interval >= 0 )
	{
		simmgr_shm->status.cardiac.qrs_interval = simmgr_shm->instructor.cardiac.qrs_interval;
		simmgr_shm->instructor.cardiac.qrs_interval = -1;
	}
	if ( simmgr_shm->instructor.cardiac.qrs_interval >= 0 )
	{
		simmgr_shm->status.cardiac.qrs_interval = simmgr_shm->instructor.cardiac.qrs_interval;
		simmgr_shm->instructor.cardiac.qrs_interval = -1;
	}
	if ( simmgr_shm->instructor.cardiac.bps_sys >= 0 )
	{
		simmgr_shm->status.cardiac.bps_sys = simmgr_shm->instructor.cardiac.bps_sys;
		simmgr_shm->instructor.cardiac.bps_sys = -1;
	}
	if ( simmgr_shm->instructor.cardiac.bps_dia >= 0 )
	{
		simmgr_shm->status.cardiac.bps_dia = simmgr_shm->instructor.cardiac.bps_dia;
		simmgr_shm->instructor.cardiac.bps_dia = -1;
	}
	
	// Respiration
	if ( strlen(simmgr_shm->instructor.respiration.left_sound_in) > 0 )
	{
		sprintf(simmgr_shm->status.respiration.left_sound_in, "%s", simmgr_shm->instructor.respiration.left_sound_in );
		sprintf(simmgr_shm->instructor.respiration.left_sound_in, "%s", "" );
	}
	if ( strlen(simmgr_shm->instructor.respiration.left_sound_out) > 0 )
	{
		sprintf(simmgr_shm->status.respiration.left_sound_out, "%s", simmgr_shm->instructor.respiration.left_sound_out );
		sprintf(simmgr_shm->instructor.respiration.left_sound_out, "%s", "" );
	}
	if ( strlen(simmgr_shm->instructor.respiration.left_sound_back ) > 0 )
	{
		sprintf(simmgr_shm->status.respiration.left_sound_back, "%s", simmgr_shm->instructor.respiration.left_sound_back );
		sprintf(simmgr_shm->instructor.respiration.left_sound_back, "%s", "" );
	}
	if ( strlen(simmgr_shm->instructor.respiration.right_sound_in ) > 0 )
	{
		sprintf(simmgr_shm->status.respiration.right_sound_in, "%s", simmgr_shm->instructor.respiration.right_sound_in );
		sprintf(simmgr_shm->instructor.respiration.right_sound_in, "%s", "" );
	}
	if ( strlen(simmgr_shm->instructor.respiration.right_sound_out ) > 0 )
	{
		sprintf(simmgr_shm->status.respiration.right_sound_out, "%s", simmgr_shm->instructor.respiration.right_sound_out );
		sprintf(simmgr_shm->instructor.respiration.right_sound_out, "%s", "" );
	}
	if ( strlen(simmgr_shm->instructor.respiration.right_sound_back ) > 0 )
	{
		sprintf(simmgr_shm->status.respiration.right_sound_back, "%s", simmgr_shm->instructor.respiration.right_sound_back );
		sprintf(simmgr_shm->instructor.respiration.right_sound_back, "%s", "" );
	}
	if ( simmgr_shm->instructor.respiration.inhalation_duration >= 0 )
	{
		simmgr_shm->status.respiration.inhalation_duration = simmgr_shm->instructor.respiration.inhalation_duration;
		simmgr_shm->instructor.respiration.inhalation_duration = -1;
	}
	if ( simmgr_shm->instructor.respiration.exhalation_duration >= 0 )
	{
		simmgr_shm->status.respiration.exhalation_duration = simmgr_shm->instructor.respiration.exhalation_duration;
		simmgr_shm->instructor.respiration.exhalation_duration = -1;
	}
	if ( simmgr_shm->instructor.respiration.rate >= 0 )
	{
		simmgr_shm->status.respiration.rate = simmgr_shm->instructor.respiration.rate;
		simmgr_shm->instructor.respiration.rate = -1;
	}
	return ( 0 );
}

int
start_scenario(const char *name )
{
	time_t rawtime;
	struct tm * timeinfo;
	char *timeBuf;
	
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	
	sprintf(simmgr_shm->status.scenario.active, "%s", name );
	timeBuf = asctime(timeinfo );
	// Remove the \n from the end of the timebuf string
	timeBuf[strlen(timeBuf)-1] = 0;
	
	sprintf(simmgr_shm->status.scenario.start, "%s", timeBuf );
	
	return ( 0 );
}