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
 * File name: local.h (FSM module)
 *
 * Description: Finit State Machine specific machine declaration
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: local.h,v 1.4 1999/09/30 00:01:53 mohsen Exp $
 */

#ifndef _LOCAL_H_
#define _LOCAL_H_

typedef struct fsm_Machine {
    QU_ELEMENT;
    Void *transDiag;
    struct FSM_State *curState;
    Void *userData;
} fsm_Machine;

#endif	/* _LOCAL_H_ */
