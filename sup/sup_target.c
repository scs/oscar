/*! @file sup_target.c
 * @brief Support module implementation for host 
 * 
 * @author Markus Berner
 */

#include "framework_types_target.h"

#include "sup_pub.h"
#include "sup_priv.h"
#include "framework_intern.h"
#include "pflags.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>

/*! @brief The module singelton instance. */
struct LCV_SUP sup;       

/*! @brief The dependencies of this module. */
struct LCV_DEPENDENCY sup_deps[] = {
        {"log", LCVLogCreate, LCVLogDestroy}
};

/*! @brief The length of the dependency array of this module. */
#define DEP_LEN (sizeof(sup_deps)/sizeof(struct LCV_DEPENDENCY))

LCV_ERR LCVSupCreate(void *hFw)
{
    struct LCV_FRAMEWORK *pFw;
    LCV_ERR err;

    pFw = (struct LCV_FRAMEWORK *)hFw;
    if(pFw->sup.useCnt != 0)
    {
        pFw->sup.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module dependencies of this module. */
    err = LCVLoadDependencies(pFw, 
            sup_deps, 
            DEP_LEN);
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load dependencies! (%d)\n",
                __func__, 
                err);
        return err;
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
    if(sup.fdWatchdog > 0)
    {
        LCVSupWdtClose();
    }

#ifdef TARGET_TYPE_LCV_IND    
    if(sup.fdLed)
    {
        LCVSupLedClose();
    }
#endif
    
    LCVUnloadDependencies(pFw, 
            sup_deps, 
            DEP_LEN);
    
    memset(&sup, 0, sizeof(struct LCV_SUP));
}

/*=========================== Watchdog =================================*/

LCV_ERR LCVSupWdtInit()
{
    if(sup.fdWatchdog != 0)
    {
        LCVLog(WARN, "%s: Watchdog already initialized!\n", __func__);
        return -EALREADY_INITIALIZED;
    }

    sup.fdWatchdog = open("/dev/watchdog", O_WRONLY);
    if (sup.fdWatchdog < 0) 
    {
        LCVLog(ERROR, "%s: Unable to open watchdog device node! (%s)\n", 
                __func__,
                strerror(errno));
        return -EDEVICE;
    }

    return SUCCESS;
}

LCV_ERR LCVSupWdtClose()
{
    char magicChar = 'V';
    
    if(sup.fdWatchdog <= 0)
    {
        /* No watchdog open. */
        LCVLog(WARN, "%s: Watchdog not initialized!\n", __func__);
        return -ENO_SUCH_DEVICE;
    }

    /* Send the magic character to disable the watchdog 
     * just before closing the file. This is in case the watchdog 
     * driver has not been configured with CONFIG_WATCHDOG_NOWAYOUT. 
     * Otherwise it resets the board in any case after the 
     * application has been closed. */
    write(sup.fdWatchdog, &magicChar, 1);
    fsync(sup.fdWatchdog);

    close(sup.fdWatchdog);
    sup.fdWatchdog = 0;

    return SUCCESS;
}

inline void LCVSupWdtKeepAlive()
{
    /* Write anything to the watchdog to disable it and 
     * make sure it is flushed. */
    write(sup.fdWatchdog, "\0", 1);
    fsync(sup.fdWatchdog);
}

/*============================= Cycles =================================*/

/*! /todo Low priority: Use whole 64 bit cycles counter. */
inline uint32 LCVSupCycGet()
{
  uint32 ret;

  /* Read out the blackfin-internal cycle count register. 
   * This is a running counter of the number of cycles since start. 
   * Together with CYCLES2 this would actually be a 64 bit value, 
   * but we currently only read out the lower 32 bits. 
   * This yields a wrap-around time of around 8 secs with a 500 Mhz CPU, 
   * which should be sufficient for most cases. */
  __asm__ __volatile__ ("%0 = CYCLES;\n":"=d"(ret));
  return ret;
}

inline uint32 LCVSupCycToMicroSecs(uint32 cycles)
{
  return (cycles/(CPU_FREQ/1000000));
}

/*============================== SRAM =================================*/
inline uint32 LCVSupSramL1ALen()
{
    return SRAM_L1A_LENGTH;
}

inline void* LCVSupSramL1A()
{
    return (void *)SRAM_L1A;
}

inline uint32 LCVSupSramL1BLen()
{
    return SRAM_L1B_LENGTH;
}

inline void* LCVSupSramL1B()
{
    return (void *)SRAM_L1B;
}

inline uint32 LCVSupSramScratchLen()
{
    return SRAM_SCRATCH_LENGTH;
}

inline void* LCVSupSramScratch()
{
    return (void *)SRAM_SCRATCH;
}

/*============================== LED =================================*/
#ifdef TARGET_TYPE_LCV_IND
LCV_ERR LCVSupLedInit()
{
    int     ret;
    
    if(sup.fdLed != 0)
    {
        LCVLog(WARN, "%s: LED already open!\n", __func__);
        return -EALREADY_INITIALIZED;
    }
    
    sup.fdLed = open("/dev/pf27", O_RDWR, 0);
    if(sup.fdLed < 0)
    {
        LCVLog(ERROR, "%s: Unable to open device! (%s)\n",
                __func__,
                strerror(errno));
        return -EDEVICE;
    }
    
    /* Set to output and disable as input. */
    ret = ioctl(sup.fdLed, SET_FIO_DIR, OUTPUT);
    ret |= ioctl(sup.fdLed, SET_FIO_INEN, INPUT_DISABLE);
    if(ret != 0)
    {
        LCVLog(ERROR, "%s: Unable to set output direction!\n", __func__);
        close(sup.fdLed);
        return -EDEVICE;
    }
    
    return SUCCESS;
}

inline void LCVSupLedWrite(char val)
{
    /* LED is low-active, so we need to switch polarity. */
    if(val == '0')
    {
        write(sup.fdLed, "1", sizeof("1"));
    } 
    else if(val == '1')
    {
        write(sup.fdLed, "0", sizeof("0"));
    } else {
        write(sup.fdLed, "T", sizeof("T"));
    }
}

void LCVSupLedClose()
{
    if(sup.fdLed != 0)
    {
        close(sup.fdLed);
    }
}

#else /* TARGET_TYPE_LCV_IND */

LCV_ERR LCVSupLedInit()
{
    return -ENO_SUCH_DEVICE;
}

inline void LCVSupLedWrite(char val)
{
}

void LCVSupLedClose()
{
}

#endif /* TARGET_TYPE_LCV_IND */
