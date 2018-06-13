#include "includes.h"

void
own_log(const char *prio, const char *fmt, ...)
{
	// NEXT const char * are the severity of log information 
	const char *priority_debug 	= "LOG_DEBUG";
	const char *priority_info 	= "LOG_INFO";
	const char *priority_notice 	= "LOG_NOTICE";
	const char *priority_warning 	= "LOG_WARNING";
	const char *priority_error 	= "LOG_ERR";
	const char *priority_critical 	= "LOG_CRIT";
	const char *priority_alert	= "LOG_ALERT";
	const char *priority_emergency  = "LOG_EMERG";

	// va_list, va_start and va_end is necessary for the last ... (varadic function) in the param of "own_log()" (https://en.wikipedia.org/wiki/Variadic_function#Variadic_functions_in_C.2C_Objective-C.2C_C.2B.2B.2C_and_D)
	va_list		args;
	char		buf[2048];

	va_start(args, fmt);
	(void)vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	
	// compare based on priority info 
	if(strcmp(prio, priority_debug) == 0)
		syslog(LOG_DEBUG, "[%s]: %s", prio, buf);
	// compare based on priority debug
	else if(strcmp(prio, priority_info) == 0)
		syslog(LOG_INFO, "[%s]: %s", prio, buf);
	// compare based on priority notice
	else if(strcmp(prio, priority_notice) == 0)
		syslog(LOG_NOTICE, "[%s]: %s", prio, buf);
	// compare based on priority warning
	else if (strcmp(prio, priority_warning) == 0)
		syslog(LOG_WARNING, "[%s]: %s", prio, buf);
	// compare based on priority error
	else if (strcmp(prio, priority_error) == 0)
		syslog(LOG_ERR, "[%s]: %s", prio, buf);
	// compare based on priority critical
	else if (strcmp(prio, priority_critical) == 0)
		syslog(LOG_CRIT, "[%s]: %s", prio, buf);
	// compare based on priority alert
	else if (strcmp(prio, priority_alert) == 0)
		syslog(LOG_ALERT, "[%s]: %s", prio, buf);
	// compare based on priority emergency
	else if (strcmp(prio, priority_emergency) == 0)
		syslog(LOG_EMERG, "[%s]: %s", prio, buf);
	// else priority is not known
       	else 
		syslog(LOG_EMERG, "UNKOWN_PRIORITY; message: %s",buf); 
}

