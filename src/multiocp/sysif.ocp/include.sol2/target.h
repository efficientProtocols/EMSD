/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 *
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 *
*/
/*+
 * File name: target.h
 *
 * Description: Target machine specific definitions
 *
-*/

/*
 * 
 * History:
 * 
 */

/*
 * RCS Revision: $Id: target.h,v 1.28 2002/10/25 19:37:56 mohsen Exp $
 */

#ifndef _TARGET_H_	/*{*/
#define _TARGET_H_

#include "oe.h"
#include "du.h"
#include "malloc.h"

/* 
 * System Wide Buffer and Timer Specifications.
 */

#define FIXED_TIMERS
#define TIME_AVAILABLE

/* 
 * Compile options:
 *   MTS_COMPILE_COMPLETE
 *   UA_COMPILE_2WAY_ONLY
 *   UA_COMPILE_3WAY_ONLY
 *   UA_COMPILE_2WAY_AND_3WAY
 *   DELIVERY_CONTROL
 *
 * #define AUTHENTICATE_DOMAIN		Domain authentication (neda.com)
 */

/*
 * Critical Section Protection Mechanism.
 */

#ifndef MSDOS /*{*/
#ifndef ENTER
#define ENTER();
#endif
#ifndef LEAVE
#define LEAVE();
#endif
#else /*}{*/
#define ENTER() {extern volatile int isrActive;if(!isrActive)_disable();}
#define LEAVE() {extern volatile int isrActive;if(!isrActive)_enable();}
#endif /*}*/

/*
 * Mode of Interprocess communication to be used
 * IMQ_SYSV, IMQ_SIMU, IMQ_BSD
 */

/*
 * Clock period in miliseconds
 */
#ifdef MSDOS
#define IMQ_SIMU
#define CLOCK_PERIOD  54
#define NUMBER_OF_TIMERS 100

#define MAX_SCH_TASKS 100
#define MAX_SAPS 4
#define IMQ_ELEMENTS 50

/*#define MAXBFSZ	1528 */
#define MAXBFSZ	2048 
#define BUFFERS 8
#define VIEWS   16
#define IMQ_QUEUES   100
#endif

#ifdef UNIX
#define IMQ_BSD	/* this will go away, keep for now so OCP will build */
#define CLOCK_PERIOD  1000
#define NUMBER_OF_TIMERS 500

#define MAX_SCH_TASKS 500
#define MAX_SAPS 4
#define IMQ_ELEMENTS 50

/*#define MAXBFSZ	1528 */
#define MAXBFSZ	16384 
#define BUFFERS 20
#define VIEWS   40
#define IMQ_QUEUES   100
/* #define DELIVERY_CONTROL */
#endif

/*
 * UDP point of control and observation conditional compilation
 *
 * #define UDP_PC_ turns on the PDU inhibit feature
 * #define UDP_PO_ enables PDU logging
 *
 */
/*#define UDP_PC_*/
#define UDP_PO_

#define NO_UPSHELL

#endif	/*}*/
