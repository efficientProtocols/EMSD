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

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)pp.h	1.1    Released: 6/13/90";
#endif /*}*/

#ifndef VM_H
#include "vm.h"
#endif

#include <estd.h>

#ifndef PP_H_
#define PP_H_

#define PP_K_DtmfNuMax	64
#define PP_CallerIDInfoBufMax		81
#define PP_CalledIDInfoBufMax		81
#define PP_CallerIDInfoNameMax		25
#define PP_CallerIDInfoNumberMax	25
#define PP_CallerIDInfoDateTimeMax	25

#define PP_Term    long int
#define PP_Num     int
#define PP_Dtmf    char
#define PP_Time    int
#define PP_MsgP    char
#define MAXLEN     10000

typedef struct PP_SapAddr {
    Byte addr[16];
    Int len;
} PP_SapAddr;

typedef Int PP_PortId;		/* Name By which the Phone Port is identified */

typedef Void *PP_PortDesc;	/* Maps onto PP_PortInfo internally */

typedef void *PP_SapDesc;

typedef struct PP_CallerIDInfo {
    Char nameBuf[PP_CallerIDInfoNameMax];
    Char numberBuf[PP_CallerIDInfoNumberMax];
    Char dateTimeBuf[PP_CallerIDInfoDateTimeMax];
} PP_CallerIDInfo;

typedef char PP_CalledIDInfo;

typedef enum PP_EventId {
    PP_EvtDisConInd,	/* No Data */
    PP_EvtErrorInd,
    PP_EvtOnHookCnf,	/* No Data */
    PP_EvtOffHookCnf,	/* No Data */
    PP_EvtRingInd,	/* No Data */
    PP_EvtPlayCnf,
    PP_EvtGetDtmfCnf,
    PP_EvtRecordCnf,
    PP_EvtDialCnf,
    PP_EvtGenDtmfCnf,    /* No Data */
    /* -- Triggering Events -- */
    PP_EvtTrigRingInd,
    PP_EvtTrigSilenceInd,
    PP_EvtTrigNoSilenceInd,
    PP_EvtTrigLoopCurrentInd,
    PP_EvtTrigNoLoopCurrentInd
} PP_EventId;


/* Bit Masks for trigger events, I like masks better than bit fields */
typedef MdUns PP_EventTriggerMask;

#define PP_EvtTrigRingMsk   0x01
#define PP_EvtTrigSilenceMsk  0x02
#define PP_EvtTrigNoSilenceMsk 0x04
#define PP_EvtTrigLoopCurrentMsk 0x08
#define PP_EvtTrigNoLoopCurrentMsk 0x10


typedef struct PP_ErrorInd {
    Int data;
} PP_ErrorInd;

typedef struct PP_PlayCnf {
    enum PP_PlayEnd {
	PP_PlayEOF,		/* End Of File Reached */
	PP_PlayDtmfTerm,	/* Terminating DTMF was observed */
	PP_PlayDtmfNu	/* Specified Number of DTMFs was observerd */
    } playEnd;
} PP_PlayCnf;

typedef struct PP_GetDtmfCnf {
    enum PP_DtmfEnd {
	PP_DtmfTimeOut,
	PP_DtmfTerm,		/* Terminating DTMF was observed */
	PP_DtmfNu	/* Specified Number of DTMFs was observerd */
    } dtmfEnd;
    Char buf[PP_K_DtmfNuMax];	
} PP_GetDtmfCnf;
    
typedef struct PP_RecordCnf {
    enum PP_RecordEnd {
	PP_RecordTimeOut,
	PP_RecordDtmfTerm		/* Terminating DTMF was observed */
    } recordEnd;
} PP_RecordCnf;

typedef struct PP_DialCnf {
    enum PP_DialEnd {
	PP_DialBusy,
	PP_DialConnected,
	PP_DialNoAnswer,
	PP_DialNoRing,
	PP_DialIntercept
    } dialEnd;
} PP_DialCnf;

typedef struct PP_Event {
    struct PP_Event *next;
    struct PP_Event *prev;
    PP_PortDesc portDesc;
    PP_EventId evtId;
    union EventData {
	PP_PlayCnf playCnf;
	PP_GetDtmfCnf getDtmfCnf;
	PP_RecordCnf recordCnf;
	PP_DialCnf dialCnf;
    } evtData;
    PP_Term term;
} PP_Event;

/*
 * Prototypes for file pp.c
 */


extern void
PP_init (void);
extern PP_SapDesc
PP_sapBind (PP_SapAddr *sapAddr);
extern SuccFail
PP_sapUnBind (PP_SapDesc sapDesc);
extern PP_PortDesc
PP_attach (PP_SapDesc sapDesc, PP_PortId portId, String serviceProviderName);
extern SuccFail
PP_detach (PP_PortDesc portDesc);
extern SuccFail
PP_sapEventGet (PP_SapDesc sapDesc, PP_Event *event);
extern SuccFail
PP_portEventGet (PP_PortDesc portDesc, PP_Event *event);
extern SuccFail
PP_onHook (PP_PortDesc portDesc);
extern SuccFail
PP_ringWait (PP_PortDesc portDesc, PP_Num nuOfRings);
extern SuccFail
PP_callerIDInfoGet (PP_PortDesc portDesc, PP_CallerIDInfo *callerIDInfo);
extern SuccFail
PP_calledIDInfoGet (PP_PortDesc portDesc, PP_CalledIDInfo *calledIDInfo);
extern SuccFail
PP_offHook (PP_PortDesc portDesc);
extern SuccFail
PP_playN (PP_PortDesc portDesc, VM_MsgDesc *vMsgDesc, PP_Num nuDtmfs, Bool typeahead);
extern SuccFail
PP_plaympN (PP_PortDesc portDesc, PP_MsgP *msgp, PP_Num nuDtmfs, Bool typeahead);
extern SuccFail
PP_getDtmfs (PP_PortDesc portDesc, PP_Num nuDtmfs, PP_Dtmf termDtmf, PP_Time wait);
extern SuccFail
PP_recordN (PP_PortDesc portDesc, VM_MsgDesc *vMsgDesc, PP_Time maxDuration, PP_Time maxSilence, PP_Num nuDtmfs);
extern SuccFail
PP_dial (PP_PortDesc portDesc, String phoneNu);
extern SuccFail
PP_genDtmf (PP_PortDesc portDesc, PP_Dtmf *phoneNu);
extern SuccFail
PP_wink (PP_PortDesc portDesc);
extern SuccFail
PP_eventTriggerMaskSet (PP_PortDesc portDesc, PP_EventTriggerMask mask);
extern SuccFail
PP_eventTriggerWait (PP_PortDesc portDesc);

#endif
