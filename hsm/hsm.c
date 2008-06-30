/*! @file hsm.c
 * @brief HSM implementation for host and target.
 * 
 ************************************************************************/

#include "hsm_pub.h"
#include "hsm_priv.h"
#include "oscar_intern.h"

struct OSC_HSM hsm; /*!< Module singelton instance */

/*! The dependencies of this module. */
struct OSC_DEPENDENCY hsm_deps[] = {
        {"log", OscLogCreate, OscLogDestroy}
};

OSC_ERR OscHsmCreate(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
    OSC_ERR err;

    pFw = (struct OSC_FRAMEWORK *)hFw;
    if(pFw->hsm.useCnt != 0)
    {
        pFw->hsm.useCnt++;
        /* The module is already allocated */
        return SUCCESS;
    }

    /* Load the module hsm_deps of this module. */
    err = OSCLoadDependencies(pFw, 
            hsm_deps, 
            sizeof(hsm_deps)/sizeof(struct OSC_DEPENDENCY));
    
    if(err != SUCCESS)
    {
        printf("%s: ERROR: Unable to load hsm_deps! (%d)\n",
                __func__, 
                err);
        return err;
    }
    
    memset(&hsm, 0, sizeof(struct OSC_HSM));
    
    
    /* Increment the use count */
    pFw->hsm.hHandle = (void*)&hsm;
    pFw->hsm.useCnt++;  
    
    return SUCCESS;
}

void OscHsmDestroy(void *hFw)
{
    struct OSC_FRAMEWORK *pFw;
        
    pFw = (struct OSC_FRAMEWORK *)hFw; 
    /* Check if we really need to release or whether we still 
     * have users. */
    pFw->hsm.useCnt--;
    if(pFw->hsm.useCnt > 0)
    {
        return;
    }
    
    OSCUnloadDependencies(pFw, 
            hsm_deps, 
            sizeof(hsm_deps)/sizeof(struct OSC_DEPENDENCY));
    
    
    memset(&hsm, 0, sizeof(struct OSC_HSM));
}


static Msg const startMsg = { START_EVT };
static Msg const entryMsg = { ENTRY_EVT };
static Msg const exitMsg  = { EXIT_EVT };
#define MAX_STATE_NESTING 8

/* .........................................................................*/
void StateCtor(State *me, char const *name, State *super, EvtHndlr hndlr) {
    me->name  = name;
    me->super = super;
    me->hndlr = hndlr;
}

/* .........................................................................*/
void HsmCtor(Hsm *me, char const *name, EvtHndlr topHndlr) {
    StateCtor(&me->top, "top", 0, topHndlr);
    me->name = name;
}

/* .........................................................................*/
void HsmOnStart(Hsm *me) {
    me->curr = &me->top;
    me->next = 0;
    StateOnEvent(me->curr, me, &entryMsg);
    while (StateOnEvent(me->curr, me, &startMsg), me->next) {
        State *entryPath[MAX_STATE_NESTING];
        register State **trace = entryPath;
        register State *s;
        *trace = 0;
        for (s = me->next; s != me->curr; s = s->super) {
            *(++trace) = s;                         /* trace path to target */
        } 
        /*while (s = *trace--) {        */     
        while (0 != (s = *trace--)) {          /* retrace entry from source */
            StateOnEvent(s, me, &entryMsg);
        }
        me->curr = me->next;
        me->next = 0;
    }
}   

/* .........................................................................*/
void HsmOnEvent(Hsm *me, Msg const *msg) {
    State *entryPath[MAX_STATE_NESTING];
    register State **trace; 
    register State *s;
    for (s = me->curr; s; s = s->super) {
        me->source = s;                 /* level of outermost event handler */
        msg = StateOnEvent(s, me, msg);
        if (msg == 0) {
            if (me->next) {                      /* state transition taken? */
                trace = entryPath;
                *trace = 0;
                for (s = me->next; s != me->curr; s = s->super) {
                    *(++trace) = s;                 /* trace path to target */
                }
                /*while (s = *trace--) {      */
                while (0 != (s = *trace--)) {     /* retrace entry from LCA */
                    StateOnEvent(s, me, &entryMsg);
                }
                me->curr = me->next;
                me->next = 0;
                while (StateOnEvent(me->curr, me, &startMsg), me->next) {
                    trace = entryPath;
                    *trace = 0;
                    for (s = me->next; s != me->curr; s = s->super) {
                        *(++trace) = s;            /* record path to target */
                    }
                    /*while (s = *trace--) {   */
                    while (0!= (s = *trace--)) {        /* retrace the entry */                        
                        StateOnEvent(s, me, &entryMsg);
                    }
                    me->curr = me->next;
                    me->next = 0;
                }
            }
            break;                                       /* event processed */
        }
    }
}

/* .........................................................................*/
void HsmExit_(Hsm *me, unsigned char toLca) {
    register State *s = me->curr;
    while (s != me->source) {
        StateOnEvent(s, me, &exitMsg);
        s = s->super;
    }
    while (toLca--) {
        StateOnEvent(s, me, &exitMsg);
        s = s->super;
    }
    me->curr = s;
}

/* .........................................................................*/
unsigned char HsmToLCA_(Hsm *me, State *target) {
    State *s, *t;
    unsigned char toLca = 0;
    if (me->source == target) {
        return 1;
    }
    for (s = me->source; s; ++toLca, s = s->super) {
        for (t = target; t; t = t->super) {
            if (s == t) {
                return toLca;
            }
        }
    }
    return 0;
}

