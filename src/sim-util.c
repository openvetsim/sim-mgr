/*
 * sim-util.c
 *
 * Common functions for the various SimMgr processes
 *
 * Copyright 2015 Terence Kelleher. All rights reserved.
 *
 * Common functions
 *
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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <termios.h>

#define SIMUTIL	1

#include "../include/simmgr.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#define RUNNING_DIR 		"/"
#define LOCK_FILE_DIR        "/var/run/"



int simmgrSHM;					// The File for shared memory
struct simmgr_shm *simmgr_shm;	// Data structure of the shared memory


/*
 * FUNCTION: initSHM
 *
 * Open the shared memory space
 *
 * Parameters: none
 *
 * Returns: 0 for success
 */

int
initSHM(int create )
{
	void *space;
	int mmapSize;
	int permission;
	
	mmapSize = sizeof(struct simmgr_shm );
	
	if ( create == OPEN_WITH_CREATE )
	{
		permission = O_RDWR | O_CREAT | O_TRUNC;
	}
	else
	{
		permission = O_RDWR;
	}
	
	// Open the Shared Memory space
	simmgrSHM = shm_open(SIMMGR_SHM_NAME, permission , 0755 );
	if ( simmgrSHM < 0 )
	{
		perror("shm_open" );
		return ( -3 );
	}
	if ( mmapSize < 4096 )
	{
		mmapSize = 4096;
	}
	
	if (  create == OPEN_WITH_CREATE )
	{
		lseek(simmgrSHM, mmapSize, SEEK_SET );
		write(simmgrSHM, "", 1 );
	}

	space = mmap((caddr_t)0,
				mmapSize, 
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				simmgrSHM,
				0 );
	if ( space == MAP_FAILED )
	{
		perror("mmap" );
		return ( -4 );
	}
	
	simmgr_shm = (struct simmgr_shm *)space;
	
	return ( 0 );
}


/*
 * Function: log_message
 *
 * Log a message to syslog. The message is also logged to a named file. The file is opened
 * for Append and closed on completion of the write. (The file write will be disabled after
 * development is completed).
 * 
 *
 * Parameters: filename - filename to open for writing
 *             message - Pointer to message string, NULL terminated
 *
 * Returns: none
 */
void log_message(const char *filename, const char* message)
{
#if LOG_TO_FILE == 1
	FILE *logfile;
#endif

	syslog(LOG_NOTICE, "%s", message);

#if LOG_TO_FILE == 1
	if ( strlen(filename > 0 )
	{
		logfile = fopen(filename,"a");
		if ( logfile )
		{
			fprintf(logfile, "%s\n", message);
			fclose(logfile );
		}
	}
#endif
}

 /*
 * Function: signal_handler
 *
 * Handle inbound signals.
 *		SIGHUP is ignored 
 *		SIGTERM closes the process
 *
 * Parameters: sig - the signal
 *
 * Returns: none
 */
void signal_handler(int sig )
{
	switch(sig) {
	case SIGHUP:
		log_message("","hangup signal catched");
		break;
	case SIGTERM:
		log_message("","terminate signal catched");
		exit(0);
		break;
	}
}
/*
 * Function: daemonize
 *
 * Convert the task to a daemon. Make it independent of the parent to allow
 * running forever.
 *
 *   Change working directory
 *   Take Lock
 *
 * Parameters: none
 *
 * Returns: none
 */
void daemonize()
{
    pid_t pid;
	int i,lfp;
	char str[10];
	char buffer[128];
	
	if ( getppid()==1 )
	{
		return; /* already a daemon */
	}
    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if ( pid < 0 )
	{
        exit(EXIT_FAILURE);
	}
    /* Success: Let the parent terminate */
    if ( pid > 0 )
	{
        exit(EXIT_SUCCESS);
	}
    /* On success: The child process becomes session leader */
    if (setsid() < 0)
	{
        exit(EXIT_FAILURE);
	}
	for ( i = getdtablesize() ; i >= 0 ; --i )
	{
		close(i); /* close all descriptors */
	}
	
	i = open("/dev/null",O_RDWR);
	dup(i);
	dup(i); /* handle standard I/O */
	
	umask(027); /* set newly created file permissions */
	chdir(RUNNING_DIR); /* change running directory */
	
	// Create a Lock File to prevent multiple daemons
	sprintf(buffer, "%s%s.pid", LOCK_FILE_DIR, program_invocation_short_name );
	lfp = open(buffer, O_RDWR|O_CREAT, 0640 );
	if ( lfp < 0 )
	{
		syslog(LOG_DAEMON | LOG_ERR, "Cannot open log file");
		exit(1); /* can not open */
	}
	if ( lockf(lfp,F_TLOCK,0) < 0 )
	{
		syslog(LOG_DAEMON | LOG_WARNING, "Cannot take lock");
		exit(0); /* can not lock */
	}
	/* first instance continues */
	sprintf(str, "%d\n", getpid());
	write(lfp, str, strlen(str)); /* record pid to lockfile */
	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler); /* catch hangup signal */
	signal(SIGTERM,signal_handler); /* catch kill signal */
	
	syslog (LOG_DAEMON | LOG_NOTICE, "%s started", program_invocation_short_name );
}

/*
 * FUNCTION: kbhit
 * 
 * Check if a user has hit a key on the keyboard.
 */
 
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

/*
 * FUNCTION: checkExit
 * 
 * If a 'q' or 'Q' is typed on the keyboard, quit the program.
 * If any other key is entered, return it.
 */
  
int
checkExit(void )
{
	int cc = 0;

	if ( kbhit() )
	{
		cc = getchar();
		if ( cc == 'q' || cc == 'Q' )
		{
			printf("\n" );
			exit ( 0 );
		}
		return ( cc );
	}
	return ( 0 );
}

// Issue a command and read the first line returned from it.
char *
do_command_read(const char *cmd_str, char *buffer, int max_len )
{
	FILE *fp;
	char *cp;
	
	fp = popen(cmd_str, "r" );
	if ( fp == NULL )
	{
		cp = NULL;
	}
	else
	{
		cp = fgets(buffer, max_len, fp );
		if ( cp != NULL )
		{
			strtok(buffer, "\n"); // Strip the newline
		}
		pclose(fp );
	}
	return ( cp );
}

void
get_date(char *buffer )
{
	time_t timer;
	time(&timer);
	sprintf(buffer, "%s", ctime(&timer) );
	strtok(buffer, "\n");
}

char eth0_ip[512] = { 0, };

char *
getETH0_IP()
{
	int fd;
	struct ifreq ifr;
	int sts;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	sts = ioctl(fd, SIOCGIFADDR, &ifr);
	if ( sts != 0 )
	{
		sprintf(eth0_ip, "no IP addr\n" );
	}
	else
	{
		sprintf(eth0_ip, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	}
	close(fd);
	return eth0_ip;
}

char *
itoa(int val, char *buf, int radix )
{
	if ( radix == 10 )
	{
		sprintf(buf, "%d", val );
	}
	else if ( radix == 16 )
	{
		sprintf(buf, "%x", val );
	}
	else if ( radix == 8 )
	{
		sprintf(buf, "%o", val );
	}
	else
	{
		sprintf(buf, "%s", "" );
	}
	return ( buf );
}
