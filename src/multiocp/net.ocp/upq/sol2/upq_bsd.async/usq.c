/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*+
 * File name: usq.c (User Shell Queue)
 *
 * Description: User Shell Queue.
 *
 * Functions:
 *   USQ_init(void)
 *   USQ_primQuCreate(void)
 *   USQ_putAction(USQ_PrimQuInfo *quInfo, Ptr data, Int size)
 *   USQ_getEvent(USQ_PrimQuInfo *quInfo, Ptr data, Int size)
 *   USQ_primQuRemove(USQ_PrimQuInfo *quInfo)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: usq.c,v 1.12 1996/11/11 00:12:11 fletch Exp $";
#endif /*}*/

#include  "estd.h"	/* Extended Standard definitions header */
#include  "eh.h"	/* Exception Handler Module Header      */
#include  "tm.h"	/* Trace Module Header                  */
#include  "imq.h"	/* InterModule Queue                    */
#include  "sp_shell.h"	/* Service Provider Shell               */
#include  "usq.h"	/* User Shell Queue                     */
#include  "sf.h"	
#include  "byteordr.h"	
 
int IMQ_clientConnect();

extern char pubQuName[];
 

/*<
 * Function:    USQ_init
 *
 * Description: Initialize user shell queue.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

LOCAL Void 
USQ_init(void)
{
    IMQ_init(IMQ_QUEUES);
}



/*<
 * Function:    USQ_primQuCreate
 *
 * Description: Create primitive queue.
 *
 * Arguments:   None.
 *
 * Returns:     Pointer to primitive queue.
 *
>*/

LOCAL USQ_PrimQuInfo *
USQ_primQuCreate(void)
{
    USQ_PrimQuInfo *quInfo;
    SP_Action action;
    Int size;
    struct sockaddr_un name;
    Int pubSockDesc;

    Char *chnlName;
    IMQ_Key  actionQuKey;
    IMQ_Key  eventQuKey;


    if ((quInfo = (USQ_PrimQuInfo *)SF_memGet(sizeof(*quInfo))) == 0 ) {
	EH_problem("USQ_primQuCreate: SF_memGet failed");
        return (USQ_PrimQuInfo *)NULL;
    }

    chnlName = IMQ_nameCreate();
    strcpy(quInfo->chnlName, chnlName);
    chnlName = quInfo->chnlName;

    action.type = SP_QUADDREQ;
    strcpy(action.un.quAddReq.chnlName, chnlName);

    
    /* Create Public Qu Name. */
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, pubQuName);

    /* Create Socket from which to read */
    if ((pubSockDesc = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) {
	perror("USQ_primQuCreate: opening datagram socket");
	EH_problem("USQ_primQuCreate: socket function failed");
        SF_memRelease(quInfo);
        return (USQ_PrimQuInfo *)NULL;
    }

    {
	unsigned char buf[8000], *pbuf = buf;
	Int len = SP_getSize(SP_QUADDREQ);
 	BO_put4(pbuf, len);
	OS_copy(pbuf, (char *)&action, len);
#if 1	
        if (sendto(pubSockDesc, (char *) buf, len+4, 0,
  		   (struct sockaddr *) &name, sizeof(name)) < 0) {
#else
        if (sendto(pubSockDesc, (char *) &action, SP_getSize(SP_QUADDREQ), 0,
  		   (struct sockaddr *) &name, sizeof(name)) < 0) {
#endif
	    perror("USQ_primQuCreate: Sending datagram message");
	    EH_problem("USQ_primQuCreate: sendto function failed");
            SF_memRelease(quInfo);
            return (USQ_PrimQuInfo *)NULL;
    	}
    }
    actionQuKey = IMQ_keyCreate(chnlName, 0);
    if ((quInfo->actionQu = IMQ_clientConnect(actionQuKey)) == 0) {
        SF_memRelease(quInfo);
        return (USQ_PrimQuInfo *)NULL;
    }

    eventQuKey = IMQ_keyCreate(chnlName, 1);
    if ((quInfo->eventQu = IMQ_clientConnect(eventQuKey)) == 0) {
        SF_memRelease(quInfo);
        return (USQ_PrimQuInfo *)NULL;
    }

    while ((size = IMQ_primRcv(quInfo->eventQu,(IMQ_PrimMsg *) &action, 
			       SP_getSize(SP_QUADDCNF))) <= 0) { 
	/* SV check for ENOMSG */
    }
    if (size != SP_getSize(SP_QUADDCNF) || action.type != SP_QUADDCNF
	|| action.un.quAddCnf.status != SUCCESS) {
	char buf[256];

	sprintf(buf, "USQ_primQuCreate: Invalid value: "
		"\nsize=%d=%d   type=%d=%d    stat=%d=%d\n", 
		size, SP_getSize(SP_QUADDCNF), action.type, SP_QUADDCNF, 
		action.un.quAddCnf.status, SUCCESS);
	EH_problem(buf);

	SF_memRelease(quInfo);
        return (USQ_PrimQuInfo *)NULL;
    }

    return ( quInfo );

} /* USQ_primQuCreate(void) */


/*<
 * Function:    USQ_putAction
 *
 * Description: Put action.
 *
 * Arguments:   Primitive queue, data, size.
 *
 * Returns:     Number bytes written on successful completion, 
 * 		a negative error value otherwise.
 *
>*/

PUBLIC SuccFail
USQ_putAction(USQ_PrimQuInfo *quInfo, Ptr data, Int size)
{
    if (quInfo == 0) {
	return (FAIL);
    }

    return IMQ_primSnd(quInfo->actionQu, (IMQ_PrimMsg *)data, size);
}


/*<
 * Function:    USQ_getEvent
 *
 * Description: Get event.
 *
 * Arguments:   Primitive queue, data, size.
 *
 * Returns:     Number of bytes read on successful completion, 
 *		a negative error value otherwise.
 *
 * 
>*/

PUBLIC Int
USQ_getEvent(USQ_PrimQuInfo *quInfo, Ptr data, Int size)
{
    return IMQ_primRcv(quInfo->eventQu, (IMQ_PrimMsg *)data, size);
}


/*<
 * Function:    USQ_primQuRemove
 *
 * Description: Remove primitive queue.
 *
 * Arguments:   Primitive queue.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

PUBLIC SuccFail
USQ_primQuRemove(USQ_PrimQuInfo *quInfo)
{
    IMQ_primDelete(quInfo->actionQu, quInfo->chnlName);
    IMQ_primDelete(quInfo->eventQu,  quInfo->chnlName);

    SF_memRelease(quInfo);

    return ( SUCCESS );

} /* USQ_primQuRemove() */

