/*	Oscar, a hardware abstraction framework for the LeanXcam and IndXcam.
	Copyright (C) 2008 Supercomputing Systems AG
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*! @file
 * @brief Logging module implementation for host
 */

#include <unistd.h>
#include <syslog.h>

#include "log.h"

OSC_ERR OscLogCreate();
OSC_ERR OscLogDestroy();

/*! @brief The module singelton instance.
 * 
 * This is called osc_log
 * instead of log because log is a internal function of the C
 * library */
struct OSC_LOG osc_log = {
	.consoleLogLevel = NONE,
	.fileLogLevel = NONE
};

struct OscModule OscModule_log = {
	.name = "log",
	.create = OscLogCreate,
	.destroy = OscLogDestroy,
	.dependencies = {
		NULL // To end the flexible array.
	}
};

OSC_ERR OscLogCreate()
{
	osc_log = (struct OSC_LOG) { };
	
	/* Set log levels to defaults. */
	osc_log.consoleLogLevel = DEFAULT_CONSOLE_LOGLEVEL;
	osc_log.fileLogLevel = DEFAULT_FILE_LOGLEVEL;
	
	osc_log.pLogF = fopen(LOG_FILE_NAME, "a");
	if(osc_log.pLogF == NULL)
	{
		printf("Error: Unable to open log file: %s\n", LOG_FILE_NAME);
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	osc_log.pSimLogF = fopen(SIM_LOG_FILE_NAME, "a");
	if(osc_log.pSimLogF == NULL)
	{
		printf("Error: Unable to open log file: %s\n", LOG_FILE_NAME);
		fclose(osc_log.pLogF);
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	return SUCCESS;
}

OSC_ERR OscLogDestroy()
{
	fclose(osc_log.pLogF);
	fclose(osc_log.pSimLogF);

	return SUCCESS;
}

inline void OscLogSetConsoleLogLevel(const enum EnOscLogLevel level)
{
	osc_log.consoleLogLevel = level;
}

inline void OscLogSetFileLogLevel(const enum EnOscLogLevel level)
{
	osc_log.fileLogLevel = level;
}

void OscLog(const enum EnOscLogLevel level, const char * strFormat, ...)
{
	va_list ap;         /*< The dynamic argument list */
		
	if(level <= osc_log.consoleLogLevel)
	{
		/* Log to console if the current log level is high enough */
		
		/* We can't use sprintf because we only have the additional
		 * arguments as a list => use vsprintf */
		va_start(ap, strFormat);
		vprintf(strFormat, ap);
		va_end(ap);
	}
	
	if(level <= osc_log.fileLogLevel)
	{
		if(osc_log.pLogF == NULL)
			return;
		
		/* Log to the log file if the current log level is high enough */
		va_start(ap, strFormat);
		vfprintf(osc_log.pLogF, strFormat, ap);
		va_end(ap);
		fflush(osc_log.pLogF); /* Flush! */
	}
	
	if(level == SIMULATION)
	{
		if(osc_log.pSimLogF == NULL)
			return;
		
		/* Log to the simulation log. */
		va_start(ap, strFormat);
		vfprintf(osc_log.pSimLogF, strFormat, ap);
		va_end(ap);
	}
}

void OscFatalErr(const char * strFormat, ...)
{
	uint16 len = 0;
	va_list ap; /*< The dynamic argument list */

	osc_log.strTemp[0] = 0; /* Mark the string as empty */

	/* Log to console */

	va_start(ap, strFormat);
	vprintf(strFormat, ap);
	va_end(ap);

	/* Log to the log file*/

	/* We can't use sprintf because we only have the additional
	 * arguments as a list => use vsprintf */
	va_start(ap, strFormat);
	len += vsprintf(osc_log.strTemp, strFormat, ap);
	va_end(ap);

	/* Write the message to the syslog daemon. */
	syslog(EMERG, osc_log.strTemp);

	exit(1);
}


