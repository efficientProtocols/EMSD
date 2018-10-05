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

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)bo_ex.c	1.1    Released: 13 Mar 1990";
#endif /*}*/

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: bo_ex.c,v 1.3 1996/11/07 02:46:32 kamran Exp $";
#endif /*}*/

#include "estd.h"
#include "byteordr.h"

typedef union CupPresentation {
    LgUns lgUns;
    MdUns mdUns;
    Byte b[4];
} CpuPresentation;

CpuPresentation cpuPres;

LgUns lgUns;
MdUns mdUns;

Byte b[4] = {0x12, 0x34, 0x56, 0x78};


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

Int
main()
{
    Byte *p;

    cpuPres.lgUns = 0x87654321;
    printf("Internal CPU presentation of 0x%08lx is:\n", cpuPres.lgUns);
    printf("b[0]=0x%02x, b[1]=0x%02x, b[2]=0x%02x, b[3]=0x%02x\n",
	    cpuPres.b[0], cpuPres.b[1], cpuPres.b[2], cpuPres.b[3]);
    cpuPres.mdUns = 0x4321;
    printf("Internal CPU presentation of 0x%04x is:\n", cpuPres.mdUns);
    printf("b[0]=0x%02x, b[1]=0x%02x\n\n",
	    cpuPres.b[0], cpuPres.b[1]);

    printf("Given: ");
    printf("b[0]=0x%02x, b[1]=0x%02x, b[2]=0x%02x, b[3]=0x%02x,\n",
	    b[0], b[1], b[2], b[3]);
    p = b;
    BO_get2(mdUns, p);
    p = b;
    BO_get4(lgUns, p);
    printf("BO_get2 produces mdUns=0x%04x and BO_get4 produces lgUns=0x%08lx\n",
	     mdUns, lgUns);

    lgUns = 0x87654321;
    p = b;
    BO_put4(p, lgUns);
    printf("BO_put4 of 0x%08lx results to:", lgUns);
    printf("b[0]=0x%02x, b[1]=0x%02x, b[2]=0x%02x, b[3]=0x%02x\n",
	    b[0], b[1], b[2], b[3]);

    mdUns = 0x4321;
    p = b;
    BO_put2(p, mdUns);
    printf("BO_put2 of 0x%04x results to:", mdUns);
    printf("b[0]=0x%02x, b[1]=0x%02x\n", b[0], b[1]);
    exit(0);
}

