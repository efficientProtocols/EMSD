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
 * RCS Revision: $Id: imq_sysv.h,v 1.3 1999/09/30 00:04:45 mohsen Exp $
 */

#ifndef _IMQ_H_	/*{*/
#define _IMQ_H_

#ifndef M_I86
#define far
#define near
#endif

typedef int IMQ_PrimDesc;
typedef int IMQ_DataDesc;

typedef key_t IMQ_Key;
typedef struct IMQ_PrimMsg {
    long mtype;
    char mtext[1];
} IMQ_PrimMsg;

#ifdef LINT_ARGS	/* Arg. checking enabled */

/* imq.c */
PUBLIC int IMQ_init();
PUBLIC int imq_exit();
PUBLIC char  *IMQ_nameCreate();
PUBLIC IMQ_Key IMQ_keyCreate();
PUBLIC IMQ_PrimDesc IMQ_primGet();
PUBLIC int IMQ_primSnd();
PUBLIC int IMQ_primRcv();
PUBLIC void IMQ_primDelete();
PUBLIC IMQ_DataDesc IMQ_dataGet();
PUBLIC int IMQ_dataSnd();
PUBLIC char far *IMQ_dataRcv();
PUBLIC int IMQ_dataRcvComplete();
PUBLIC void IMQ_dataDelete();
PUBLIC int farToHereCopy();
PUBLIC int hereToFarCopy();

#else

/* imq.c */
PUBLIC int IMQ_init();
PUBLIC int imq_exit();
PUBLIC char  *IMQ_nameCreate();
PUBLIC IMQ_Key IMQ_keyCreate();
PUBLIC IMQ_PrimDesc IMQ_primGet();
PUBLIC int IMQ_primSnd();
PUBLIC int IMQ_primRcv();
PUBLIC void IMQ_primDelete();
PUBLIC IMQ_DataDesc IMQ_dataGet();
PUBLIC int IMQ_dataSnd();
PUBLIC char far *IMQ_dataRcv();
PUBLIC int IMQ_dataRcvComplete();
PUBLIC void IMQ_dataDelete();
PUBLIC int farToHereCopy();
PUBLIC int hereToFarCopy();

#endif /* LINT_ARGS */

#endif	/*}*/
