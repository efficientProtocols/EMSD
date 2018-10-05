/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of Neda-EMSD. An implementation of RFC-2524.
 *
 *Neda-EMSD is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with Neda-EMSD; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
/* -*- Mode: c++ -*- */
/*---------------------------------------------------------------------------*\
 *
 * (c) Copyright Microsoft Corp. 1994 All Rights Reserved
 *
 *  module: emsd_mtsp.h
 *
 *  purpose: EMSD Mail Trasport Service Private Definitions
 *
\*---------------------------------------------------------------------------*/

#ifndef _EMSD_MTSP_H_
#define _EMSD_MTSP_H_

// Maximum size of a directory
// we assume filenames are never longer than 2*MAX_DIR_SIZE
#define MAX_DIR_SIZE	    256

// Assumed maximum size of key data stored in registry
#define MAX_HANDLE_SIZE	    64

// Max number of messages this library works with
// this number was arbitrarily chosen to limit the amount
// of memory used.  If we were writing a "real" service,
// we would have allowed an unlimited number of messages
// (or limited the number to a service-specifed max) and
// used dynamic memory allocation
#define MAX_MSGS	    64

// macro used to record error information
#define ERR(e)  return (hService->iErrLine = __LINE__,hService->iErr = TRANS_ERR_##e,FALSE)

// short cut for specifying unicode text
#ifndef _T
#define _T(x) TEXT(x)
#endif

// These are the error numbers used for error returns
#define	TRANS_ERR_NONE		    0
#define	TRANS_ERR_NO_SVC_DIR	    1
#define	TRANS_ERR_HANDLE_NOT_ALLOC  2
#define TRANS_ERR_CANT_SEND	    3
#define TRANS_ERR_CANT_OPEN	    4
#define TRANS_ERR_INV_MSG_ID	    5
#define TRANS_ERR_NO_MSG_BUF	    6
#define TRANS_ERR_READ		    7

// this is our private definition of the service.  everything
// outside our library only has a handle to this structure
typedef struct tagservice {
    int		iErr;			// in case of an error, this
					// value gets the number of the
					// error.
    int		iErrLine;		// in case of an error, this
					// value gets the source line
					// of the error, it is then
					// displayed by pmail
    HWND	hWndNotify;		// window to send notification
					// messages to
    BOOL	(*CallBack)(int,BOOL);	// this is used to hold the
					// address of the callback routine
					// specified in Connect
    BOOL	fRecActive;		// this flag tells us if the
					// service is currently active for
					// receiving (it is not used in
					// this example).
    BOOL	fTransActive;		// this flag tells us if the
					// service is currently active for
					// transmission (it is not used
					// in this example).
    WORD	wNumMsgs;		// Number of messages stored on service
    SYSTEMTIME	connLocalTime;		// Time of Current Connection 
    WORD	msgCount;		// Count of messages submitted during this connection
    WCHAR	Uids[MAX_MSGS][11];	// This array hold the unique ids of each message in the service
                                        // This memory needs to be allocated during the Connect() and kept alive
                                        // for as long as the service is active.
    BOOL	fDeathRow[MAX_MSGS];	// Marked for deletion at TransportDisconnect() time 
                                        // for as long as the service is active.
    WCHAR	szMailboxFileName[128];	// Name of mailbox filename
    // File Locking Info	
    WCHAR	szLockFile[128];	// Name of lock file for concurrency control with uash
    int		iGrabLockMaxRetries; 	// Num times to try getting the lock before giving up
    int		iGrabLockRetryInterval;	// Interval between grab lock tries
} SERVICE, *LPSERVICE;

// This structure contains values that are used during the
// parsing of messages
typedef struct msgxtra {
    WORD    BufSize;			// size of message buffer
    WORD    BufPos;			// current position in msgbuf
    WORD    BodySize;			// allocated size of body
    WORD    BodyLen;			// number of bytes used (strlen) of body
} MSGXTRA, *LPMSGXTRA;

extern HINSTANCE	g_hinst;	// instance of library
extern HANDLE		ghLibHeap;	// global heap used for libary
					// Each message has a heap which
					// is used for data relating to
					// that message.  This is used
					// for variables related to the
					// library itself.	

// support function headers

// This routine parses the incoming headers from a file and places
// them into MsgPtr->pwcHeaders.  It returns a FALSE if it fails.
BOOL
ParseHeaders(LPSERVICE hService, // Service handle.  This is only passed
				// to allow for returns of error conditions.
	     LPCWSTR szMsgTxt,	// This is a buffer containing the entire
				// text of the message (headers & body).
	     MailMsg * MsgPtr,	// This is a pointer to the structure into
				// which the headers will be placed
	     LPMSGXTRA XtraPtr	// This structure contains extra information
				// used for processing
	     );

// This routine is called by ParseHeaders() and sets the message's time
// to the current system time
void
ParseDate(MailMsg *MsgPtr	// Message pointer to get file time
	  );

// This routine parses the body of an incoming message.  It counts
// the number of lines and bytes and truncates the message after the
// specified number of lines or max message size.  This routine, in
// order to be efficient in the use of memory, allocates the buffer for
// the message body in small blocks and lets this grow as necessary.
BOOL
ParseBody(LPSERVICE hService,	// Service handle.  This is only passed
				// to allow for returns of error conditions.
	  LPCWSTR szMsgTxt,	// This is a buffer containing the entire
				// text of the message (headers & body).
	  MailMsg * MsgPtr,	// This is a pointer to the structure into
				// which the body will be placed.
	  LPMSGXTRA XtraPtr	// This structure contains extra information
				// used for processing
	  );

// This routine reads the next line of text from szMsgTxt (a line
// is always terminated with \r\n).  It uses pointers in XtraPtr to
// keep its offset from the start of the buffer.   
// This routine returns -1 if there are no lines left or the number
// of characters placed into Line
int
GetLine(LPCWSTR szMsgTxt,	// This is a buffer containing the entire
				// text of the message (headers & body)
	LPMSGXTRA XtraPtr,	// This structure contains extra information,
				// such as the current offset from the start
				// of the message and the total size of the
				// message.
	LPWSTR Line,		// This is a character buffer which will
				// receive the next line of text
	WORD wBufLen		// This tells the max size of the buffer. At
				// most, wBufLen-1 chars will be written into
				// Line
	);

#endif
