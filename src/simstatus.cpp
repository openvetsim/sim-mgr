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

struct paramCmds paramCmds[] =
{
	{ "check", },
	{ "uptime", },
	{ "date", },
	{ "",  }
};
int debug = 0;

int
main( int argc, const char* argv[] )
{
    char buffer[256];
	char cmd[32];
    //struct tm* tm_info;
	//unsigned int val;
	//int index;
	int i;
	int sts;
	char *cp;
	std::string key;
	std::string value;
	std::string command;
	
	// Cgicc formData;
	const_form_iterator iter;
	Cgicc formData;
	
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
			else
			{
				makejson(cout, "Invalid Command", cmd );
			}
		}		
	}
	catch(exception& e) 
	{
		// Caught a standard library exception
		makejson(cout, "status", "Fail");
		cout << "\n}\n";
		return ( 0 );
	}
	
	cout << "\n}\n";
	return ( 0 );
}