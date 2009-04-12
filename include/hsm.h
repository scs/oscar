/*	Oscar, a hardware abstraction framework for the LeanXcam and IndXcam.
	Copyright (C) 2008 Supercomputing Systems AG
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*! @file hsm_pub.h
 * @brief API definition for Hierarchical State Machine module
 * 
	************************************************************************/
#ifndef HSM_PUB_H_
#define HSM_PUB_H_

#include "oscar.h"

/*! Module-specific error codes.
 * These are enumerated with the offset
 * assigned to each module, so a distinction over
 * all modules can be made */
enum EnOscHsmErrors
{
	EHSM_PARSING_FAILURE = OSC_HSM_ERROR_OFFSET
};

#define START_EVT ((Event)(-1))     /*!< @brief Predefined Start event */
#define ENTRY_EVT ((Event)(-2))     /*!< @brief Predefined Entry event */
#define EXIT_EVT  ((Event)(-3))     /*!< @brief Predefined Exit event */

/*!@brief Event type */
typedef int Event;
/*!@brief Message class */
typedef struct {
	Event evt;        /*!< @brief Event */
} Msg;

/*!@brief Hsm type */
typedef struct Hsm Hsm;
/*!@brief EvtHndlr type */
typedef Msg const *(*EvtHndlr)(Hsm*, Msg const*);

/*!@brief State type */
typedef struct State State;
/*!@brief State class */
struct State {
	State *super;           /*!< @brief Pointer to superstate */
	EvtHndlr hndlr;         /*!< @brief State's handler function */
	char const *name;       /*!< @brief State name */
};

/*********************************************************************//*!
 * @brief State Constructor
 * 
 * @param me        Pointer to hsm
 * @param name      State name
 * @param super     Pointer to super state
 * @param hndlr     Pointer to state handler
 *//*********************************************************************/
void StateCtor(State *me, char const *name, State *super, EvtHndlr hndlr);
#define StateOnEvent(me_, ctx_, msg_) \
	(*(me_)->hndlr)((ctx_), (msg_))

/*!@brief Hierarchical State Machine base class */
struct Hsm {
	char const *name; /*!< @brief Pointer to static name */
	State *curr;      /*!< @brief Current state */
	State *next;      /*!< @brief Next state (non 0 if transition taken) */
	State *source;    /*!< @brief Source state during last transition */
	State top;        /*!< @brief Top-most state object */
};

/*********************************************************************//*!
 * @brief Find # of levels to Least Common Ancestor
 * 
 * @param me        Pointer to hsm
 * @param target    Pointer to target state
 * @return Number of levels from current state to LCA
 *//*********************************************************************/
unsigned char HsmToLCA_(Hsm *me, State *target);

/*********************************************************************//*!
 * @brief Exit current states and all superstates up to LCA
 * 
 * @param me        Pointer to hsm
 * @param toLca    Number of level to LCA
 *//*********************************************************************/
void HsmExit_(Hsm *me, unsigned char toLca);

/*!< @brief Get current state */
#define STATE_CURR(me_) (((Hsm *)me_)->curr)

/*!< @brief Take start transition (no states need to be exited) */
#define STATE_START(me_, target_) (((Hsm *)me_)->next = (target_))

/*!< @brief Take a state transition (exit states up to the LCA) */

/*assert(((Hsm *)me_)->next == 0); \ do assertion test in marco*/
#define STATE_TRAN(me_, target_) if (1) { \
	static unsigned char toLca_ = 0xFF; \
	if (toLca_ == 0xFF) \
		toLca_ = HsmToLCA_((Hsm *)(me_), (target_)); \
	HsmExit_((Hsm *)(me_), toLca_); \
	((Hsm *)(me_))->next = (target_); \
} else ((void)0)

/*********************************************************************//*!
 * @brief Hsm Constructor
 * 
 * @param me        Pointer to hsm
 * @param name      Hsm name
 * @param topHndlr  Pointer to Hsm top handler
 *//*********************************************************************/
void HsmCtor(Hsm *me, char const *name, EvtHndlr topHndlr);

/*********************************************************************//*!
 * @brief Enter and start the top state
 * 
 * @param me        Pointer to hsm
 *//*********************************************************************/
void HsmOnStart(Hsm *me);

/*********************************************************************//*!
 * @brief State machine "engine"
 * 
 * @param me        Pointer to hsm
 * @param msg       Pointer to message
 *//*********************************************************************/
void HsmOnEvent(Hsm *me, Msg const *msg);



/*====================== API functions =================================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscHsmCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscHsmDestroy(void *hFw);



#endif /*HSM_PUB_H_*/
