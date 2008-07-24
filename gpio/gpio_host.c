/*! @file gpio_target.c
 * @brief GPIO module implementation for host 
 * 
 */

#include "oscar_types_host.h"

#include "gpio_pub.h"
#include "gpio_priv.h"
#include "oscar_intern.h"
#include <time.h>

/*! @brief The module singelton instance. */
struct OSC_GPIO gpio;       

/*! @brief The dependencies of this module. */
struct OSC_DEPENDENCY gpio_deps[] = {
        {"log", OscLogCreate, OscLogDestroy}
};

/*! @brief The length of the dependency array of this module. */
#define DEP_LEN (sizeof(gpio_deps)/sizeof(struct OSC_DEPENDENCY))

OSC_ERR OscGpioCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
    OSC_ERR err;

    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->gpio.useCnt != 0)
    {
        pFw->gpio.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }
    
    /* Load the module dependencies of this module. */
    err = OscLoadDependencies(pFw, 
            gpio_deps, 
            DEP_LEN);
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load dependencies! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    memset(&gpio, 0, sizeof(struct OSC_GPIO));
    
    /* Increment the use count */
    pFw->gpio.hHandle = (void*)&gpio;
    pFw->gpio.useCnt++; 
    
    return SUCCESS;
}

void OscGpioDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
            
    pFw = (struct OSC_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->gpio.useCnt--;
    if(pFw->gpio.useCnt > 0)
    {
        return;
    }
    
    OscUnloadDependencies(pFw, 
            gpio_deps, 
            DEP_LEN);
    
    memset(&gpio, 0, sizeof(struct OSC_GPIO));
}

