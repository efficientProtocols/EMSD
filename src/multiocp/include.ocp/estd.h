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
 * File name: estd.h
 *
 * Description:
 *   The purpose of the standard definitions include file is to provide
 *   consistent data-type definitions across different environments
 *   and to improve readability by imposing consistency.
 *   Thus, a SmInt (small integer)  is always 8 bits, no matter which machine 
 *   it is compiled on. In the same, way a MdInt (medium integer) is always
 *   16 bits and LgInt (large integer) is always 32 bits.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: estd.h,v 1.8 1996/10/11 19:33:52 kamran Exp $
 */

#ifndef _ESTD_H_
#define _ESTD_H_

/* standard include files */
#include "os.h"
#include "hw.h"

#define SMSIZE	8	        /* 8 bits in "small" */
#define MDSIZE	16	        /* 16 bits in "medium" */
#define LGSIZE	32           	/* 32 bits in "large" */

typedef OS_Sbits8 	SmInt;	/* SMSIZE-bit signed integer */
typedef OS_Sbits16	MdInt;	/* MDSIZE-bit signed integer */
typedef OS_Sbits32 	LgInt;	/* LGSIZE-bit signed integer */

typedef OS_Ubits8 	SmUns;	/* SMSIZE-bit unsigned integer */
typedef OS_Ubits16 	MdUns;	/* MDSIZE-bit unsigned integer */
typedef OS_Ubits32 	LgUns;	/* LGSIZE-bit unsigned integer */

typedef unsigned char  SmBits;	/* SMSIZE-bit bit string */
typedef unsigned short MdBits;	/* MDSIZE-bit bit string */
typedef unsigned long  LgBits;	/* LGSIZE-bit bit string */

typedef int Int;		/* generic integer */
typedef unsigned int Uns;       /* generic unsigned integer */

typedef OS_Boolean Bool;	/* boolean, true or false variable */

/*
 * General-use definitions useful to everyone
 */

#ifndef NULL
#define	NULL	(0)
#endif


/*
 * For backwards compatibility with old versions of estd.h, we'll define
 * SUCCESS and FAIL to map to two of our new-style return codes.
 */
typedef int SuccFail;	    /* Typical return Values */
#undef SUCCESS
#undef FAIL

#define SUCCESS		Success
#define FAIL		OldFail

#undef TRUE
#undef FALSE
#define	FALSE		(0)
#define	TRUE		(! FALSE)

extern char *	__applicationName;

typedef unsigned char Byte;	/* SMSIZE, smallest unit of addressable store */
typedef char Char;		/* Single ASCII character */
typedef Char * String;		/* Pointer to null-terminated charSequence */

typedef char * Ptr;		/* pointer to arbitrary type */
typedef LgUns Arg; 		/* Uninterpreted LGSIZE-bit word */

#define ZPTR ""			/* Pointer to '\0', empty string */

#ifndef Void
# define Void void		/* null return type of procedures */
#endif

#define STATIC  static		/* Names not needed outside this src module  */

#ifndef LOCAL
# define LOCAL              /* Names not needed outside this software module */
#endif

#define LCL_XTRN extern     /* Names defined within this software module */

#ifndef PUBLIC
# define PUBLIC             /* Names needed outside this software module */
#endif

#define EXTERN  extern      /* Names defined outside this software module */

/*
 * Define a mechanism to indicate that a function (or variable) is
 * declared somewhere farther forward in a file.
 */
#define	FORWARD


#define DIMOF(array) (sizeof array / sizeof array[0])
#define ENDOF(array) (array - 1 + (sizeof array / sizeof array[0]))

#include "oe.h"

#endif	/* _ESTD_H_ */
