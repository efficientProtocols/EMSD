/*+
 * Description:
 *
 *  PP_SPI_ 
 *  Phone Port Service Provider Interface
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */


#ifndef _PP_SPI_H_	/*{*/
#define _PP_SPI_H_

#include "pp.h"
#include "ppif.h"

/*
 *
 */

typedef Void * PP_SP_PortDesc;

typedef struct PP_SPI_PortInfo {
    struct PP_SPI_PortInfo *next;
    struct PP_SPI_PortInfo *prev;
    PP_PortInfo  *pp_portInfo;
    struct PP_SPI_ServiceInfo *spi_serviceInfo;
    PP_SP_PortDesc devPortDesc;
} PP_SPI_PortInfo;


typedef struct PP_SPI_ServiceInfo {
    struct PP_SPI_ServiceInfo *next;
    struct PP_SPI_ServiceInfo *prev;
    String name;
    PP_SPI_PortInfo * (*attach) (PP_PortInfo *portInfo);
    SuccFail (*detach) (PP_SPI_PortInfo *spi_portInfo);
    SuccFail (*onHook) (PP_SPI_PortInfo *spi_portInfo);
    SuccFail (*offHook) (PP_SPI_PortInfo *spi_portInfo);
    SuccFail (*ringWait) (PP_PortDesc portDesc, PP_Num nuOfRings);
    
    SuccFail (*callerIDInfoGet) (PP_PortDesc portDesc, PP_CallerIDInfo *callerIDInfo);
    SuccFail (*calledIDInfoGet) (PP_PortDesc portDesc, PP_CalledIDInfo *calledIDInfo);
    SuccFail (*playN) (PP_PortDesc portDesc, VM_MsgDesc *vMsgDesc,
                                                          PP_Num nuDtmfs, Bool typeahead);
    SuccFail (*recordN) (PP_PortDesc portDesc, VM_MsgDesc *vMsgDesc,
	                         PP_Time maxDuration, PP_Time maxSilence, PP_Num nuDtmfs);
    SuccFail (*getDtmfs) (PP_PortDesc portDesc,
	                                  PP_Num nuDtmfs, PP_Dtmf termDtmf, PP_Time wait);
    SuccFail (*dial) (PP_PortDesc portDesc, String phoneNu);
    SuccFail (*genDtmf) (PP_PortDesc portDesc, PP_Dtmf *phoneNu);
    SuccFail (*wink) (PP_PortDesc portDesc);
    SuccFail (*configPortSetParam) (PP_SPI_PortInfo * spi_portInfo,
		      String paramName,
		      String paramValues,
		      String *resultOrError);

    SuccFail (*configPortGetParam) (PP_PortDesc portDesc,
		      String paramName,
		      String *resultOrError);

    SuccFail (*configServiceProviderSetParam) (String serviceProviderName,
				 String paramName,
				 String paramValues,
				 String *resultOrError);

    SuccFail (*configServiceProviderGetParam) (String serviceProviderName,
		      String paramName,
		      String *resultOrError);
   
} PP_SPI_ServiceInfo;

/* PP_SPI_PortInfo is on a per line basis */


Void
PP_SPI_init(void);

PP_SPI_ServiceInfo *
PP_SPI_serviceInfoObtain(void);

SuccFail
PP_SPI_serviceInfoSetDefault(String serviceProviderName);

PUBLIC SuccFail
PP_SPI_serviceInfoRegister(PP_SPI_ServiceInfo  *serviceInfo);

PUBLIC SuccFail
PP_SPI_serviceInfoDeregister(PP_SPI_ServiceInfo *serviceInfo);

PP_SPI_ServiceInfo *
PP_SPI_serviceInfoFind(String serviceProviderName);

PP_SPI_PortInfo *
PP_SPI_portInfoObtain(void);

PUBLIC SuccFail
PP_SPI_portInfoRelease(PP_SPI_PortInfo *portInfo);

extern PP_Event *
pp_eventObtain (void);

extern SuccFail 
pp_eventRelease (PP_Event *event);

extern SuccFail 
pp_sapEventPut (PP_SapDesc sapDesc, PP_Event *event);


#endif	/*}*/
