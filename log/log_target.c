/*! @file log_target.c
 * @brief Logging module implementation for target
 * 
 */

#include "oscar_types_target.h"

#include "log_pub.h"
#include "log_priv.h"
#include "oscar_intern.h"
#include <syslog.h>

/*! @brief The module singelton instance.
 * 
 * This is called osc_log
 * instead of log because log is a internal function of the C
 * library */
struct OSC_LOG osc_log;

OSC_ERR OscLogCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->log.useCnt != 0)
	{
		pFw->log.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	memset(&osc_log, 0, sizeof(struct OSC_LOG));
	/* Enable all logging by default */
	osc_log.consoleLogLevel = DEFAULT_CONSOLE_LOGLEVEL;
	osc_log.fileLogLevel = DEFAULT_FILE_LOGLEVEL;

	/* logName must remain persistent, which is why we move it to the
	 * module structure */
	sprintf(osc_log.logName, LOG_NAME);
	/* Initialize the connection to syslog */
	openlog(osc_log.logName, 0, LOG_USER);
	
	/* Increment the use count */
	pFw->log.hHandle = (void*)&osc_log;
	pFw->log.useCnt++;

	return SUCCESS;
}

void OscLogDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->log.useCnt--;
	if(pFw->log.useCnt > 0)
	{
		return;
	}
		
	/* Close the connection to syslog */
	closelog();
	
	memset(&osc_log, 0, sizeof(struct OSC_LOG));
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
	uint16 len;
	va_list ap; /*< The dynamic argument list */

	osc_log.strTemp[0] = 0; /* Mark the string as empty */

	if (level <= osc_log.consoleLogLevel)
	{
		/* Log to console if the current log level is high enough */

		/* We can't use sprintf because we only have the additional
		 * arguments as a list => use vsprintf */
		va_start(ap, strFormat);
		vprintf(strFormat, ap);
		va_end(ap);
	}

	if (level <= osc_log.fileLogLevel)
	{
		/* Log to the log file if the current log level is high enough */

		/* We can't use sprintf because we only have the additional
		 * arguments as a list => use vsprintf */
		va_start(ap, strFormat);
		len += vsprintf(osc_log.strTemp, strFormat, ap);
		va_end(ap);

		/* Write the message to the syslog daemon. */
		syslog(level, osc_log.strTemp);
	}
}
