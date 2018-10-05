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
 * File name: imq_simu.h
 *
 * Description:
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: imq_simu.h,v 1.6 1999/09/30 00:04:21 mohsen Exp $
 */

#ifndef _IMQ_H_	/*{*/
#define _IMQ_H_

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

#define SP_CHNLNAMELEN	64

typedef Ptr IMQ_PrimDesc;
typedef Ptr IMQ_DataDesc;

typedef Char * IMQ_Key;
typedef struct IMQ_PrimMsg {
    long mtype;
    char mtext[1];
} IMQ_PrimMsg;

#ifdef LINT_ARGS	/* Arg. checking enabled */

/* imq.c */
int    IMQ_init(int );
void   imq_exit(int );
char * IMQ_nameCreate(void);
char * IMQ_keyCreate(char  *,int );
char * IMQ_primGet(char  *);
int    IMQ_primSnd(char  *,struct IMQ_PrimMsg  *,int );
int    IMQ_primRcv(char  *,struct IMQ_PrimMsg  *,int );
PUBLIC Void IMQ_primDelete(IMQ_PrimDesc primDesc, char *chnlName);
char * IMQ_dataGet(char  *,int );
int    IMQ_dataSnd(char  *,char  *,int );
char * IMQ_dataRcv(char  *,int  *);
int    IMQ_dataRcvComplete(char  *,char  *);
void   IMQ_dataDelete(char  *);
Void * IMQ_blockQuAdd(IMQ_PrimDesc quDesc);
Void   IMQ_blockQuRemove(IMQ_PrimDesc quDesc);

#else

PUBLIC int     IMQ_init();
PUBLIC void    imq_exit();
PUBLIC char *  IMQ_nameCreate();
PUBLIC IMQ_Key IMQ_keyCreate();
PUBLIC IMQ_PrimDesc IMQ_primGet();
PUBLIC int     IMQ_primSnd();
PUBLIC int     IMQ_primRcv();
PUBLIC void    IMQ_primDelete();
PUBLIC IMQ_DataDesc IMQ_dataGet();
PUBLIC int     IMQ_dataSnd();
PUBLIC Ptr     IMQ_dataRcv();
PUBLIC int     IMQ_dataRcvComplete();
PUBLIC void    IMQ_dataDelete();
PUBLIC Int     IMQ_blockQuAdd();
PUBLIC Void    IMQ_blockQuRemove();

#endif /* LINT_ARGS */

#endif	/*}*/
