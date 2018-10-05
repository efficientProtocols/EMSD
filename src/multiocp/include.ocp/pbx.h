/*
 * RCSInfo Revision: $Id: pbx.h
 */
 
/*+
 * Description:
 *
 *
-*/

/*
 * Author: Massoud Mohaghegh
 * History:
 *
 */

#ifndef VM_H
#include "vm.h"
#endif

#include <estd.h>
#ifndef PBX_H_
#define PBX_H_

SuccFail PBX_transfer(PP_PortDesc portDesc, Char *toStation);

SuccFail PBX_conference(PP_PortDesc portDesc, Char *toStation);

SuccFail PBX_confPartnerPlus(PP_PortDesc portDesc, Char *toStation, PP_SapDesc sapDesc, PP_Event *event);

#endif
