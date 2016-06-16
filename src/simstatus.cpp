/*
 * simstatus.cpp
 *
 * Open the SHM segment and provide status/control operations.
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

#include <iostream>
#include <vector>  
#include <string>  
#include <cstdlib>
#include <sstream>
#include <utility>

#include "../include/simmgr.h" 

#include <cgicc/CgiDefs.h> 
#include <cgicc/Cgicc.h> 
#include <cgicc/HTTPHTMLHeader.h> 
#include <cgicc/HTMLClasses.h>  


using namespace std;
using namespace cgicc;

struct paramCmds
{
	char cmd[16];
};

void makejson(ostream & output, string key, string content)
{
    output << "\"" << key << "\":\"" << content << "\"";
}

std::vector<std::string> explode(std::string const & s, char delim)
{
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); )
    {
        result.push_back(token );
    }

    return result;
}

void
releaseLock(int taken )
{
	if ( taken )
	{
		sem_post(&simmgr_shm->instructor.sema);
	}
}

struct paramCmds paramCmds[] =
{
	{ "check", },
	{ "uptime", },
	{ "date", },
	{ "",  }
};
int debug = 0;
int iiLockTaken = 0;

/*
 * Function: simstatus_signal_handler
 *
 * Handle inbound signals.
 *		SIGHUP is ignored 
 *		SIGTERM closes the process
 *
 * Parameters: sig - the signal
 *
 * Returns: none
 */
void simstatus_signal_handler(int sig )
{
	switch(sig) {
	case SIGHUP:
	case SIGTERM:
		log_message("","terminate signal catched");
		if ( iiLockTaken )
		{
			releaseLock( iiLockTaken );
		}
		exit(0);
		break;
	}
}

int
main( int argc, const char* argv[] )
{
    char buffer[256];
	char cmd[32];
    //struct tm* tm_info;
	//unsigned int val;
	//int index;
	int i;
	int set_count = 0;
	int sts;
	char *cp;
	
	int trycount;

	std::string key;
	std::string value;
	std::string command;
	std::vector<std::string> v;
	
	// Cgicc formData;
	const_form_iterator iter;
	Cgicc formData;

	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,simstatus_signal_handler); /* catch hangup signal */
	signal(SIGTERM,simstatus_signal_handler); /* catch kill signal */
	signal(SIGSEGV,signal_fault_handler );   // install our handler
	cout << "Content-Type: application/json\r\n\r\n";
	cout << "{\n";

	sts = initSHM(OPEN_ACCESS );
	if ( sts < 0 )
	{
		makejson(cout, "error", "initSHM failed" );
		cout << "\n}\n";
		return ( 0 );
	}
	

	i = 0;
	sprintf(cmd, "none" );
	try 
	{
		Cgicc cgi;

		// If any "set" commands are in the GET/POST list, then take the Instructor Interface lock
		for( iter = cgi.getElements().begin(); iter != cgi.getElements().end(); ++iter )
		{
			key = iter->getName();
			if ( key.compare(0, 4, "set:" ) == 0 )
			{
				// Take the mutex for the Instructor Command section
				trycount = 0;
				while ( ( sts = sem_trywait(&simmgr_shm->instructor.sema) ) != 0 )
				{
					if ( trycount++ > 50 )
					{
						makejson(cout, "status", "Fail");
						cout << ",\n    ";
						makejson(cout, "error", "Could not get Instructor Mutex");
						cout << "\n}\n";
						return ( 0 );
					}
					usleep(1000 );
				}
				iiLockTaken = 1;
				break;
			}
		}
		iter = cgi.getElement("time" );
		if ( iter != cgi.getElements().end())
		{
			makejson(cout, "time", simmgr_shm->server.server_time );
			cout << ",\n";
		}
		// Parse the submitted GET/POST elements
		for( iter = cgi.getElements().begin(); iter != cgi.getElements().end(); ++iter )
		{
			key = iter->getName();
			value = iter->getValue();
			if ( i > 0 )
			{
				cout << ",\n";
			}
			i++;
			if ( key.compare("check" ) == 0 )
			{
				makejson(cout, "check", "check is ok" );
			}
			else if ( key.compare("uptime" ) == 0 )
			{
				cp = do_command_read("/usr/bin/uptime", buffer, sizeof(buffer)-1 );
				if ( cp == NULL )
				{
					makejson(cout, "uptime", "no data");
				}
				else
				{
					makejson(cout, "uptime", buffer);
				}
			}
			else if ( key.compare("date" ) == 0 )
			{
				get_date(buffer );
				makejson(cout, "date", buffer );
			}
			else if ( key.compare("ip" ) == 0 )
			{
				makejson(cout, "ip_addr", simmgr_shm->server.ip_addr );
			}
			else if ( key.compare("host" ) == 0 )
			{
				makejson(cout, "hostname", simmgr_shm->server.name );
			}
			else if ( key.compare("time" ) == 0 )
			{
				makejson(cout, "time", simmgr_shm->server.server_time );
			}
			else if ( key.compare("status" ) == 0 )
			{
				// The meat of the task - Return the content of the SHM
				cout << " \"scenario\" : {\n";
				makejson(cout, "active", simmgr_shm->status.scenario.active );
				cout << ",\n";
				makejson(cout, "start", simmgr_shm->status.scenario.start );
				cout << ",\n";
				makejson(cout, "runtime", simmgr_shm->status.scenario.runtime );
				cout << ",\n";
				makejson(cout, "state", simmgr_shm->status.scenario.state );
				cout << "\n},\n";
				
				cout << " \"logfile\" : {\n";
				makejson(cout, "active", itoa(simmgr_shm->logfile.active, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "filename", simmgr_shm->logfile.filename );
				cout << ",\n";
				makejson(cout, "lines_written", itoa(simmgr_shm->logfile.lines_written, buffer, 10 ) );
				cout << "\n},\n";
				
				cout << " \"cardiac\" : {\n";
				makejson(cout, "rhythm", simmgr_shm->status.cardiac.rhythm	);
				cout << ",\n";
				makejson(cout, "vpc", simmgr_shm->status.cardiac.vpc	);
				cout << ",\n";
				makejson(cout, "pea", itoa(simmgr_shm->status.cardiac.pea, buffer, 10 )	);
				cout << ",\n";
				makejson(cout, "vpc_freq", itoa(simmgr_shm->status.cardiac.vpc_freq, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "vfib_amplitude", simmgr_shm->status.cardiac.vfib_amplitude	);
				cout << ",\n";
				makejson(cout, "rate", itoa(simmgr_shm->status.cardiac.rate, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "nibp_rate", itoa(simmgr_shm->status.cardiac.nibp_rate, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "pulseCount", itoa(simmgr_shm->status.cardiac.pulseCount, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "pwave", simmgr_shm->status.cardiac.pwave );
				cout << ",\n";
				makejson(cout, "pr_interval", itoa(simmgr_shm->status.cardiac.pr_interval, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "qrs_interval", itoa(simmgr_shm->status.cardiac.qrs_interval, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "bps_sys", itoa(simmgr_shm->status.cardiac.bps_sys, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "bps_dia", itoa(simmgr_shm->status.cardiac.bps_dia, buffer, 10 ) );
				cout << ",\n";
				switch ( simmgr_shm->status.cardiac.pulse_strength )
				{
					case 0:
						makejson(cout, "pulse_strength", "none" );
						break;
					case 1:
						makejson(cout, "pulse_strength", "weak" );
						break;
					case 2:
						makejson(cout, "pulse_strength", "medium" );
						break;
					case 3:
						makejson(cout, "pulse_strength", "strong" );
						break;
					default:	// Should never happen
						makejson(cout, "pulse_strength", itoa(simmgr_shm->status.cardiac.pulse_strength, buffer, 10) );
						break;
				}
				cout << ",\n";
				makejson(cout, "heart_sound_volume", itoa(simmgr_shm->status.cardiac.heart_sound_volume, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "heart_sound_mute", itoa(simmgr_shm->status.cardiac.heart_sound_mute, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "heart_sound", simmgr_shm->status.cardiac.heart_sound );
				cout << ",\n";
				makejson(cout, "ecg_indicator", itoa(simmgr_shm->status.cardiac.ecg_indicator, buffer, 10 ) );
				cout << "\n},\n";
				
				cout << " \"respiration\" : {\n";
				makejson(cout, "left_lung_sound", simmgr_shm->status.respiration.left_lung_sound );
				cout << ",\n";
				makejson(cout, "left_lung_sound_volume", itoa(simmgr_shm->status.respiration.left_lung_sound_volume, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "left_lung_sound_mute", itoa(simmgr_shm->status.respiration.left_lung_sound_mute, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "right_lung_sound", simmgr_shm->status.respiration.right_lung_sound );
				cout << ",\n";
				makejson(cout, "right_lung_sound_volume", itoa(simmgr_shm->status.respiration.right_lung_sound_volume, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "right_lung_sound_mute", itoa(simmgr_shm->status.respiration.right_lung_sound_mute, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "inhalation_duration", itoa(simmgr_shm->status.respiration.inhalation_duration, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "exhalation_duration", itoa(simmgr_shm->status.respiration.exhalation_duration, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "breathCount", itoa(simmgr_shm->status.respiration.breathCount, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "spo2", itoa(simmgr_shm->status.respiration.spo2, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "etco2", itoa(simmgr_shm->status.respiration.etco2, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "rate", itoa(simmgr_shm->status.respiration.rate, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "etco2_indicator", itoa(simmgr_shm->status.respiration.etco2_indicator, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "spo2_indicator", itoa(simmgr_shm->status.respiration.spo2_indicator, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "chest_movement", itoa(simmgr_shm->status.respiration.chest_movement, buffer, 10 ) );
				cout << "\n},\n";
				
				cout << " \"auscultation\" : {\n";
				makejson(cout, "side", itoa(simmgr_shm->status.auscultation.side, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "row", itoa(simmgr_shm->status.auscultation.row, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "col", itoa(simmgr_shm->status.auscultation.col, buffer, 10 ) );
				cout << "\n},\n";
				
				cout << " \"general\" : {\n";
				makejson(cout, "temperature", itoa(simmgr_shm->status.general.temperature, buffer, 10 ) );
				cout << "\n},\n";
				
				cout << " \"vocals\" : {\n";
				makejson(cout, "filename", simmgr_shm->status.vocals.filename );
				cout << ",\n";
				makejson(cout, "repeat", itoa(simmgr_shm->status.vocals.repeat, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "volume", itoa(simmgr_shm->status.vocals.volume, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "play", itoa(simmgr_shm->status.vocals.play, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "mute", itoa(simmgr_shm->status.vocals.mute, buffer, 10 ) );
				cout << "\n},\n";
				
				cout << " \"pulse\" : {\n";
				makejson(cout, "position", itoa(simmgr_shm->status.pulse.position, buffer, 10 ) );
				cout << "\n},\n";

				cout << " \"cpr\" : {\n";
				makejson(cout, "last", itoa(simmgr_shm->status.cpr.last, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "compression", itoa(simmgr_shm->status.cpr.compression, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "release", itoa(simmgr_shm->status.cpr.release, buffer, 10 ) );
				cout << "\n},\n";
				
				cout << " \"defibrillation\" : {\n";
				makejson(cout, "last", itoa(simmgr_shm->status.defibrillation.last, buffer, 10 ) );
				cout << ",\n";
				makejson(cout, "energy", itoa(simmgr_shm->status.defibrillation.energy, buffer, 10 ) );
				cout << "\n}\n";
				
			}
			else if ( key.compare(0, 4, "set:" ) == 0 )
			{
				set_count++;
				// set command: Split to segments to construct the reference
				v = explode(key, ':');
				cout << " \"set_" << set_count << "\" : {\n    ";
				makejson(cout, "class", v[1] );
				cout << ",\n    ";
				makejson(cout, "param", v[2] );
				cout << ",\n    ";
				makejson(cout, "value", value );
				cout << ",\n    ";
				sts = 0;
				
				if ( v[1].compare("cardiac" ) == 0 )
				{
					if ( v[2].compare("rhythm" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.cardiac.rhythm, "%s", value.c_str() );
					}
					else if ( v[2].compare("vpc" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.cardiac.vpc, "%s", value.c_str() );
					}
					else if ( v[2].compare("pea" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.pea = atoi(value.c_str() );
					}
					else if ( v[2].compare("vpc_freq" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.vpc_freq = atoi(value.c_str() );
					}
					else if ( v[2].compare("vfib_amplitude" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.cardiac.vfib_amplitude, "%s", value.c_str() );
					}
					else if ( v[2].compare("pwave" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.cardiac.pwave, "%s", value.c_str() );
					}
					else if ( v[2].compare("rate" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.rate = atoi(value.c_str() );
					}
					else if ( v[2].compare("transfer_time" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.transfer_time = atoi(value.c_str() );
					}
					else if ( v[2].compare("pr_interval" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.pr_interval = atoi(value.c_str() );
					}
					else if ( v[2].compare("qrs_interval" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.qrs_interval = atoi(value.c_str() );
					}
					else if ( v[2].compare("bps_sys" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.bps_sys = atoi(value.c_str() );
					}
					else if ( v[2].compare("bps_dia" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.bps_dia = atoi(value.c_str() );
					}
					else if ( v[2].compare("nibp_rate" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.nibp_rate = atoi(value.c_str() );
					}
					else if ( v[2].compare("ecg_indicator" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.ecg_indicator = atoi(value.c_str() );
					}
					else if ( v[2].compare("heart_sound" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.cardiac.heart_sound, "%s", value.c_str() );
					}
					else if ( v[2].compare("heart_sound_volume" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.heart_sound_volume = atoi(value.c_str() );
					}
					else if ( v[2].compare("heart_sound_mute" ) == 0 )
					{
						simmgr_shm->instructor.cardiac.heart_sound_mute = atoi(value.c_str() );
					}
					else if ( v[2].compare("pulse_strength" ) == 0 )
					{
						if ( value.compare("none" ) == 0 )
						{
							simmgr_shm->instructor.cardiac.pulse_strength = 0;
						}
						else if ( value.compare("weak" ) == 0 )
						{
							simmgr_shm->instructor.cardiac.pulse_strength = 1;
						}
						else if ( value.compare("medium" ) == 0 )
						{
							simmgr_shm->instructor.cardiac.pulse_strength = 2;
						}
						else if ( value.compare("strong" ) == 0 )
						{
							simmgr_shm->instructor.cardiac.pulse_strength = 3;
						}
						else
						{
							sts = 3;
						}
					}
					else
					{
						sts = 1;
					}
				}
				else if ( v[1].compare("scenario" ) == 0 )
				{
					if ( v[2].compare("active" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.scenario.active, "%s", value.c_str() );
					}
					else if ( v[2].compare("state" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.scenario.state, "%s", value.c_str() );
					}
					else
					{
						sts = 1;
					}
				}
				else if ( v[1].compare("respiration" ) == 0 )
				{
					if ( v[2].compare("left_lung_sound" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.respiration.left_lung_sound, "%s", value.c_str() );
					}
					else if ( v[2].compare("right_lung_sound" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.respiration.right_lung_sound, "%s", value.c_str() );
					}
					else if ( v[2].compare("inhalation_duration" ) == 0 )
					{
						simmgr_shm->instructor.respiration.inhalation_duration = atoi(value.c_str() );
					}
					else if ( v[2].compare("exhalation_duration" ) == 0 )
					{
						simmgr_shm->instructor.respiration.exhalation_duration = atoi(value.c_str() );
					}
					else if ( v[2].compare("left_lung_sound_volume" ) == 0 )
					{
						simmgr_shm->instructor.respiration.left_lung_sound_volume = atoi(value.c_str() );
					}
					else if ( v[2].compare("left_lung_sound_mute" ) == 0 )
					{
						simmgr_shm->instructor.respiration.left_lung_sound_mute = atoi(value.c_str() );
					}
					else if ( v[2].compare("right_lung_sound_volume" ) == 0 )
					{
						simmgr_shm->instructor.respiration.right_lung_sound_volume = atoi(value.c_str() );
					}
					else if ( v[2].compare("right_lung_sound_volume" ) == 0 )
					{
						simmgr_shm->instructor.respiration.right_lung_sound_volume = atoi(value.c_str() );
					}
					else if ( v[2].compare("right_lung_sound_mute" ) == 0 )
					{
						simmgr_shm->instructor.respiration.right_lung_sound_mute = atoi(value.c_str() );
					}
					else if ( v[2].compare("spo2" ) == 0 )
					{
						simmgr_shm->instructor.respiration.spo2 = atoi(value.c_str() );
					}
					else if ( v[2].compare("etco2" ) == 0 )
					{
						simmgr_shm->instructor.respiration.etco2 = atoi(value.c_str() );
					}
					else if ( v[2].compare("transfer_time" ) == 0 )
					{
						simmgr_shm->instructor.respiration.transfer_time = atoi(value.c_str() );
					}
					else if ( v[2].compare("etco2_indicator" ) == 0 )
					{
						simmgr_shm->instructor.respiration.etco2_indicator = atoi(value.c_str() );
					}
					else if ( v[2].compare("spo2_indicator" ) == 0 )
					{
						simmgr_shm->instructor.respiration.spo2_indicator = atoi(value.c_str() );
					}
					else if ( v[2].compare("chest_movement" ) == 0 )
					{
						simmgr_shm->instructor.respiration.chest_movement = atoi(value.c_str() );
					}
					else
					{
						sts = 1;
					}
				}
				else if ( v[1].compare("general" ) == 0 )
				{
					if ( v[2].compare("temperature" ) == 0 )
					{
						simmgr_shm->instructor.general.temperature = atoi(value.c_str() );
					}
					else if ( v[2].compare("transfer_time" ) == 0 )
					{
						simmgr_shm->instructor.general.transfer_time = atoi(value.c_str() );
					}
					else
					{
						sts = 1;
					}
				}
				else if ( v[1].compare("vocals" ) == 0 )
				{
					if ( v[2].compare("filename" ) == 0 )
					{
						sprintf(simmgr_shm->instructor.vocals.filename, "%s", value.c_str() );
					}
					else if ( v[2].compare("repeat" ) == 0 )
					{
						simmgr_shm->instructor.vocals.repeat = atoi(value.c_str() );
					}
					else if ( v[2].compare("volume" ) == 0 )
					{
						simmgr_shm->instructor.vocals.volume = atoi(value.c_str() );
					}
					else if ( v[2].compare("play" ) == 0 )
					{
						simmgr_shm->instructor.vocals.play = atoi(value.c_str() );
					}
					else if ( v[2].compare("mute" ) == 0 )
					{
						simmgr_shm->instructor.vocals.mute = atoi(value.c_str() );
					}
					else
					{
						sts = 1;
					}
				}
				else
				{
					sts = 2;
				}
				if ( sts == 1 )
				{
					makejson(cout, "status", "invalid param" );
				}
				else if ( sts == 2 )
				{
					makejson(cout, "status", "invalid class" );
				}
				else if ( sts == 3 )
				{
					makejson(cout, "status", "invalid parameter" );
				}
				else
				{
					makejson(cout, "status", "ok" );
				}
				cout << "\n    }";
			}
			else
			{
				makejson(cout, "Invalid Command", cmd );
			}
		}
	}
	catch(exception& ex) 
	{
		// Caught a standard library exception
		makejson(cout, "status", "Fail");
		cout << ",\n    ";
		makejson(cout, "err", ex.what() );
		cout << "\n}\n";
		releaseLock( iiLockTaken );
		return ( 0 );
	}
	
	cout << "\n}\n";
	releaseLock( iiLockTaken );
	return ( 0 );
}
