/*! @file log_host.c
 * @brief Logging module implementation for host 
 * 
 * @author Markus Berner
 */

#include "framework_types_host.h"

#include "log_pub.h"
#include "log_priv.h"
#include "framework_intern.h"
#include <unistd.h>

/*! @brief The module singelton instance. 
 * 
 * This is called lcv_log
 * instead of log because log is a internal function of the C
 * library */
struct LCV_LOG lcv_log;		


LCV_ERR LCVLogCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;

    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->log.useCnt != 0)
    {
        pFw->log.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }    
        
	memset(&lcv_log, 0, sizeof(struct LCV_LOG));
    /* Set log levels to defaults. */
    lcv_log.consoleLogLevel = DEFAULT_CONSOLE_LOGLEVEL;
    lcv_log.fileLogLevel = DEFAULT_FILE_LOGLEVEL;
    
	lcv_log.pLogF = fopen(LOG_FILE_NAME, "a");
	if(lcv_log.pLogF == NULL)
	{
		printf("Error: Unable to open log file: %s\n", LOG_FILE_NAME);
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	lcv_log.pSimLogF = fopen(SIM_LOG_FILE_NAME, "a");
	if(lcv_log.pSimLogF == NULL)
	{
		printf("Error: Unable to open log file: %s\n", LOG_FILE_NAME);
		fclose(lcv_log.pLogF);
		return -EUNABLE_TO_OPEN_FILE;
	}
		
    /* Increment the use count */
    pFw->log.hHandle = (void*)&lcv_log;
    pFw->log.useCnt++;    

	return SUCCESS;
}

void LCVLogDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
        
    pFw = (struct LCV_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->log.useCnt--;
    if(pFw->log.useCnt > 0)
    {
        return;
    }
    
	fclose(lcv_log.pLogF);
	fclose(lcv_log.pSimLogF);
	
	memset(&lcv_log, 0, sizeof(struct LCV_LOG));
}

inline void LCVLogSetConsoleLogLevel(const enum EnLcvLogLevel level)
{
	lcv_log.consoleLogLevel = level;
}

inline void LCVLogSetFileLogLevel(const enum EnLcvLogLevel level)
{
	lcv_log.fileLogLevel = level;
}

void LCVLog(const enum EnLcvLogLevel level, const char * strFormat, ...)
{
	char strTemp[256];
	va_list ap;			/*< The dynamic argument list */
	
	strTemp[0] = 0;	/* Mark the string as empty */
	
	if(level <= lcv_log.consoleLogLevel)
	{
		/* Log to console if the current log level is high enough */
		
		/* We can't use sprintf because we only have the additional 
		 * arguments as a list => use vsprintf */
		va_start(ap, strFormat);
		vprintf(strFormat, ap);
		va_end(ap);
	}
	
	if(level <= lcv_log.fileLogLevel)
	{
		if(lcv_log.pLogF == NULL)
			return;
		
		/* Log to the log file if the current log level is high enough */
		va_start(ap, strFormat);
		vfprintf(lcv_log.pLogF, strFormat, ap);
		va_end(ap);
		fflush(lcv_log.pLogF); /* Flush! */
	}
	
	if(level == SIMULATION)
	{
		if(lcv_log.pSimLogF == NULL)
			return;
		
		/* Log to the simulation log. */
		va_start(ap, strFormat);
		vfprintf(lcv_log.pSimLogF, strFormat, ap);
		va_end(ap);		
	}
}

