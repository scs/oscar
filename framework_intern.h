/*! @file framework_intern.h
 * @brief Framework internal definitions
 */

#ifndef FRAMEWORK_INTERN_H_
#define FRAMEWORK_INTERN_H_

#include "framework_dependencies.h"
#ifdef LCV_HOST
    #include "framework_types_host.h"
#endif
#ifdef LCV_TARGET
    #include "framework_types_target.h"
#endif

#define RND_MOD_Unbiased
/*#define RND_MOD_Biased*/



/*! @brief Describes an LCV module and keeps track of how many users
 * hold references to it. */
struct LCV_MODULE
{
    /*! @brief Handle to the module. */
    void *      hHandle; 
    /*! @brief Used to prevent premature unloading. */
    int         useCnt;  
};

/*! @brief The LCV framework object with handle to modules*/
struct LCV_FRAMEWORK 
{
    struct LCV_MODULE log;     /*!< @brief logging */
    struct LCV_MODULE cam;     /*!< @brief camera */
    struct LCV_MODULE cpld;    /*!< @brief cpld */
    struct LCV_MODULE lgx;     /*!< @brief logic */
    struct LCV_MODULE sim;     /*!< @brief simulation */
    struct LCV_MODULE bmp;     /*!< @brief bitmap */
    struct LCV_MODULE swr;     /*!< @brief stimulation writer */
    struct LCV_MODULE srd;     /*!< @brief stimulation reader*/
    struct LCV_MODULE ipc;     /*!< @brief interprocess communication */
    struct LCV_MODULE sup;     /*!< @brief support */
    struct LCV_MODULE frd;     /*!< @brief filename reader */
    struct LCV_MODULE rtl;     /*!< @brief DSP runtime library */
    struct LCV_MODULE dma;     /*!< @brief Memory DMA */
    struct LCV_MODULE hsm;     /*!< @brief hierarchical state machine*/
    struct LCV_MODULE cfg;     /*!< @brief configuration file reader and writer*/
    struct LCV_MODULE clb;     /*!< @brief camera calibration*/    
    struct LCV_MODULE vis;     /*!< @brief Vision library*/    
};

#endif /* FRAMEWORK_INTER_H_ */

