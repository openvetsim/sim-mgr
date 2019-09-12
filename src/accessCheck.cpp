#include <cgicc/CgiDefs.h> 
#include <cgicc/Cgicc.h> 
#include <cgicc/HTTPHTMLHeader.h> 
#include <cgicc/HTMLClasses.h> 

int
accessCheck()
{
	int accessLevel = 0;
	std::string key;
	std::string value;
	
	cgicc::const_cookie_iterator c_iter;	
	cgicc::Cgicc cgi;
	const cgicc::CgiEnvironment& env = cgi.getEnvironment();
		
	// Get user credentials from cookies
	for(c_iter = env.getCookieList().begin(); 
	c_iter != env.getCookieList().end(); 
		++c_iter ) 
	{
		key = c_iter->getName();
		if ( key.compare("PHPSESSID" ) == 0 )
		{
			value = c_iter->getValue();
			sprintf(sesid, "%s", value.c_str() );
			//printf("\"sesid\": \"%s\",\n", sesid );
		}
		else if ( key.compare("accessLevel" ) == 0 )
		{
			value = c_iter->getValue();
			accessLevel = atoi(value.c_str() );
			//printf("\"accessLevel\": \"%d\",\n", accessLevel );
		}
		else if ( key.compare("userName" ) == 0 )
		{
			value = c_iter->getValue();
			sprintf(userName, "%s", value.c_str() );
			//printf("\"userName\": \"%s\",\n", userName );
		}
	}