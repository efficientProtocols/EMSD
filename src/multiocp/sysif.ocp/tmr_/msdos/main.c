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
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)main.c	1.1    Released: 2/4/88";
#endif /*}*/

/* #includes */
#include <stdio.h>
#include <signal.h>
#include  "estd.h"
#include "tmr.h"

/* 
 * Simple Function to handle a timer
 */
tmrExpirationHandler(tmrData)
Byte *tmrData;
{
    TMR_Desc tmrDesc;
    Byte *newData;

    printf("*tmrData=%x, freeCount=%ld\n", *tmrData, TMR_getFreeCnt());

    tmrDesc = TMR_create((LgInt)3000, tmrExpirationHandler);
    newData = TMR_getData(tmrDesc);
    *newData = 30;
}



/*<
 * Program Entry Point.
>*/
main(argc, argv)
int argc;
char *argv[];
{
    TMR_Desc tmrDesc;
    Byte *tmrData;
    Int i=0;

    signal(SIGINT, TMR_stopClockInterrupt); 	/* Interrupt */

#define CLOCK_PERIOD	54		/* Milliseconds */
    TMR_init(100, CLOCK_PERIOD);
    TMR_startClockInterrupt(CLOCK_PERIOD);

    tmrDesc = TMR_create((LgInt)4000, tmrExpirationHandler);
    tmrData = TMR_getData(tmrDesc);
    *tmrData = 40;

    printf("Starting, freeCount=%ld\n", TMR_getFreeCnt());

    while ( TRUE ) {
	if (i >= 20000) {
	    printf("Polling, freeCount=%ld\n", TMR_getFreeCnt());
	    i = 0;
	}
	++i;
	TMR_poll();
    }
}

