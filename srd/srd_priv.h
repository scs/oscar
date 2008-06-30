/*! @file srd_priv.h
 * @brief Private stimuli reader module definition
 * 
 * @author Samuel Zahnd
 ************************************************************************/
#ifndef SRD_PRIV_H_
#define SRD_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef LCV_HOST
#include <framework_types_host.h>
#else
#include <framework_types_target.h>
#endif /* LCV_HOST */

#define MAX_NR_READER   10
#define MAX_NR_SIGNAL_PER_READER    20

/*!@brief Reader signal object struct */
struct LCV_SRD_SIGNAL
{
    char* strName;          /*!< @brief Signal name string */
    bool bReadValue;        /*!< @brief Read value from last parsed stimuli line */
    bool bActiveValue;      /*!< @brief Active signal value */
};

/*!@brief Reader object struct */
struct LCV_SRD_READER
{
    FILE* pFile;                    /*!< @brief Handle to reader file */
    char* strFile;                  /*!< @brief Reader file name*/
    bool bDescriptorParsed;         /*!< @brief Stimuli descriptor line already parsed */
    uint32 readTime;                /*!< @brief Timestamp of recent file readline */
    void (*pUpdateCallback)(void);  /*!< @brief Callback to inform about update */
    uint16 nrOfSignals;             /*!< @brief Number of managed signal instances */
	/*! Signal object array*/
    struct LCV_SRD_SIGNAL sig[MAX_NR_SIGNAL_PER_READER];
};

/*!@brief Stimuli reader module object struct */
struct LCV_SRD
{
    uint16 nrOfReaders;		/*!< @brief Number of managed readers */
	/*! @brief Reader object array */
#if defined(LCV_HOST) || defined(LCV_SIM)
    struct LCV_SRD_READER rd[MAX_NR_READER];
#endif 
};

#if defined(LCV_HOST) || defined(LCV_SIM)

/*********************************************************************//*!
 * @brief Cycle Callback for Sim module registration (host only)
 *
 *//*********************************************************************/
static void LCVSrdCycleCallback( void);

/*********************************************************************//*!
 * @brief Read descriptor line in all stimuli files (host only)
 *
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static LCV_ERR ReadAllDescriptor( void);

/*********************************************************************//*!
 * @brief Get next values states (host only)
 *
 * @param rdId Reader array index
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static LCV_ERR GetNext( uint16 rdId);

/*********************************************************************//*!
 * @brief Read next line from stimuli file (host only)
 *
 * @param rdId Reader array index
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static LCV_ERR ReadLine( uint16 rdId);
#endif /*LCV_HOST or LCV_SIM*/

#endif /* SRD_PRIV_H_ */
