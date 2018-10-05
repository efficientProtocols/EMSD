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
 * File name: hw.h (Motorola 680X0 family portation)
 *
 * Description: Hareware-specific issues.
 *
-*/

/*
 * Author: Mohsen Banan / Derrell Lipman
 * History:
 * 
 */

/*
 * RCS Revision: $Id: hw.h,v 1.4 1996/11/21 19:23:49 kamran Exp $
 */

#ifndef _HW_H_
#define _HW_H_

/*
 * Tell 'em what type of hardware they're running on.
 */
#define	HW_TYPE		"M68K"
#define	HW_TYPE_M68K

/*
 * Hardware-specific Byte Ordering
 *
 * HW_ByteOrder_ facilities define an interface for machine independent
 * presentation of:
 *
 *      Octets, 16 bit Values, 32 bit Values and Octet Strings.
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
 * The following macros are VERY environment specific.  Great care must be
 * taken to provide the services expected from these facilities.  In addition
 * to the CPU specific presentation of 2 byte and 4 byte values, different
 * compilers my handle BO_Swap differently.  Verify the compilers generated
 * code before using these facilities.
 *
 */

/*
 * "union BO_Swap swap;"
 * Should be defined for BO_{put2,get2,put4,get4} macros
 * before they can be used.
 */
typedef union HW_Swap
{
    unsigned long   longVal;
    unsigned short  shortVal;
    unsigned char   b[4];
} HW_Swap;


#define HW_ByteOrder_put2(netPtr, cpuValue)	\
	{					\
	    HW_Swap sw;				\
	    sw.shortVal = cpuValue;		\
	    *netPtr++ = sw.b[0];		\
	    *netPtr++ = sw.b[1];		\
	}

#define HW_ByteOrder_put4(netPtr, cpuValue)	\
	{					\
	    HW_Swap sw;				\
	    sw.longVal = cpuValue;		\
	    *netPtr++ = sw.b[0];		\
	    *netPtr++ = sw.b[1];		\
	    *netPtr++ = sw.b[2];		\
	    *netPtr++ = sw.b[3];		\
	}

#define HW_ByteOrder_get2(cpuValue, netPtr)	\
	{					\
	    HW_Swap sw;				\
	    sw.b[0] = *netPtr++;		\
	    sw.b[1] = *netPtr++;		\
	    cpuValue = sw.shortVal;		\
	}

#define HW_ByteOrder_get4(cpuValue, netPtr)	\
	{					\
	    HW_Swap sw;				\
	    sw.b[0] = *netPtr++;		\
	    sw.b[1] = *netPtr++;		\
	    sw.b[2] = *netPtr++;		\
	    sw.b[3] = *netPtr++;		\
	    cpuValue = sw.longVal;		\
	}



/*
 * Method to use to display an address.
 */

#define	HW_prAddr(p, pBuf)				\
	sprintf(pBuf, "0x%lx", (unsigned long) (p))

#endif	/* _HW_H_ */
