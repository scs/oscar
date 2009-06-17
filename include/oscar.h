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
 * @brief API definition for Oscar framework
 * 
 * Must be included by the application.
 */

#ifndef OSCAR_INCLUDE_OSCAR_H_
#define OSCAR_INCLUDE_OSCAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Support file for the NIH design pattern. */
#include "nih.h"
#include "version.h"

#if defined(OSC_HOST)
/* Defined as stumps because it is needed in code shared by target and
 * host. */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x) (x)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x) (x)
#elif defined(OSC_TARGET)
/* Bluntly copied from linux/compiler.h from uclinux */
/*! @brief Used to mark likely expressions for compiler optimization */
#define likely(x) __builtin_expect(!!(x), 1)
/*! @brief Used to mark unlikely expressions for compiler optimization */
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#error "Neither OSC_HOST nor OSC_TARGET is defined as a macro."
#endif

/*! @brief The opposite of while (condition) { block }; */
#define until(a) while (!(a))
/*! @brief An endless loop, equivalent to while (true) { block }; */
#define loop while (true)
/*! @brief Gives the length of a field (Does not work on pointers!). */
#define length(a) ((sizeof (a)) / sizeof *(a))

/* OscFunction(Begin|Catch|End): These macros are needed to used the 
 * Osc(Fail|Assert|Call)* macros. */
/*! @brief This macro is needed at the beginning of function that uses the 
 * error handling macros. */
#define OscFunctionBegin \
	OSC_ERR _OscInternal_err_ = SUCCESS;
/*! @brief This macro is needed between the function body and error handling 
 * code in a function that uses the error handling macros. */
#define OscFunctionCatch \
	return SUCCESS; \
fail: __attribute__ ((unused))
/*! @brief This macro is needed at the end of function that uses the error 
 * handling macros. */
#define OscFunctionEnd \
	return _OscInternal_err_;

#define OscMark_format(fmt, args ...) OscLog(ERROR, "%s: %s(): Line %d" fmt "\n", __FILE__, __FUNCTION__, __LINE__, ## args)

/* OscMark[_m](): Macros to log a line whenever a source code line is hit. m allows a custom message to be passed. */
/*! @brief Log a marker with a default message whenever a source code line is hit. */
#define OscMark() OscMark_format("")
/*! @brief Log a marker with a custom message whenever a source code line is hit */
#define OscMark_m(m, args ...) OscMark_format(": " m, ## args)

/* OscFail_[e](s|m)(): Macros to abort the current function and execute the exception handler. e is to pass a custom error code. s is not print a message. m is to print a custom message. */
/*! @brief Abort the current function and jump to the exception handler after 'fail:'. */
#define OscFail_es(e) { _OscInternal_err_ = e; goto fail; }
/*! @brief Abort the current function while printing a custom error mesage and jump to the exception handler after 'fail:'. */
#define OscFail_em(e, m, args ...) { OscMark_m(m, ## args); OscFail_es(e); }
/*! @brief Abort the current function with a custom error code and jump to the exception handler after 'fail:'. */
#define OscFail_e(e) OscFail_es(e)
/*! @brief Abort the current function and jump to the exception handler after 'fail:'. */
#define OscFail_s() OscFail_es(EGENERAL)
/*! @brief Abort the current function with a custom error code while printing a custom error mesage and jump to the exception handler after 'fail:'. */
#define OscFail_m(m, args ...) OscFail_em(EGENERAL, m, ## args)

/* OscAssert_[e][s|m](): Macros check an assertion and abort the current function and execute the exception handler on failure. e is to pass a custom error code. s is not print a message. m is to print a custom message. By defualt a general message is printed. */
/*! @brief Check a condition and abort the current function. */
#define OscAssert_es(expr, e) { if (!(expr)) OscFail_es(e) }
/*! @brief Check a condition and abort the current function while printing a default message. */
#define OscAssert_e(expr, e) { if (!(expr)) OscFail_e(e) }
/*! @brief Check a condition and abort the current function while printing a custom message. */
#define OscAssert_em(expr, e, m, args ...) { if (!(expr)) OscFail_em(e, m, ## args) }
/*! @brief Check a condition and abort the current function with a custom error code. */
#define OscAssert_s(expr) OscAssert_es(expr, EASSERT)
/*! @brief Check a condition and abort the current function with a custom error code while printing a default message. */
#define OscAssert(expr) OscAssert_es(expr, EASSERT)
/*! @brief Check a condition and abort the current function while printing a custom message. */
#define OscAssert_m(expr, m, args ...) OscAssert_em(expr, EASSERT, m, ## args)

/* OscCall[_s](): Macros call a function and and abort the current function and execute the exception handler on failure. e is to pass a custom error code. */
/*! @brief Call a function and check it's return code, aborting the current function on an error. */
#define OscCall_s(f, args ...) { OSC_ERR err_ = f(args); if (err_ != SUCCESS) OscFail_es(err_); }
/*! @brief Call a function and check it's return code, aborting the current function with a default message on an error. */
#define OscCall(f, args ...) { OSC_ERR err_ = f(args); if (err_ != SUCCESS) OscFail_m("%s(): Error %d", #f, (int) err_); }

/*! @brief Define general non-module-specific error codes for the OSC framework */
enum EnOscErrors {
	SUCCESS = 0,
	EGENERAL,
	EASSERT,
	EOUT_OF_MEMORY,
	ETIMEOUT,
	EUNABLE_TO_OPEN_FILE,
	EINVALID_PARAMETER,
	EDEVICE,
	ENOTHING_TO_ABORT,
	EDEVICE_BUSY,
	ECANNOT_DELETE,
	EBUFFER_TOO_SMALL,
	EFILE_ERROR,
	ECANNOT_UNLOAD,
	ENR_OF_INSTANCES_EXHAUSTED,
	EFILE_PARSING_ERROR,
	EALREADY_INITIALIZED,
	ENO_SUCH_DEVICE,
	EUNABLE_TO_READ,
	ETRY_AGAIN,
	EINTERRUPTED,
	EUNSUPPORTED
};

enum OscModule
{
	OscModule_log,
	OscModule_cam,
	OscModule_cpld,
	OscModule_lgx,
	OscModule_sim,
	OscModule_bmp,
	OscModule_swr,
	OscModule_srd,
	OscModule_ipc,
	OscModule_sup,
	OscModule_frd,
	OscModule_dspl,
	OscModule_dma,
	OscModule_hsm,
	OscModule_cfg,
	OscModule_clb,
	OscModule_vis,
	OscModule_gpio,
	OscModule_jpg
};

/* Define an offset for all modules, which allows it to define module-specific errors that do not overlap. */
/*! @brief Error identifier offset of the cam module. */
#define OSC_CAM_ERROR_OFFSET OscModule_log * 100
/*! @brief Error identifier offset of the cpld module. */
#define OSC_CPLD_ERROR_OFFSET 200
/*! @brief Error identifier offset of the lgx module. */
#define OSC_LGX_ERROR_OFFSET 300
/*! @brief Error identifier offset of the log module. */
#define OSC_LOG_ERROR_OFFSET 400
/*! @brief Error identifier offset of the sim module. */
#define OSC_SIM_ERROR_OFFSET 500
/*! @brief Error identifier offset of the bmp module. */
#define OSC_BMP_ERROR_OFFSET 600
/*! @brief Error identifier offset of the swr module. */
#define OSC_SWR_ERROR_OFFSET 700
/*! @brief Error identifier offset of the srd module. */
#define OSC_SRD_ERROR_OFFSET 800
/*! @brief Error identifier offset of the ipc module. */
#define OSC_IPC_ERROR_OFFSET 900
/*! @brief Error identifier offset of the frd module. */
#define OSC_FRD_ERROR_OFFSET 1000
/*! @brief Error identifier offset of the dma module. */
#define OSC_DMA_ERROR_OFFSET 1100
/*! @brief Error identifier offset of the hsm module. */
#define OSC_HSM_ERROR_OFFSET 1200
/*! @brief Error identifier offset of the cfg module. */
#define OSC_CFG_ERROR_OFFSET 1300
/*! @brief Error identifier offset of the clb module. */
#define OSC_CLB_ERROR_OFFSET 1400

/*! @brief Describes a module dependency of a module and all necessary information to load and unload that module. */
struct OSC_DEPENDENCY
{
	/*! @brief The name of the dependency. */
	char strName[24];
	/*! @brief The constructor of the dependency. */
	OSC_ERR (*create)(void *);
	/*! @brief The destructor of the dependency. */
	void (*destroy)(void *);
};

/*********************************************************************//*!
 * @brief Loads the module depencies give in a list of modules.
 * 
 * Goes through the given dependency array and tries to create all
 * member modules. If it fails at some point, destroy the dependencies
 * already created and return with an error code.
 * 
 * @param pFw Pointer to the framework
 * @param aryDeps Array of Dependencies to be loaded.
 * @param nDeps Length of the dependency array.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscLoadDependencies(void *pFw, const struct OSC_DEPENDENCY aryDeps[], const uint32 nDeps);

/*********************************************************************//*!
 * @brief Unloads the module depencies give in a list of modules.
 * 
 * Goes through the given dependency array backwards and destroys
 * all members.
 * 
 * @param pFw Pointer to the framework
 * @param aryDeps Array of Dependencies to be unloaded.
 * @param nDeps Length of the dependency array.
 *//*********************************************************************/
void OscUnloadDependencies(void *pFw, const struct OSC_DEPENDENCY aryDeps[], const uint32 nDeps);

/*! @brief Constructor for framework
	@param modules A list of modules to load as defined in enum OscModule.
	@return SUCCESS or appropriate error code otherwise.
*/
#define OscCreate(modules ...) \
	({ \
		enum OscModule _modules[] = { modules }; \
		_OscCreate(length(_modules), _modules); \
	})

OSC_ERR _OscCreate(int count, enum OscModule * modules);

/*********************************************************************//*!
 * @brief Destructor for framework
 * 
 * Fails if not all loaded modules have been destroyed.
 * 
 * @param hFw Pointer to the handle of the framework to be destroyed.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscDestroy();

/*********************************************************************//*!
 * @brief Get framework version numbers
 * 
 * Used scheme: major.minor[.revsion]
 * 
 * The major number is used for significant changes in functionality or 
 * supported plattform. Instable pre releases use a major number of 0.
 * The minor number decodes small feature changes.
 * The patch number is intended for bug fixes without changes of API.
 * 
 * @param hMajor Pointer to major version number.
 * @param hMinor Pointer to minor version number.
 * @param hPatch Pointer to patch number.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGetVersionNumber(char *hMajor, char *hMinor, char *hPatch);

/*********************************************************************//*!
 * @brief Get framework version string
 * 
 * Version string format: v<major>.<minor>[-p<patch>]  eg: v1.3  or v1.3-p1
 * The patch number is not printed if no bug-fixes are available (patch=0).
 *  
 * See @see OscGetVersionNumber for number interpretation.
 * 
 * @param hMajor Pointer to formated version string.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OscGetVersionString(char *hVersion);

/* Include the public header files of the different modules, which
 * contain the declarations of the API functions of the respective
 * module.
 */
#include "board.h"

#ifdef __cplusplus
}
#endif

#endif /* OSCAR_INCLUDE_OSCAR_H_ */
