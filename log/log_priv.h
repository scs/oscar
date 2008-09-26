/*! @file log_priv.h
 * @brief Private logging module definition
 * 
 */
#ifndef LOG_PRIV_H_
#define LOG_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "log_pub.h"

/*! @brief The name of the log file for general messages */
#define LOG_FILE_NAME "osc_log"
/*! @brief The name of the log file for simulation results */
#define SIM_LOG_FILE_NAME "osc_simlog"
/*! @brief The default log identity prepended to syslog messages. */
#define LOG_NAME "OSC"

/*! @brief The default minimum log level for console output. */
#define DEFAULT_CONSOLE_LOGLEVEL WARN
/*! @brief The default minimum log level for file output. */
#define DEFAULT_FILE_LOGLEVEL INFO

/*! @brief The object struct of the camera module */
struct OSC_LOG {
	/*! @brief The log identity prepended to syslog messages. */
	char logName[64];
	/*! @brief The highest log level that is output to the console */
	enum EnOscLogLevel consoleLogLevel;
	/*! @brief The highest log level that is output to the log file */
	enum EnOscLogLevel fileLogLevel;
	/*! @brief Temporary string for log messages generation */
	char strTemp[65536];
#ifdef OSC_HOST
	/*! @brief Host only: The log file handle */
	FILE * pLogF;
	/*! @brief Host only: The simulation log file handle */
	FILE * pSimLogF;
#endif /* OSC_HOST */
};
#endif /*LOG_PRIV_H_*/
