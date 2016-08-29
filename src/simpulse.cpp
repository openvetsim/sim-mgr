/*
 * simpulse.cpp
 *
 * Time clock for the Cardiac and Respiratory systems. This application monitors the shared
 * memory to get the rate parameters and issues sync signals to the various systems.
 *
 * This process runs independently from the SimMgr. It has two timers; one for the heart rate (pulse) and
 * one for the breath rate (respiration). It runs as two threads. The primary thread listens for connections
 * from clisents, and the child thread monitors the pulse and breath counts to send sync messages to the
 * clients. 
 *
 * Listen for a connections on Port 50200 (SimMgr Event Port)
 *
 * 		1 - On connection, the daemon will fork a task to support the connection
 *		2 - Each connection waits on sync messages
 *
 * Copyright (C) 2016 Terence Kelleher. All rights reserved.
 *
 */
 
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

#include <iostream>
#include <vector>  
#include <string>  
#include <cstdlib>
#include <sstream>

#include <ctime>
#include <math.h>       /* modf */
#include <netinet/in.h>
#include <netinet/ip.h> 
// #define DEBUG

#include "../include/simmgr.h" 

char msgbuf[512];

int quit_flag = 0;
timer_t pulse_timer;
timer_t breath_timer;
struct sigevent pulse_sev;
struct sigevent breath_sev;
int currentPulseRate;
int currentBreathRate;

void set_beat_time(int bpm );


/* struct to hold data to be passed to a thread
   this shows how multiple data items can be passed to a thread */
struct listener
{
    int allocated;
	int thread_no;
	int cfd;
};
#define MAX_LISTENERS 20

struct listener listeners[MAX_LISTENERS];

pthread_t threadInfo;
void *process_child(void *ptr );


#define PORT	50200
#define PULSE_TIMER_SIG		SIGRTMIN
#define BREATH_TIMER_SIG	(SIGRTMIN+1)

char pulseWord[] = "pulse\n";
char breathWord[] = "breath\n";

static void
beat_handler(int sig, siginfo_t *si, void *uc)
{
	int i;
	int tot;
	int samples;
	int msec;
	int index;
	int avg;
	
	if ( sig == PULSE_TIMER_SIG )
	{
		if ( simmgr_shm->status.cardiac.rate > 0 )
		{
			simmgr_shm->status.cardiac.pulseCount++;
			msec = MS_PER_MIN / simmgr_shm->status.cardiac.rate;
		}
		else
		{
			msec = 0;
		}
#if 0
		// avg_rate Calculation
		simmgr_shm->cardiacTimeList[simmgr_shm->cardiacTimeNextWrite] = msec;
		index = simmgr_shm->cardiacTimeNextWrite;
		if ( ( index + 1 ) == CARDIAC_HISTORY_DEPTH )
		{
			simmgr_shm->cardiacTimeNextWrite = 0;
		}
		else
		{
			simmgr_shm->cardiacTimeNextWrite = index + 1;
		}
		tot = 0;
		samples = 0;
		for ( i = 0 ; i < CARDIAC_HISTORY_DEPTH ; i++ )
		{
			msec = simmgr_shm->cardiacTimeList[index];
			if ( msec )
			{
				tot += msec;
				samples++;
			}
			else
			{
				if ( tot == 0 )	// Null reading. Count as samples if this is after valid samples
				{
					samples++;
				}
				else
				{
					break;	// If before valid samples, then stop counting
				}
			}
			index--;
			if ( index < 0 )
			{
				index = CARDIAC_HISTORY_DEPTH - 1;
			}
		}
		if ( ! samples )
		{
			simmgr_shm->status.cardiac.avg_rate = 0;
		}
		else
		{
			avg = tot / samples;
			simmgr_shm->status.cardiac.avg_rate = MS_PER_MIN / avg;
		}
#endif
	}
	else if ( sig == BREATH_TIMER_SIG )
	{
		if ( simmgr_shm->status.respiration.rate > 0 )
		{
			simmgr_shm->status.respiration.breathCount++;
			msec =  MS_PER_MIN / simmgr_shm->status.respiration.rate;
		}
		else
		{
			msec = 0;
		}
		// awRR Calculation
		index = simmgr_shm->respTimeNextWrite;
		simmgr_shm->respTimeList[index] = msec;
		
		if ( ( index + 1 ) >= RESP_HISTORY_DEPTH )
		{
			simmgr_shm->respTimeNextWrite = 0;
		}
		else
		{
			simmgr_shm->respTimeNextWrite = index + 1;
		}
		tot = 0;
		samples = 0;
		for ( i = 0 ; i < RESP_HISTORY_DEPTH ; i++ )
		{
			msec = simmgr_shm->respTimeList[index];
			if ( msec )
			{
				tot += msec;
				samples++;
			}
			else
			{
				if ( tot == 0 )	// Null reading. Count as samples if this is after valid samples
				{
					samples++;
				}
				else
				{
					break;	// If before valid samples, then stop counting
				}
			}
			index--;
			if ( index < 0 )
			{
				index = RESP_HISTORY_DEPTH - 1;
			}
		}
		if ( ! samples )
		{
			simmgr_shm->status.respiration.awRR = 0;
		}
		else
		{
			avg = tot / samples;
			if ( avg == 0 )
			{
				simmgr_shm->status.respiration.awRR = 0;
			}
			else
			{
				simmgr_shm->status.respiration.awRR = MS_PER_MIN / avg;
			}
		}
	}	
}

/*
 * Calculate and set the wait time in usec for the beats.
*/

void
set_pulse_rate(int bpm )
{
	float tempo;
	float beat_per_sec;
	double intpart;
	float fractpart;
	float wait_time_nsec;
	struct itimerspec its;
	
	if ( bpm == 0 )
	{
		bpm = 60 / (DECAY_SECONDS/RESP_HISTORY_DEPTH);
	}
	
	tempo = (float)bpm;
	beat_per_sec = tempo / 60;
	fractpart = modf( ( 1/beat_per_sec ), &intpart );
	wait_time_nsec = ( fractpart * 1000 * 1000 * 1000 );
	
	// Reset the interval timer with the new value
	its.it_value.tv_sec = intpart;
	its.it_value.tv_nsec = wait_time_nsec;
	
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;
	
	if (timer_settime(pulse_timer, 0, &its, NULL) == -1)
	{
		perror("set_pulse_rate: timer_settime");
		sprintf(msgbuf, "set_pulse_rate(%d): timer_settime: %s", bpm, strerror(errno) );
		log_message("", msgbuf );
		exit ( -1 );
	}

}

void
set_breath_rate(int bpm )
{
	float tempo;
	float beat_per_sec;
	double intpart;
	float fractpart;
	float wait_time_nsec;
	struct itimerspec its;
	
	if ( bpm == 0 )
	{
		bpm = 999;
	}
	tempo = (float)bpm;
	beat_per_sec = tempo / 60;
	fractpart = modf( ( 1/beat_per_sec ), &intpart );
	wait_time_nsec = ( fractpart * 1000 * 1000 * 1000 );
	
	// Reset the interval timer with the new value
	its.it_value.tv_sec = intpart;
	its.it_value.tv_nsec = wait_time_nsec;

	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;
	
	if (timer_settime(breath_timer, 0, &its, NULL) == -1)
	{
		perror("set_breath_rate: timer_settime");
		sprintf(msgbuf, "set_breath_rate(%d): timer_settime %s", bpm, strerror(errno)  );
		log_message("", msgbuf );
		exit ( -1 );
	}
}

void
termination_handler (int signum)
{
	quit_flag = 1;
}

int
main(int argc, char *argv[] )
{
	int port = PORT;
	socklen_t socklen;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int optval;
	int sfd;
	int cfd;
	struct sigaction new_action;
	sigset_t mask;
	int i;
	
	for ( i = 0 ; i < MAX_LISTENERS ; i++ )
	{
		listeners[i].allocated = 0;
	}
	
#ifndef DEBUG
	daemonize();
#endif

	// Wait for Shared Memory to become available
	while ( initSHM(OPEN_ACCESS ) != 0 )
	{
		sprintf(msgbuf, "pulse: SHM Failed - waiting" );
		log_message("", msgbuf );
		sleep(10 );
	}
	
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( sfd < 0 ) 
	{
		sprintf(msgbuf, "pulse: socket() failed - exit %s", strerror(errno) );
		log_message("", msgbuf );
		exit ( -1 );
	}
	optval = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	bzero((char *) &server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	if ( bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
	{		
		sprintf(msgbuf, "pulse: bind() for port %d failed - exit %s", port, strerror(errno) );
		log_message("", msgbuf );
		exit ( -1 );
	}
	listen(sfd, 5);
	socklen = sizeof(client_addr);	

	// Setup Signal Handler
	new_action.sa_handler = termination_handler;
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;
	//sigaction (SIGINT, &new_action, NULL);
	//sigaction (SIGHUP, &new_action, NULL);
	//sigaction (SIGTERM, &new_action, NULL);
	sigaction (SIGPIPE, &new_action, NULL);

	// Pulse Timer Setup
	new_action.sa_flags = SA_SIGINFO;
	new_action.sa_sigaction = beat_handler;
	sigemptyset(&new_action.sa_mask);
	if (sigaction(PULSE_TIMER_SIG, &new_action, NULL) == -1)
	{
		perror("sigaction");
		sprintf(msgbuf, "sigaction() fails for Pulse Timer: %s", strerror(errno) );
		log_message("", msgbuf );
		exit ( -1 );
	}
	// Block timer signal temporarily
	sigemptyset(&mask);
	sigaddset(&mask, PULSE_TIMER_SIG);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
	{
		perror("sigprocmask");
		sprintf(msgbuf, "sigprocmask() fails for Pulse Timer %s", strerror(errno) );
		log_message("", msgbuf );
		exit ( -1 );
	}
	// Create the Timer
	pulse_sev.sigev_notify = SIGEV_SIGNAL;
	pulse_sev.sigev_signo = PULSE_TIMER_SIG;
	pulse_sev.sigev_value.sival_ptr = &pulse_timer;
	
	if ( timer_create(CLOCK_MONOTONIC, &pulse_sev, &pulse_timer ) == -1 )
	{
		perror("timer_create" );
		sprintf(msgbuf, "timer_create() fails for Pulse Timer %s", strerror(errno) );
		log_message("", msgbuf );
		exit (-1);
	}
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
    {
		perror("sigprocmask");
		sprintf(msgbuf, "sigprocmask() fails for Pulse Timer%s ", strerror(errno) );
		log_message("", msgbuf );
		exit ( -1 );
	}
	
	// Breath Timer Setup
	new_action.sa_flags = SA_SIGINFO;
	new_action.sa_sigaction = beat_handler;
	sigemptyset(&new_action.sa_mask);
	if (sigaction(BREATH_TIMER_SIG, &new_action, NULL) == -1)
	{
		perror("sigaction");
		sprintf(msgbuf, "sigaction() fails for Breath Timer %s", strerror(errno) );
		log_message("", msgbuf );
		exit(-1 );
	}
	// Block timer signal temporarily
	sigemptyset(&mask);
	sigaddset(&mask, BREATH_TIMER_SIG);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
	{
		perror("sigprocmask");
		sprintf(msgbuf, "sigprocmask() fails for Breath Timer %s", strerror(errno) );
		log_message("", msgbuf );
		exit ( -1 );
	}
	// Create the Timer
	breath_sev.sigev_notify = SIGEV_SIGNAL;
	breath_sev.sigev_signo = BREATH_TIMER_SIG;
	breath_sev.sigev_value.sival_ptr = &breath_timer;
	
	if ( timer_create(CLOCK_MONOTONIC, &breath_sev, &breath_timer ) == -1 )
	{
		perror("timer_create" );
		sprintf(msgbuf, "timer_create() fails for Breath Timer %s", strerror(errno) );
		log_message("", msgbuf );
		exit (-1);
	}
	
	
	currentPulseRate = simmgr_shm->status.cardiac.rate; 
	set_pulse_rate(currentPulseRate );
	simmgr_shm->status.cardiac.pulseCount = 0;
	
	currentBreathRate = simmgr_shm->status.respiration.rate;
	set_breath_rate(currentBreathRate );
	simmgr_shm->status.respiration.breathCount = 0;
	
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
    {
		perror("sigprocmask");
		sprintf(msgbuf, "sigprocmask() fails for Breath Timer %s", strerror(errno) );
		log_message("", msgbuf );
		exit ( -1 );
	}	
	
	pthread_create (&threadInfo, NULL, &process_child,(void *) NULL );
	
	while ( 1 )
	{
		cfd = accept(sfd, (struct sockaddr *) &client_addr, &socklen);
		if ( cfd >= 0 )
		{	
			printf("Accept returns %d\n", cfd );
			for ( i = 0 ; i < MAX_LISTENERS ; i++ )
			{
				if ( listeners[i].allocated == 0 )
				{
					listeners[i].allocated = 1;
					listeners[i].cfd = cfd;
					listeners[i].thread_no = i;

					break;
				}
			}
			if ( i == MAX_LISTENERS )
			{
				// Unable to allocate
				close(cfd );
			}
		}
	}
	sprintf(msgbuf, "simpulse terminates" );
	log_message("", msgbuf );
	exit (0 );
}

void *
process_child(void *ptr )
{
	int fd;
	int len;
	int i;
	int count;
	unsigned int last_pulse = simmgr_shm->status.cardiac.pulseCount;
	unsigned int last_breath = simmgr_shm->status.respiration.breathCount;
	int checkCount = 0;
	
	while ( 1 )
	{
		usleep(5000 );		// 5 msec wait
		if ( last_pulse != simmgr_shm->status.cardiac.pulseCount )
		{
			count = 0;
			for ( i = 0 ; i < MAX_LISTENERS ; i++ )
			{
				if ( listeners[i].allocated == 1 )
				{
					fd = listeners[i].cfd;
					
					len = write(fd, pulseWord, strlen(pulseWord) );
					if ( len < 0) // This detects closed or disconnected listeners.
					{
						close(fd );
						listeners[i].allocated = 0;
					}
					else
					{
						count++;
					}
				}
			}
			last_pulse = simmgr_shm->status.cardiac.pulseCount;
#ifdef DEBUG
			if ( count )
			{
				printf("Pulse sent to %d listeners\n", count );
			}
#endif
		}
		if ( last_breath != simmgr_shm->status.respiration.breathCount )
		{
			count = 0;
			for ( i = 0 ; i < MAX_LISTENERS ; i++ )
			{
				if ( listeners[i].allocated == 1 )
				{
					fd = listeners[i].cfd;
					
					len = write(fd, breathWord, strlen(breathWord) );
					if ( len < 0) // This detects closed or disconnected listeners.
					{
						close(fd );
						listeners[i].allocated = 0;
					}
					else
					{
						count++;
					}
				}
			}
			last_breath = simmgr_shm->status.respiration.breathCount;
#ifdef DEBUG
			if ( count )
			{
				printf("Breath sent to %d listeners\n", count );
			}
#endif
		}
		if ( checkCount++ >= 100 )	// 100 is an interval of 500 ms.
		{
			checkCount = 0;
			// If the pulse rate has changed, then reset the timer
			if ( currentPulseRate != simmgr_shm->status.cardiac.rate )
			{
				set_pulse_rate(simmgr_shm->status.cardiac.rate );
				currentPulseRate = simmgr_shm->status.cardiac.rate;
	#ifdef DEBUG
				sprintf(msgbuf, "Set Pulse to %d", currentPulseRate );
				log_message("", msgbuf );
	#endif
			}
			
			// If the breath rate has changed, then reset the timer
			if ( currentBreathRate != simmgr_shm->status.respiration.rate )
			{
				set_breath_rate(simmgr_shm->status.respiration.rate );
				currentBreathRate = simmgr_shm->status.respiration.rate;
	#ifdef DEBUG
				sprintf(msgbuf, "Set Breath to %d", currentBreathRate );
				log_message("", msgbuf );
	#endif
			}
		}
	}

	return ( 0 );
}
