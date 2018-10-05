/*+
 * Description:
 *     SPA_ Synchronous Phone Port Application
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * SCCS Revision: @(#)spa.h	1.1    Released: 6/13/90
 */

#ifndef _SPA_H_     /*{*/
#define _SPA_H_


#ifndef PP_H_
#include "pp.h"
#endif

typedef enum SPA_PlayResult {
    SPA_PlayEOF	= 1,
    SPA_PlayDtmfTerm = 2,		/* Terminating DTMF was observed */
    SPA_PlayDtmfNu = 3,			/* Specified Number was observed */
    SPA_PlayFail = FAIL
} SPA_PlayResult;


SuccFail SPA_vMsgDoaminSet();
Char *   SPA_vMsgNameGet();
Void SPA_vMsgDomainSet(String domain);


/* Void SPA_vMsgPlayN();
Void SPA_dtmfGenerate();
Void SPA_dtmfWaitFor();
Void SPA_mfWaitFor(); */

extern SPA_PlayResult SPA_vMsgPlayN (PP_PortDesc portDesc, String vMsg, Int nuDtmfs);
extern SPA_PlayResult SPA_wordSeqPlayN (PP_PortDesc portDesc, String wordSeq, Int nuDtmfs);
extern int SPA_dtmfWaitFor (PP_PortDesc portDesc, Int nuDtmfs, String dtmfTerms, Int wait, Char *buf, Char *bufEnd);
extern int SPA_mfWaitFor (PP_PortDesc portDesc, Int nuDtmfs, String dtmfTerms, Int wait, Char *buf, Char *bufEnd);
extern int SPA_dtmfGenerate (PP_PortDesc portDesc, String dtmfStr);
 
#endif   	/*}*/
