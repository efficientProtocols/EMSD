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
 * File name: psq.h
 *
 * Description: Service Provider Shell Queue Interface (PSQ_)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: psq.h,v 1.8 1999/09/30 00:04:22 mohsen Exp $
 */

#ifndef _PSQ_H_
#define _PSQ_H_

#include "tm.h"
#include "queue.h"
#include "imq.h"
#include "sp_shell.h"

typedef struct PSQ_PrimQuInfo {
    struct PSQ_PrimQuInfo *next;
    struct PSQ_PrimQuInfo *prev;
    char chnlName[SP_CHNLNAMELEN];
    IMQ_PrimDesc  actionQu;
    IMQ_PrimDesc  eventQu;
    struct PSQ_QuRegPrvdrSeq *quRegPrvdrSeq;
} PSQ_PrimQuInfo;

typedef struct PSQ_QuRegPrvdrSeq {
    struct PSQ_QuRegPrvdr *first;
    struct PSQ_QuRegPrvdr *last;
} PSQ_QuRegPrvdrSeq; 

typedef struct PSQ_QuRegPrvdr {
    struct PSQ_QuRegPrvdr *next;
    struct PSQ_QuRegPrvdr *prev;
    Int lowerEnd;
    Int upperEnd;
    Int (*procAction)();
    Int (*cleanSap)();
} PSQ_QuRegPrvdr; 

/* Provider's action queue can carry 4096 bytes (plus IMQ primitive header) */
#define MAXACTIONQU (sizeof(IMQ_PrimMsg)+4096)

#ifdef LINT_ARGS	/* Arg. checking enabled */

void PSQ_init(void);
void PSQ_poll(void);
int PSQ_putEvent(struct PSQ_PrimQuInfo  *,unsigned char  *,int );
struct PSQ_PrimQuInfo * PSQ_getQuInfo(char *);
int PSQ_procActionQu(struct IMQ_PrimMsg  *,int, PSQ_PrimQuInfo * );
int PSQ_primQuInit(int );
int PSQ_primQuAdd(struct SP_Action  *);
int PSQ_primQuRemove(struct PSQ_PrimQuInfo  *);
PUBLIC  Int PSQ_procPubQu(Ptr data, Int size);

#else

EXTERN Void PSQ_init();
EXTERN Void PSQ_poll();
EXTERN SuccFail PSQ_putEvent();
EXTERN  PSQ_PrimQuInfo *PSQ_getQuInfo();
EXTERN SuccFail PSQ_procActionQu();
EXTERN SuccFail PSQ_primQuInit();
EXTERN int PSQ_primQuAdd();
EXTERN int  PSQ_primQuRemove();
EXTERN Int PSQ_procPubQu();

#endif /* LINT_ARGS */

#endif	/* _PSQ_H_ */
