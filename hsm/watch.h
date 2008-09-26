/*! @file mainstate.h
 * @brief Definitions for main state machine
 * 
	************************************************************************/
#ifndef MAINSTATE_H
#define MAINSTATE_H

#include "sht.h"

enum MainStateEvents {
		TEACH_EVT,     /* Transition to learning mode. */
		IPC_TEACH_EVT,     /* Transition to learning mode over web interface. */
		FRAMESEQ_EVT,   /* frame ready to process (sequentially to next) */
		FRAMEPAR_EVT,    /* frame ready to process (parallel to next capture) */
		IPC_GET_ALGO_STATE_EVT, /* IPC request to read current algorithm state. */
		IPC_GET_LAST_FAILED_IMG_EVT, /* IPC request to read last failed image. */
		IPC_GET_REF_IMG_EVT,    /* IPC request to read the reference image. */
		IPC_GET_LIVE_IMG_EVT,   /* IPC request to read the current live image.*/
		IPC_SET_CORR_SENS_EVT,   /* IPC request to set the correlation sensitivity. */
		IPC_SET_STOP_CRIT_EVT,   /* IPC request to set the "machine stop" criterium. */
		IPC_SET_SHEET_RECT_EVT,  /* IPC request to set the coordinates of the rectangle designating the "sheet" area.*/
		IPC_RESET_STATS_EVT,    /* IPC request to reset the control mode statistics. */
		IPC_SET_OPT_DEBUG_OUT_EVT, /* IPC request to set the debut output option. */
		IPC_GO_IDLE_EVT         /* IPC request to go to idle mode to change the sheet format. */
};


/*typedef struct MainState MainState;*/
typedef struct MainState {
	Hsm super;
	State idle, learning, control;
	State dumpFrame, stage1, stage2, stage3, stage4;
	
	int Iteration;
	void* pPic;
	bool bMakuActive;   /* Makulatur; skip verrification */
} MainState;


void MainStateConstruct(MainState *me);


#endif /*MAINSTATE_H*/
