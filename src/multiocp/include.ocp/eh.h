/*
 * This file is part of the Open C Platform (OCP) Library. OCP is a
 * portable library for development of Data Communication Applications.
 *
 * Copyright (C) 1995 Neda Communications, Inc.
 *	Prepared by Mohsen Banan (mohsen@neda.com)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.  This library is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.  You should have received a copy of the GNU
 * Library General Public License along with this library; if not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
*/
/*+
 * File name: eh.h
 *
 * Description: Exception Handler Module header
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: eh.h,v 1.14 1997/10/18 23:42:28 mohsen Exp $
 */

#ifndef _EH_H_
#define _EH_H_

#include "estd.h"

#if defined(OS_VARIANT_WinCE)
extern Bool EH_exitReq;
#endif /* OS_VARIANT_WinCE */

extern FILE *	eh_hFd;
extern void  (* eh_pfDisplayMessage)(char * pMessageType,
				     char * pFileName,
				     int lineNum,
				     char * pMessage);

PUBLIC Void
EH_init (void);

PUBLIC SuccFail
EH_config(char *ehFileName);

PUBLIC void
EH_displayMessages(void (* pfDisplayMessage)(char * pMessageType,
					     char * pFileName,
					     int lineNum,
					     char * pMessage));

#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
#define EH_message(msgType, str)					\
	{								\
	    if (! OS_isrIsActive())					\
	    {								\
		if (eh_pfDisplayMessage != NULL)			\
		{							\
		    (* eh_pfDisplayMessage)(msgType,			\
					    __FILE__, __LINE__, str);	\
		}							\
		else if (eh_hFd != NULL)				\
		{							\
		    fprintf(eh_hFd,					\
			    msgType ": %s, %d: %s\n",			\
			    __FILE__, __LINE__, (str));			\
		}							\
	    }								\
	}
#else
#define EH_message(msgType, str)					\
	{								\
	    if (! OS_isrIsActive())					\
	    {								\
		if (eh_hFd != NULL)					\
		{							\
		    fprintf(eh_hFd,					\
			    msgType ": %s, %d: %s\n",			\
			    __FILE__, __LINE__, (str));			\
		}							\
		else if (eh_pfDisplayMessage != NULL)			\
		{							\
		    (* eh_pfDisplayMessage)(msgType,			\
					    __FILE__, __LINE__, str);	\
		}							\
	    }								\
	}
#endif

/*
 * EH_assert( expr )
 *         Emits Line number and file if (!expr).
 *         Should never happen points to a software error.
 */
#define	EH_assert(expr)					\
		if (! (expr))				\
		{					\
		    EH_message("Assertion failed", "");	\
		}


/*
 * EH_oops()
 *         Emits Line number and file. 
 *         Should never happen points to a software error.
 */
#ifndef EH_oops
# define EH_oops()	EH_message("OOPS", "")
#endif



/*
 * EH_fatal(str)
 *         A major problem has been detected. 
 *         File, Line and str are printed. Program is terminated.
 */
#ifndef EH_fatal
#if defined(OS_VARIANT_WinCE)
# define EH_fatal(str)				\
		{				\
		    EH_message("FATAL", str);	\
		    EH_exitReq = TRUE;		\
		}
#else
# define EH_fatal(str)				\
		{				\
		    EH_message("FATAL", str);	\
		    exit(1);			\
		}
#endif /* OS_VARIANT_WinCE */
#endif /* EH_fatal */


/*
 * EH_problem( str ): A minor problem has been detected.
 * File, Line and str are printed.
 */
#ifndef EH_problem
# define EH_problem(str)	EH_message("PROBLEM", str)
#endif


/*
 * EH_violation( str )
 *         A peer entity is misbehaving.
 *         File, Line and str are printed.
 */
#ifndef EH_violation
# define EH_violation(str)	EH_message("VIOLATION", str)
#endif


#ifndef EH_badUsage
#define EH_badUsage(str)						\
	{								\
	    if (eh_pfDisplayMessage != NULL)				\
	    {								\
		(* eh_pfDisplayMessage)("BAD USAGE",			\
					__FILE__, __LINE__, str);	\
	    }								\
	    else if (eh_hFd != NULL)					\
	    {								\
		fprintf(eh_hFd,						\
			"BAD USAGE: %s, %d: %s\n",			\
			__FILE__, __LINE__, (str));			\
	    }								\
	}
#endif


#ifndef EH_vmsProblem
extern Void
eh_vmsProblem(String file, Int line, LgUns status);

# define EH_vmsProblem(status)  eh_vmsProblem(__FILE__, __LINE__, status)
#endif


#ifndef EH_vmsVerify
extern SuccFail
eh_vmsVerify(String file, Int line, LgUns status);

# define EH_vmsVerify(status)  eh_vmsVerify(__FILE__, __LINE__, status)
#endif


#ifndef EH_unixProblem
extern void
eh_unixProblem(char * pFileName,
	       int lineNum,
	       char * pMsg);

# define EH_unixProblem(str)  eh_unixProblem(__FILE__, __LINE__, str)
#endif

#endif /* _EH_H_ */
