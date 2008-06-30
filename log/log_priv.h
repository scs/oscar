/*! @file log_priv.h
 * @brief Private logging module definition
 * 
 * @author Markus Berner
 */
#ifndef LOG_PRIV_H_
#define LOG_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "log_pub.h"

/*! @brief The name of the log file for general messages */
#define LOG_FILE_NAME "lcv_log"
/*! @brief The name of the log file for simulation results */
#define SIM_LOG_FILE_NAME "lcv_simlog"
/*! @brief The default log identity prepended to syslog messages. */
#define LOG_NAME "LCV"

/*! @brief The default minimum log level for console output. */
#define DEFAULT_CONSOLE_LOGLEVEL WARN
/*! @brief The default minimum log level for file output. */
#define DEFAULT_FILE_LOGLEVEL INFO

/*! @brief The object struct of the camera module */
struct LCV_LOG {
    /*! @brief The log identity prepended to syslog messages. */
    char logName[64];
	/*! @brief The highest log level that is output to the console */
	enum EnLcvLogLevel consoleLogLevel;
	/*! @brief The highest log level that is output to the log file */
	enum EnLcvLogLevel fileLogLevel;
	/*! @brief Temporary string for log messages generation */
    char strTemp[65536];
#ifdef LCV_HOST
	/*! @brief Host only: The log file handle */
	FILE * pLogF;
	/*! @brief Host only: The simulation log file handle */
	FILE * pSimLogF;
#endif /* LCV_HOST */
};
#endif /*LOG_PRIV_H_*/
