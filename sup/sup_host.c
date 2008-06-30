/*! @file sup_host.c
 * @brief Support module implementation for host
 * 
 * @author Markus Berner
 */

#include "framework_types_host.h"

#include "sup_pub.h"
#include "sup_priv.h"
#include "framework_intern.h"
#include <time.h>

/*! @brief The module singelton instance. */
struct LCV_SUP sup;		


LCV_ERR LCVSupCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;

    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->sup.useCnt != 0)
    {
        pFw->sup.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
	memset(&sup, 0, sizeof(struct LCV_SUP));
	
    /* Increment the use count */
    pFw->sup.hHandle = (void*)&sup;
    pFw->sup.useCnt++; 
    
    return SUCCESS;
}

void LCVSupDestroy(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
            
    pFw = (struct LCV_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->sup.useCnt--;
    if(pFw->sup.useCnt > 0)
    {
        return;
    }
    
    /* Make sure the watchdog is closed, otherwise the board may
     * be reset by it after the application has closed. */
    LCVSupWdtClose();
    
	memset(&sup, 0, sizeof(struct LCV_SUP));
}

/*=========================== Watchdog =================================*/

LCV_ERR LCVSupWdtInit()
{
    /* There is no watchdog on the host => do nothing. */

    return SUCCESS;
}

LCV_ERR LCVSupWdtClose()
{
    /* There is no watchdog on the host => do nothing. */

    return SUCCESS;
}

inline void LCVSupWdtKeepAlive()
{
    /* There is no watchdog on the host => do nothing. */
}

/*============================= Cycles =================================*/
inline uint32 LCVSupCycGet()
{
  /* The host implementation uses the ANSI C function clock() 
   * with much lower precision than on the blackfin. 
   * The actual precision is dependent on the platform. */

  /* ANSI C function, low resolution */
  return ((uint32)clock()); 
}

inline uint32 LCVSupCycToMicroSecs(uint32 cycles)
{
  return (cycles/(CLOCKS_PER_SEC/1000000));
}

/*============================== SRAM =================================*/
inline uint32 LCVSupSramL1ALen()
{
    return SRAM_L1A_LENGTH;
}

inline void* LCVSupSramL1A()
{
    return sup.pL1A;
}

inline uint32 LCVSupSramL1BLen()
{
    return SRAM_L1B_LENGTH;
}

inline void* LCVSupSramL1B()
{
    return sup.pL1B;
}

inline uint32 LCVSupSramScratchLen()
{
    return SRAM_SCRATCH_LENGTH;
}

inline void* LCVSupSramScratch()
{
    return sup.pScratch;
}

/*============================== LED =================================*/
#ifdef TARGET_TYPE_LCV_IND
LCV_ERR LCVSupLedInit()
{
    return -SUCCESS;
}
#else /* TARGET_TYPE_LCV_IND */
LCV_ERR LCVSupLedInit()
{
    return -ENO_SUCH_DEVICE;
}
#endif /* TARGET_TYPE_LCV_IND */

inline void LCVSupLedWrite(char val)
{
}

void LCVSupLedClose()
{
}


