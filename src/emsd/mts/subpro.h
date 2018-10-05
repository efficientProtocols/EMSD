/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of Neda-EMSD. An implementation of RFC-2524.
 *
 *Neda-EMSD is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with Neda-EMSD; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
/*
 * 
 * Copyright (C) 1995,1997  Neda Communications, Inc. All rights reserved.
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:
 *
 */

#ifndef __SUBPRO_H__
#define	__SUBPRO_H__

/* TM Trace Flags */
#define	SUB_TRACE_ERROR   	(1 << 0)
#define	SUB_TRACE_WARNING   	(1 << 1)
#define	SUB_TRACE_DETAIL   	(1 << 2)
#define	SUB_TRACE_ACTIVITY	(1 << 3)
#define	SUB_TRACE_INIT		(1 << 4)
#define	SUB_TRACE_COMPARE	(1 << 9)

#define NSAP_HASH_SIZE          (1 << 14)	/* 16K entries */

typedef enum  { DM_Undefined = 0, DM_Complete = 1, DM_Efficient = 2 }  MTS_DeliverMode;
typedef enum  { SM_Undefined = 0, SM_Serial = 1, SM_Parallel = 2 }     MTS_SerialMode;

typedef struct SUB_DeviceProfile
{
    OS_Uint8		protocolVersionMajor;
    OS_Uint8		protocolVersionMinor;
    N_SapAddr		nsap;
    char 		deviceType[32];
    MTS_DeliverMode	emsdDeliveryMode;
    MTS_SerialMode	emsdSerialMode;
} SUB_DeviceProfile;

typedef struct SUB_Profile
{
  char                	subscriberId[40];
  char                	subscriberName[64];
  char                	subscriberPassword[32];
  char 			emsdAua[40];
  char 			emsdNickname1[40];
  char 			emsdNickname2[40];
  char 			emsdFrom[128];
  char 			emsdReplyTo[128];
  char 			emsdUaPassword[16];
  char                  IVR_nanpPhone[16];
  char                  IVR_intlPhone[24];
  OS_Uint8              IVR_defaultAua;
#				define SUB_IVR_UNDEFINED     0
#				define SUB_IVR_EMAIL         1
#				define SUB_IVR_FAX           2
#				define SUB_IVR_PAGER         3
#				define SUB_IVR_POCKETNET     4
  char                  IVR_emailAua[128];
  char                  IVR_faxAua[24];
  char                  IVR_pagerAua[24];
  char                  IVR_pocketNetAua[64];
  SUB_DeviceProfile     deviceProfile;

} SUB_Profile;


typedef struct SUB_Device
{
  char              deviceType[32];
  OS_Uint32         emsdDeliveryMode;
  OS_Uint32         emsdSerialMode;
  OS_Uint8          protocolVersionMajor;
  OS_Uint8          protocolVersionMinor;
} SUB_Device;

typedef int  SUB_Index;

typedef struct {
  OS_Uint32       count;
  OS_Uint32       deviceCount;
  SUB_Profile    *subproArray;
  SUB_Device     *deviceArray;
  SUB_Index      *indexSubscriberId;
  SUB_Index      *indexNsap;
} SUB_Subscribers;

/*
 * Forward Declarations
 */

ReturnCode      SUB_init        (const char *pConfigFileName);
SUB_Profile *   SUB_getEntry    (const char *subscriberId, const N_SapAddr *pNsap);

#endif   /* __SUBPRO_H__ */
