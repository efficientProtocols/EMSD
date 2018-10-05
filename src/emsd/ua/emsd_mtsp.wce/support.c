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
 *  module: support.c
 *
 *  purpose: This file contains the routines that support the API routines.
 *
\*---------------------------------------------------------------------------*/
#include <windows.h>
#include <tchar.h>

#include <msgstore.h>

#include <tm.h>
#include <os.h>

#ifdef TM_ENABLED
extern TM_ModDesc MTSP_tmDesc;
#endif

#include "transprt.h"
#include "emsd_mtsp.h"

static WCHAR *
SkipLeadingWhiteSpace(WCHAR *String);

/*****************************************************************************/
/* Get date information out of header					     */
/*****************************************************************************/
// This routine is called by ParseHeaders() and is used to set the msg time
void
ParseDate(MailMsg *MsgPtr)	    // Message pointer to get file time
{
  SYSTEMTIME 	MsgTime;
  WCHAR	*DateString;
  WCHAR	*start;
  WCHAR	tz[16];
  WCHAR	month[3];
  int		idx;
  char	scratch[128];
  static WCHAR *monthNames[] = {TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"), TEXT("May"), TEXT("Jun"), 
				TEXT("Jul"), TEXT("Aug"), TEXT("Sep"), TEXT("Oct"), TEXT("Nov"), TEXT("Dec")};

  TM_TRACE((MTSP_tmDesc, TM_ENTER, "ParseDate() called."));

  // set file time to 0 in case of error
  memset(&(MsgPtr->ftDate), 0x00, sizeof(FILETIME));
  DateString = MailGetField(MsgPtr, TEXT("Date"), FALSE);

  /* no swscanf, so have to do it manually... */

  /* date is in the form: "07 Jun 97 17:04:43 GMT" */

  /* day */
  start = DateString;
  if ( !DateString ) {
    goto done;
  }
  wcstombs(scratch, DateString, sizeof(scratch));
  TM_TRACE((MTSP_tmDesc, TM_DEBUG, "ParseDate(): Date string is <%s>.", scratch));    
  MsgTime.wDay = (WORD)_ttol(start); 

  /* month */
  start = wcschr(start, TEXT(' '));
  if ( !start ) {
    goto done;
  }
  start = SkipLeadingWhiteSpace(start);
  wcsncpy(month, start, 3);
  for (MsgTime.wMonth = 0, idx = 0; idx < 12; idx++) {
    if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, month, 3, monthNames[idx], 3) == 2) {
      MsgTime.wMonth = (WORD)(idx + 1);
      break;
    }
  }
  if ( MsgTime.wMonth == 0 ) {
    goto done;
  }
    
  /* year */
  start = wcschr(++start, TEXT(' '));
  if ( !start ) {
    goto done;
  }
  start = SkipLeadingWhiteSpace(start);
  MsgTime.wYear = (WORD)_ttol(start);
  if ( MsgTime.wYear < 100 ) MsgTime.wYear += 1900;

  /* hour */
  start = wcschr(++start, TEXT(' '));
  if ( !start ) {
    goto done;
  }
  start = SkipLeadingWhiteSpace(start);
  MsgTime.wHour = (WORD)_ttol(start);

  /* minute */
  start = wcschr(++start, TEXT(':'));
  if ( !start ) {
    goto done;
  }
  start++;			// skip over ':'
  MsgTime.wMinute = (WORD)_ttol(start);
    
  /* second */
  start = wcschr(++start, TEXT(':'));
  if ( !start ) {
    goto done;
  }
  start++;			// skip over ':'
  MsgTime.wSecond = (WORD)_ttol(start);
    
  /* timezone */
  start = wcschr(++start, TEXT(' '));
  if ( !start ) {
    goto done;
  }
  start = SkipLeadingWhiteSpace(start);
  wcscpy(tz, start);
  wcstombs(scratch, tz, sizeof(scratch));

done:

  SystemTimeToFileTime(&MsgTime, &(MsgPtr->ftDate));
}

/*****************************************************************************/
/* Parse header lines in incoming mail msg				     */
/*****************************************************************************/
// This routine parses the incoming headers from a file and places
// them into MsgPtr->pwcHeaders.  It returns FALSE if it fails.
BOOL
ParseHeaders(LPSERVICE hService, // Service handle.  This is only passed
				// to allow for returns of error conditions.
	     LPCWSTR szMsgTxt,	// This is a buffer containing the entire
				// text of the message (headers & body).
	     MailMsg * MsgPtr,	// This is a pointer to the structure into
				// which the headers will be placed
	     LPMSGXTRA XtraPtr)	// This structure contains extra information
				// used for processing
{
    // returns:
    //		    1 if successful
    //		    0 if error
    WCHAR 	InBuf[1030];
    WCHAR 	szHeader[1024];
    DWORD	len;
    char	buf[1024];
    WCHAR	*match;
	
    TM_TRACE((MTSP_tmDesc, TM_ENTER, "ParseHeaders(): called. BufSize = %d; BufPos = %d.",
	      XtraPtr->BufSize, XtraPtr->BufPos));

    // setup header field in message
    MsgPtr->pwcHeaders=(WCHAR *)HeapAlloc(MsgPtr->hHeap, 0, sizeof(WCHAR));
    if (!(MsgPtr->pwcHeaders)) {
	HeapFree(MsgPtr->hHeap, 0, szHeader);
	ERR(NO_MSG_BUF);
    }
	
    *(MsgPtr->pwcHeaders) = _T('\0');

    // parse headers
    // In this example, we'll assume the first line is the to,
    // the second line is the from, the third line is cc
    // and fourth is subject
    while ( GetLine(szMsgTxt, XtraPtr, InBuf, sizeof(InBuf)/sizeof(WCHAR)) != -1 ) {

	if (lstrlen(InBuf) == 0) break;	// end of headers

	if ((len = lstrlen(TEXT("From:")))
	    && (match = TEXT("From:"))
	    && (lstrlen(InBuf) > len) 
	    && ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
			       InBuf, len, match, len) == 2 )) {
	    MailSetField(MsgPtr, _T("From"), SkipLeadingWhiteSpace(InBuf + len));
	}
	else if ((len = lstrlen(TEXT("To:")))
		 && (match = TEXT("To:"))
		 && (lstrlen(InBuf) > len) 
		 && ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
				    InBuf, len, match, len) == 2 )) {
	    MailSetField(MsgPtr, _T("To"), SkipLeadingWhiteSpace(InBuf + len));
	}
	else if ((len = lstrlen(TEXT("Cc:")))
		 && (match = TEXT("Cc:"))
		 && (lstrlen(InBuf) > len) 
		 && ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
				    InBuf, len, match, len) == 2 )) {
	    MailSetField(MsgPtr, _T("Cc"), SkipLeadingWhiteSpace(InBuf + len));
	}
	else if ((len = lstrlen(TEXT("Subject:")))
		 && (match = TEXT("Subject:"))
		 && (lstrlen(InBuf) > len) 
		 && ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
				    InBuf, len, match, len) == 2 )) {
	    MailSetField(MsgPtr, _T("Subject"), SkipLeadingWhiteSpace(InBuf + len));
	}
	else if ((len = lstrlen(TEXT("Date:")))
		 && (match = TEXT("Date:"))
		 && (lstrlen(InBuf) > len) 
		 && ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
				    InBuf, len, match, len) == 2 )) {
	    MailSetField(MsgPtr, _T("Date"), SkipLeadingWhiteSpace(InBuf + len));
	}
	else {
	    wcstombs(buf, InBuf, sizeof(buf));
	}
    }
    ParseDate(MsgPtr);
    return TRUE;
}

/*****************************************************************************/
/* Parse body of incoming mail msg					     */
/*****************************************************************************/
// This routine parses the body of an incoming message.  It should count
// the number of lines and bytes and truncate the message after the
// specified number of lines or max message size.  This routine should
// be efficient in the use of memory and should probably allocate the buffer for
// the message body in small blocks and lets this grow as necessary.
BOOL
ParseBody(LPSERVICE hService,	// Service handle.  This is only passed
				// to allow for returns of error conditions.
	  LPCWSTR szMsgTxt,	// This is a buffer containing the entire
				// text of the message (headers & body).
	  MailMsg * MsgPtr,	// This is a pointer to the structure into
				// which the body will be placed.
	  LPMSGXTRA XtraPtr)	// This structure contains extra information
				// used for processing
{
    // these are the block sizes and max block sizes used by the Pegasus store.
    // Normally, we'd need to make sure there are never more than MAX_BODY_SIZE
    // characters in the body of the message.   We'd also want to add a message
    // to the end of the message (contained within the MAX_BODY_SIZE characters)
    // saying that the message was truncated.
#define BLOCKSIZE	2046
#define MAX_BODY_SIZE	((16*BLOCKSIZE)-1)
    
    WCHAR InBuf[1030];
    int NumRead;
    WORD iGotSoFar=0;

    // allocate space for body
    XtraPtr->BodySize = XtraPtr->BufSize - XtraPtr->BufPos + 1;
    MsgPtr->szBody=(LPWSTR)HeapAlloc(MsgPtr->hHeap, 0, 
				     sizeof(WCHAR)*XtraPtr->BodySize);
    // should probably return an error if this fails...
    if (!MsgPtr->szBody)	ERR(NO_MSG_BUF);
	
    *(MsgPtr->szBody) = _T('\0');

    XtraPtr->BodyLen = 0;

    while (1) {
	    
	// now read next line
	NumRead=GetLine(szMsgTxt, XtraPtr, InBuf, sizeof(InBuf)/sizeof(WCHAR));
	{
	    char scratch[4096];
	    wcstombs(scratch, InBuf, sizeof(scratch));
	    TM_TRACE((MTSP_tmDesc, TM_DEBUG, "GetLine() returned <%s>.", scratch));
	}
	// are we done?
	if (NumRead<0)  break;

	// Report on (approximate) progress (approximately every 7 lines)
	if (hService->CallBack && !(++iGotSoFar & 0x7))
	    (*(hService->CallBack))(iGotSoFar >> 3, TRUE);
		
	// normally, we'd check the number of lines, and once we 
	// pass the desired number, we'd through the rest of the data
	// away.   Here, we'll assume the message is always small!
	    
	lstrcat(MsgPtr->szBody, InBuf);
		
	lstrcat(MsgPtr->szBody, _T(OS_EOL_CHARS));
	XtraPtr->BodyLen+=NumRead+2;
    }

    return TRUE;
}

/*****************************************************************************/
/* Get a single line back						     */
/*****************************************************************************/

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
	WORD wBufLen)		// This tells the max size of the buffer. At
				// most, wBufLen-1 chars will be written into
				// Line
{
    WCHAR	* Ptr;
    int		LineLen;
    int		iCrLen;    

    TM_TRACE((MTSP_tmDesc, TM_ENTER, "GetLine(): called. BufSize = %d; BufPos = %d; Max Line Size = %d.",
	      XtraPtr->BufSize, XtraPtr->BufPos, wBufLen));

    // are there any characters left?
    if (XtraPtr->BufPos >= XtraPtr->BufSize)	    return -1;
    
    // now search for linefeed
    if (Ptr=wcsstr(szMsgTxt+XtraPtr->BufPos, _T(OS_EOL_CHARS))) {

	// we found end of line, so copy data
	*Ptr = _T('\0');
	LineLen=Ptr-szMsgTxt-XtraPtr->BufPos;
    }
    else {
	LineLen = lstrlen(szMsgTxt+XtraPtr->BufPos);
    }

    // since we don't know if we have a cr/lf, set length to 0
    iCrLen = 0;

    // now copy string into buffer
    if (LineLen > wBufLen-1)    wBufLen--;
    else {
	wBufLen=LineLen;
	    
	// we need to account for cr/lf, so set len to 2
	iCrLen=2;
    }
    wcsncpy(Line, szMsgTxt+XtraPtr->BufPos, wBufLen);
    Line[wBufLen] = _T('\0');

    // now, update msgpos
    XtraPtr->BufPos += wBufLen+iCrLen;

    return wBufLen;
}


/*****************************************************************************/
/* Skip past white space                                                     */
/*****************************************************************************/
static WCHAR *
SkipLeadingWhiteSpace(WCHAR *String)
{
    while ((*String == TEXT(' ')) ||
	   (*String == TEXT('\t'))) {
	String++;
    }
    return String;
}    
