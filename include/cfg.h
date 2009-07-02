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
 * @brief API definition for configuration file module
 * 
 * Sample configuration file (eeprom.txt):
 * 
 *     IP: 193.168.1.1@n
 *     MAC: 00:20:e3:22:00:00@n
 *     DEL: 99@n
 * 
 * 
 * Sample usage of API:
 * 
 *     // variable declaration@n
 *     struct CFG_VAL_STR val;@n
 *     struct CFG_KEY key;@n
 *     CFG_FILE_CONTENT_HANDLE hCfgHandle;@n
 *     const char fileName[] = "eeprom.txt";@n
 *     int16 tmpInt = 0;@n
 * 
 *     // register file and read it to memory@n
 *     OscCfgRegisterFile(&hCfgHandle, fileName, 256);@n
 * 
 *     // read out string@n
 *     key.strSection = NULL;@n
 *     key.strTag = "IP";@n
 *     OscCfgGetStr(hCfgHandle, &key, &val);@n
 * 
 *     // read int@n
 *     key.strSection = NULL;@n
 *     key.strTag = "DEL";@n
 *     OscCfgGetInt(hCfgHandle, &key, &tmpInt);@n
 * 
 *     // set new string value@n
 *     key.strSection = NULL;@n
 *     key.strTag = "MAC";@n
 *     OscCfgSetStr(hCfgHandle, &key, "00:20:e3:22:00:01");@n
 * 
 *     // flush the file to disk@n
 *     OscCfgFlushContent(hCfgHandle);@n
 * 
 */
#ifndef CFG_PUB_H_
#define CFG_PUB_H_

extern struct OscModule OscModule_cfg;

/*! @brief Module-specific error codes.
 * 
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscCfgErrors {
	ECFG_UNSUPPORTED_FORMAT = OSC_CFG_ERROR_OFFSET,
	ECFG_INVALID_RANGE,
	ECFG_INVALID_VAL,
	ECFG_INVALID_KEY,
	ECFG_NO_HANDLES,
	ECFG_INVALID_FUNC_PARAMETER,
	ECFG_UNABLE_TO_OPEN_FILE,
	ECFG_UNABLE_TO_CLOSE_FILE,
	ECFG_UNABLE_TO_WRITE_FILE,
	ECFG_UNABLE_TO_READ_FILE,
	ECFG_USED_DEFAULT,
	ECFG_ERROR,
	ECFG_UBOOT_ENV_READ_ERROR,
	ECFG_UBOOT_ENV_NOT_FOUND
};
/*! @brief Macro defining the maximal value string size*/
#define CONFIG_VAL_MAX_SIZE 1024

/*! @brief Type for Config File Handles */
typedef uint16 CFG_FILE_CONTENT_HANDLE;

/*! @brief Structure for key */
struct CFG_KEY {
	char *strSection; /* Section string (global if set to NULL) */
	char *strTag; /* Tag string */
};

/*! @brief Structure for configuration values */
struct CFG_VAL_STR {
	char str[CONFIG_VAL_MAX_SIZE];
};

/*! @brief Structure for generic package version informations. */
struct OscCfgVersion {
	int major, minor, patch, rc;
	char * version;
};

struct OscSystemInfo {
	struct {
		struct {
			enum {
				OscSystemInfo_boardType_leanXcam,
				OscSystemInfo_boardType_indXcam,
				OscSystemInfo_boardType_mesaSR4K,
			} boardType;
			int major, minor;
			char * assembly, * revision;
		} board;
		struct {
			bool hasBayernPattern;
			int imageWidth, imageHeight;
		} imageSensor;
#ifdef TARGET_TYPE_INDXCAM
		struct {
			bool flashable;
		} CPLD;
#endif
	} hardware;
	struct {
		struct OscCfgVersion uClinux, UBoot, Oscar;
	} software;
};

/*====================== API functions =================================*/

/*********************************************************************//*!
 * @brief Opens a new file and reads its content to the file content structure
 * 
 * @param pFileContentHandle Ptr to handle to the File content. Is set by this function.
 * @param strFileName Configuration file name.
 * @param maxFileSize maximal file length.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgRegisterFile(
		CFG_FILE_CONTENT_HANDLE* pFileContentHandle,
		const char *strFileName,
		const unsigned int maxFileSize);

/*********************************************************************//*!
 * @brief Delete configuration file
 * 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgDeleteAll( void);

/*********************************************************************//*!
 * @brief Writes content to the file content structure
 * 
 * @param hFileContent Handle to the File content.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgFlushContent(const CFG_FILE_CONTENT_HANDLE hFileContent);

/*********************************************************************//*!
 * @brief Writes content of the whole allocated buffer to the file.
 * If the used string is smaller than the allocated buffer, the content
 * of the rest of the file is cleared with '\0'.
 * 
 * @param hFileContent Handle to the File content.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgFlushContentAll(const CFG_FILE_CONTENT_HANDLE hFileContent);

/*********************************************************************//*!
 * @brief Read string from content structure
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param pVal Return value (string).
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetStr(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		struct CFG_VAL_STR *pVal);

/*********************************************************************//*!
 * @brief Write string to content structure
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param strNewVal Write value (string).
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgSetStr(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		const char *strNewVal);
		
/*********************************************************************//*!
 * @brief Write boolean value to content structure
 * 
 * Use strings TRUE or FALSE.
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param val Write value (boolean).
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgSetBool(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		const bool val);		
		
/*********************************************************************//*!
 * @brief Write integer value to content structure
 * 
 * Use strings TRUE or FALSE.
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param val Write value (integer).
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgSetInt(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		const int val);			

/*********************************************************************//*!
 * @brief Read Int from content structure
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetUInt8(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint8 *iVal);

/*********************************************************************//*!
 * @brief Read Int from content structure
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetInt(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int16 *iVal);

/*********************************************************************//*!
 * @brief Read Int from content structure
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetInt32(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int32 *iVal);

/*********************************************************************//*!
 * @brief Read Int from content structure
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetUInt32(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint32 *iVal);

/*********************************************************************//*!
 * @brief Read Int from content structure and check range
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @param min Min value for range check.
 * @param max Max value for range check. Ignored if -1.
 * @param def Use default value in case of failure. 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetIntRange(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int16 *iVal,
		const int16 min,
		const int16 max,
		const int16 def);

/*********************************************************************//*!
 * @brief Read Int from content structure and check range
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @param min Min value for range check.
 * @param max Max value for range check. Ignored if -1.
 * @param def Use default value in case of failure. 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetUInt16Range(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint16 *iVal,
		const uint16 min,
		const uint16 max,
		const uint16 def);

/*********************************************************************//*!
 * @brief Read Int from content structure and check range
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @param min Min value for range check.
 * @param max Max value for range check. Ignored if -1.
 * @param def Use default value in case of failure. 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetInt32Range(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		int32 *iVal,
		const int32 min,
		const int32 max,
		const int32 def);

/*********************************************************************//*!
 * @brief Read Int from content structure and check range
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @param min Min value for range check.
 * @param max Max value for range check. Ignored if -1.
 * @param def Use default value in case of failure. 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetUInt32Range(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		uint32 *iVal,
		const uint32 min,
		const uint32 max,
		const uint32 def);
		
/*********************************************************************//*!
 * @brief Read string from content structure and length check
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param pVal Return value (string).
 * @param len Max string length check. Ignored if -1.
 * @param pDefault Use default string in case of failure. Ignored if NULL. 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetStrRange(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		struct CFG_VAL_STR *pVal,
		const uint32 len,
		const char* pDefault);		
		
/*********************************************************************//*!
 * @brief Read boolean from content structure
 * 
 * Allowed false strings: 0, false, FALSE
 * Allowed true strings:  1, true, TRUE
 * 
 * @param hFileContent Handle to the File content.
 * @param pKey The name of the section and tag.
 * @param iVal Return value.
 * @param def Use default value in case of failure. 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscCfgGetBool(
		const CFG_FILE_CONTENT_HANDLE hFileContent,
		const struct CFG_KEY *pKey,
		bool *iVal,
		const bool def);

/*!
	@brief Get a pointer to static instance of struct OscSystemInfo.
	@param ppInfo Will be set to point to the struct OscSystemInfo.
*/
OscFunctionDeclare(OscCfgGetSystemInformation, struct OscSystemInfo ** ppInfo)		

#endif /*CFG_PUB_H_*/
