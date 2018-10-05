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
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: imq_bsd.h,v 1.11 1999/09/30 00:04:20 mohsen Exp $
 */

#ifndef _IMQ_H_	/*{*/
#define _IMQ_H_

#include "estd.h"

#ifdef TM_ENABLED

#include "tm.h"
extern TM_ModuleCB *IMQ_modCB;
/*
 *  Trace flag definitions
 */
#define	IMQ_TRACE_ERROR   	(1 << 0)
#define	IMQ_TRACE_WARNING   	(1 << 1)
#define	IMQ_TRACE_ACTIVITY	(1 << 2)
#define	IMQ_TRACE_DETAIL   	(1 << 3)
#define	IMQ_TRACE_INIT		(1 << 4)
#define	IMQ_TRACE_VALIDATION	(1 << 5)
#define	IMQ_TRACE_PREDICATE	(1 << 6)
#define	IMQ_TRACE_STATE  	(1 << 8)
#define	IMQ_TRACE_ADDRESS	(1 << 7)
#define	IMQ_TRACE_PDU		(1 << 9)

#endif

#define SP_CHNLNAMELEN	64

typedef int IMQ_PrimDesc;
typedef int IMQ_DataDesc;

typedef char *IMQ_Key;
typedef struct IMQ_PrimMsg {
    long mtype;
    char mtext[1];
} IMQ_PrimMsg;

#ifdef LINT_ARGS	/* Arg. checking enabled */

/* imq.c */
SuccFail IMQ_init(Int );
Void     imq_exit(int );
char *   IMQ_nameCreate(void );
IMQ_Key  IMQ_keyCreate(String name, Int subID);
IMQ_PrimDesc IMQ_primGet(IMQ_Key key);

int   IMQ_primSnd(int ,struct IMQ_PrimMsg  *,int );
int   IMQ_primRcv(int ,struct IMQ_PrimMsg  *,int );
Void  IMQ_primDelete(int, char * );
IMQ_DataDesc IMQ_dataGet(IMQ_Key key, Int size);
int   IMQ_dataSnd(int ,char  *,int );
char *IMQ_dataRcv(int ,int  *);
int   IMQ_dataRcvComplete(int ,char *);
void  IMQ_dataDelete(int );
int   farToHereCopy(char *,char *,int );
int   hereToFarCopy(char  *,char *,int );
PUBLIC IMQ_PrimDesc IMQ_acceptConn(IMQ_Key key);
IMQ_PrimDesc IMQ_connect(IMQ_PrimDesc queue);
Void *IMQ_blockQuAdd(IMQ_PrimDesc quDesc);
Void  IMQ_blockQuRemove(IMQ_PrimDesc quDesc);

#else

/* imq.c */
PUBLIC int    IMQ_init();
PUBLIC Void   imq_exit();
PUBLIC char  *IMQ_nameCreate();
PUBLIC IMQ_Key IMQ_keyCreate();
PUBLIC int    IMQ_primGet();
PUBLIC int    IMQ_primSnd();
PUBLIC int    IMQ_primRcv();
PUBLIC void   IMQ_primDelete();
PUBLIC int    IMQ_dataGet();
PUBLIC int    IMQ_dataSnd();
PUBLIC char * IMQ_dataRcv();
PUBLIC int    IMQ_dataRcvComplete();
PUBLIC void   IMQ_dataDelete();
PUBLIC IMQ_PrimDesc IMQ_acceptConn();
PUBLIC IMQ_PrimDesc IMQ_connect();
PUBLIC Void * IMQ_blockQuAdd();
PUBLIC Void   IMQ_blockQuRemove();

#endif /* LINT_ARGS */

#endif	/*}*/
