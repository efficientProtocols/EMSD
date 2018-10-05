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
 * File name: oe.h
 *
 * Description:
 *   Operating Environment Specification: 
 *   CPU, Operating System, Compiler and such.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: oe.h,v 1.13 1996/11/21 19:20:15 kamran Exp $
 */


#ifndef _OE_H_	/*{*/
#define _OE_H_

/* Include system header files in case they define what we would redeinfe */
#include "estd.h"

/* 
 * CPU.
 * Used for byte order representation of integers.
 * Supported CPUs are:
 *	INTEL	for 8086 family (8088, 80186, 80188, 80286, 80386)
 *	VAX	for DEC VAX mini-computers.
 *	M68K	for Motorola MC68000 family processors
 *	NS16	for National Semiconductor NS16000 series processors.
 */
#define INTEL

/* 
 * Operating System.
 * Used for OS specific interprocess communication and
 * other OS specific facilities.
 * Supported Operating Systems are:
 *	UNHOSTED    No operating system at all.
 *	UNIX	    
 *	    SYSV    System V Unix (release {2,3}.
 *	    BSD	    Berkeley  Unix (release 4.{2,3}.
 *      VMS
 *	MSDOS	
 */
#define UNIX
#ifndef BSD
# define BSD
#endif

/*
 * Compiler.
 * Used for compiler specific options.
 * Supported Compilers are:
 *	KANDR	    Traditional C Language Definitions. (Default)
 *	X3J11	    New standard definition.
 */
#define LINT_ARGS
/* #define __STDC__ */
#define X3J11

#endif	/*}*/
