/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*+
 * File name: sch.c
 *
 * Description: Scheduler.
 *
-*/

/*
 * Author: Mohsen Banan
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sch.c,v 1.29 1996/09/04 19:37:41 kamran Exp $";
#endif /*}*/

#if 0 /* djl */
#include <sys/types.h>
#include <sys/socket.h>

#ifndef MSDOS
#include <sys/select.h> 
#include <sys/un.h>
#include <sys/time.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef MSDOS
#include <time.h>
#endif

#ifdef MSDOS
#include  <signal.h>
#else
#ifdef UCBSIGNALS /* djl */
#include "/usr/ucbinclude/sys/signal.h"
#else
#include <signal.h>
#endif
#endif

#include <errno.h>

#ifndef MSDOS /* NOT TESTED YET, MB */
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include <ctype.h>

#ifndef MSDOS
extern int errno;
#endif

#endif /* djl */

#include "estd.h"
#include "sf.h"
#include "eh.h"
#include "tm.h"
#include "tmr.h"
#include "sf.h"
#include "sch.h"

#ifdef TM_ENABLED
#include "target.h"
#endif

#ifdef MSDOS
#undef EH_problem
#define EH_problem(str)  {extern int G_isrActive;if(!G_isrActive)fprintf(stderr,"PROBLEM: %s, %d: %s\n",__FILE__, __LINE__, (str));}
#endif


extern void free(void *);
extern int  sigblock(int);
extern int  sigsetmask(int);
extern void *memset(void *s, int c, size_t n);

TM_ModuleCB *SCH_modCB;	     /* Trace Module: Scheduler module control block */

/* 
 * Scheduler Information.
 */
typedef struct SchInfo {
    struct SchInfo *next;	/* Pointer to next element of the queue */
    struct SchInfo *prev;	/* Pointer to prev element of the queue */
    Void (*func)();		/* Function to Call 			*/
    Void *arg;			/* Pointer to arguments of the function */
    SCH_Event event;		/* Event type (-1 = intern, not 0 = sockd*/
#ifdef TM_ENABLED
    char taskName[64];
#endif
} SchInfo;

typedef struct SchInfoSeq {
    SchInfo *first;
    SchInfo *last;
} SchInfoSeq;

STATIC SchInfoSeq availSchInfo;		/* available event buffer 	*/
STATIC SchInfoSeq activeSchInfo;	/* active events queue		*/
STATIC SchInfoSeq runSchInfo;		/* ready to run events queue	*/

STATIC SchInfo *schInfoBuf;
STATIC SchInfo *schInfoBufEnd;

#ifdef TM_ENABLED
int schQuCnt = MAX_SCH_BUFFER; 
#endif

/*<
 * Function:	SCH_init
 *
 * Description:	Initialize scheduler
 *
 * Arguments:	Max length of scheduler queue.
 *
 * Returns:	0 if successful, a negative error value otherwise.
 *
>*/

Int
SCH_init(Int maxSchInfo)
{
    static Bool virgin = TRUE;
			int n;

#ifndef FAST
    if (maxSchInfo <= 0 ) {
	EH_problem("SCH_init: Invalid maxSchInfo");
	return -2;
    }
#endif

    if (!virgin) {
	return 0;
    }
    virgin = FALSE;
	
#ifdef TM_ENABLED
    if ( ! (SCH_modCB = TM_open("SCH_") )) {
	    EH_problem("SCH_init: TM_open failed");
	    return -1;
    }
#endif

    /*
     * Create a Pool
     */


    QU_INIT(&availSchInfo);
    QU_INIT(&activeSchInfo);
    QU_INIT(&runSchInfo);

	for (n  = 0; n < maxSchInfo; n++)
	{
		SchInfo *sch;
		sch = (SchInfo *) SF_memObtain(sizeof(SchInfo));
		if (sch == (SchInfo *) 0) EH_fatal("out of memory");
		QU_INIT(sch);
		SF_quInsert(&availSchInfo,sch);
	}
	

    return 0;

} /* SCH_init() */

/*<
 * Function:	SCH_term
 *
 * Description: Terminate scheduler.
 *
 * Arguments:	None.
 *
 * Returns:	None.
 *
>*/
Void
SCH_term(Void)
{
/*
    SF_memRelease(schInfoBuf);
*/
}

/*<
 * Function:	SCH_submit
 *
 * Description:	Add a function to scheduler queue.
 *
 * Arguments:	Function, arguments, event type.
 *
 * Returns:	0 if successful, a negative error value otherwise.
 *
 * Caution:	Under MSDOS, called from within the timer tick interrupt
 *              service routine. Bracket printf(), etc. with a call to
 *              OS_isrIsActive().
 *
>*/

#ifndef MSDOS /* NOTYET - hugh */

Int
#ifdef TM_ENABLED
SCH_submit(Void (*func)(), Ptr arg, SCH_Event event, String taskName)
#else
SCH_submit(Void (*func)(), Ptr arg, SCH_Event event)
#endif
{
    SchInfo *schInfo;
    Int status;

    /*
     * Queue Up the function and argument for synchronus processing.
     * Notice The Qu insertion must be protected.
     */

    if ((status = SF_quRemove(&availSchInfo, &schInfo)) != 0) {
        SchInfo *curPnt;		/* Current Pointer */

#ifndef WINDOWS
	if (!OS_isrIsActive()) {
	  EH_problem("Memory not available");
	}
#endif

        for (curPnt = (SchInfo *)activeSchInfo.first; 
	     curPnt != (SchInfo *)&activeSchInfo; curPnt = curPnt->next) {
	  if (!OS_isrIsActive()) {
#ifdef TM_ENABLED
	    fprintf(stderr, "activeSchInfo: 0x%lx  %s\n", curPnt , 
		    curPnt->taskName);
#else
	    fprintf(stderr, "activeSchInfo: 0x%lx\n", curPnt);
#endif
	  }
	}
        for (curPnt = (SchInfo *)runSchInfo.first; 
	     curPnt != (SchInfo *)&runSchInfo; curPnt = curPnt->next) {
	  if (!OS_isrIsActive()) {
#ifdef TM_ENABLED
	    fprintf(stderr, "runSchInfo: 0x%lx  %s\n", curPnt, curPnt->taskName);
#else
	    fprintf(stderr, "runSchInfo: 0x%lx\n", curPnt);
#endif
	  }
	}
	exit(13);
    }

    if (!OS_isrIsActive()) {
#ifdef TM_ENABLED1
      printf("\n----- addr=0x%x   QuSize=%d  Avail=%d", schInfo, MAX_SCH_BUFFER,
	     --schQuCnt);
#endif
    }
    schInfo->func  = func;
    schInfo->arg   = arg;
    schInfo->event = event;

#ifdef TM_ENABLED
	{
	int i;
	for (i = 0; i < 50; i++)
	{
		if (!isprint(taskName[i]) && taskName[i] != '\r' && 
			taskName[i] != '\n') break;
		if (taskName[i] == 0)
			break;
	}
	if (i >= 50) schInfo->taskName = "fake taskName";
	else schInfo->taskName = strdup(taskName);
	}
/*
	if (taskName) schInfo->taskName = strdup(taskName);
	else schInfo->taskName = "hey! No task name!";
*/
#endif

    TM_TRACE((SCH_modCB, TM_ENTER, 
             "SCH_submit: Submit:    event=%ld, taskName= %s\n",
   	     (signed long) event, taskName));
    
    if (SF_quInsert(&activeSchInfo, schInfo) != 0) {
      if (!OS_isrIsActive()) {
	EH_problem("SCH_submit: SF_quInsert failed");
      }
      return -2;
    }

    return 0;

} /* SCH_submit() */ 

#endif /* ndef MSDOS */

#ifdef MSDOS

Int
SCH_submit(Void (*func)(), Ptr arg, SCH_Event event, String taskName)
{
   SchInfo *schInfo;
   Int status;

   /*
    * Queue Up the function and argument for synchronus processing.
    * Notice The Qu insertion must be protected.
    */
   if ((status = SF_quRemove(&availSchInfo, &schInfo)) != 0) {
      return -1;
   }

   schInfo->func  = func;
   schInfo->arg   = arg;
   schInfo->event = event;

   if (SF_quInsert(&activeSchInfo, schInfo) != 0) {
      EH_problem("SCH_submit: SF_quInsert failed");
      return -2;
   }

   return 0;

} /* SCH_submit() */ 

#endif /* def MSDOS */


int tickflag = 0;
time_t lasttick = 0;
/*<
 * Function:	SCH_block
 *
 * Description:	Block until an event arrives.
 *
 * Arguments:	None.
 *
 * Returns:	0 if successful, a negative error value otherwise.
 *
>*/
Int
SCH_block(Void)
{
    fd_set readFdSet;		/* File descriptor set for select function */
    Int fd;			/* File descriptor */

    Int   maxFd = 0;		/* Largest file descriptor number */
    Bool  internTask = FALSE;	/* Internal task is schedulted */
    Int   cnt;			/* Counter */
    Int   activeFds;		/* Active file descriptors */

    SchInfo *curPnt;		/* Current Pointer */
    SchInfo *movPnt;		/* Move printer */

#ifdef MSDOS
    struct timeval timeout;     
    timeout.tv_usec = 0;
    timeout.tv_sec = 0;
#define TIMEOUT (&timeout)
#else
#define TIMEOUT ((struct timeval *)0)
#endif

#ifdef WINDOWS
	{
	MSG msg;
	void TMR_poll();
	time_t thistick;

	thistick = time(NULL);
	if (thistick > lasttick)
	{
		lasttick = thistick;
        	SCH_submit((Void *)TMR_poll, (Ptr)0, (SCH_Event)SCH_PSEUDO_EVENT
#ifdef TM_ENABLED
		,"tick");
#else
		);
#endif
		tickflag = 0;
	}
    }
#endif

    if (activeSchInfo.first == (SchInfo *)&activeSchInfo) {
	return 0;
    }

    /* Add the internal events to run queue */
    for (curPnt = (SchInfo *)activeSchInfo.first; 
	 curPnt != (SchInfo *)&activeSchInfo; ) {
	if ((fd = curPnt->event) == SCH_PSEUDO_EVENT) {
	    movPnt = curPnt;
            curPnt = curPnt->next;
    	    if (SF_quMove(&runSchInfo, movPnt))
		EH_problem("SCH_block: SF_quMove failed");
	    internTask = TRUE;
	} else {
            curPnt = curPnt->next;
	}
    }

    if (internTask == TRUE)	/* if any internal task in queue */
	return 0;		/* return and run them */

    FD_ZERO(&readFdSet);

    /* Prepare the file descriptor mask for select */
    for (curPnt = (SchInfo *)activeSchInfo.first; 
	 curPnt != (SchInfo *)&activeSchInfo; 
	 curPnt = curPnt->next) {
	if ((fd = curPnt->event) != SCH_PSEUDO_EVENT) {
    	    FD_SET(fd, &readFdSet);
    	    if (fd >= maxFd) {
		maxFd = fd + 1;
	    }
#ifdef TM_ENABLED
    TM_trace(SCH_modCB, TM_ENTER, 
	     "SCH_block:  Block on   event=%2d, taskName= %s\n", 
             fd, curPnt->taskName);
#endif
        }
    }

#ifdef TM_ENABLED
    TM_trace(SCH_modCB, TM_ENTER, 
	     "SCH_block:  Before select: maxFd=%d\n", maxFd);
#endif

    activeFds = select(maxFd, &readFdSet, (fd_set *)0, (fd_set *)0, TIMEOUT);

#ifdef WINDOWS  /* should be ifdef _QC i.e. Quick C (QuickWin) */
    _wyield();		/* Allow message loop to run */
#endif

    if ( activeFds == 0 ) {
#ifdef TM_ENABLED
	TM_trace(SCH_modCB, TM_ENTER, 
		 "SCH_block:  select() time limit expired\n", maxFd);
#endif
	return 0;
    } else if ( activeFds < 0 ) {
	    switch (errno) {
	    case EINTR: 
#ifdef TM_ENABLED
    TM_trace(SCH_modCB, TM_ENTER, 
	     "SCH_block:  Interupt signal (EINTR) interupted select\n");
#endif
		return 0;
	    case EBADF: 
		perror("SCH_block: Bad File descriptor: ");
		return -2;
	    default:
		perror("SCH_block: select failed: ");
		return -3;
	    }
    }
#ifdef TM_ENABLED
    TM_trace(SCH_modCB, TM_ENTER, 
	     "SCH_block:  After select: active fds #= %d\n", activeFds);
#endif

    for (cnt = 0, curPnt = (SchInfo *)activeSchInfo.first; 
	 curPnt != (SchInfo *)&activeSchInfo; ) {

        if (FD_ISSET(curPnt->event, &readFdSet)) {
#ifdef TM_ENABLED
	    TM_trace(SCH_modCB, TM_ENTER, 
		     "SCH_block:  Scheduled: event=%ld, taskName= %s\n", 
		     (signed long) curPnt->event, curPnt->taskName);
#endif 
	    movPnt = curPnt;
            curPnt = curPnt->next;

	    if (SF_quMove(&runSchInfo, movPnt)) {
		EH_problem("SCH_block: SF_quMove failed");
	    }
	    cnt++;

	} else {
            curPnt = curPnt->next;
	}
    }
	
    return activeFds;

} /* SCH_block() */

/*<
 * Function:	SCH_run
 *
 * Description:	Run all tasks of run queue.
 *
 * Arguments:	None.
 *
 * Returns:	0 if successful, -1 otherwise.
 *
>*/
SuccFail
SCH_run(Void)
{
    SchInfo sI, *schInfo = &sI;


    while (SF_quRemove(&runSchInfo, &schInfo) == 0) {
#ifdef TM_ENABLED
	TM_trace(SCH_modCB, TM_ENTER, 
		 "SCH_run:    Running:   event=%ld  taskName= %s\n", 
		 (signed long) schInfo->event, schInfo->taskName);
#endif

	(*schInfo->func)(schInfo->arg);

	if (SF_quInsert(&availSchInfo, schInfo)) {
	    EH_problem("SCH_run: quInsert failed");
	    return (FAIL);
 	}
#ifdef TM_ENABLED1
printf("\n+++++ addr=0x%x   QuSize=%d  Avail=%d", schInfo, MAX_SCH_BUFFER, ++schQuCnt);
#endif
    }
    return (SUCCESS);

} /* SCH_run() */


/*<
 * Function:	SCH_taskDelete
 *
 * Description:	Delete a task from active queue
 *
 * Arguments:	Event descriptor.
 *
 * Returns:	0 if successful, -1 otherwise.
 *
>*/
SuccFail
SCH_taskDelete(SCH_Event event, Void (*func)(), Void *arg)
{
    SchInfo *curPnt;		/* Current Pointer */

    TM_TRACE((SCH_modCB, TM_ENTER, 
	     "SCH_eleteTask: Task deleted: Event=%ld\n", (signed long) event)); 
   
    for (curPnt = (SchInfo *)activeSchInfo.first; 
	 curPnt != (SchInfo *)&activeSchInfo; 
	 curPnt = curPnt->next) {
	if (event != SCH_PSEUDO_EVENT  	||  
	    (event == SCH_PSEUDO_EVENT 	&& 
	     func == curPnt->func      	&& 
	     arg == curPnt->arg)) {
	    if (event == curPnt->event) {
	    	if (SF_quInsert(&availSchInfo, curPnt)) {
		    EH_problem("SCH_submit: SF_quInsert failed");
		    return -2;
	    	}
                return (SUCCESS);
	    }
	}
    }
    
    if (curPnt == (SchInfo *) &activeSchInfo) {
	return (FAIL);
    }

    return (SUCCESS);
}

/*<
 * Function:	SCH_allTaskDelete
 *
 * Description:	Delete all task related to the given event from active queue
 *
 * Arguments:	Event descriptor.
 *
 * Returns:	0 if successful, -1 otherwise.
 *
>*/
SuccFail
SCH_allTaskDelete(SCH_Event event, Void (*func)(), Void *arg)
{
    Int retVal = FAIL;

    while (SCH_taskDelete(event, func, arg) == SUCCESS) {
	retVal = SUCCESS;
    }

    return retVal;
}

