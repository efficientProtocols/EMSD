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
static char sccs[] = "@(#)fifo_ex.c	1.1    Released: 13 Mar 1990";
#endif /*}*/

/* #includes */
#include  <stdio.h>
#include  "eh.h"
#include  "estd.h"
#include  "queue.h"
#include  "fifo.h"

#define MAXBFSZ 30
typedef struct RxBuf {
    struct RxBuf *next;
    struct RxBuf *prev;
    Byte data[MAXBFSZ];
} RxBuf;

#define RXFIFOSIZE 5
FIFO_CtrlBlk  rxFifoCtrlBlk;
FIFO_Element rxBufFifo[RXFIFOSIZE+1];  	/* One waisted element */

QU_Head rxQuHead;	/* Producer for this Qu is the interrupt routine,
			 * consumer is MAC_poll.
			 */
RxBuf *curRxBuf;

main()
{
    init();
    
    /* Setup Interrupt Vectors and enable Interrupt */

    while (TRUE) {
	poll();
    }
}

/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Int
init()
{
    Int i;
    Int psw;
    
    /* rxBufFifo is continously maintained, so that we don't 
     * have to do allocs at interrupt time.
     */
    FIFO_init(&rxFifoCtrlBlk, rxBufFifo, ENDOF(rxBufFifo));

    for (i=0; i < DIMOF(rxBufFifo)-1; ++i) {
	RxBuf *bp;

	bp = (RxBuf *)malloc(sizeof(*bp));
	if ( FIFO_put(&rxFifoCtrlBlk, (FIFO_Element) bp) ) {
	    EH_oops();
	}
    }
    
    QU_init((QU_Element *) &rxQuHead);

    /* You need to get the first buffer for the interrupt routine,
     * This probably is not the best place for doing this.
     */
    {
	FIFO_Element *newBuf;    
	if ( ! (newBuf = FIFO_get(&rxFifoCtrlBlk)) ) {
	    EH_oops();
	}
	curRxBuf = (RxBuf *)*newBuf;
    }

}
 


/*<
 * Function:
 * Description:
 *	This function is the consumer for rxQuHead.
 *	Producer is the interrupt routine.
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Int
poll( )
{
    RxBuf *bp;
    Int psw; /* Process Status Word, if needed */
    
    /* Keep Receive Buffer Pool (rxBufFifo) full */
    psw = cx_dis_int(); /* Enter Critical Section */
    while ( ! FIFO_full(&rxFifoCtrlBlk) ) {
	cx_ena_int(psw); /* End Critical Section */
	bp = (RxBuf *) malloc(sizeof(*bp));
	psw = cx_dis_int(); /* Enter Critical Section */
	if ( FIFO_put(&rxFifoCtrlBlk, (FIFO_Element) bp) ) {
	    EH_oops();
	}
    }
    cx_ena_int(psw); /* End Critical Section */

    /* Process Any information that is available */
    psw = cx_dis_int(); /* Enter Critical Section */
    while ( (bp = (RxBuf *) rxQuHead.first) != (RxBuf *) &rxQuHead ) {
	QU_remove(bp);
	cx_ena_int(psw); /* End Critical Section */
	/*
	 * Process The Data here
	 * ...
	 */
	free(bp);
	psw = cx_dis_int(); /* Enter Critical Section */
    }
    cx_ena_int(psw); /* End Critical Section */
}

/*
 * Disable Interrupt
 */
cx_dis_int()
{
}

/*
 * Enable Interrupt
 */
cx_ena_int()
{
}

/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
interruptHandler(  )
{
    FIFO_Element *newBuf;

    /* curRxBuf is assumed to be available for recption */
    /* Read the Data into curRxBuf */

    /* Put it in the rxQuHead, so that poll()
     * can retrieve it.
     */
    QU_insert(curRxBuf, &rxQuHead);

    if ( ! (newBuf = FIFO_get(&rxFifoCtrlBlk)) ) {
	EH_oops();
    }
    curRxBuf = (RxBuf *)*newBuf;
}
