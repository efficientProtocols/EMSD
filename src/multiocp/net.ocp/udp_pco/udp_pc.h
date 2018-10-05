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
 * File: udp_pc.h
 *
 * Description: UDP Point of Control
 *
-*/

/*
 * Author: Mohsen Banan, Hugh Shane
 * History:
 * 
 */

/*
 * RCS Revision: $Id: udp_pc.h,v 1.4 1999/09/30 00:04:24 mohsen Exp $
 */

#ifndef _UDP_PC_H_    /*{*/
#define _UDP_PC_H_

#include "target.h"

#define UDP_PC_INHIBIT_SEND        0
#define UDP_PC_INHIBIT_RECEIVE     1

#ifdef LINT_ARGS	/* Arg. checking enabled */

extern SuccFail UDP_PC_inhibit(Int, Int);
extern SuccFail UDP_PC_lossy(Int, Int);
extern SuccFail UDP_PC_dataReqInhibit(Void);
extern SuccFail UDP_PC_receiveInhibit(Void);

#else

extern SuccFail UDP_PC_inhibit();
extern SuccFail UDP_PC_lossy();
extern SuccFail UDP_PC_dataReqInhibit();
extern SuccFail UDP_PC_receiveInhibit();

#endif /* LINT_ARGS */

#endif /*}*/







