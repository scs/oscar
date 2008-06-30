/*! @file sup_target.c
 * @brief Support module implementation for host 
 * 
 */

#include "oscar_types_target.h"

#include "sup_pub.h"
#include "sup_priv.h"
#include "oscar_intern.h"
#include "pflags.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>

/*! @brief The module singelton instance. */
struct OSC_SUP sup;       

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY sup_deps[] = {
        {"log", OscLogCreate, OscLogDestroy}
};

/*! @brief The length of the dependency array of this module. */
#define DEP_LEN (sizeof(sup_deps)/sizeof(struct OSC_DEPENDENCY))

OSC_ERR OscSupCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
    OSC_ERR err;

    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->sup.useCnt != 0)
    {
        pFw->sup.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module dependencies of this module. */
    err = OSCLoadDependencies(pFw, 
            sup_deps, 
            DEP_LEN);
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load dependencies! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    memset(&sup, 0, sizeof(struct OSC_SUP));
    
    /* Increment the use count */
    pFw->sup.hHandle = (void*)&sup;
    pFw->sup.useCnt++; 
    
    return SUCCESS;
}

void OscSupDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
            
    pFw = (struct OSC_FRAMEWORK *)hFw; 
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
        OscSupWdtClose();
    }

#ifdef TARGET_TYPE_INDXCAM    
    if(sup.fdLed)
    {
        OscSupLedClose();
    }
#endif
    
    OSCUnloadDependencies(pFw, 
            sup_deps, 
            DEP_LEN);
    
    memset(&sup, 0, sizeof(struct OSC_SUP));
}

/*=========================== Watchdog =================================*/

OSC_ERR OscSupWdtInit()
{
    if(sup.fdWatchdog != 0)
    {
        OscLog(WARN, "%s: Watchdog already initialized!\n", __func__);
        return -EALREADY_INITIALIZED;
    }

    sup.fdWatchdog = open("/dev/watchdog", O_WRONLY);
    if (sup.fdWatchdog < 0) 
    {
        OscLog(ERROR, "%s: Unable to open watchdog device node! (%s)\n", 
                __func__,
                strerror(errno));
        return -EDEVICE;
    }

    return SUCCESS;
}

OSC_ERR OscSupWdtClose()
{
    char magicChar = 'V';
    
    if(sup.fdWatchdog <= 0)
    {
        /* No watchdog open. */
        OscLog(WARN, "%s: Watchdog not initialized!\n", __func__);
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

inline void OscSupWdtKeepAlive()
{
    /* Write anything to the watchdog to disable it and 
     * make sure it is flushed. */
    write(sup.fdWatchdog, "\0", 1);
    fsync(sup.fdWatchdog);
}

/*============================= Cycles =================================*/

/*! /todo Low priority: Use whole 64 bit cycles counter. */
inline uint32 OscSupCycGet()
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

inline uint32 OscSupCycToMicroSecs(uint32 cycles)
{
  return (cycles/(CPU_FREQ/1000000));
}

/*============================== SRAM =================================*/
inline uint32 OscSupSramL1ALen()
{
    return SRAM_L1A_LENGTH;
}

inline void* OscSupSramL1A()
{
    return (void *)SRAM_L1A;
}

inline uint32 OscSupSramL1BLen()
{
    return SRAM_L1B_LENGTH;
}

inline void* OscSupSramL1B()
{
    return (void *)SRAM_L1B;
}

inline uint32 OscSupSramScratchLen()
{
    return SRAM_SCRATCH_LENGTH;
}

inline void* OscSupSramScratch()
{
    return (void *)SRAM_SCRATCH;
}

/*============================== LED =================================*/
#ifdef TARGET_TYPE_INDXCAM
OSC_ERR OscSupLedInit()
{
    int     ret;
    
    if(sup.fdLed != 0)
    {
        OscLog(WARN, "%s: LED already open!\n", __func__);
        return -EALREADY_INITIALIZED;
    }
    
    sup.fdLed = open("/dev/pf27", O_RDWR, 0);
    if(sup.fdLed < 0)
    {
        OscLog(ERROR, "%s: Unable to open device! (%s)\n",
                __func__,
                strerror(errno));
        return -EDEVICE;
    }
    
    /* Set to output and disable as input. */
    ret = ioctl(sup.fdLed, SET_FIO_DIR, OUTPUT);
    ret |= ioctl(sup.fdLed, SET_FIO_INEN, INPUT_DISABLE);
    if(ret != 0)
    {
        OscLog(ERROR, "%s: Unable to set output direction!\n", __func__);
        close(sup.fdLed);
        return -EDEVICE;
    }
    
    return SUCCESS;
}

inline void OscSupLedWrite(char val)
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

void OscSupLedClose()
{
    if(sup.fdLed != 0)
    {
        close(sup.fdLed);
    }
}

#else /* TARGET_TYPE_INDXCAM */

OSC_ERR OscSupLedInit()
{
    return -ENO_SUCH_DEVICE;
}

inline void OscSupLedWrite(char val)
{
}

void OscSupLedClose()
{
}

#endif /* TARGET_TYPE_INDXCAM */
