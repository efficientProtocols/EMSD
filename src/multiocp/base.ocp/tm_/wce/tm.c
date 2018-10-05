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
static char rcs[] = "$Id: tm.c,v 1.3 2002/10/25 19:37:51 mohsen Exp $";
#endif /*}*/

/* #includes */
#include  "estd.h"
#include  "oe.h"
#include  "tm.h"
#include  "eh.h"
#include  "queue.h"
#include  "seq.h"
#include  "pf.h"
 
typedef struct TmInfoSeq {
    QU_HEAD;
} TmInfoSeq;

Char * tm_loc;
#if defined(OS_VARIANT_WinCE)
#define TM_USE_COM1_FILESPEC	"\\EMSD\\CONFIG\\TM_COM1.TXT"
#define TM_USE_COM2_FILESPEC	"\\EMSD\\CONFIG\\TM_COM2.TXT"
#define TM_USE_COM3_FILESPEC	"\\EMSD\\CONFIG\\TM_COM3.TXT"
#define TM_DEFAULT_COM_PORT	9 /* pick a non-existant com port */
Int		tmDefaultComPort;
HINSTANCE	tmFile;
#else
FILE *tmFile;
#endif /* OS_VARIANT_WinCE */

#ifdef TM_ENABLED

#define TM_WINCE_LINE_MAX_CHARS OS_WCE_FMT_MAXLEN

#if defined(OS_VARIANT_WinCE)
#include "sf.h"
STATIC WCHAR	tm_line[TM_WINCE_LINE_MAX_CHARS];
STATIC DWORD	tm_dwBytesWritten;
#endif /* OS_VARIANT_WinCE */

/* There has been a TM_open */
STATIC SEQ_PoolDesc activePool;
STATIC TmInfoSeq activeSeq;		

/* There has been a TM_setMask but not a corresponding TM_open */
STATIC SEQ_PoolDesc setUpPool;
STATIC TmInfoSeq setUpSeq;	   


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
TM_init (void)
{
    static Bool virgin = TRUE;

    if ( virgin ) {
	virgin = FALSE;
	activePool = SEQ_poolCreate(sizeof(tm_ModInfo), 0);
	setUpPool = SEQ_poolCreate(sizeof(tm_ModInfo), 0);
	QU_INIT(&activeSeq);
	QU_INIT(&setUpSeq);

#if defined(OS_VARIANT_WinCE)
	{
	    HANDLE	hFileComPortSpec;
	    HANDLE	fp;
	    WCHAR	szStringBuf[OS_WCE_FMT_MAXLEN];

	    /* 
	     * set the default trace output file
	     */

	    if ( (hFileComPortSpec = CreateFile(TEXT(TM_USE_COM1_FILESPEC),
						0, 0, NULL,
						OPEN_EXISTING, 0, NULL))
		 != INVALID_HANDLE_VALUE ) {
		/* COM1 specified as default trace output */
		tmDefaultComPort = 1;
		CloseHandle(hFileComPortSpec);
	    }
	    else if ( (hFileComPortSpec = CreateFile(TEXT(TM_USE_COM2_FILESPEC),
						     0, 0, NULL,
						     OPEN_EXISTING, 0, NULL))
		      != INVALID_HANDLE_VALUE ) {
		/* COM2 specified as default trace output */
		tmDefaultComPort = 2;
		CloseHandle(hFileComPortSpec);
	    }
	    else if ( (hFileComPortSpec = CreateFile(TEXT(TM_USE_COM3_FILESPEC),
						     0, 0, NULL,
						     OPEN_EXISTING, 0, NULL))
		      != INVALID_HANDLE_VALUE ) {
		/* COM2 specified as default trace output */
		tmDefaultComPort = 3;
		CloseHandle(hFileComPortSpec);
	    }
	    else {
		/* Unspecified trace ouput, so use TM_DEFAULT_COM_PORT */
		tmDefaultComPort = TM_DEFAULT_COM_PORT;
	    }
	    
	    wsprintf(szStringBuf, TEXT("COM%d:"), tmDefaultComPort);
	    fp = CreateFile(szStringBuf,
			    GENERIC_WRITE,
			    FILE_SHARE_WRITE,
			    NULL, OPEN_EXISTING, 0, NULL);
	    if ( fp != INVALID_HANDLE_VALUE ) {
		tmFile = fp;
		wsprintf(szStringBuf,
			 TEXT("TM_ using COM%d: by default."),
			 tmDefaultComPort);
		MessageBox(NULL, szStringBuf, szStringBuf, MB_OK);
	    }
	    else {
		tmFile = NULL;
	    }
	}
#elif (defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Win16)) || defined(OS_VARIANT_WinCE)
	tmFile = NULL;
#else
	tmFile = stdout;
	setbuf(tmFile, (char *)0);
#endif
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
QU_Head *
TM_getHead(void)
{
    return ((QU_Head *)&activeSeq);
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
TM_config(Char *tmFileName)
{
    FILE *fp;

    /* if there is an existing tmFile, close it first */
    if ( tmFile ) {
	fclose(tmFile);
	tmFile = NULL;
    }

    if ( (fp = fopen(tmFileName, "w")) ) {
	tmFile = fp;
	setbuf(tmFile, (char *)0); /* unbuffered */
	return ( SUCCESS );
    }

    return ( FAIL );
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
PUBLIC tm_ModInfo *
TM_open(Char *moduleName)
{
    tm_ModInfo *quIndex;
    tm_ModInfo *newModule;

    newModule = (tm_ModInfo *)0;
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
	     * than one TM_open.
	     */
	    EH_problem("TM_open: Duplicate module...");
	    EH_problem(moduleName);
	    return ( quIndex );
	}
    }

    /* So It needs to be allocated and added to the Queue */
    if ( ! (newModule = (tm_ModInfo *) SEQ_elemObtain(activePool)) ) {
	EH_problem("TM_open: Not enough memory for new module");
	return ( (tm_ModInfo *) 0 );
    }
    (Void) strcpy(newModule->moduleName, moduleName);
    newModule->mask = TM_NOTRACE;
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
	    newModule->mask = quIndex->mask;
	    QU_REMOVE(quIndex);
	    SEQ_elemRelease(setUpPool, quIndex);
	    break;
	}
    }

    return ( newModule );
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
PUBLIC SuccFail
TM_close(Char *moduleName)
{
#if defined(OS_VARIANT_WinCE)
    if ( FALSE == CloseHandle((HANDLE)tmFile) )
	return Fail;
#endif
    return Success;

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
PUBLIC SuccFail
TM_setMask(Char *moduleName, TM_Mask moduleMask)
{
    tm_ModInfo *quIndex;

    /* Cherck to see if it already has been opened, if
     * so just set the tracing mask.
     */
    for ( quIndex = QU_FIRST(&activeSeq);
	  ! QU_EQUAL(quIndex, &activeSeq);
	  quIndex = QU_NEXT(quIndex)) {
	if ( ! strcmp(quIndex->moduleName, moduleName) ) {
	    quIndex->mask = moduleMask;
	    return ( SUCCESS );
	}
    }

    /* So It needs to be allocated and added to the Queue */
    if ( ! (quIndex = (tm_ModInfo *) SEQ_elemObtain(setUpPool)) ) {
	EH_problem("");
	return ( FAIL );
    }
    (Void) strcpy(quIndex->moduleName, moduleName);
    quIndex->mask = moduleMask;
    QU_INIT(quIndex);
    QU_INSERT(&setUpSeq, quIndex);

    return ( SUCCESS );
}
    

/*<
 * Function: tm_trace
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
tm_trace(tm_ModInfo *modInfo, TM_Mask mask, Char *format, ...)
{
    va_list ap;
#if defined(OS_VARIANT_WinCE)
    static char msgBuf[TM_WINCE_LINE_MAX_CHARS];
#elif defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
    static char msgBuf[128]; /* Careful! No stack hog under MSDOS */
#else
    char    msgBuf[1024];
#endif

    if (tmFile == NULL) {
        return SUCCESS;
    }

    /* Check the mask bits before printing. This logic allows 
       printing of trace messages by EH_ functions as well as
       by TM_TRACE */
    if ( !(mask && modInfo && !(modInfo->mask & mask) ) ) {
	va_start (ap, format);
	OS_timestamp (msgBuf, sizeof msgBuf, mask, 
		      modInfo ? modInfo->moduleName : "null", tm_loc);
	fputs (msgBuf, tmFile);

	/* strip leading whitespace */
	while (*format == '\n' || *format == ' ' || *format == '\t') {
	    format++;
	}

#if defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Dos)
	_vsnprintf (msgBuf, (sizeof(msgBuf)-4), format, ap);
#else
	vsprintf (msgBuf, format, ap);
#endif

	/* force a new line */
	if (msgBuf[strlen(msgBuf) - 1] != '\n') {  /* \n OK for DOS & UNIX (but not Macs?) */
	    strcat (msgBuf, OS_EOL_CHARS);
	}
	fputs (msgBuf, tmFile);

	va_end (ap);
	return ( SUCCESS );

    } else {
      return ( FAIL );
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
 * 
>*/
PUBLIC Bool
TM_query(tm_ModInfo *modInfo, TM_Mask mask)
{
    if ( modInfo->mask & mask ) {
	return ( TRUE );
    } else {
	return ( FALSE );
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
 * 
>*/
PUBLIC SuccFail
TM_validate(void)
{
    tm_ModInfo *quIndex;
    tm_ModInfo *tmpQuIndex;
    tm_ModInfo *tmpQuNext;
    Char *moduleName;
    Bool retVal;

    if (tmFile == NULL) {
        return SUCCESS;
    }

    retVal = SUCCESS;

    tmpQuIndex = QU_FIRST(&setUpSeq);
    while (! QU_EQUAL(tmpQuIndex, &setUpSeq)) {
	Bool validModuleFlag;

	validModuleFlag = FALSE;
	tmpQuNext = tmpQuIndex->next;
	moduleName = tmpQuIndex->moduleName;

	for ( quIndex = QU_FIRST(&activeSeq);
	      ! QU_EQUAL(quIndex, &activeSeq);
	      quIndex = QU_NEXT(quIndex)) {
	    if ( ! strcmp(quIndex->moduleName, moduleName) ) {
		quIndex->mask = tmpQuIndex->mask;
		validModuleFlag = TRUE;
		break;
	    } 
	}
	if (! validModuleFlag) {
	    fprintf(tmFile, 
	    	    "TM_validate: %s, not a valid Module Name\n", moduleName);
	    retVal = FAIL;
	}

	/* Free Up the Element */
	QU_REMOVE(tmpQuIndex);
	SEQ_elemRelease(setUpPool, tmpQuIndex);

	tmpQuIndex = tmpQuNext;
    }

    return ( retVal );
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
tm_here(Char *fileName, Int lineNu)
{
    static Char buff[OS_MAX_FILENAME_LEN + 16];
#if defined(OS_VARIANT_WinCE)
    Char *pos = strrchr(fileName, OS_DIR_PATH_SEPARATOR[0]);

    (Void) sprintf(buff, "%s:%d", pos?++pos:fileName, lineNu);
#else
    (Void) sprintf(buff, "%s:%d", fileName, lineNu);
#endif
    return ( buff );
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
PUBLIC Int
TM_setUp(Char * str)
{
    Char TM_modName[10];
    Int TM_modMask;
    Char * charPtr1;
    Char * charPtr2;

    charPtr2 = TM_modName;
    for (charPtr1=str; *charPtr1; ++charPtr1) {
	if ( charPtr2 <= ENDOF(TM_modName) ) {
	    if ( *charPtr1 != ',' ) {
		*charPtr2++ = *charPtr1;
	    } else {
		*charPtr2 = '\0';
		break;
	    }
	} else {
	    EH_problem("moduleName too long");
	    /* Make sure that *charPtr1 is not ',' */
	    *charPtr1 = '\0';
	    break;
	}
    }
    if ( *charPtr1 != ',' ) {
	return ( FAIL );
    } 
    if ( PF_getUns(charPtr1+1, &TM_modMask, 0x0, 0x0, ~(0x0)) ) {
	EH_problem("mask out of range");
	return ( FAIL );
    } else {
	if ( TM_setMask(TM_modName, (TM_Mask) TM_modMask) != SUCCESS) {
#if 0
	    fprintf(stderr, "%s %s is not a valid module", HERE(), str);
#endif
	    return ( FAIL );
	}
    }
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
>*/
PUBLIC String
TM_prAddr(Ptr p)
{
    static char	    buf[20];

    HW_prAddr(p, buf);
    return buf;
}


static Void
printisc (FILE *outFile, Int *chars, Int length)
{
    Int i;

    if (outFile == NULL) {
        return;
    }

    for (i=0; i<length; ++i) {
#if defined(OS_VARIANT_WinCE)
      wsprintf(tm_line, TEXT("."));
      WriteFile(outFile, tm_line, sizeof(WCHAR)*lstrlen(tm_line),
		&tm_dwBytesWritten, NULL);
#else  /* not WinCE */
        if (isprint (chars[i])) {
	    fprintf (outFile, "%c", chars[i]);
        } else {
	    fprintf (outFile, ".");
        }
#endif /* OS_VARIANT_WinCE */
    }
}

/*<
 * Function:    TM_hexDump
 *
 * Description: Hex dump
 *
 * Arguments:   Module, mask, string, address, length
 *
 * Returns:
 *
>*/

PUBLIC LgInt
TM_hexDump(tm_ModInfo *modInfo, TM_Mask mask, String str, 
  	   unsigned char *address, Int length)
{
#define BYTESPERLINE 16
    unsigned char *lastAddr;

    LgInt i = 0L;
    Int c;
    Int chars[BYTESPERLINE];
    Int j=0;
    Int k;

    if ( tmFile == NULL || !(modInfo->mask & mask) ) {
	/* Then we do NOT need to do any thing */
	return 0;
    } 

    for (i = 0,  c = *address, lastAddr = address + length; 
	 address < lastAddr; 
	 c = *++address) {

        if (! (i % BYTESPERLINE)) {  
                j=0;  
		fprintf(tmFile, "\n%05ld:", i);
        }
        if (!(i++ & 1)) {
	    fprintf (tmFile, " ");
        }
        fprintf (tmFile, "%02x", c);
        chars[j++] = c;

	if (! (i % BYTESPERLINE)) {
	    fprintf (tmFile, " ");
	    printisc (tmFile, chars, BYTESPERLINE);
	}
    }
    if ( (k = (i % BYTESPERLINE)) ) {
        Int jj;
        k = BYTESPERLINE - k;
        for (jj=0; jj < (5*k/2)  ;++jj) {
	    fprintf (tmFile, " ");
        }
    }
    if ( i ) {
      fprintf (tmFile, " ");
      printisc (tmFile, chars, (Int) (i % BYTESPERLINE));
      fprintf (tmFile, "\n");
    }

    return i;

} /* TM_hexDump() */


/*<
 * Function:    TM_call
 *
 * Description: Call a user-supplied function
 *
 * Arguments:   Module, mask, function pointer, two parameters to function
 *
 * Returns:
 *
>*/

PUBLIC Void
TM_call(tm_ModInfo *modInfo,
	TM_Mask mask,
	void (* pfCallMe)(void * hParam1, void * hParam2),
	void * hParam1,
	void * hParam2)
{
    if ( !(modInfo->mask & mask) ) {
	/* Then we do NOT need to any thing */
	return;
    } 

    (* pfCallMe)(hParam1, hParam2);
}
#endif
