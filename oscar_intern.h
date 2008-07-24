/*! @file oscar_intern.h
 * @brief Framework internal definitions
 */

#ifndef OSCAR_INTERN_H_
#define OSCAR_INTERN_H_

#include "oscar_dependencies.h"
#ifdef OSC_HOST
    #include "oscar_types_host.h"
#endif
#ifdef OSC_TARGET
    #include "oscar_types_target.h"
#endif

#define RND_MOD_Unbiased
/*#define RND_MOD_Biased*/



/*! @brief Describes an OSC module and keeps track of how many users
 * hold references to it. */
struct OSC_MODULE
{
    /*! @brief Handle to the module. */
    void *      hHandle; 
    /*! @brief Used to prevent premature unloading. */
    int         useCnt;  
};

/*! @brief The Oscar framework object with handle to modules*/
struct OSC_FRAMEWORK 
{
    struct OSC_MODULE log;     /*!< @brief logging */
    struct OSC_MODULE cam;     /*!< @brief camera */
    struct OSC_MODULE cpld;    /*!< @brief cpld */
    struct OSC_MODULE lgx;     /*!< @brief logic */
    struct OSC_MODULE sim;     /*!< @brief simulation */
    struct OSC_MODULE bmp;     /*!< @brief bitmap */
    struct OSC_MODULE swr;     /*!< @brief stimulation writer */
    struct OSC_MODULE srd;     /*!< @brief stimulation reader*/
    struct OSC_MODULE ipc;     /*!< @brief interprocess communication */
    struct OSC_MODULE sup;     /*!< @brief support */
    struct OSC_MODULE frd;     /*!< @brief filename reader */
    struct OSC_MODULE dspl;    /*!< @brief DSP runtime library */
    struct OSC_MODULE dma;     /*!< @brief Memory DMA */
    struct OSC_MODULE hsm;     /*!< @brief hierarchical state machine*/
    struct OSC_MODULE cfg;     /*!< @brief configuration file reader and writer*/
    struct OSC_MODULE clb;     /*!< @brief camera calibration*/    
    struct OSC_MODULE vis;     /*!< @brief Vision library*/   
    struct OSC_MODULE gpio;	   /*!< @brief GPIO */ 
};

#endif /* OSCAR_INTER_H_ */

