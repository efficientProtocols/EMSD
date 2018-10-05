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

/*
 * +
 * File name: schsel.c
 *
 * This is the Scheduler module which implements blocking with the
 * select() system call.  It may be used for all environments which
 * support the select() call.
 *
 * Description: Scheduler.
 *   SCH_init(Int maxSchInfo)
 *   SCH_term(Void)
 *   SCH_submit(Void (*func)(), Ptr arg, SCH_Event event, String taskName)
 *   SCH_block(Void)
 *   SCH_run(Void)
 *   SCH_taskDelete(SCH_Event event, Void (*func)(), Void *arg)
 *   SCH_allTaskDelete(SCH_Event event, Void (*func)(), Void *arg)
 *
 * Functions:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: schsel.c,v 1.14 1997/10/20 23:33:59 mohsen Exp $";
#endif /*}*/

#ifndef MSDOS
extern int errno;
#endif

#include "estd.h"		/* Extended stdio.h */
#include "sf.h"			/* System Facilities */
#include "eh.h"			/* Exception Handler */
#include "tm.h"			/* Trace Module */
#include "queue.h"		/* Queue */
#include "sch.h"		/* Scheduler */
#include "target.h"		/* Target Environment */


#ifdef TM_ENABLED
TM_ModuleCB *SCH_modCB;	     /* Trace Module: Scheduler module control block */
#endif

/* 
 * Scheduler Information.
 */
typedef struct SchInfo {
    struct SchInfo *next;	/* Pointer to next element of the queue */
    struct SchInfo *prev;	/* Pointer to prev element of the queue */

    Void (*func)();		/* Function to Call 			*/
    Void *arg;			/* Pointer to arguments of the function */
    Int event;			/* Event type (0 = intern, not 0 = sockd*/

#ifdef TM_ENABLED
    char taskName[64];		/* Task Name */
#endif
} SchInfo;

typedef struct SchInfoSeq {
    SchInfo *first;
    SchInfo *last;
} SchInfoSeq;

STATIC SchInfoSeq availSchInfo;		/* available event buffer 	*/
STATIC SchInfoSeq sch_activeSchInfo;	/* active events queue		*/
STATIC SchInfoSeq sch_runSchInfo;	/* ready to run events queue	*/

STATIC SchInfo *schInfoBuf;
STATIC SchInfo *schInfoBufEnd;

#ifdef TM_ENABLED
int schQuCnt = MAX_SCH_TASKS; 
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
    SchInfo *schInfo;
    static Bool virgin = TRUE;

#ifndef FAST
    if (maxSchInfo <= 0 || maxSchInfo > 65535 / sizeof(SchInfo)) {
	EH_problem("SCH_init: Invalid maxSchInfo");
	return SCH_INVALID_MAX_SCH_INFO;
    }
#endif

    if (!virgin) {
	return SCH_NOT_VIRGIN;
    }
    virgin = FALSE;
	
    if (TM_OPEN(SCH_modCB, "SCH_") == NULL) {
	EH_problem("SCH_init: TM_open failed");
	return -1;
    }

    /*
     * Create a Pool
     */
    schInfoBuf = (SchInfo *)SF_memObtain(maxSchInfo * sizeof(*schInfoBuf));
    if (!schInfoBuf) {
	EH_fatal("SCH_init: SF_memObtain failed for scheduler buffer\n");
    }
    schInfoBufEnd = &schInfoBuf[maxSchInfo - 1];

    QU_INIT(&availSchInfo);
    QU_INIT(&sch_activeSchInfo);
    QU_INIT(&sch_runSchInfo);

    for (schInfo = schInfoBuf; schInfo <= schInfoBufEnd; ++schInfo) {
	QU_INIT(schInfo);
	SF_QUINSERT(&availSchInfo, schInfo);	
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
    SF_memRelease(schInfoBuf);
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
>*/


/*
 * This function is called (possibly) at interrupt time by the timer
 * interrupt handler.  Disable stack probes for this function.
 */
#pragma check_stack( off )

#ifdef TM_ENABLED
# define DEBUG_STRING	, String taskName
#else
# define DEBUG_STRING
#endif

Int
SCH_submit(Void (*func)(), Void *arg, SCH_Event event  DEBUG_STRING)
{
    SchInfo *schInfo;
    Int status;
#ifdef OS_TYPE_UNIX
    int canPrint = TRUE;
#else
    int canPrint = (! OS_atomicOperationInProgress && ! OS_isrActive);
#endif

    /*
     * Queue Up the function and argument for synchronus processing.
     * Notice The Qu insertion must be protected.
     */

#if TM_ENABLED
    if ((status = SF_QUREMOVE(&availSchInfo, &schInfo)) != 0 ||
	 MAX_SCH_TASKS - schQuCnt > 30) {
#else
    if ((status = SF_QUREMOVE(&availSchInfo, &schInfo)) != 0 ) {
#endif
        SchInfo *curPnt;		/* Current Pointer */

	if (canPrint) {
	    char tmpBuf[256];

	    if (status != 0) {
#if TM_ENABLED
	    	sprintf(tmpBuf, "SCH_submit: Scheduler buffer is full: Max=%d,"
				" Current=%d\n\n"
	    		   	"\tSCHEDULER QUEUE CONTENTS:\n\n", 
				MAX_SCH_TASKS, schQuCnt);
#else
	    	sprintf(tmpBuf, "SCH_submit: Scheduler buffer is full: Max=%d"
	    		   	"\tSCHEDULER QUEUE CONTENTS:\n\n", 
				MAX_SCH_TASKS);
#endif
	    	EH_problem(tmpBuf);
	    } else {
	    	EH_problem("SCH_submit: Too many tasks scheduled\n\n"
	    		   "\tSCHEDULER QUEUE CONTENTS:\n\n");
	    }

	    EH_problem("sch_activeSchInfo: Current Pointer    TaskName\n");
	    for (curPnt = (SchInfo *)sch_activeSchInfo.first; 
		 curPnt != (SchInfo *)&sch_activeSchInfo;
		 curPnt = curPnt->next) {

#if TM_ENABLED
		sprintf(tmpBuf, "   0x%lx  %s\n", 
			(unsigned long int)curPnt , curPnt->taskName);
#else
		sprintf(tmpBuf, "   0x%lx\n", (unsigned long int)curPnt);
#endif
		EH_problem(tmpBuf);
	    }

	    EH_problem("sch_runSchInfo: Current Pointer    TaskName\n");
	    for (curPnt = (SchInfo *)sch_runSchInfo.first; 
		 curPnt != (SchInfo *)&sch_runSchInfo;
		 curPnt = curPnt->next) {

#if TM_ENABLED
		sprintf(tmpBuf, "   0x%lx  %s\n", 
			(unsigned long int)curPnt , curPnt->taskName);
#else
		sprintf(tmpBuf, "   0x%lx\n", (unsigned long int)curPnt);
#endif
		EH_problem(tmpBuf);
	    }
	}
#if 0
    	while (SF_QUREMOVE(&sch_activeSchInfo, &schInfo) != 0) {
	    extern udp_receive();
	    if (schInfo->func == udp_receive) {
	        func  = schInfo->func;
	        arg   = schInfo->arg;
	        event = schInfo->event;
	    }
    	    SF_QUINSERT(&availSchInfo, schInfo);
	}
    	SF_QUREMOVE(&availSchInfo, &schInfo);
#endif
    }

    if (status != 0) {
      /* NOTYET, all of them should use EH_, EH_ should then do the 
       * right thing based on which OS.
       */ 
#if defined(OS_TYPE_MSDOS)
        exit(1);
#elif defined(OS_VARIANT_WinCE)
	EH_fatal("SF_QUREMOVE() failed");
#else
	abort();
#endif
    }

    if (canPrint) {

	TM_TRACE((SCH_modCB, TM_ENTER, 
		 "----- addr=0x%lx   QuSize=%d  Avail=%d  taskName=%s\n",
		 schInfo, MAX_SCH_TASKS, --schQuCnt, taskName));
    }

    schInfo->func  = func;
    schInfo->arg   = arg;
    schInfo->event = event;

#ifdef TM_ENABLED
    if (taskName != NULL) {

	strncpy(schInfo->taskName, taskName, sizeof(schInfo->taskName) - 1);
	schInfo->taskName[sizeof(schInfo->taskName) - 1] = '\0';
    } else {

	strcpy(schInfo->taskName, "<null>");
    }

    if (canPrint) {

	TM_TRACE((SCH_modCB, TM_ENTER, 
		 "SCH_submit: Submit:    event=%2d, taskName= %s\n",
		 event, taskName));
    }
#endif
    
    if (SF_QUINSERT(&sch_activeSchInfo, schInfo) != 0) {

	if (canPrint) {

	    EH_problem("SCH_submit: SF_quInsert failed");
	}

	return -2;
    }

    return 0;

} /* SCH_submit() */ 
/* Reset stack checking to the default (/Gs disables it) */
#pragma check_stack ()


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
    struct timeval noblock;     

#if (defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Windows)) || defined(OS_VARIANT_WinCE)
    /* MSG msg ; */	/* unused */
    struct timeval timeout;     

    timeout.tv_usec = 0;
    timeout.tv_sec = 1;
# define TIMEOUT (&timeout)
#elif defined(OS_TYPE_MSDOS) && !defined(OS_VARIANT_Windows)
    struct timeval timeout;     

    timeout.tv_usec = 0;
    timeout.tv_sec = 0;
# define TIMEOUT (&timeout)
#else
# define TIMEOUT ((struct timeval *)0)
#endif

    noblock.tv_usec = 0;
    noblock.tv_sec = 0;
#define NO_BLOCK (&noblock)

    if (QU_EQUAL(QU_FIRST(&sch_activeSchInfo), &sch_activeSchInfo)) {
	return 0;
    }

    OS_EnterAtomic();

    /* Add the internal events to run queue */
    for (curPnt = QU_FIRST(&sch_activeSchInfo); 
	 ! QU_EQUAL(curPnt, &sch_activeSchInfo); ) {
	if ((fd = curPnt->event) == SCH_PSEUDO_EVENT) {
	    movPnt = curPnt;
            curPnt = QU_NEXT(curPnt);
    	    QU_MOVE(movPnt, &sch_runSchInfo);
	    internTask = TRUE;
	} else {
            curPnt = QU_NEXT(curPnt);
	}
    }

    OS_LeaveAtomic();

#ifndef OS_TYPE_MSDOS
    if (internTask == TRUE)	/* if any internal task in queue */
	return 0;		/* return and run them */
#endif

    FD_ZERO(&readFdSet);

    OS_EnterAtomic();

    /* Prepare the file descriptor mask for select */
    for (curPnt = QU_FIRST(&sch_activeSchInfo); 
	 ! QU_EQUAL(curPnt, &sch_activeSchInfo); 
	 curPnt = QU_NEXT(curPnt)) {
	if ((fd = curPnt->event) != SCH_PSEUDO_EVENT) {
    	    FD_SET(fd, &readFdSet);
    	    if (fd >= maxFd) {
		maxFd = fd + 1;
	    }
        }
    }

    OS_LeaveAtomic();

    TM_TRACE((SCH_modCB, TM_SCH_SELECT, 
	     "SCH_block:  Before select: maxFd=%d\n", maxFd));

    activeFds = select(maxFd, &readFdSet, (fd_set *)0, (fd_set *)0, TIMEOUT);

#if defined(OS_TYPE_MSDOS) 
# if defined(OS_VARIANT_QuickWin)
    /* Allow message loop to run */
    _wyield();		
# endif
#endif

    if (activeFds == 0) {
	return 0;
    } else if ( activeFds < 0 ) {
	    switch (errno) {
	    case EINTR: 
    		TM_TRACE((SCH_modCB, TM_ENTER, 
	     		"SCH_block:  Interupt signal (EINTR) interupted select\n", 
             		 activeFds));
		return 0;
	    case EBADF: 
		perror("SCH_block: Bad File descriptor: ");
		OS_EnterAtomic();
    		for (curPnt = QU_FIRST(&sch_activeSchInfo); 
	 	     ! QU_EQUAL(curPnt, &sch_activeSchInfo); ) {
	            movPnt = curPnt;
                    curPnt = QU_NEXT(curPnt);
		    if ((fd = movPnt->event) == SCH_PSEUDO_EVENT) {
		        continue;
		    }
		    FD_ZERO(&readFdSet);
		    FD_SET(fd, &readFdSet);
		    maxFd = fd + 1;
		    errno = 0;
		    (void) select(maxFd, &readFdSet,
		    		  (fd_set *)0, (fd_set *)0, NO_BLOCK);
		    if (errno == EBADF) {
			OS_LeaveAtomic();
			SCH_allTaskDelete(movPnt->event, NULL, NULL);
			OS_EnterAtomic();
		    }
		OS_LeaveAtomic();
		}
		return 0;
	    default:
		perror("SCH_block: select failed: ");
		return -3;
	    }
    }

    TM_TRACE((SCH_modCB, TM_SCH_SELECT, 
	     "SCH_block:  After select: active fds #= %d\n", activeFds));

    OS_EnterAtomic();

    for (cnt = 0, curPnt = QU_FIRST(&sch_activeSchInfo); 
	 ! QU_EQUAL(curPnt, &sch_activeSchInfo); ) {

        if (curPnt->event == SCH_PSEUDO_EVENT) {
	    movPnt = curPnt;
            curPnt = QU_NEXT(curPnt);
    	    QU_MOVE(movPnt, &sch_runSchInfo);
	    internTask = TRUE;
	    continue;
	}

        if (FD_ISSET(curPnt->event, &readFdSet)) {
	    TM_TRACE((SCH_modCB, TM_ENTER, 
		     "SCH_block:  Scheduled: event=%d, taskName= %s\n", 
		     curPnt->event, curPnt->taskName));

    	    FD_CLR(curPnt->event, &readFdSet);
	    movPnt = curPnt;
            curPnt = QU_NEXT(curPnt);

	    QU_MOVE(movPnt, &sch_runSchInfo);
	    cnt++;

	} else {
            curPnt = QU_NEXT(curPnt);
	}
    }

    OS_LeaveAtomic();
	
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
    OS_Uint32    sanityCounter = 0;


    while (SF_QUREMOVE(&sch_runSchInfo, &schInfo) == 0) {
	TM_TRACE((SCH_modCB, TM_ENTER, 
		 "SCH_run:    Running:   event=%d  taskName= %s\n", 
		 schInfo->event, schInfo->taskName));

        if (sanityCounter++ > 200)
	  {
	    TM_TRACE((SCH_modCB, TM_ENTER, 
		 "SCH_run:  Sanity counter exceeded, leaving SCH_run"));
	    break;
	  }

	(*schInfo->func)(schInfo->arg);

	if (&availSchInfo == (SchInfoSeq *) schInfo) {
	    EH_problem("SCH_run: quInsert into itself");
 	}

	if (SF_QUINSERT(&availSchInfo, schInfo)) {
	    EH_problem("SCH_run: quInsert failed");
	    return (FAIL);
 	}

        TM_TRACE((SCH_modCB, TM_ENTER, 
		"+++++ addr=0x%lx   QuSize=%d  Avail=%d  taskName=%s\n",
		schInfo, MAX_SCH_TASKS, ++schQuCnt, schInfo->taskName));
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
	     "SCH_eleteTask: Task deleted: Event=%d\n", event)); 
   
    OS_EnterAtomic();

    for (curPnt = (SchInfo *)sch_activeSchInfo.first; 
	 curPnt != (SchInfo *)&sch_activeSchInfo; 
	 curPnt = curPnt->next) {
	if (event != SCH_PSEUDO_EVENT  	||  
	    (event == SCH_PSEUDO_EVENT 	&& 
	     func == curPnt->func      	&& 
	     arg == curPnt->arg)) {
	    if (event == curPnt->event) {
		QU_MOVE(curPnt, &availSchInfo);
		OS_LeaveAtomic();
                return (SUCCESS);
	    }
	}
    }

    OS_LeaveAtomic();
    return (FAIL);

} /* SCH_taskDelete() */

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

} /* SCH_allTaskDelete() */

