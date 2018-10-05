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
 * File name: eh.c
 *
 * Description: Exception Handler
 *
 * Functions:
 *	EH_init( )
 *      EH_config(char *ehFileName)
 *      ehFatal(Char *str)
 *      eh_vmsProblem(String file, Int line, LgUns status)
 *      eh_vmsVerify(String file, Int line, LgUns status)
 *      eh_unixProblem(String file, Int line, String str)
 *
 */

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: eh.c,v 1.16 2002/03/22 08:30:53 mohsen Exp $";
#endif /*}*/

#include  "os.h"
#include  "estd.h"
#include  "eh.h"

/* VMS STUFF */
#ifdef VMS
#include <descrip.h>
#include <dvidef.h>
#include <iodef.h>
#include <msgdef.h>
#include <nfbdef.h>
#include <ssdef.h>
#include <stsdef.h>
#endif

#if defined(OS_VARIANT_WinCE)
Bool EH_exitReq = FALSE;	/* for use by WinMain */
#endif /* OS_VARIANT_WinCE */

/* File descriptor for EH_message() output */
#if defined(OS_VARIANT_WinCE)
FILE *		eh_hFd = stdout;
void	     (* eh_pfDisplayMessage)(char * pMessageType,
				     char * pFileName,
				     int lineNum,
				     char * pMessage) = NULL;
#elif ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Win16)
/* NOTYET, LINUX, can break Solaris */
#if 0
FILE *		eh_hFd = stdout;
#else
FILE *		eh_hFd = NULL;
#endif
void	     (* eh_pfDisplayMessage)(char * pMessageType,
				     char * pFileName,
				     int lineNum,
				     char * pMessage) = NULL;
#else
FILE *		eh_hFd = NULL;
void	     (* eh_pfDisplayMessage)(char * pMessageType,
				     char * pFileName,
				     int lineNum,
				     char * pMessage) = NULL;
#endif


/*<
 * Function:	EH_init
 *
 * Description: Initialize Exception Handler module.
 *
 * Arguments:	None.
 *
 * Returns:	None.
 *
 * 
>*/
PUBLIC Void
EH_init (void)
{
}

/*<
 * Function:	EH_config
 *
 * Description: Direct EH outputs to a file.
 *
 * Arguments:	File name.
 *
 * Returns:	0 if successful, -1 otherwise.
 *
>*/
PUBLIC SuccFail
EH_config(char *ehFileName)
{
    FILE *hFd;

    if ((hFd = fopen(ehFileName, "w")) != NULL) {
	eh_hFd = hFd;
	setbuf(eh_hFd, (char *) NULL);
	return SUCCESS;
    }

    return FAIL;
}

/*<
 * Function:	EH_displayMessages
 *
 * Description:	Set function pointer to be used for displaying messages
 *
 * Arguments:	Function pointer pointing to the func that displays messages.
 *
 * Returns:	None.
 *
>*/
PUBLIC void
EH_displayMessages(void (* pfDisplayMessage)(char * pMessageType,
					     char * pFileName,
					     int lineNum,
					     char * pMessage))
{
    eh_pfDisplayMessage = pfDisplayMessage;
}

/*<
 * Function:	ehFatal
 *
 * Description: Fatal exception
 *
 * Arguments:	Message.
 *
 * Returns:	None.
 *
>*/
LOCAL Void
ehFatal(Char *str)
{
}

/*<
 * Function:	eh_vmsProblem
 *
 * Description: VMS problem.
 *
 * Arguments:	File, line no, status.
 *
 * Returns:	None.
 *
>*/
Void
eh_vmsProblem(String file, Int line, LgUns status)
{
#ifdef OS_TYPE_VMS
    LgUns retVal;
    static Char msgBuf[256];	    
    static  struct  dsc$descriptor_s msgDesc =
		{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, &msgBuf[0]}; 

    msgDesc.dsc$w_length = sizeof(msgBuf);
    msgDesc.dsc$a_pointer = &msgBuf[0];
    retVal = sys$getmsg(status, &msgDesc, &msgDesc, 15, 0);
    if (!(retVal & STS$M_SUCCESS)) {
	EH_fatal("eh_vmsProblem: sys$getmsg failed");
	/* NOTREACHED */
    }
    msgBuf[msgDesc.dsc$w_length] = '\0';
    fprintf(eh_hFd, "VMS PROBLEM: %s, %d: %s\n", file, line, &msgBuf[0]);
#endif
}

/*<
 * Function:	eh_vmsVerify
 *
 * Description:	VMS verify
 *
 * Arguments:	File, line no, status.
 *
 * Returns:	0 if successful, -1 otherwise.
 *
>*/
SuccFail
eh_vmsVerify(String file, Int line, LgUns status)
{
#ifdef OS_TYPE_VMS
    if (!(status & STS$M_SUCCESS)) {
	eh_vmsProblem(status);
	return (FAIL);
    } 
#endif
    return (SUCCESS); 
}

/*<
 * Function:	en_unixProblem.
 *
 * Description:	Unix problem.
 *
 * Arguments:	File name, line no, message.
 *
 * Returns:	None.
 *
>*/

Void
eh_unixProblem(String file, Int line, String str)
{
    /*
     * IF YOU HAVE A PROBLEM COMPILING THIS BECAUSE sys_errlist OR
     * errno IS NOT DECLARED, DECLARE THEM IN os.h, NOT HERE.
     */

#ifdef OS_TYPE_UNIX
    
    fprintf(eh_hFd, "\nUNIX PROBLEM: %s, %d\n\t%s\n",
	    file, line, strerror(errno));
#endif
}
