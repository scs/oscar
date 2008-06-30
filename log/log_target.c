/*! @file log_target.c
 * @brief Logging module implementation for target
 * 
 * @author Markus Berner
 */

#include "framework_types_target.h"

#include "log_pub.h"
#include "log_priv.h"
#include "framework_intern.h"
#include <syslog.h>

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
    /* Enable all logging by default */
    lcv_log.consoleLogLevel = DEFAULT_CONSOLE_LOGLEVEL;
    lcv_log.fileLogLevel = DEFAULT_FILE_LOGLEVEL;

    /* logName must remain persistent, which is why we move it to the
     * module structure */
    sprintf(lcv_log.logName, LOG_NAME);
    /* Initialize the connection to syslog */
    openlog(lcv_log.logName, 0, LOG_USER);
    
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
        
    /* Close the connection to syslog */
    closelog();
    
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
    uint16 len;
    va_list ap; /*< The dynamic argument list */

    lcv_log.strTemp[0] = 0; /* Mark the string as empty */

    if (level <= lcv_log.consoleLogLevel)
    {
        /* Log to console if the current log level is high enough */

        /* We can't use sprintf because we only have the additional 
         * arguments as a list => use vsprintf */
        va_start(ap, strFormat);
        vprintf(strFormat, ap);
        va_end(ap);
    }

    if (level <= lcv_log.fileLogLevel)
    {
        /* Log to the log file if the current log level is high enough */

        /* We can't use sprintf because we only have the additional 
         * arguments as a list => use vsprintf */
        va_start(ap, strFormat);
        len += vsprintf(lcv_log.strTemp, strFormat, ap);
        va_end(ap);

        /* Write the message to the syslog daemon. */
        syslog(level, lcv_log.strTemp);
    }
}
