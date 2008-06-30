/*! @file swr_priv.h
 * @brief Private stimuli writer module definition
 *
 * @author Samuel Zahnd
 ************************************************************************/
#ifndef SWR_PRIV_H_
#define SWR_PRIV_H_

#include <string.h>
#include <stdio.h>

#include <log/log_pub.h>

#ifdef LCV_HOST
#include <framework_types_host.h>
#else
#include <framework_types_target.h>
#endif /* LCV_HOST */


#if defined(LCV_HOST) || defined(LCV_SIM)
/*! @brief Limited number of writer instances */
#define MAX_NR_WRITER   			10
/*! @brief Limited number of signal instances per writer */
#define MAX_NR_SIGNAL_PER_WRITER    20

/*! @brief Limited string length for string type value */
#define MAX_LENGTH_STRING_VALUE    200

/*!@brief Union for signal value (function argument)*/
union uLCV_SWR_VALUE
{
    int32 nValue;    /*!< @brief Integer coded value */
    float fValue;    /*!< @brief Float coded value */
    char strValue[MAX_LENGTH_STRING_VALUE]; /*!< @brief String coded value */
};

/*!@brief Signal object struct */
struct LCV_SWR_SIGNAL
{
    char* strName;	/*!< @brief Signal name for stimuli file report*/
    enum EnLcvSwrSignalType type; /*!< @brief Signal value type*/
    union uLCV_SWR_VALUE value;	/*!< @brief Signal value */
    char strFormat[20]; /*!< @brief logging format string */
};

/*!@brief Writer object struct */
struct LCV_SWR_WRITER
{
    FILE* pFile;              /*!< @brief Handle to writer file */
    char strFile[64];
    uint16 nrOfSignals;       /*!< @brief Number of managed signals */
    bool bDescriptorPrinted;  /*!< @brief File descripter line already printed */
    bool bReportTime;
    bool bReportCyclic;   
    /*! @brief Signal instance array*/
    struct LCV_SWR_SIGNAL sig[ MAX_NR_SIGNAL_PER_WRITER];
};
#endif /* LCV_HOST or LCV_SIM*/

/*!@brief Stimuli writer module object struct */
struct LCV_SWR
{
    uint16 nrOfWriters;		/*!< @brief Number of managed writers */
    /*! @brief Writer instance array */
    #if defined(LCV_HOST) || defined(LCV_SIM)
    /*! @brief Array of managed writers */
    struct LCV_SWR_WRITER wr[ MAX_NR_WRITER];
    #endif
};

#if defined(LCV_HOST) || defined(LCV_SIM)
/*! @brief Fuction */
void LCVSwrCycleCallback( void);
void LCVSwrReport( const void* pWriter);
#endif /*LCV_HOST or LCV_SIM*/


#endif /* SWR_PRIV_H_ */
