/*! @file log_pub.h
 * @brief API definition for logging module
 * 
 * @author Markus Berner
 */
#ifndef LOG_PUB_H_
#define LOG_PUB_H_

#include "framework_error.h"
#ifdef LCV_HOST
    #include "framework_types_host.h"
    #include "framework_host.h"
#else
    #include "framework_types_target.h"
    #include "framework_target.h"
#endif /* LCV_HOST */

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnLcvLogErrors {
	ELOG_INVALID_EXPOSURE_VALID = LCV_LOG_ERROR_OFFSET
};

/*! @brief The different log levels of the logging module */
enum EnLcvLogLevel {
	EMERG,
	ALERT,
	CRITICAL,
	ERROR,
	WARN,
	NOTICE,
	INFO,
	DEBUG,
	NONE,
	/*! @brief A special loglevel used to report simulation results
	 *  in the host implementation */	
	SIMULATION = 255  
};

/*=========================== API functions ============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
LCV_ERR LCVLogCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void LCVLogDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Sets the highest log level to be output to the console
 * 
 * Set the log level to NONE to disable logging.
 * 
 * @param level The log level to set.
 *//*********************************************************************/
inline void LCVLogSetConsoleLogLevel(const enum EnLcvLogLevel level);

/*********************************************************************//*!
 * @brief Sets the highest log level to be output to the log file
 * 
 * Set the log level to NONE to disable logging.
 * 
 * @param level The log level to set.
 *//*********************************************************************/
inline void LCVLogSetFileLogLevel(const enum EnLcvLogLevel level);

/*********************************************************************//*!
 * @brief Logs a message.
 * 
 * The console and the log file have different log levels. This log level
 * is first checked before writing out a message.
 * 
 * @param level The log level of the message.
 * @param strFormat Format string of the message.
 * @param ... Format parameters of the message.
 *//*********************************************************************/
void LCVLog(const enum EnLcvLogLevel level, const char * strFormat, ...);


#endif /*LOG_PUB_H_*/
