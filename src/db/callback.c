/* callback.c */
/* share/src/db  $Id$ */

/* general purpose callback tasks		*/
/*
 *      Original Author:        Marty Kraimer
 *      Date:   	        07-18-91
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01	12-12-91	mrk	moved from dbScan.c to callback.c
 * .02	04-23-92	jba	Fixed test on priority
*/

#include	<vxWorks.h>
#include	<stdlib.h>
#include	<types.h>
#include	<semLib.h>
#include	<rngLib.h>
#include 	<lstLib.h>

#include	<dbDefs.h>
#include	<callback.h>
#include	<taskwd.h>
#include	<task_params.h>

#define QUEUESIZE 1000
static SEM_ID callbackSem[NUM_CALLBACK_PRIORITIES];
static RING_ID callbackQ[NUM_CALLBACK_PRIORITIES];
static int callbackTaskId[NUM_CALLBACK_PRIORITIES];
volatile int callbackRestart=FALSE;

/* forward references */
static void wdCallback(long);	/*callback from taskwd*/
static void start();		/*start or restart a callbackTask*/

/*public routines */
long callbackInit()
{
    int i;

    for(i=0; i<NUM_CALLBACK_PRIORITIES; i++) {
	start(i);
    }
    return(0);
}

/* Routine which places requests into callback queue*/
/* This routine can be called from interrupt routine*/
void callbackRequest(CALLBACK *pcallback)
{
    int priority = pcallback->priority;
    int lockKey;
    int nput;
    static int status;

    if(priority<0 || priority>=(NUM_CALLBACK_PRIORITIES)) {
	char msg[80];

	sprintf(msg,"callbackRequest called with invalid priority=%d",priority);
	errMessage(-1,msg);
	return;
    }
    lockKey = intLock();
    nput = rngBufPut(callbackQ[priority],(void *)&pcallback,sizeof(pcallback));
    intUnlock(lockKey);
    if(nput!=sizeof(pcallback)) errMessage(-1,"callbackRequest ring buffer full");
    if((status=semGive(callbackSem[priority]))!=OK) {
/*semGive randomly returns garbage value*/
/*
		errMessage(-1,"semGive returned error in callbackRequest\n");
*/
    }
    return;
}

/* General purpose callback task */
/*static*/
 void callbackTask(int priority)
{
    volatile CALLBACK *pcallback;
    int nget;

    while(TRUE) {
	/* wait for somebody to wake us up */
        if(semTake(callbackSem[priority],WAIT_FOREVER)!=OK ){
		errMessage(0,"semTake returned error in callbackTask\n");
		taskSuspend(0);
	}
	while(rngNBytes(callbackQ[priority])>=sizeof(pcallback)) {
	    nget = rngBufGet(callbackQ[priority],(void *)&pcallback,sizeof(pcallback));
	    if(nget!=sizeof(pcallback)) {
		errMessage(0,"rngBufGet failed in callbackTask");
		taskSuspend(0);
	    }
	    (*pcallback->callback)(pcallback);
	}
    }
}

static void start(int ind)
{
    int     priority;

    if((callbackSem[ind] = semBCreate(SEM_Q_FIFO,SEM_EMPTY))==NULL)
	errMessage(0,"semBcreate failed while starting a callback task\n");
    if(ind==0) priority = CALLBACK_PRI_LOW;
    else if(ind==1) priority = CALLBACK_PRI_MEDIUM;
    else if(ind==2) priority = CALLBACK_PRI_HIGH;
    else {
	errMessage(0,"semBcreate failed while starting a callback task\n");
	return;
    }
    if((callbackQ[ind] = rngCreate(sizeof(CALLBACK *) * QUEUESIZE)) == NULL) 
	errMessage(0,"rngCreate failed while starting a callback task");
    callbackTaskId[ind] = taskSpawn(CALLBACK_NAME,priority,
    			CALLBACK_OPT,CALLBACK_STACK,
    			(FUNCPTR)callbackTask,ind,0,0,0,0,0,0,0,0,0);
    if(callbackTaskId[ind]==ERROR) {
	errMessage(0,"Failed to spawn a callback task");
	return;
    }
    taskwdInsert(callbackTaskId[ind],wdCallback,(void *)(long)ind);
}


static void wdCallback(long ind)
{
    taskwdRemove(callbackTaskId[ind]);
    if(!callbackRestart)return;
    if(taskDelete(callbackTaskId[ind])!=OK)
	errMessage(0,"taskDelete failed while restarting a callback task\n");
    if(semDelete(callbackSem[ind])!=OK)
	errMessage(0,"semDelete failed while restarting a callback task\n");
    rngDelete(callbackQ[ind]);
    start(ind);
}
