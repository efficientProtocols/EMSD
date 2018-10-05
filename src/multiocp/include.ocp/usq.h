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
 * File name: usq.h
 *
 * Description: User Shell Queue.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: usq.h,v 1.6 1999/09/30 00:04:25 mohsen Exp $
 */

#ifndef _USQ_H_
#define _USQ_H_

#include "tm.h"
#include "queue.h"
#include "imq.h"
#include "sp_shell.h"

typedef struct USQ_PrimQuInfo {
    struct USQ_PrimQuInfo *next;
    struct USQ_PrimQuInfo *prev;
    char chnlName[SP_CHNLNAMELEN];
    IMQ_PrimDesc  actionQu;
    IMQ_PrimDesc  eventQu;
} USQ_PrimQuInfo;

#ifdef LINT_ARGS	/* Arg. checking enabled */

void USQ_init(void);
struct USQ_PrimQuInfo  *USQ_primQuCreate(void);
int ps_primQuDelete(struct USQ_PrimQuInfo  *);
int USQ_putAction(struct USQ_PrimQuInfo  *,char  *,int );
int USQ_getEvent(struct USQ_PrimQuInfo  *,char  *,int );
PUBLIC SuccFail USQ_primQuRemove(USQ_PrimQuInfo *quInfo);

#else

PUBLIC Void USQ_init();
PUBLIC USQ_PrimQuInfo *USQ_primQuCreate();
PUBLIC Void USQ_primQuDelete();
PUBLIC SuccFail USQ_putAction();
PUBLIC Int USQ_getEvent();
PUBLIC SuccFail USQ_primQuRemove();

#endif /* LINT_ARGS */

#endif	/* _USQ_H_ */
