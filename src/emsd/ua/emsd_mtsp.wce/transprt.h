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
/*---------------------------------------------------------------------------*\
 *
 * (c) Copyright Microsoft Corp. 1994 All Rights Reserved
 *
 *  module: transprt.h
 *
 *  purpose: Service API
 *
\*---------------------------------------------------------------------------*/
#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

//Flags for TransportRecv
#define TRANSPORT_SEND          0x0000001
#define TRANSPORT_RECV          0x0000002

//Messages that transport can send back to pmail
    // tell pmail that connection has closed in background
    // (this would be sent if, for example, you were connected
    // over RAS and lost the modem connection)
    // both wparam and lparam are ignored
    // return is always 0
#define TRANSPORT_CONN_LOST	(WM_USER+1)

    // tell pmail to refresh its connection to the service
    // this would be used if pmail has an open connection to the service
    // and the service determines a new message has arrived
    // both wparam and lparam are ignored
    // return is always 0
#define TRANSPORT_REFRESH	(WM_USER+2)

// prototypes

#ifdef __cplusplus
extern "C" {
#endif
    
// Get last mail error as integer
int TransportError(
    HANDLE hService);           // Service

// Get last mail error (printable)
int TransportErrorMsg(                  
    HANDLE  InPtr,              // Service
    LPWSTR  szBuf,              // Buffer to fill
    int     iBufLen,            // Length of buffer in chars
    int*    piSrcLine);         // Line where error occured (or NULL)

// initialize transport.
//      - creates handle
//      - pulls logon information from registry
//      - starts ras if necessary
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportInit(
    HANDLE *NewHandle,          // Pointer to hold new service
    LPWSTR  szCommonKey,        // Top level registry key
    LPWSTR  szTransportKey);    // This transport's reg key

// Connect to a service for transmission/receiving
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportConnect(
    HANDLE  hService,           // Service
    DWORD   dwFlags,            // Transmit/Receive Flags
				    // Values specified above
    HWND    hWndNotify,		// Window which will handle notification
				// messages
    BOOL    (*CallBack)(int,BOOL)); // Callback routine.  If this
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


// Disconnect from current connection
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportDisconnect(
    HANDLE hService);           // Service

// release transport
//              frees any global vars used for connection
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportRelease(
    HANDLE hService);           // Service

// send specified message
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportSend(
    HANDLE hService,            // Service
    MailMsg *MsgPtr);           // Message to send

// gets count of messages in mailbox
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportCount(
    HANDLE hService,            // Service
    LPWORD lpwNumMsgs);         // Pointer to word which will
				// receive Number of Messages

// gets a message from the service
// NOTE: this allocates a buffer which must be freed by calling TransportFreeMsg();
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportRecv(
    HANDLE hService,            // Service
    WORD wSessionId,                // Session Id to read.  Should
				    // be in range 1 to NumMsgs.
				    // If this value is 0, it is
				    // assumed that the Unique
				    // Message Id is specified
				    // in msg
    short sNumLines,            // Number of lines to receive
				    // positive value of lines to read
				    // or see #defines below
    MailMsg *msg,               // Mail Message that will contain
				    // Message.  Memory is allocated
				    // in here that needs to be 
				    // freed by TransportFreeMsg()
    HANDLE Reserved);           // Should be set to NULL

// possible values for wNumLines
#define     TRANSPORT_HEADERS       0
#define     TRANSPORT_ALL_LINES     -1
#define     TRANSPORT_MSG_EXIST     -2

// Free buffer space allocated by TransportRecv()
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportFreeMsg(
    MailMsg *MsgPtr);           // Message containing memory to be
				    // freed.  This MailMsg structure
				    // should have been initialized by
				    // TransportRecv()

// Delete the message specified by MsgPtr
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportDel(
    HANDLE hService,            // Service
    MailMsg *MsgPtr);           // Message to delete.  The Unique
				    // ID field of this structure
				    // must point to the message
				    
// Update transport properties from the user
// Returns: TRUE if routine successful
//          FALSE if unsuccessful (use TransportError() to get error)
BOOL
TransportProps(
    HWND    hwndParent,         // Parent window for dialog/wizard
    LPWSTR  szCommonKey,        // Top level registry key
    LPWSTR  szTransportKey);    // This transport's reg key

// View a mail message (most transports just return FALSE for this call)
// Returns: TRUE if routine successful
//          FALSE if unsuccessful
BOOL
TransportView(
    HWND    hwndParent,         // Parent window
    MailMsg *MsgPtr);           // Message to view.

#ifdef __cplusplus
}
#endif

#endif
