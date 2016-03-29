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

#define SCENARIO_LOOP_COUNT	10		// Run the scenario every SCENARIO_LOOP_COUNT iterations of the 10 msec loop

using namespace std;

int start_scenario(const char *name );
int scenario_run(void );
void comm_check(void );
void time_update(void );


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
	int scenarioCount = SCENARIO_LOOP_COUNT;
	
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
	
	// instructor/sema
	sem_init(&simmgr_shm->instructor.sema, 1, 1 ); // pshared =1, value =1
	
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
	simmgr_shm->instructor.general.temperature = -1;
	simmgr_shm->status.general.temperature = 1013;
	
	// status/scenario
	(void)start_scenario("default" );
	
	while ( 1 )
	{
		comm_check();
		
		if ( scenarioCount-- <= 0 )
		{
			scenarioCount = SCENARIO_LOOP_COUNT;
			time_update();
			(void)scenario_run();
			
		}
	
		usleep(10000);	// Sleep for 10 msec
	}
}
/*
 * time_update
 *
 * Get the localtime and write it as a string to the SHM data
 */ 
void
time_update(void )
{
	char buf[256];
	struct tm tm;
	struct timeb timeb;
	
	(void)ftime(&timeb );
	(void)localtime_r(&timeb.time, &tm );
	(void)asctime_r(&tm, buf );
	sprintf(simmgr_shm->server.server_time, "%s", buf );
	simmgr_shm->server.msec_time = (((tm.tm_hour*60*60)+(tm.tm_min*60)+tm.tm_sec)*1000)+ timeb.millitm;
}
/*
 * comm_check
 *
 * verify that the communications path to the SimCtl is open and ok.
 * If not, try to reestablish.
 */
void
comm_check(void )
{
	// TBD
}

/*
 * Cardiac Process
 *
 * Based on the rate and target selected, modify the pulse rate
 */
struct trend cardiacTrend;
struct trend respirationTrend;
struct trend sysTrend;
struct trend diaTrend;
struct trend tempTrend;

#define SCENARIO_TICK_TIME 100000	// in usec

int 
clearTrend(struct trend *trend, int current )
{
	trend->end = current;
	trend->current = current;
	
	return ( trend->current );
}
int 
setTrend(struct trend *trend, int end, int current, int duration )
{
	trend->end = end;
	trend->tickCount = 0;

	if ( duration > 0 )
	{
		trend->current = current;
		trend->ticksPerStep = ( (duration*1000*1000)/SCENARIO_TICK_TIME ) / ( abs(end - current) );
	}
	else
	{
		trend->current = end;
	}
	return ( trend->current );
}

int
trendProcess(struct trend *trend )
{
	if ( trend->end != trend->current )
	{
		if ( trend->tickCount++ == trend->ticksPerStep )
		{
			trend->tickCount = 0;
			if ( trend->end > trend->current )
			{
				trend->current++;
			}
			else
			{
				trend->current--;
			}
		}
	}
	return ( trend->current );
}


/*
 * Scenario execution
 *
 * Reads II commands and changes operating parameters
 */
int
scenario_run(void )
{
	int sts;
	int trycount;
	// Lock the command interface before processing commands
	// Release the MUTEX
	
	trycount = 0;
	while ( ( sts = sem_trywait(&simmgr_shm->instructor.sema) ) != 0 )
	{
		if ( trycount++ > 10 )
		{
			// Could not get lock soon enough. Try again next time.
			return ( -1 );
		}
		usleep(SCENARIO_TICK_TIME );
	}
	
	// Check for instructor commands
	
	// Scenario
	if ( strlen(simmgr_shm->instructor.scenario.active) > 0 )
	{
		sts = start_scenario(simmgr_shm->instructor.scenario.active );
		if ( sts )
		{
			// Start Failure - revert to default
			(void)start_scenario("default" );
			
			// TODO: Add a failure message back to the Instructor
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
		simmgr_shm->status.cardiac.rate = setTrend(&cardiacTrend, 
											simmgr_shm->instructor.cardiac.rate,
											simmgr_shm->status.cardiac.rate,
											simmgr_shm->instructor.cardiac.transfer_time );
		simmgr_shm->instructor.cardiac.rate = -1;
		simmgr_shm->instructor.cardiac.transfer_time = -1;
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
		simmgr_shm->status.cardiac.bps_sys = setTrend(&sysTrend, 
											simmgr_shm->instructor.cardiac.bps_sys,
											simmgr_shm->status.cardiac.bps_sys,
											simmgr_shm->instructor.cardiac.transfer_time );
		simmgr_shm->instructor.cardiac.bps_sys = -1;
		simmgr_shm->instructor.cardiac.transfer_time = -1;
	}
	if ( simmgr_shm->instructor.cardiac.bps_dia >= 0 )
	{
		simmgr_shm->status.cardiac.bps_dia = setTrend(&diaTrend, 
											simmgr_shm->instructor.cardiac.bps_dia,
											simmgr_shm->status.cardiac.bps_dia,
											simmgr_shm->instructor.cardiac.transfer_time );
		simmgr_shm->instructor.cardiac.bps_dia = -1;
		simmgr_shm->instructor.cardiac.transfer_time = -1;
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
		simmgr_shm->status.respiration.rate = setTrend(&respirationTrend, 
											simmgr_shm->instructor.respiration.rate,
											simmgr_shm->status.respiration.rate,
											simmgr_shm->instructor.respiration.transfer_time );
		simmgr_shm->instructor.respiration.rate = -1;
		simmgr_shm->instructor.respiration.transfer_time = -1;
	}
	simmgr_shm->status.respiration.rate = trendProcess( &respirationTrend );
	
	if ( simmgr_shm->instructor.general.temperature >= 0 )
	{
		simmgr_shm->status.general.temperature = setTrend(&tempTrend, 
											simmgr_shm->instructor.general.temperature,
											simmgr_shm->status.general.temperature,
											simmgr_shm->instructor.general.transfer_time );
		simmgr_shm->status.general.temperature = -1;
		simmgr_shm->instructor.general.transfer_time = -1;
	}
	// Release the MUTEX
	sem_post(&simmgr_shm->instructor.sema);

	// Process the trends
	
	simmgr_shm->status.cardiac.rate = trendProcess(&cardiacTrend );
	simmgr_shm->status.cardiac.bps_sys = trendProcess(&sysTrend );
	simmgr_shm->status.cardiac.bps_dia = trendProcess(&diaTrend );
	simmgr_shm->status.respiration.rate = trendProcess(&respirationTrend );
	simmgr_shm->status.general.temperature = trendProcess(&tempTrend );
	
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

	// Clear running trends
	simmgr_shm->status.cardiac.rate = clearTrend(&cardiacTrend, 80 );
	simmgr_shm->status.cardiac.bps_sys = clearTrend(&sysTrend, 105 );
	simmgr_shm->status.cardiac.bps_dia = clearTrend(&diaTrend, 90  );
	simmgr_shm->status.respiration.rate = clearTrend(&respirationTrend, 25 );
	simmgr_shm->status.general.temperature = clearTrend(&tempTrend, 1017 );
	
	sprintf(simmgr_shm->status.scenario.active, "%s", name );
	timeBuf = asctime(timeinfo );
	// Remove the \n from the end of the timebuf string
	timeBuf[strlen(timeBuf)-1] = 0;
	
	sprintf(simmgr_shm->status.scenario.start, "%s", timeBuf );
	
	return ( 0 );
}