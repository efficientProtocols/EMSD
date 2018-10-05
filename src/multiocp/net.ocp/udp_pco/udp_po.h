/*
 * 
 * Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 *
 * This software is furnished under a license and use, duplication,
 * disclosure and all other uses are restricted to the rights specified
 * in the written license between the licensee and copyright holders.
 *
*/
/*+
 * File name: udp_po.h
 *
 * Description: UDP point of observation log generator
 *
-*/

/*
 * Author: Mohsen Banan, Hugh Shane
 * History:
 *
 */

/*
 * RCS Revision: $Id: udp_po.h,v 1.8 1999/09/30 00:04:25 mohsen Exp $
 */

#ifndef _UDP_PO_H_    /*{*/
#define _UDP_PO_H_

#include "target.h"
#include "du.h"
#include "addr.h"

#define UDP_PO_PDU_IN   1
#define UDP_PO_PDU_OUT  2

extern Bool UDP_noLogSw;

typedef struct UDP_PO_LogRecord {
	short magic; 		/* magic number = 0x4711 */
	short size;		/* size of this log entry */
	short code;		/* the code for this entry */
	long int tmx;		/* time stamp */
} UDP_PO_LogRecord;

/* Log code descriptors */
#define IN_PDU    	1
#define OUT_PDU   	2
#define STAMP_LONG 	3
#define STAMP		4	/* string for SI stamp command */
#define EOTSDU      	5

#ifdef LINT_ARGS	/* Arg. checking enabled */

extern SuccFail UDP_PO_init(String, String);
SuccFail UDP_PO_logIn(Byte*, Int, N_SapAddr *);
SuccFail UDP_PO_logOut(Byte*, Int, N_SapAddr *);
SuccFail UDP_PO_errIn(Byte*, Int);
SuccFail UDP_PO_errOut(Byte*, Int);

#else

SuccFail UDP_PO_init();
SuccFail UDP_PO_logIn();
SuccFail UDP_PO_logOut();
SuccFail UDP_PO_errIn();
SuccFail UDP_PO_errOut();

#endif /* LINT_ARGS */

#endif /*}*/
