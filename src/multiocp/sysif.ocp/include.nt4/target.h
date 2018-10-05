/*
 * Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 * Copyright (C) 1995  Neda Communications, Inc. All rights reserved.
 *
 * This software is furnished under a license and use, duplication,
 * disclosure and all other uses are restricted to the rights specified
 * in the written license between the licensee and copyright holders.
 *
*/
/*+
 * File name: target.h
 *
 * Description: Target machine specific definitions
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: target.h,v 1.2 1999/09/30 00:04:51 mohsen Exp $
 */

#ifndef _TARGET_H_	/*{*/
#define _TARGET_H_

#define FIXED_TIMERS
#define TIME_AVAILABLE

/*
 * Mode of Interprocess communication to be used
 */

/*
 * Period of one interrupt clock tick, in milliseconds. Must be modulo
 * 54 for MSDOS.
 */
#define CLOCK_PERIOD		1080	/* Clock period in miliseconds */
#define NUMBER_OF_TIMERS	0	/* No preallocation of timers */

#define MAX_SCH_TASKS		100	/* Maximum number of SCH tasks */

#define MAX_SAPS		4	/* # of service access points */

#define MAXBFSZ			1528	/* maxPdu=1500; leave some extra */
#define BUFFERS			20	/* number of buffers */
#define VIEWS			40	/* number of views */

#define IMQ_ELEMENTS 50			/* Used for IMQ_SIMU */

#define UDP_PO_

#if !defined(NO_UPSHELL) && !defined(IMQ_SIMU)
Hey! one of NO_UPSHELL or IMQ_SIMU must be defined!
#endif

#ifdef DEVICE_DEMO
#define AUTHENTICATE_DOMAIN
#endif

#endif	/*}*/
