#ifndef FRAMEWORK_DEPENDENCIES_H_
#define FRAMEWORK_DEPENDENCIES_H_

#ifdef OSC_HOST
    #include "oscar_types_host.h"
#endif
#ifdef OSC_TARGET
    #include "oscar_types_target.h"
#endif

/*! @brief Describes a module dependency of a module and all necessary
 * information to load and unload that module. */
struct OSC_DEPENDENCY
{
    /*! @brief The name of the dependency. */
    char                strName[24];
    /*! @brief The constructor of the dependency. */
    OSC_ERR             (*create)(void *);
    /*! @brief The destructor of the dependency. */
    void                (*destroy)(void *);
};

/*********************************************************************//*!
 * @brief Loads the module depencies give in a list of modules.
 * 
 * Goes through the given dependency array and tries to create all
 * member modules. If it fails at some point, destroy the dependencies
 * already created and return with an error code.
 * 
 * @param pFw Pointer to the framework
 * @param aryDeps Array of Dependencies to be loaded.
 * @param nDeps Length of the dependency array.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR OSCLoadDependencies(void *pFw,
        const struct OSC_DEPENDENCY aryDeps[], 
        const uint32 nDeps);

/*********************************************************************//*!
 * @brief Unloads the module depencies give in a list of modules.
 * 
 * Goes through the given dependency array backwards and destroys
 * all members.
 * 
 * @param pFw Pointer to the framework
 * @param aryDeps Array of Dependencies to be unloaded.
 * @param nDeps Length of the dependency array.
 *//*********************************************************************/
void OSCUnloadDependencies(void *pFw,
        const struct OSC_DEPENDENCY aryDeps[], 
        const uint32 nDeps);
#endif /*FRAMEWORK_DEPENDENCIES_H_*/
