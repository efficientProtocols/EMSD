/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of Neda-EMSD. An implementation of RFC-2524.
 * 
 * Neda-EMSD is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Neda-EMSD; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/* -*- Mode: c++ -*- */
/*---------------------------------------------------------------------------*\
 *
 * (c) Copyright Microsoft Corp. 1996 All Rights Reserved
 *
 *  module: service.c
 *
 *  purpose: This file contains the API routines and DllMain for the
 *           EMSD mail transport service library
 *
\*---------------------------------------------------------------------------*/
#include <windows.h>

#include <msgstore.h>

#include <tm.h>
#include <eh.h>
#include <config.h>
#include <os.h>

#include "transprt.h"
#include "emsd_mtsp.h"
#include "resource.h"	

static Void
MTSP_init();

#if defined TM_ENABLED
TM_ModDesc MTSP_tmDesc;
#endif

static BOOL   
StartUaShellDaemon(void); 

static BOOL
AsciiWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
	       LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

WCHAR		g_Line[256];	// buffer for trace output
DWORD		g_dwBytesWritten; // number of bytes written to trace output

HINSTANCE       g_hinst         = NULL;     // instance of library
HANDLE          ghLibHeap       = NULL;     // global heap used for libary
					    // Each message has a heap which
					    // is used for data relating to
					    // that message.  This is used
					    // for variables related to the
					    // library itself.

#define EMSD_SPOOL_SCRATCH_MBOX		"\\EMSD\\spool\\#mbox#" // for compacting

#define EMSD_MAX_CHARS_PER_LINE	256

// empty string to prevent access to null pointer
#define NULL_STR        TEXT("")

/*****************************************************************************/
/* Config file (emsd_mtsp.ini) parameters                                     */
/*****************************************************************************/
static struct
{
    char	*pLockFile;
    char	*pGrabLockMaxRetries;
    char	*pGrabLockRetryInterval;
    char	*pLicenseFile;
    char	*pNewMessageDirectory;
    char	*pMailBox;
    char	*pReturnAddress;
} config = { NULL };


static struct ConfigParams
{
    char *	    pSectionName;
    char *	    pTypeName;
    char **	    ppValue;
} configParams[] = {
    {
	"Internals",
	"Lock File",
	&config.pLockFile
    },
    {
	"Internals",
	"Grab Lock Max Retries",
	&config.pGrabLockMaxRetries
    },
    {
	"Internals",
	"Grab Lock Retry Interval",
	&config.pGrabLockRetryInterval
    },
    {
	"License",
	"License File",
	&config.pLicenseFile
    },
    {
	"Submission",
	"New Message Directory",
	&config.pNewMessageDirectory
    },
    {
	"Delivery",
	"MailBox",
	&config.pMailBox
    },
    {
	"General",
	"Return Address",
	&config.pReturnAddress
    }

};

/*****************************************************************************/
/* Initialize transport                                                      */
/*****************************************************************************/

// initialize transport.
//      - creates handle
//      - pulls logon information from registry
//      - starts ras if necessary
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportInit(HANDLE *NewService,         // Pointer to hold new service
	      LPWSTR  szCommonKey,        // Top level registry key
	      LPWSTR  szTransportKey)     // This transport's reg key
{
    LPSERVICE hService;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportInit() called."));

    // make sure we received a valid pointer
    if (!NewService)        return FALSE;

    // make sure handle that was passed in is NULL in case
    // of an error
    *NewService=NULL;

    // try to allocate the service handle and return an error condition
    // if it fails.  ERR() is not used since there is no handle in which
    // to store the error line and error code.
    if ((hService = HeapAlloc(ghLibHeap, 0, sizeof(SERVICE)))==NULL) {
	return FALSE;
    }

    // set the new service handle and initialize it
    *NewService = hService;
    hService->iErr = TRANS_ERR_NONE;

    return TRUE;
}

/*****************************************************************************/
/* Connect to transport                                                      */
/*****************************************************************************/

// Connect to a service for transmission/receiving
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportConnect(HANDLE  InPtr,	// Service
		 DWORD   dwFlags, // Transmit/Receive Flags
				// Values specified above
		 HWND    hWndNotify,// This is the window that will
				// receive any notification messages
				// we choose to send
		 BOOL    (*CallBack)(int,BOOL))// Callback routine.  If this
				// routine is not NULL, it
				// will be called periodically
				// and given an integer between
				// 0-99 indicating the progress
				// towards completion.
				// A return value of FALSE
				// from this routine will cause
				// the current action to be
				// cancelled.  (This is currently
				// not implemented).
				// The second param should be TRUE if
				// the MAX value could be >99 (unknown max)
{
    LPSERVICE hService=(LPSERVICE)InPtr;
    int	idx;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportConnect() called."));

    // make a UNICODE version of mailbox file
    wsprintf(hService->szMailboxFileName, TEXT("%S"), config.pMailBox);

    // fill in the locking info
    wsprintf(hService->szLockFile, TEXT("%S"), config.pLockFile);
    hService->iGrabLockMaxRetries = atol(config.pGrabLockMaxRetries);
    hService->iGrabLockRetryInterval = atol(config.pGrabLockRetryInterval);

    // record system time for use in filenames
    GetLocalTime(&hService->connLocalTime);
    hService->msgCount = 0;

    // initialize Uids and DeathRow
    for (idx = 0; idx < MAX_MSGS; idx++) {
	lstrcpy((hService->Uids)[idx], TEXT(""));
	hService->fDeathRow[idx] = FALSE;
    }

    // verify that the handle exists
    if (!hService)      return FALSE;
    hService->iErr = TRANS_ERR_NONE;

    // store passed in values
    hService->CallBack   = CallBack;

    hService->fRecActive    = (dwFlags & TRANSPORT_RECV) ? TRUE: FALSE;
    hService->fTransActive  = (dwFlags & TRANSPORT_SEND) ? TRUE: FALSE;

    hService->hWndNotify	= hWndNotify;

    // maybe startup the UA shell process
    {
	static	BOOL	startedBefore = FALSE;

	if ( startedBefore == FALSE ) {

	    HANDLE		hTickFile;
	    FILETIME 	lastAccessFileTime;
	    SYSTEMTIME 	lastAccessSystemTime;
	    FILETIME	nowFileTime;
	    SYSTEMTIME	nowSystemTime;
	    BOOL		res;
	    unsigned long	secsSinceHeartBeat;

	    hTickFile = CreateFile(TEXT("\\EMSD\\uashwce_one.tick"),
				   GENERIC_READ,
				   FILE_SHARE_WRITE,
				   NULL, 
				   OPEN_EXISTING,
				   FILE_ATTRIBUTE_NORMAL,
				   NULL);

	    if ( hTickFile == INVALID_HANDLE_VALUE ) {
		// tick file doesn't exist ==> UA daemon not running
		TM_TRACE((MTSP_tmDesc, TM_DEBUG, "No tick file, starting uash daemon..."));
		startedBefore = StartUaShellDaemon();
		if ( startedBefore == FALSE ) return FALSE;
		goto doTransportCount;
	    }

	    res = GetFileTime(hTickFile, NULL, &lastAccessFileTime, NULL);
	    if ( res = FALSE ) {
		TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Error getting FILETIME of tick file!"));
		return FALSE;
	    }
	
	    CloseHandle(hTickFile);

	    FileTimeToSystemTime(&lastAccessFileTime, &lastAccessSystemTime);
	    GetSystemTime(&nowSystemTime);
	    SystemTimeToFileTime(&nowSystemTime, &nowFileTime);
	    secsSinceHeartBeat = (nowFileTime.dwLowDateTime - lastAccessFileTime.dwLowDateTime) / 10000000;

	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "seconds since last heartbeat: %lu", secsSinceHeartBeat));

	    if ( secsSinceHeartBeat > 15 ) {
		startedBefore = StartUaShellDaemon();
	    }
	}
    }

doTransportCount:

    // send a WM_COPYDATA to tell the uash to issue a delivery control	
    {
	static COPYDATASTRUCT cds;
	LRESULT res;

	res = SendMessage(HWND_BROADCAST, WM_COPYDATA, (WPARAM)hWndNotify, (LPARAM)&cds);
	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "SendMessage() of WM_COPYDATA returned <%ld>.", res));
    }

    // get count of messages
    return TransportCount(InPtr, NULL);
}

/*****************************************************************************/
/* Return last error                                                         */
/*****************************************************************************/

// Get last mail error as integer
int
TransportError(    HANDLE InPtr)               // Service
{
    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportError() called."));

    return (InPtr) ? ((LPSERVICE)InPtr)->iErr :
		TRANS_ERR_HANDLE_NOT_ALLOC;
}

/*****************************************************************************/
/* Return text string from last error                                        */
/*****************************************************************************/

// initialize transport.
//      - creates handle
//      - pulls logon information from registry
//      - starts ras if necessary
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
int
TransportErrorMsg(                  // Get last mail error (printable)
    HANDLE  InPtr,                      // Service
    LPWSTR  szBuf,                      // Buffer to fill
    int     iBufLen,                    // Length of buffer in chars
    int*    piSrcLine)                  // Line where error occured (or NULL)
{
    LPSERVICE hService=(LPSERVICE)InPtr;
    int Err;
    int ErrLine;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportErrorMsg() called."));

    // if the handle is non NULL, the error line and code should
    // be stored there
    if (hService) {
	Err = hService->iErr;
	ErrLine = hService->iErrLine;
    }
    else {
	// otherwise, there was an error in the allocation of the
	// handle
	Err = TRANS_ERR_HANDLE_NOT_ALLOC;
	ErrLine = 0;
    }

    // load the error message from the resource string
    if (!LoadString(g_hinst,Err,szBuf,iBufLen))
	wsprintf(szBuf,TEXT("Unknown error"));

    // set the error line in the source
    if (piSrcLine)  *piSrcLine   = ErrLine;

    return Err;
}

/*****************************************************************************/
/* disconnect from services                                                  */
/*****************************************************************************/

// Disconnect from current connection
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportDisconnect(HANDLE InPtr) // service handle
{
    int		idx;
    int		nDel;		// number of delete requests
    LPSERVICE 	hService = (LPSERVICE)InPtr;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportDisconnect() called."));


    // any deleted messages?
    for (nDel = 0, idx = 0; idx < hService->wNumMsgs; idx++) {
	if ( hService->fDeathRow[idx] == TRUE ) {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Message %d was marked for deletion.", idx));
	    nDel++;
	}
    }

    // need to compact the mailbox
    if ( nDel ) {
	int	msgCount;
	Bool	fSkip;
	HANDLE	hLockFile;
	HANDLE	hNewMbox;
	HANDLE	hOldMbox;
	char	aLine[1024];
	char	*paLine;
	int	retries = hService->iGrabLockMaxRetries;


	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Compacting mailbox..."));

	// grab the lock
	while ( INVALID_HANDLE_VALUE
		== (hLockFile = CreateFile(hService->szLockFile,
					   GENERIC_READ,
					   0,	// no sharing
					   NULL,
					   OPEN_ALWAYS,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL)) ) {
	    if ( retries-- > 0 ) {
		TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Could not lock shared resource--retrying..."));
		// MessageBeep(MB_OK); 
		Sleep((DWORD)hService->iGrabLockRetryInterval);
	    }
	    else {
		TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Could not lock shared resource--giving up!"));
		return FALSE;
	    }
	}
	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Lock acquired."));

	// make a copy the mbox file (for use in compaction)
	if ( CopyFile(hService->szMailboxFileName, TEXT(EMSD_SPOOL_SCRATCH_MBOX), FALSE) == FALSE ) {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Error compacting mailbox (CopyFile failed) ."));
	    return FALSE;
	}

	// create a new mbox file
	hNewMbox = CreateFile(hService->szMailboxFileName,
			      GENERIC_WRITE,
			      0, // no sharing while creating new mbox
			      NULL,
			      CREATE_ALWAYS, // overwrite existing
			      FILE_ATTRIBUTE_NORMAL,
			      NULL);

	if ( hNewMbox == INVALID_HANDLE_VALUE ) {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Serious error! Error creating new mailbox, mail may be lost."));
	    CloseHandle(hLockFile);
	    // try to restore the mbox from the copy
	    MoveFile(TEXT(EMSD_SPOOL_SCRATCH_MBOX), hService->szMailboxFileName);
	    return FALSE;
	}

	// open the mbox copy
	hOldMbox = CreateFile(TEXT(EMSD_SPOOL_SCRATCH_MBOX),
			      GENERIC_READ,
			      0,
			      NULL,
			      OPEN_EXISTING,
			      FILE_ATTRIBUTE_NORMAL,
			      NULL);
	if ( hOldMbox == INVALID_HANDLE_VALUE ) {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Serious error! Error reading old mailbox, mail may be lost."));
	    CloseHandle(hLockFile);
	    return FALSE;
	}

	msgCount = 0;
	fSkip = FALSE;
	while ( paLine = fgets(aLine, sizeof(aLine), hOldMbox) ) {
	    // a message begins with a line "From <sender> <date>"
	    if ((strlen(aLine) > 5) && (strncmp(aLine, "From ", 5) == 0)) {
		fSkip = hService->fDeathRow[msgCount];
		msgCount++;
	    }

	    if ( !fSkip ) {
		fputs(aLine, hNewMbox);
		fputs("\n", hNewMbox);
	    }
	}

	CloseHandle(hOldMbox);
	DeleteFile(TEXT(EMSD_SPOOL_SCRATCH_MBOX));

	CloseHandle(hNewMbox);
	CloseHandle(hLockFile);

	// reset wNumMsgs and fDeathRow
	for (idx = 0; idx < MAX_MSGS; idx++) hService->fDeathRow[idx] = FALSE;
	hService->wNumMsgs -= nDel;
    }

    return TRUE;
}

/*****************************************************************************/
/* free up all memory allocated for transport                                */
/*****************************************************************************/

BOOL
TransportRelease(
    HANDLE InPtr)           // service handle
{
    LPSERVICE hService = (LPSERVICE)InPtr;
    BOOL RetCode=TRUE;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportRelease() called."));

    // if service is out there, free it.
    if (hService) {
	hService->iErr = TRANS_ERR_NONE;
	RetCode=TransportDisconnect(InPtr);
	HeapFree(ghLibHeap,0,hService);
    }
    return RetCode;
}

/*****************************************************************************/
/* send message out thru server                                              */
/*****************************************************************************/

// send specified message
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportSend(HANDLE InPtr,               // Service
	      MailMsg *MsgPtr)            // Message to send
{
    LPSERVICE 	hService = (LPSERVICE)InPtr;
    char	aLine[EMSD_MAX_CHARS_PER_LINE];	
    WCHAR 	wLine[EMSD_MAX_CHARS_PER_LINE];
    LPWSTR 	wLineStart;
    LPWSTR	wLineEnd;
    LPWSTR 	Data;
    int 	SentSoFar;
    int 	TotalSize;
    WCHAR 	szFileName[2*MAX_DIR_SIZE];
    int		retries = hService->iGrabLockMaxRetries;
    HANDLE  	hLockFile;
    HANDLE  	hFile;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportSend() called."));

    // verify that the handle exists
    if (!hService)      return FALSE;
    hService->iErr = TRANS_ERR_NONE;

    // generate filename
    wsprintf(szFileName, TEXT("%S\\S_%02d%02d%02d%02d%02d_%02d"),
	     config.pNewMessageDirectory,
	     hService->connLocalTime.wMonth,
	     hService->connLocalTime.wDay,
	     hService->connLocalTime.wHour,
	     hService->connLocalTime.wMinute,
	     hService->connLocalTime.wSecond,
	     ++(hService->msgCount));

    wcstombs(aLine, szFileName, sizeof(aLine));
    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Creating message file <%s>", aLine));

    // grab the lock
    while ( INVALID_HANDLE_VALUE
	    == (hLockFile = CreateFile(hService->szLockFile,
				       GENERIC_READ,
				       0,	// no sharing
				       NULL,
				       OPEN_ALWAYS,
				       FILE_ATTRIBUTE_NORMAL,
				       NULL)) ) {
	if ( retries-- > 0 ) {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Could not lock shared resource--retrying..."));
	    // MessageBeep(MB_OK); 
	    Sleep((DWORD)hService->iGrabLockRetryInterval);
	}
	else {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Could not lock shared resource--giving up!"));
	    return FALSE;
	}
    }
    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Lock acquired."));

    // now, open file
    hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL,
		       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile==INVALID_HANDLE_VALUE)    ERR(CANT_SEND);

    // calculate sizes to for callback routine
    if (hService->CallBack) {
	TotalSize = lstrlen(MsgPtr->szBody);
	SentSoFar = 0;
    }

    // write to, from, cc, subject each as a separate line.
    // ignore all other headers
    Data=MailGetField(MsgPtr, TEXT("To"), FALSE);
    if (!Data)      Data= NULL_STR;
    wsprintf(wLine, TEXT("To: %s\r\n"), Data);
    AsciiWriteFile(hFile, wLine, sizeof(WCHAR)*lstrlen(wLine),
	      &g_dwBytesWritten, NULL);

    wsprintf(wLine, TEXT("From: %S\r\n"), config.pReturnAddress);
    AsciiWriteFile(hFile, wLine, sizeof(WCHAR)*lstrlen(wLine),
	      &g_dwBytesWritten, NULL);

    Data=MailGetField(MsgPtr, TEXT("Cc"), FALSE);
    if (Data) {
	wsprintf(wLine, TEXT("Cc: %s\r\n"), Data);
	AsciiWriteFile(hFile, wLine, sizeof(WCHAR)*lstrlen(wLine),
		       &g_dwBytesWritten, NULL);
    }

    Data=MailGetField(MsgPtr, TEXT("Date"), FALSE);
    if (Data) {
	wsprintf(wLine, TEXT("Date: %s\r\n"), Data);
	AsciiWriteFile(hFile, wLine, sizeof(WCHAR)*lstrlen(wLine),
		       &g_dwBytesWritten, NULL);
    }

    Data=MailGetField(MsgPtr, TEXT("Subject"), FALSE);
    if (Data) {
	wsprintf(wLine, TEXT("Subject: %s\r\n"), Data);
	AsciiWriteFile(hFile, wLine, sizeof(WCHAR)*lstrlen(wLine),
		       &g_dwBytesWritten, NULL);
    }

    // end the message header
    wsprintf(wLine, TEXT(OS_EOL_CHARS));
    AsciiWriteFile(hFile, wLine, sizeof(WCHAR)*lstrlen(wLine),&g_dwBytesWritten, NULL);

    // now go thru message line by line
    // this is only necessary so we can call progress Callback routine
    wLineStart = MsgPtr->szBody;
    while ((wLineStart) && (*wLineStart)) {
	if (wLineEnd = wcschr(wLineStart, TEXT('\r'))) {

	    // we found \r
	    // terminate string and send
	    // NOTE:  We're assuming the msgbody is writable.
	    //        if it is not, we'd need to copy each line into
	    //        a temp string...
	    *wLineEnd = TEXT('\0');
	}

	// now write line to file
	AsciiWriteFile(hFile, wLineStart, sizeof(WCHAR)*lstrlen(wLineStart),
		  &g_dwBytesWritten, NULL);
	AsciiWriteFile(hFile, TEXT("\r\n"), 4, &g_dwBytesWritten, NULL);

	// call callback
	if (hService->CallBack && wLineEnd) {
	    SentSoFar += wLineEnd-wLineStart + 2;
	    (*(hService->CallBack))(100*SentSoFar/TotalSize, FALSE);
	}

	if (wLineEnd) {
	    // put \r back into msg (so we don't change original)
	    // is this really necessary??
	    *wLineEnd = TEXT('\r');
	    wLineEnd += 2;
	}
	// we'll set wLineStart to NULL or start of line
	wLineStart = wLineEnd;
    }

    CloseHandle(hFile);
    CloseHandle(hLockFile);

    return TRUE;
}
/*****************************************************************************/
/* Get the number of messages waiting in the mailbox                         */
/*****************************************************************************/

BOOL
TransportCount(
    HANDLE InPtr,           // Service Pointer
    LPWORD lpNumMsgs)       // pointer to receive number of messages
{
    LPSERVICE           hService = (LPSERVICE)InPtr;
    int			retries = hService->iGrabLockMaxRetries;
    WORD                wCount;
    HANDLE		hFile;
    HANDLE		hLockFile;
    char		aLine[1024];
    char		*paLine;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportCount() called."));

    // verify handle exists
    if (!hService)      return FALSE;
    hService->iErr = TRANS_ERR_NONE;

    // grab the lock
    while ( INVALID_HANDLE_VALUE
	    == (hLockFile = CreateFile(hService->szLockFile,
				       GENERIC_READ,
				       0,	// no sharing
				       NULL,
				       OPEN_ALWAYS,
				       FILE_ATTRIBUTE_NORMAL,
				       NULL)) ) {
	if ( retries-- > 0 ) {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Could not lock shared resource--retrying..."));
	    // MessageBeep(MB_OK); 
	    Sleep((DWORD)hService->iGrabLockRetryInterval);
	}
	else {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Could not lock shared resource--giving up!"));
	    return FALSE;
	}
    }
    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Lock acquired."));

    // got the lock, now access the mbox file

    if ( INVALID_HANDLE_VALUE
	 == (hFile = CreateFile(hService->szMailboxFileName,
				GENERIC_READ,
				0,	// no sharing
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL)) ) {
	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Shouldn't happen!  Could not open mbox <%s> for reading!", config.pMailBox));
	return FALSE;
    }

    // count number of messages in mailbox file
    // denoted by lines beginning with "From "

    wCount = 0;		// initialize
    while ( paLine = fgets(aLine, sizeof(aLine), hFile) ) {
	// a message begins with a line "From <sender> <date>"
	if ((strlen(aLine) > 5) && (strncmp(aLine, "From ", 5) == 0)) {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Found Msg [%s]!", aLine));
	    wsprintf((hService->Uids)[wCount], TEXT("EMSD %d"), wCount);
	    wCount++;
	}
    }

    // done with mbox file
    CloseHandle(hFile);

    // release lock
    CloseHandle(hLockFile);

    // save counts
    hService->wNumMsgs = wCount;

    // if a pointer was passed in, set it
    if (lpNumMsgs)      *lpNumMsgs = wCount;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportCount(): found %d messages.", hService->wNumMsgs));
    return TRUE;
}

/*****************************************************************************/
/* Get a particular message from server                                      */
/*****************************************************************************/

BOOL
TransportRecv(HANDLE InPtr,	// Service
	      WORD wSessionId,	// Msg Id to read.  Should
				// be in range 1 to NumMsgs.
				// It is assumed that this id will never
				// change while we are connected to the
				// service. (during the session)
				// If this value is 0, it is
				// assumed that the Unique
				// Message Id is specified
				// in msg
	      short sNumLines,	// Number of lines to receive
				// positive value of lines to read
				// or see #defines in .h file
	      MailMsg *MsgPtr,	// Mail Message that will contain
				// Message.  Memory is allocated
				// in here that needs to be
				// freed by TransportFreeMsg()
	      HANDLE Reserved)	// this parameter is not currently used
				// and should be forced to NULL
{
    LPSERVICE 	hService = (LPSERVICE)InPtr;
    int		retries = hService->iGrabLockMaxRetries;
    HANDLE 	hLockFile;
    HANDLE 	hFile;
    MSGXTRA 	Xtra;
    DWORD   	dwNumRead;
    DWORD   	dwMboxMsgLen;	// length of message in mbox file (ascii with \n-style EOL)
    LPWSTR  	szMsgTxt;
    LPSTR   	szAsciiMsgTxtRaw;
    LPSTR   	szAsciiMsgTxt;
    int		count;
    char	aLine[1024];
    char	*paLine;
    int		msgStart;
    int		msgEnd;
    int		prevMsgEnd;
    int		currLineStart;
    int		nextLineStart;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportRecv() called with wSessionId %d, sNumLines %d.",
	      wSessionId, sNumLines));

    if (!hService)      return FALSE;
    hService->iErr = TRANS_ERR_NONE;

    // if wSessionId is 0, then the UID is specified in the msg
    // structure.
    if (wSessionId == 0) {
	wcstombs(aLine, MsgPtr->szSvcId, sizeof(aLine));
	TM_TRACE((MTSP_tmDesc, TM_ENTER, "Looking for message matching service id <%s>.", aLine));
	for ( ; wSessionId < hService->wNumMsgs; wSessionId++) {
	    wcstombs(aLine, (hService->Uids)[wSessionId], sizeof(aLine));
	    TM_TRACE((MTSP_tmDesc, TM_ENTER, "testing <%s> ...", aLine));
	    if (!lstrcmpi((hService->Uids)[wSessionId], MsgPtr->szSvcId)) {
		TM_TRACE((MTSP_tmDesc, TM_ENTER, "Found!", aLine));
		break;
	    }
	}
	// now increment, since table is 0 based, but message list
	// starts at 1
	wSessionId++;
    }

    if ((wSessionId < 1) || (wSessionId>hService->wNumMsgs)) ERR(INV_MSG_ID);

    // if we're just looking for id, we found it
    if (sNumLines==TRANSPORT_MSG_EXIST) {
	MsgPtr->szSvcId = (hService->Uids)[wSessionId-1];
	MsgPtr->hHeap = NULL;
	return TRUE;
    }

    // grab the lock
    while ( INVALID_HANDLE_VALUE
	    == (hLockFile = CreateFile(hService->szLockFile,
				       GENERIC_READ,
				       0,	// no sharing
				       NULL,
				       OPEN_ALWAYS,
				       FILE_ATTRIBUTE_NORMAL,
				       NULL)) ) {
	if ( retries-- > 0 ) {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Could not lock shared resource--retrying..."));
	    // MessageBeep(MB_OK); 
	    Sleep((DWORD)hService->iGrabLockRetryInterval);
	}
	else {
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Could not lock shared resource--giving up!"));
	    ERR(CANT_OPEN);
	}
    }
    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Lock acquired."));
    
    // now try to open mailbox file
    if (INVALID_HANDLE_VALUE
	== (hFile = CreateFile(hService->szMailboxFileName,
			       GENERIC_READ,
			       0,
			       NULL,
			       OPEN_EXISTING,  
			       FILE_ATTRIBUTE_NORMAL, 
			       NULL))) {
	CloseHandle(hLockFile);
	ERR(CANT_OPEN);
    }

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "Looking for msg# %d in %s.",
	      wSessionId, config.pMailBox));

    // search for the message requested
    msgStart = INT_MAX;
    msgEnd = 0;
    count = 0;
    currLineStart = 0;
    nextLineStart = 0;
    while (( paLine = fgets(aLine, sizeof(aLine), hFile)) && ( wSessionId >= count )) {

	currLineStart = nextLineStart;
	nextLineStart = ftell(hFile);

	// a message begins with a line "From <sender> <date>"
	if ((strlen(aLine) > 5) && (strncmp(aLine, "From ", 5) == 0)) {
	    count++;
	    prevMsgEnd = currLineStart - 1;

	    if ( count == wSessionId ) {
		msgStart = nextLineStart;
	    }
	    else if ( count == wSessionId + 1 ) {
		msgEnd = prevMsgEnd;
	    }
	}
    }

    if ( msgEnd == 0 ) {
	// wSessionId must have been the last message, so set
	// msgEnd to the end of file
	msgEnd = ftell(hFile); // set to end of file
	TM_TRACE((MTSP_tmDesc, TM_ENTER, "msgEnd == 0, setting it to be %d.", msgEnd));
    }

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "Found msg# %d. start: %d, end: %d",
	      wSessionId, msgStart, msgEnd));

    dwMboxMsgLen = msgEnd - msgStart;

    // create msg's heap
    if ((MsgPtr->hHeap = HeapCreate(0,2048,0))==NULL) {
	CloseHandle(hFile);
	CloseHandle(hLockFile);
	ERR(NO_MSG_BUF);
    }

    // allocate buffer for entire message.
    szAsciiMsgTxtRaw=(LPSTR)HeapAlloc(MsgPtr->hHeap, 0, sizeof(CHAR)*(dwMboxMsgLen+1));
    if (!szAsciiMsgTxtRaw) {
	TM_TRACE((MTSP_tmDesc, TM_ENTER, "Out of heap space!"));
	CloseHandle(hFile);
	CloseHandle(hLockFile);
	ERR(NO_MSG_BUF);
    }

    // now read entire message
    fseek(hFile, msgStart, SEEK_SET);
    if (!ReadFile(hFile, szAsciiMsgTxtRaw, dwMboxMsgLen, &dwNumRead, NULL)) {
	HeapFree(MsgPtr->hHeap, 0, szAsciiMsgTxtRaw);
	CloseHandle(hFile);
	CloseHandle(hLockFile);
	ERR(NO_MSG_BUF);
    }

    CloseHandle(hFile);
    CloseHandle(hLockFile);
    szAsciiMsgTxtRaw[dwMboxMsgLen] = '\0'; // null terminate

    // generate UNICODE version of message
    {
	char	*curr;
	char	*curr1;
	int	lfCount;
#define	MSG_HEAD_SIZE 255
	char	msgHead[MSG_HEAD_SIZE+1];
#define	MSG_TAIL_SIZE 255
	char	msgTail[MSG_TAIL_SIZE+1];

	for (lfCount = 0, curr = szAsciiMsgTxtRaw; *curr != '\0' ; curr++) {
	    if ( *curr == '\n' ) lfCount++;
	}

	szAsciiMsgTxt=(LPSTR)HeapAlloc(MsgPtr->hHeap, 0, sizeof(CHAR)*(lfCount + dwMboxMsgLen + 1));
	if (!szAsciiMsgTxt) {
	    TM_TRACE((MTSP_tmDesc, TM_ENTER, "Out of heap space!"));
	    ERR(NO_MSG_BUF);
	}

	// convert \n-style eol to \r\n-style eol
	for (curr = szAsciiMsgTxtRaw, curr1 = szAsciiMsgTxt;
	     *curr != '\0';
	     curr++, curr1++) {
	    if ( *curr == '\n' ) {
		*curr1 = '\r';
		curr1++;
	    }

	    *curr1 = *curr;
	}
	// don't need original ascii msg buffer anymore
	HeapFree(MsgPtr->hHeap, 0, szAsciiMsgTxtRaw);

	// now make unicode version
	MsgPtr->dwMsgLen = strlen(szAsciiMsgTxt);

	if ( MsgPtr->dwMsgLen >  MSG_HEAD_SIZE + MSG_TAIL_SIZE ) {
	    strncpy(msgHead, szAsciiMsgTxt, MSG_HEAD_SIZE);
	    msgHead[MSG_HEAD_SIZE] = '\0';
	    strncpy(msgTail, szAsciiMsgTxt + MsgPtr->dwMsgLen - MSG_TAIL_SIZE - 1, MSG_TAIL_SIZE);
	    msgTail[MSG_TAIL_SIZE] = '\0';
	    TM_TRACE((MTSP_tmDesc, TM_ENTER, "Message read is (head): [%s ...]", msgHead));
	    TM_TRACE((MTSP_tmDesc, TM_ENTER, "Message read is (tail): [... %s]", msgTail));
	}
	else {
	    TM_TRACE((MTSP_tmDesc, TM_ENTER, "Message read is: [%s]", szAsciiMsgTxt));
	}

	TM_TRACE((MTSP_tmDesc, TM_ENTER, "\n-style size: %d, \r\n-style size: %d",
		  dwMboxMsgLen, MsgPtr->dwMsgLen));
	szMsgTxt=(LPWSTR)HeapAlloc(MsgPtr->hHeap, 0, sizeof(WCHAR)*(MsgPtr->dwMsgLen + 1));
	if (!szMsgTxt) {
	    TM_TRACE((MTSP_tmDesc, TM_ENTER, "Out of heap space!"));
	    CloseHandle(hFile);
	    ERR(NO_MSG_BUF);
	}

	// the output string of wsprintf() appears to have a maximum
	// of WSPRINTF_MAXSTRLEN so we convert szAsciiMsgTxt in
	// WSPRINTF_MAXSTRLEN chunks.
	{
#define WSPRINTF_MAXSTRLEN 1023
	    unsigned int	count;
	    unsigned int	ntimes;
	    char 	frag[WSPRINTF_MAXSTRLEN+1];
	    WCHAR	*wcStartPos;

	    ntimes = (MsgPtr->dwMsgLen / WSPRINTF_MAXSTRLEN); // integer divide
	    if ((ntimes * WSPRINTF_MAXSTRLEN) < MsgPtr->dwMsgLen) ntimes++; // handle remainder

	    for ( count = 0; count < ntimes; count++ ) {
		strncpy(frag, szAsciiMsgTxt+(count * WSPRINTF_MAXSTRLEN), WSPRINTF_MAXSTRLEN);
		wcStartPos = szMsgTxt+(count * WSPRINTF_MAXSTRLEN);
		wsprintf(wcStartPos, TEXT("%S"), frag);
	    }

	    // done with piecewise ascii to unicode conversion
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "ascii size: %d, unicode size: %d",
		      strlen(szAsciiMsgTxt), lstrlen(szMsgTxt)));
	}
    }

    // if we are requesting a specific number of lines >0, ask for the
    // the number of lines+1 so that we can determine if we're truncating
    // the message
    if (sNumLines>0)    sNumLines+1;

    // initialize extra info
    memset( &Xtra, 0x00, sizeof(MSGXTRA));
    Xtra.BufSize = (WORD)MsgPtr->dwMsgLen;

    // now read headers
    if (!ParseHeaders(hService, szMsgTxt, MsgPtr, &Xtra)) {
	HeapFree(MsgPtr->hHeap, 0, szMsgTxt);
	HeapFree(MsgPtr->hHeap, 0, szAsciiMsgTxt);
	
	return FALSE;
    }
    
    // were we only looking at headers?
    if (sNumLines == TRANSPORT_HEADERS) {
	// set body to NULL
	MsgPtr->szBody = NULL;
    }
    else {
	// now read message
	if (!ParseBody(hService, szMsgTxt, MsgPtr, &Xtra)){
	    HeapFree(MsgPtr->hHeap, 0, szMsgTxt);
	    HeapFree(MsgPtr->hHeap, 0, szAsciiMsgTxt);
	    return FALSE;
	}
    }

    // now set pointer to unique id
    MsgPtr->szSvcId = (LPWSTR)(hService->Uids[wSessionId-1]);
    HeapFree(MsgPtr->hHeap, 0, szMsgTxt);
    HeapFree(MsgPtr->hHeap, 0, szAsciiMsgTxt);

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "exiting TransportRecv()."));
    return TRUE;
}

/*****************************************************************************/
/* Release memory allocated for incoming mail msg                            */
/*****************************************************************************/

// Free buffer space allocated by TransportRecv()
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportFreeMsg(MailMsg * MsgPtr)
{

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportFreeMsg() called."));

    // Free all memory associated with this mail message
    if (MsgPtr && MsgPtr->hHeap) {
	HeapDestroy(MsgPtr->hHeap);

	// now that memory is free, make sure we reset all pointers to NULL
	MsgPtr->hHeap = MsgPtr->pwcHeaders = MsgPtr->szBody = NULL;
	return TRUE;
    }

    return FALSE;
}

/*****************************************************************************/
/* Delete a particular message                                               */
/*****************************************************************************/

// Delete the message specified by MsgPtr
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportDel(
    HANDLE InPtr,           // service handle
    MailMsg *MsgPtr)    // this structure has its unique id field set
						// to specify the message we wish to delete
{
    LPSERVICE hService = (LPSERVICE)InPtr;
    int wSessionId;

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportDel() called."));

    // make sure message is valid
    if (!hService)      return FALSE;
    hService->iErr = TRANS_ERR_NONE;

    // verify that this message exists
    for (wSessionId=0; wSessionId<hService->wNumMsgs; wSessionId++) {
	if (!lstrcmpi((hService->Uids)[wSessionId], MsgPtr->szSvcId))       break;
    }

    if ((wSessionId < 0) || (wSessionId>=hService->wNumMsgs))   ERR(INV_MSG_ID);

    // mark for deletion at TransportDisconnect time
    (hService->fDeathRow)[wSessionId] = TRUE;
    TM_TRACE((MTSP_tmDesc, TM_ENTER, "Message %d marked for deletion.", wSessionId));

    return TRUE;
}

/*****************************************************************************/
/* Update transport properties from the user                                 */
/*****************************************************************************/

BOOL CALLBACK
TransportPropsDlgProc(HWND	hDlg,
		      UINT	message,
		      WPARAM	wParam,
		      LPARAM	lParam)
{
    WORD wNotifyCode, wID;
    HWND hwndCtl;
    TCHAR szBuf[50];
    wNotifyCode = HIWORD(wParam); // notification code
    wID = LOWORD(wParam);         // item, control, or accelerator identifier
    hwndCtl = (HWND) lParam;      // handle of control

    switch(message)
	{
	case WM_COMMAND:
	    {
		switch(wID)
		    {
		    case IDOK:
		    case IDCANCEL:
			EndDialog(hDlg, TRUE);
			break;
		    default:
			return(FALSE);
		    }
		break;
	    }
	case WM_INITDIALOG:
	    {
		wsprintf(szBuf, TEXT("%S"), config.pReturnAddress);
		SetDlgItemText(hDlg,IDC_FROM, szBuf);
		wsprintf(szBuf, TEXT("%S"), config.pMailBox);
		SetDlgItemText(hDlg,IDC_MBOX, szBuf);

		break;
	    }
	default:
	    return(FALSE);
	}
    return (TRUE);
}
				    

BOOL
TransportProps(HWND    hwndParent,         // Parent window for dialog/wizard
	       LPWSTR  szCommonKey,        // Top level registry key
	       LPWSTR  szTransportKey)     // This transport's reg key
{
    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportProps() called."));

    DialogBox(g_hinst, MAKEINTRESOURCE(IDD_TRANSPORTPROPS), hwndParent,  
	      (WNDPROC)TransportPropsDlgProc);

    return TRUE;
}

/*****************************************************************************/
/* View a mail message (most transports just return FALSE for this call)     */
/*****************************************************************************/

BOOL
TransportView(HWND    hwndParent,         // Parent window
	      MailMsg *MsgPtr)            // Message to view.
{

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "TransportView() called."));

    MessageBox(hwndParent,TEXT("Got MSGBOX_DEBUG request to view message"),
	       TEXT("EMSD_MTSP.DLL TransportView()"),MB_OK);
    return TRUE;
}

/*****************************************************************************/
/* The entry point into this DLL.                                            */
/*****************************************************************************/

BOOL CALLBACK
DllMain(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpv)
{
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:

	TM_INIT();
	CONFIG_init();
	OS_init();
	MTSP_init();
	TM_VALIDATE();

	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "DllMain:DLL_PROCESS_ATTACH msg recvd."));

	if (!g_hinst)   g_hinst = hinst;
	if ((ghLibHeap = HeapCreate(0,2048,0))==NULL) {
	    return FALSE;
	}
	break;
    case DLL_PROCESS_DETACH:

	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "DllMain:DLL_PROCESS_DETACH msg recvd."));
	TM_CLOSE();

	if (ghLibHeap)      HeapDestroy( ghLibHeap);
	break;
    }
    return TRUE;
}




/*****************************************************************************/
/* Support Functions.                                                        */
/*****************************************************************************/

static BOOL
AsciiWriteFile(HANDLE hFile,
	       LPCVOID lpBuffer,
	       DWORD nNumberOfBytesToWrite,
	       LPDWORD lpNumberOfBytesWritten,
	       LPOVERLAPPED lpOverlapped)
{
    static char	aLine[EMSD_MAX_CHARS_PER_LINE];
    BOOL result;

    if ( nNumberOfBytesToWrite > EMSD_MAX_CHARS_PER_LINE ) {
	nNumberOfBytesToWrite = EMSD_MAX_CHARS_PER_LINE;

	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "AsciiWriteFile() line too long--truncating!"));
    }

    wcstombs(aLine, lpBuffer, nNumberOfBytesToWrite);
    result = WriteFile(hFile,
		       aLine,
		       nNumberOfBytesToWrite/2,	// 2 == (sizeof WCHAR/sizeof CHAR)
		       lpNumberOfBytesWritten,
		       NULL);

    return(result);
}


Void
MTSP_init()
{
    void 		*hConfig;
    struct ConfigParams *pConfigParam;
    static char		pConfigFile[256];
    char 		tmpbuf[256];
    ReturnCode		rc;

    TM_OPEN(MTSP_tmDesc, "MTSP_");
    TM_SETUP("MTSP_,ffff");

#define MTSP_DEFAULT_INI_FILE "\\EMSD\\config\\emsd_mtsp.ini"

    strcpy(pConfigFile, MTSP_DEFAULT_INI_FILE);

    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "loading .ini file: <%s>", pConfigFile)); // debug

    /* Open the configuration file.  Get a handle to the configuration data */
    if ((rc = CONFIG_open(pConfigFile, &hConfig)) != Success) {
	sprintf(tmpbuf, "EMSD_MTSP.DLL: Could not open configuration file (%s), reason 0x%04x!",
		pConfigFile, rc);
	EH_fatal(tmpbuf);
    }

    /*
     * Read the configuration
     */

    /* Get each of the configuration parameter values */
    for (pConfigParam = &configParams[0];
	 pConfigParam < &configParams[sizeof(configParams) /
				     sizeof(configParams[0])];
	 pConfigParam++)
	{
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "pConfigParam: %s %s %lu",
		      pConfigParam->pSectionName,
		      pConfigParam->pTypeName,
		      pConfigParam->ppValue));

	    if ((rc = CONFIG_getString(hConfig,
				       pConfigParam->pSectionName,
				       pConfigParam->pTypeName,
				       pConfigParam->ppValue)) != Success)
		{
		    sprintf(tmpbuf,
			    "EMSD_MTSP.DLL: Configuration parameter\n\t%s/%s\n"
			    "\tnot found, reason 0x%04x",
			    pConfigParam->pSectionName,
			    pConfigParam->pTypeName,
			    rc);
		    EH_fatal(tmpbuf);
		}

	    TM_TRACE((MTSP_tmDesc, TM_DEBUG,
		      "\n    Configuration parameter\n"
		      "\t[%s]:%s = <%s>",
		      pConfigParam->pSectionName,
		      pConfigParam->pTypeName,
		      *pConfigParam->ppValue));
	}

}

/*****************************************************************************/
/* Skip past white space                                                     */
/*****************************************************************************/
static BOOL   
StartUaShellDaemon(void) 
{
    BOOL		res; 
    PROCESS_INFORMATION	procinfo;

    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "StartUaShellDaemon() called."));

    res = CreateProcess(TEXT("uashwce_one.exe"), TEXT(""),
			NULL, NULL, FALSE, 0,
			NULL, NULL, NULL, &procinfo);
    if ( res = FALSE ) {
	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "Error starting UA Shell process!"));
	return FALSE;
    }
    else {
	TM_TRACE((MTSP_tmDesc, TM_DEBUG, "UA Shell process started."));
	return TRUE;
    }
}
