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

/*
 * Author: Derrell Lipman
 * History:
 *
 */

#ifndef __STRING_H__
#define	__STRING_H__

#include "estd.h"

typedef enum
{
    STR_RC_MaxLengthExceeded			= (1 | ModId_Str)
} STR_ReturnCode;


typedef void * STR_String;


/* Length indicator for null-terminated strings */
#define	STR_ZSTRING	0xffff



ReturnCode
STR_alloc(OS_Uint16 len,
	  STR_String * phString);

ReturnCode
STR_attachString(OS_Uint16 maxLen,
		 OS_Uint16 len,
		 unsigned char * pOctets,
		 OS_Boolean toBeFreed,
		 STR_String * phString);

ReturnCode
STR_attachZString(char * pZString,
		  OS_Boolean toBeFreed,
		  STR_String * phString);

void
STR_addReference(STR_String hString);

void
STR_free(STR_String hString);

ReturnCode
STR_assignString(STR_String hString,
		 OS_Uint16 len,
		 unsigned char * pOctets);

void
STR_replaceString(STR_String * phDest,
		  STR_String hSrc);

ReturnCode
STR_appendString(STR_String hString,
		 OS_Uint16 len,
		 unsigned char * pOctets);

ReturnCode
STR_assignZString(STR_String * phString,
		  char * pZString);

unsigned char *
STR_stringStart(STR_String hString);

unsigned char *
STR_stringEnd(STR_String hString);

OS_Uint16
STR_getMaxLength(STR_String hString);

OS_Uint16
STR_getStringLength(STR_String hString);

ReturnCode
STR_setStringLength(STR_String hString,
		    OS_Uint16 len);

OS_Boolean
STR_bitStringGetBit(STR_String hString,
		    OS_Uint16 bit);

ReturnCode
STR_bitStringSetBit(STR_String hString,
		    OS_Uint16 bit,
		    OS_Boolean value);

void
STR_dump(unsigned char * p,
	 OS_Uint16 len,
	 OS_Uint16 indentLevel);

#endif /* __STRING_H__ */
