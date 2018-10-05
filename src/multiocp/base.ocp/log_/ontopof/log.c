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
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: log.c,v 1.3 2002/02/12 23:37:39 pinneke Exp $";
#endif /*}*/

/* #includes */
#include  "estd.h"
#include  "oe.h"
#include  "tm.h"
#include  "log.h"
#include  "eh.h"
#include  "queue.h"
#include  "seq.h"
#include  "pf.h"
#include  "outstrm.h"
 
typedef struct LogInfoSeq {
    QU_HEAD;
} LogInfoSeq;

Char * log_loc;
#if 0
FILE *logFile;
#endif

STATIC OUTSTRM_ModDesc outstrmDesc;

/* There has been a LOG_open */
STATIC SEQ_PoolDesc activePool;
STATIC LogInfoSeq activeSeq;		

/* There has been a LOG_setMask but not a corresponding LOG_open */
STATIC SEQ_PoolDesc setUpPool;
STATIC LogInfoSeq setUpSeq;	   

STATIC LOG_ModDesc G_logDesc;


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Void
LOG_init (void)
{
    static Bool virgin = TRUE;

    if ( virgin ) {
	virgin = FALSE;
	activePool = SEQ_poolCreate(sizeof(log_ModInfo), 0);
	setUpPool = SEQ_poolCreate(sizeof(log_ModInfo), 0);
	QU_INIT(&activeSeq);
	QU_INIT(&setUpSeq);

	/* Open an Outstream */
	OUTSTRM_init();
        outstrmDesc = OUTSTRM_open("COSOLE", OUTSTRM_K_console);

	/* Open the Global Log Module */
	G_logDesc = LOG_open("G_");
    }
}

/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC SuccFail
LOG_config(Char *logFileName)
{
 outstrmDesc = OUTSTRM_open(logFileName, OUTSTRM_K_file);

#if 0
    This should be converted to the OUTSTRM now

    FILE *fp;
    if ( (fp = fopen(logFileName, "w")) ) {
	logFile = fp;
	setbuf(logFile, (char *)0);
	return ( SUCCESS );
    }
    return ( FAIL );
#endif
}



/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC log_ModInfo *
LOG_open(Char *moduleName)
{
    log_ModInfo *quIndex;
    log_ModInfo *newModule;

    newModule = (log_ModInfo *)0;
    if ( ! (strlen(moduleName) < sizeof(newModule->moduleName) ) ) {
	EH_problem("Module Name Too Long:");
	EH_problem(moduleName);
	return (newModule);
    }	

    /* Check to make sure it is not duplicate */	
    for ( quIndex = QU_FIRST(&activeSeq);
	  ! QU_EQUAL(quIndex, &activeSeq);
	  quIndex = QU_NEXT(quIndex)) {
	if ( ! strcmp(quIndex->moduleName, moduleName) ) {
	    /* For each module we should really not have more
	     * than one LOG_open.
	     */
	    EH_problem("LOG_open: Duplicate module");
	    return ( quIndex );
	}
    }

    /* So It needs to be allocated and added to the Queue */
    if ( ! (newModule = (log_ModInfo *) SEQ_elemObtain(activePool)) ) {
	EH_problem("LOG_open: Not enough memory for new module");
	return ( (log_ModInfo *) 0 );
    }
    (Void) strcpy(newModule->moduleName, moduleName);
    QU_INIT(newModule);
    QU_INSERT(newModule, &activeSeq);

    /* Now let's see if there has been a prior set mask for
     * this module and if so, set the mask to the requested value.
     */
    for ( quIndex = QU_FIRST(&setUpSeq);
	  ! QU_EQUAL(quIndex, &setUpSeq);
	  quIndex = QU_NEXT(quIndex)) {
	if ( ! strcmp(quIndex->moduleName, moduleName) ) {
	    /* So there has been */
	    QU_REMOVE(quIndex);
	    SEQ_elemRelease(setUpPool, quIndex);
	    break;
	}
    }

    return ( newModule );
}

	
 

/*<
 * Function: log_trace
 * Description: Print trace messages, dependent on trace level
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/

/* VARARG */
PUBLIC Bool
log_modEvent(log_ModInfo *modInfo, Char *format, ...)
{
    va_list ap;

    static char msgBuf[128]; /* Careful! No stack hog under MSDOS */

    /* Check the mask bits before printing. This logic allows 
       printing of trace messages by EH_ functions as well as
       by LOG_TRACE */
    va_start (ap, format);
#if 0
    OS_timestamp (msgBuf, sizeof msgBuf, mask, 
		  modInfo ? modInfo->moduleName : "null", log_loc);
    fputs (msgBuf, logFile);
#else
    OUTSTRM_puts(outstrmDesc, log_loc);
    OUTSTRM_puts(outstrmDesc, " ");
#endif

    /* strip leading whitespace */
    while (*format == '\n' || *format == ' ' || *format == '\t') {
	  format++;
    }

    vsprintf (msgBuf, format, ap);

    if (msgBuf[strlen(msgBuf) - 1] != '\n') {
	strcat (msgBuf, "\n");
    }


    OUTSTRM_puts(outstrmDesc, msgBuf);
    va_end (ap);
    return ( SUCCESS );
}


/*<
 * Function: log_trace
 * Description: Print trace messages, dependent on trace level
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/

/* VARARG */
PUBLIC Bool
log_event(Char *format, ...)
{
    /* This uses the G_ module explicitly and 
     * because of efficiecy does not call log_modEvent
     * but instead replecates the code.
     */

    va_list ap;
    static char msgBuf[128]; /* Careful! No stack hog under MSDOS */

    va_start (ap, format);
#if 0
    OS_timestamp (msgBuf, sizeof msgBuf, mask, 
		  modInfo ? modInfo->moduleName : "null", log_loc);
    fputs (msgBuf, logFile);
#else
    OUTSTRM_puts(outstrmDesc, log_loc);
    OUTSTRM_puts(outstrmDesc, " ");
#endif
    /* strip leading whitespace */
    while (*format == '\n' || *format == ' ' || *format == '\t') {
	  format++;
    }

    vsprintf (msgBuf, format, ap);

    if (msgBuf[strlen(msgBuf) - 1] != '\n') {
	strcat (msgBuf, "\n");
    }

    OUTSTRM_puts(outstrmDesc, msgBuf);
    va_end (ap);
    return ( SUCCESS );
}



/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Char *
log_here(Char *fileName, Int lineNu)
{
    static Char buff[OS_MAX_FILENAME_LEN + 16];

    (Void) sprintf(buff, "%s:%d", fileName, lineNu);
    return ( buff );
}

