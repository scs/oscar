/*! @file log_host.c
 * @brief Logging module implementation for host 
 * 
 */

#include "oscar_types_host.h"

#include "log_pub.h"
#include "log_priv.h"
#include "oscar_intern.h"
#include <unistd.h>

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
    
	fclose(osc_log.pLogF);
	fclose(osc_log.pSimLogF);
	
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
	char strTemp[256];
	va_list ap;			/*< The dynamic argument list */
	
	strTemp[0] = 0;	/* Mark the string as empty */
	
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

