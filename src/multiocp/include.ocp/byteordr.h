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
 * File name: byteorder.h
 *
 * Description: Byte Order aware i/o
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: byteordr.h,v 1.7 1996/03/15 01:41:15 mohsen Exp $
 */

#ifndef _BYTEORDER_H_ /*{*/
#define _BYTEORDER_H_

#include "oe.h"

/* 
 * BYTE ORDER (BO_):
 * BO_ facilities define an interface for machine independent presentation
 * of:
 *	Octets, 16 bit Values, 32 bit Values and Octet Strings.
 *
 * These facilities can be used to convert CPU presentation
 * of values to a machine independent byte ordering and vice versa.
 * Most Significant Byte (MSB) of a value is always at the higher address.
 *
 *  Get and Put verbs are with respect to the network.
 *  Get always gets from the network.
 *  Get always has the CPU as the first argument.
 *  Put always puts into the network.
 *  Put always has the NET as the first argument.
 *
 *  NET is always incremented.
 * 
 * ENVIRONMENT DEPENDENCIES:
 * The following macros are VERY environment specific.
 * Great care must be taken to provide the services expected
 * from these facilities.
 * In addition to the CPU specific presentation of 2 byte and 4 byte
 * values, different compilers my handle BO_Swap differently.
 * Verify the compilers generated code before using these facilities.
 *
 */

/*
 *  Use a #define to identify the proper architecture.
 *  The following symbols are currently defined:
 *	INTEL	for 8086 type architectures (8088, 80186, 80188, 80286)
 *	VAX	for DEC VAX mini-computers.
 *	M68K	for Motorola MC68000 family processors
 *	NS16	for National Semiconductor NS16000 series processors.
 */

/*
 * "union BO_Swap swap;"
 * Should be defined for BO_{put2,get2,put4,get4} macros
 * before they can be used.
 */
typedef union Swap {
    unsigned long longVal;
    unsigned short shortVal;
    unsigned char b[4];
} Swap;

typedef union BO_Swap {
    unsigned long longVal;
    unsigned short shortVal;
    unsigned char b[4];
} BO_Swap;

/* BO_{put1,puts,get1,gets} are really machine independent and
 * don't deal with CPU type.
 */

#define BO_put1(netPtr, cpuValue)	{*netPtr++ = cpuValue;}
#define BO_get1(cpuValue, netPtr)	{cpuValue = *netPtr++;}

#define BO_putN(netPtr, cpuPtr, n) {BS_memCopy(cpuPtr, netPtr, n); netPtr += n;}
#define BO_getN(cpuPtr, netPtr, n) {BS_memCopy(netPtr, cpuPtr, n); netPtr += n;}


#ifdef INTEL
#define BO_put2(netPtr, cpuValue)	{Swap sw;\
					 sw.shortVal=cpuValue;\
					 *netPtr++=sw.b[1];\
    					 *netPtr++=sw.b[0];}
#define BO_put4(netPtr, cpuValue)	{Swap sw;\
					 sw.longVal=cpuValue;\
					 *(netPtr)++=sw.b[3];\
					 *(netPtr)++=sw.b[2];\
					 *(netPtr)++=sw.b[1];\
					 *(netPtr)++=sw.b[0];} 
#endif
#ifdef VAX
#define BO_put2(netPtr, cpuValue)  {Swap sw; sw.shortVal = cpuValue; \
	*netPtr++ = sw.b[1]; *netPtr++ = sw.b[0];}
#define BO_put4(netPtr, cpuValue)  {Swap sw; sw.longVal = cpuValue; \
	*netPtr++ = sw.b[3]; *netPtr++ = sw.b[2];\
	*netPtr++ = sw.b[1]; *netPtr++ = sw.b[0];}
#endif
#ifdef M68K
#define BO_put2(netPtr, cpuValue)	sw.shortVal = cpuValue; \
			*netPtr++ = sw.b[0]; *netPtr++ = sw.b[1]
#define BO_put4(netPtr, cpuValue)	sw.longVal = cpuValue; \
			*netPtr++ = sw.b[0]; *netPtr++ = sw.b[1];\
			*netPtr++ = sw.b[2]; *netPtr++ = sw.b[3]
#endif
#ifdef NS16
#define BO_put2(netPtr, cpuValue)	sw.shortVal = cpuValue; \
			*netPtr++ = sw.b[1]; *netPtr++ = sw.b[0]
#define BO_put4(netPtr, cpuValue)	sw.longVal = cpuValue; \
			*netPtr++ = sw.b[3]; *netPtr++ = sw.b[2];\
			*netPtr++ = sw.b[1]; *netPtr++ = sw.b[0]
#endif

#ifdef INTEL
#define BO_get2(cpuValue, netPtr)	{Swap sw;\
					 sw.b[1] = *netPtr++;\
					 sw.b[0] = *netPtr++; \
					 cpuValue = sw.shortVal;}
#define BO_get4(cpuValue, netPtr)	{Swap sw;\
					 sw.b[3] = *netPtr++;\
					 sw.b[2] = *netPtr++;\
					 sw.b[1] = *netPtr++;\
					 sw.b[0] = *netPtr++;\
					 cpuValue = sw.longVal;}
#endif
#ifdef VAX
#define BO_get2(cpuValue, netPtr)   {Swap sw; sw.b[1] = *netPtr++; \
	sw.b[0] = *netPtr++; cpuValue = sw.shortVal;}
#define BO_get4(cpuValue, netPtr)   {Swap sw; sw.b[3] = *(netPtr)++; \
	sw.b[2] = *(netPtr)++; sw.b[1] = *(netPtr)++; sw.b[0] = *(netPtr)++; \
	(cpuValue) = sw.longVal;}
#endif
#ifdef M68K
#define BO_get2(cpuValue, netPtr)	sw.b[0] = *netPtr++; sw.b[1] = *netPtr++; \
			cpuValue = sw.shortVal
#define BO_get4(cpuValue, netPtr)	sw.b[0] = *netPtr++; sw.b[1] = *netPtr++; \
			sw.b[2] = *netPtr++; sw.b[3] = *netPtr++; \
			cpuValue = sw.longVal
#endif
#ifdef NS16
#define BO_get2(cpuValue, netPtr)	sw.b[1] = *netPtr++; sw.b[0] = *netPtr++; \
			cpuValue = sw.shortVal
#define BO_get4(cpuValue, netPtr) {Swap sw; sw.b[3] = *netPtr++; \
	sw.b[2] = *netPtr++;sw.b[1] = *netPtr++; \
	sw.b[0] = *netPtr++; cpuValue = sw.longVal};
#endif

#endif	/*}*/
