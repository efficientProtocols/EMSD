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
 * File name: tm.h
 *
 * Description: Trace Module header file.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: tm.h,v 1.1.1.1 1997/10/20 23:22:24 mohsen Exp $
 */

#ifndef _TM_H_
#define _TM_H_

#include "estd.h"
#include "queue.h"

/*
 * Define the functions that they can use even if TM_ENABLED is false.
 */

#ifndef TM_here
# define TM_here()  tm_here(__FILE__, __LINE__)
#endif

PUBLIC Char *
tm_here(Char *fileName, Int lineNu);

PUBLIC String
TM_prAddr(Ptr p);


/*
 * The remainder of the functions are for use only if TM_ENABLED is true.  We
 * give them some null macros for when TM_ENABLED is false, so they don't need
 * to fill up their code with #ifdefs.
 */


typedef unsigned short TM_Mask;

typedef struct tm_ModInfo {
    struct tm_ModInfo *next;
    struct tm_ModInfo *prev;
    Char moduleName[16];
    TM_Mask mask;
} tm_ModInfo;

#define TM_ModuleCB tm_ModInfo     /* For backwards compatibility */
typedef TM_ModuleCB *TM_ModDesc;

#ifdef TM_ENABLED

#define TM_NOTRACE      0x0000
#define TM_ALL	        0xffff
#define TM_BIT0		0x0001
#define TM_BIT1		0x0002
#define TM_BIT2		0x0004
#define TM_BIT3		0x0008
#define TM_BIT4		0x0010
#define TM_BIT5		0x0020
#define TM_BIT6		0x0040
#define TM_BIT7		0x0080
#define TM_BIT8		0x0100
#define TM_BIT9		0x0200
#define TM_BIT10	0x0400
#define TM_BIT11	0x0800
#define TM_BIT12	0x1000
#define TM_BIT13	0x2000
#define TM_BIT14	0x4000
#define TM_BIT15	0x8000

/* 
 * Some general-purpose trace flags
 */

#define TM_ENTER	TM_BIT0
#define TM_EXIT		TM_BIT1
#define TM_VERBOSE	TM_BIT14
#define TM_DEBUG	TM_BIT15

LCL_XTRN String tm_loc;
#define TM_trace    tm_loc = TM_here(); tm_trace


/*
 * Public interface to the TM module.  USE THESE! (NOT DIRECT FUNCTION CALLS)
 */

#define	TM_INIT				TM_init
#define	TM_GETHEAD			TM_getHead();
#define	TM_CONFIG(pFileName)		TM_config(pFileName)
#define	TM_OPEN(hModCB, modName)	(hModCB = TM_open(modName))
#define	TM_CLOSE			TM_close
#define	TM_TRACE(args)			TM_trace args
#define	TM_QUERY(hModCB, mask)		TM_query(hModCB, mask)
#define	TM_VALIDATE			TM_validate
#define	TM_SETUP(optarg)		TM_setUp(optarg)
#define	TM_HEXDUMP(hModCB, bits, msg, pdu, len)			\
			TM_hexDump(hModCB, bits, msg, pdu, len)
#define	TM_CALL(hModCB, bits, pFunc, hParam1, hParam2)			\
			TM_call(hModCB, bits,				\
				(void (*)(void *, void *)) pFunc,	\
				hParam1, hParam2)

/*
 * Don't call these functions directly.  Use the upper-case macros, defined
 * above.  By doing so, you don't need #ifdef TM_ENABLED around each use.  The
 * macros take care of stripping the code when TM_ENABLED is false.
 *
 * These are defined here solely to aid the compiler in giving you nice error
 * messages.
 */

PUBLIC Void
TM_init (void);

QU_Head *
TM_getHead(void);

PUBLIC SuccFail
TM_config(Char *tmFileName);

PUBLIC tm_ModInfo *
TM_open(Char *moduleName);

PUBLIC SuccFail
TM_close();

PUBLIC SuccFail
TM_setMask(Char *moduleName, TM_Mask moduleMask);

PUBLIC Bool
tm_trace(tm_ModInfo *modInfo, TM_Mask mask, Char *format, ...);

PUBLIC Bool
TM_query(tm_ModInfo *modInfo, TM_Mask mask);

PUBLIC SuccFail
TM_validate(void);

PUBLIC Int
TM_setUp(Char * str);

PUBLIC LgInt
TM_hexDump(tm_ModInfo *modInfo, TM_Mask mask, String str, 
  	   unsigned char *address, Int length);

PUBLIC Void
TM_call(tm_ModInfo *modInfo,
	TM_Mask mask,
	void (* pfCallMe)(void * hParam1, void * hParam2),
	void * hParam1,
	void * hParam2);

#else /* not TM_ENABLED */

/*
 * Public interface to the TM module
 */

#define	TM_INIT()
#define	TM_GETHEAD
#define	TM_CONFIG(pFileName)		(SUCCESS)
#define	TM_SETUP(optArg)		(SUCCESS)
#define	TM_OPEN(hModCB, modName)	((tm_ModInfo *) 1)
#define TM_CLOSE()
#define	TM_TRACE(args)
#define	TM_VALIDATE()			(SUCCESS)
#define	TM_HEXDUMP(hModCB, bits, msg, pdu, len)
#define	TM_CALL(hModCB, bits, pFunc, hParam1, hParam2)

#endif /* TM_ENABLED */


#endif /* _TM_H_ */
