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

#ifndef OSCAR_INCLUDE_ERROR_H_
#define OSCAR_INCLUDE_ERROR_H_

/* OscFunction[Finally|Catch|End]: Declare a function using these macros to use the Osc(Fail|Assert|Call)* error handling macros. */
/*! @brief Use this macro to declare a function. */

#define OscFunction(name, args ...) \
	OSC_ERR name(args) { \
		OSC_ERR _osc_internal_err_ = SUCCESS; \
		bool in_body = false, in_catch __attribute__ ((unused)) = false, in_fail __attribute__ ((unused)) = false, in_finally __attribute__ ((unused)) = false; \
	_osc_internal_top_: __attribute__ ((unused)) \
		if (!in_body) { \
			in_body = true;

#define OscFunctionCatch(e) \
		} else if (({ \
			bool res = false; \
			if (!in_catch) { \
				OSC_ERR const errs[] = { e }; \
				res = length(errs) == 0; \
				for (int i = 0; !res && i < length(errs); i += 1) \
					res = _osc_internal_err_ == errs[i]; \
			} \
			res; \
			in_catch = true; \
		})) {

#define OscFunctionFail(e) \
		} else if (({ \
			bool res = false; \
			if (!in_fail) { \
				OSC_ERR const errs[] = { e }; \
				res = length(errs) == 0; \
				for (int i = 0; !res && i < length(errs); i += 1) \
					res = _osc_internal_err_ == errs[i]; \
			} \
			in_catch = true; \
			in_fail = true; \
			res; \
		})) {

#define OscFunctionFinally() \
		} \
		if (!in_finally) { \
			in_catch = true; \
			in_fail = true; \
			in_finally = true;

#define OscFunctionEnd() \
		} \
		if (in_catch && !in_fail) \
			return SUCCESS; \
		return _osc_internal_err_; \
	}

#define OscMark_format(fmt, args ...) OscLog(ERROR, "%s: %s(): Line %d" fmt "\n", __FILE__, __FUNCTION__, __LINE__, ## args)

/* OscMark[_m](): Macros to log a line whenever a source code line is hit. m allows a custom message to be passed. */
/*! @brief Log a marker with a default message whenever a source code line is hit. */
#define OscMark() OscMark_format("")
/*! @brief Log a marker with a custom message whenever a source code line is hit */
#define OscMark_m(m, args ...) OscMark_format(": " m, ## args)

/* OscFail_[e](s|m)(): Macros to abort the current function and execute the exception handler. e is to pass a custom error code. s is not print a message. m is to print a custom message. */
/*! @brief Abort the current function and jump to the exception handler after 'fail:'. */
#define OscFail_es(e) { _osc_internal_err_ = e; goto _osc_internal_top_; }
/*! @brief Abort the current function while printing a custom error mesage and jump to the exception handler after 'fail:'. */
#define OscFail_em(e, m, args ...) { OscMark_m(m, ## args); OscFail_es(e); }
/*! @brief Abort the current function with a custom error code and jump to the exception handler after 'fail:'. */
#define OscFail_e(e) OscFail_es(e)
/*! @brief Abort the current function and jump to the exception handler after 'fail:'. */
#define OscFail_s() OscFail_es(EGENERAL)

#define OscFail() OscFail_e(EGENERAL)
/*! @brief Abort the current function with a custom error code while printing a custom error mesage and jump to the exception handler after 'fail:'. */
#define OscFail_m(m, args ...) OscFail_em(EGENERAL, m, ## args)

/* OscAssert_[e][s|m](): Macros to check an assertion and abort the current function and execute the exception handler on failure. e is to pass a custom error code. s is not print a message. m is to print a custom message. By defualt a general message is printed. */
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
#define OscCall_s(f, args ...) { _osc_internal_err_ = f(args); if (_osc_internal_err_ < SUCCESS) OscFail_es(_osc_internal_err_); }
/*! @brief Call a function and check it's return code, aborting the current function with a default message on an error. */
#define OscCall(f, args ...) { _osc_internal_err_ = f(args); if (_osc_internal_err_ < SUCCESS) OscFail_m("%s(): Error %d", #f, (int) _osc_internal_err_); }

#define OscCall_is(f, args ...) { _osc_internal_err_ = f(args); }

#define OscCall_i(f, args ...) { _osc_internal_err_ = f(args); if (_osc_internal_err_ < SUCCESS) OscMark_m("%s(): Error %d", #f, (int) _osc_internal_err_); }

#define OscLastError() \
	_osc_internal_err_

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

/* Define an offset for all modules, which allows it to define module-specific errors that do not overlap. */
/*! @brief Error identifier offset of the cam module. */
#define OSC_CAM_ERROR_OFFSET 100
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

#endif /* OSCAR_INCLUDE_ERROR_H_ */
