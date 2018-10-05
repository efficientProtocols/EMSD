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
 *
 * File: imq.c
 *
 * Description:
 *   IMQ_ Inter Module Queue Interface.
 *   A simple interface based on QU_ module, 
 *   Address and size of data is put/get on some named queue.
 *   Named queue is in memory (vs socket in the case of upq_bsd)
 *
 * Functions:
 *   IMQ_init(Int nuOfElems)
 *   imq_exit(Int code)
 *   IMQ_nameCreate()
 *   IMQ_keyCreate(String name, Int subID)
 *   IMQ_primGet(IMQ_Key key)
 *   IMQ_primSnd(IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size)
 *   IMQ_primRcv(IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size)
 *   IMQ_primDelete(IMQ_PrimDesc primDesc, char *chnlName)
 *   IMQ_dataGet(IMQ_Key key, Int size)
 *   IMQ_dataSnd(IMQ_DataDesc dataDesc, Ptr data, Int size)
 *   IMQ_duSnd(IMQ_DataDesc dataDesc, QU_Head *duHead, Int size)
 *   IMQ_dataRcv(IMQ_DataDesc dataDesc, Int *size)
 *   IMQ_dataRcvComplete(IMQ_DataDesc dataDesc, Ptr data)
 *   IMQ_dataDelete(IMQ_DataDesc dataDesc)
 *   IMQ_blockQuAdd(IMQ_PrimDesc quDesc)
 *   IMQ_blockQuRemove(IMQ_PrimDesc quDesc)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: imq.c,v 1.1 1996/11/07 02:55:40 kamran Exp $";
#endif /*}*/


#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#ifdef MSDOS
#include  <io.h>
#endif

#include  "estd.h"
#include  "eh.h"
#include  "queue.h"
#include  "imq.h"
#include  "sp_shell.h"
#include  "tm.h"
#include  "target.h"
#include  "sf.h"
#include  "du.h"

#ifdef TM_ENABLED
TM_ModuleCB *IMQ_modCB;
#endif

/* A Data Element Going through any Queue */
typedef struct imq_Elem {
    struct imq_Elem *next;
    struct imq_Elem *prev;
    Ptr data;
    Int size;
} imq_Elem;

typedef struct imq_ElemHead {
    imq_Elem *first;
    imq_Elem *last;
} imq_ElemHead;

/* Information About An InterModule Queue */
typedef struct IMQ_Info {
    struct IMQ_Info  *next;
    struct IMQ_Info  *prev;
    imq_ElemHead elemQuHead;
    Char key[SP_CHNLNAMELEN];
} IMQ_Info;

typedef struct imq_InfoHead {
    IMQ_Info *first;
    IMQ_Info *last;
} imq_InfoHead;

static imq_InfoHead availQu;
static imq_InfoHead activeQu;


/*<
 * Function:	IMQ_init
 *
 * Description:	Initialize IMQ Module
 *
 * Arguments:	Number of queue elements
 *
 * Returns:	0 if successful, -1 otherwise.
 *
 * 
>*/
PUBLIC SuccFail 
IMQ_init(Int nuOfElems)
{
    Int i;
    IMQ_Info *elem;
    static Bool virgin = TRUE;

    if ( !virgin ) {
	return (SUCCESS);
    }

    virgin = FALSE;

#ifdef TM_ENABLED
    if ( ! (IMQ_modCB = TM_open("IMQ_") )) {
        EH_problem("IMQ_init: TM_open failed\n");
        return ( FAIL );
    }

#endif

    QU_INIT(&availQu);
    QU_INIT(&activeQu);
    for (i = 0; i < nuOfElems; i++) {
       	if ((elem=(IMQ_Info *) SF_memGet(sizeof(*elem))) == (IMQ_Info *)0) {
	    EH_fatal("IMQ_init: Out of memory while initializing queues\n");
        }
        QU_INIT(elem);
        QU_INSERT(elem, &availQu);
    }

    return ( SUCCESS );
}


/*<
 * Function:	IMQ_exit
 *
 * Description:	Exit IMQ module.
 *
 * Arguments:	Exit code.
 *
 * Returns:	None.
 *
>*/
void
IMQ_exit(Int code)
{
    TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_exit(simu)"));
    /* Free queue memory */
}


/*<
 * Function:	IMQ_nameCreate
 *
 * Description:	Create name
 *
 * Arguments:	None
 *
 * Returns:	Name.
 *
>*/
PUBLIC String
IMQ_nameCreate()
{
    static Int chnlNu = 0;
    static Char name[SP_CHNLNAMELEN];
    String retVal;

    retVal = (String)0;
    sprintf(name, "/tmp/TP4%02dXXXXXX", chnlNu);
    mktemp(name);

    ++chnlNu;
    retVal = name;

    TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_nameCreate(simu): name=%s\n", name));

    return ( retVal );
}


/*<
 * Function:	IMQ_keyCreate
 *
 * Description:	Create key
 *
 * Arguments:	Name, id
 *
 * Returns:	Key.
 *
>*/
PUBLIC IMQ_Key
IMQ_keyCreate(String name, Int subID)
{
    static Char  newName[SP_CHNLNAMELEN];
    IMQ_Key key;

    sprintf(newName, "%s.%02d", name, subID);
    key = newName;

    TM_TRACE((IMQ_modCB, TM_ENTER, 
	     "IMQ_keyCreate(simu): key=%s, name=%s, id=%d\n",
	     key, name, subID));

    return ( key );
}


/*<
 * Function:	IMQ_primGet
 *
 * Description:	Create queue
 *
 * Arguments:	Key
 *
 * Returns:	IMQ primitive
 *
 * 
>*/
PUBLIC IMQ_PrimDesc
IMQ_primGet(IMQ_Key key)
{
    IMQ_Info *elem;

    for (elem = activeQu.first;
	    elem != (IMQ_Info *)&activeQu; 
	    elem = elem->next) {
	if (strcmp(key, elem->key) == 0) {
	    /* we Got it */
	    break;
	}
    }

    if (elem == (IMQ_Info *)&activeQu) {
	/* So it is not already active and must be created */
	if ( (elem = availQu.first) == (IMQ_Info *) &availQu ) {
	    EH_problem("IMQ_primGet: Out of Queues");
	    return ((IMQ_PrimDesc) 0);
	}
	strcpy(elem->key, key);
	QU_INIT(&elem->elemQuHead);
	QU_MOVE(elem, &activeQu);
    } 

    return ( (IMQ_PrimDesc) elem );
}


/*<
 * Function:	IMQ_primSnd
 *
 * Description:	Send primitive
 *
 * Arguments:	Primitive
 *
 * Returns:	
 *
 * 
>*/
PUBLIC Int
IMQ_primSnd(IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size)
{
    IMQ_Info *quInfo;
    imq_Elem *elem;
    Ptr locData;

#ifndef MSDOS 
    TM_TRACE((IMQ_modCB, TM_ENTER,
	     "IMQ_primSnd(simu): primDesc=0x%x data=%s size=%d\n",
	     primDesc, TM_prAddr((Ptr) data), size));
#endif

    quInfo = (IMQ_Info *)primDesc;

    elem = (imq_Elem *) SF_memGet(sizeof(*elem));

    locData = (Ptr) SF_memGet(size);

    OS_copy(locData, data, size);
    elem->data = locData;
    elem->size = size;
    QU_INIT(elem);
    QU_INSERT(elem, &quInfo->elemQuHead);

    return 0;
}


/*<
 * Function:	IMQ_primRcv
 *
 * Description:	Receive primitive
 *
 * Arguments:	primitive descriptor, data, size
 *
 * Returns:	
 *
 * 
>*/
PUBLIC Int
IMQ_primRcv(IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size)
{
    IMQ_Info *quInfo;
    imq_Elem *elem;
    Int retVal;
#ifndef MSDOS
    extern int errno;
#endif
 
    quInfo = (IMQ_Info *)primDesc;

    elem = quInfo->elemQuHead.first; 
    if (elem !=  (imq_Elem *) &quInfo->elemQuHead) {
	if (size < elem->size ) {
	    EH_problem("IMQ_primRcv: buffer smaller than data size\n");
	    retVal = -1;
	}
	QU_REMOVE(elem);
        OS_copy(data, elem->data, elem->size);
	retVal =  elem->size;
	SF_memRelease(elem->data);
	SF_memRelease(elem);
    } else {
	retVal = -1;
	errno = 0; /* Just so it is considered an interrupted system call */
    }
    return ( retVal );
}


/*<
 * Function:	IMQ_primDelete
 *
 * Description:	Delete primitive
 *
 * Arguments:	Primitive descriptor, channel name
 *
 * Returns:	None.
 *
>*/
PUBLIC Void
IMQ_primDelete(IMQ_PrimDesc primDesc, char *chnlName)
{
    QU_MOVE(primDesc, &availQu);

    TM_TRACE((IMQ_modCB, TM_ENTER,
	    "IMQ_primDelete(simu): primDesc=0x%x\n", primDesc));
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC IMQ_DataDesc
IMQ_dataGet(IMQ_Key key, Int size)
{
    IMQ_Info *elem;

    for (elem = activeQu.first;
	    elem != (IMQ_Info *)&activeQu; 
	    elem = elem->next) {
	if (strcmp(key, elem->key) == 0) {
	    /* we Got it */
	    break;
	}
    }

    if (elem == (IMQ_Info *)&activeQu) {
	/* So it is not already active and must be created */
	if ( (elem = availQu.first) == (IMQ_Info *) &availQu ) {
	    EH_problem("out of Qus");
	    return ((IMQ_PrimDesc) 0);
	}
	strcpy(elem->key, key);
	QU_INIT(&elem->elemQuHead);
	QU_move(elem, &activeQu);
    } 

    return ( (IMQ_PrimDesc) elem );
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Int
IMQ_dataSnd(IMQ_DataDesc dataDesc, Ptr data, Int size)
{
    IMQ_Info *quInfo;
    imq_Elem *elem;
    Ptr locData;

    TM_TRACE((IMQ_modCB, TM_ENTER,
	"IMQ_dataSnd(simu): dataDesc=0x%x data=%s size=%d\n",
	dataDesc, TM_prAddr(data), size));

    quInfo = (IMQ_Info *)dataDesc;


    elem = (imq_Elem *) SF_memGet(sizeof(*elem));

    locData = (Ptr) SF_memGet(size);

    OS_copy(locData, data, size);
    elem->data = locData;
    elem->size = size;
    QU_INIT(elem);
    QU_INSERT(elem, &quInfo->elemQuHead);

    return 0;
}

/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Int
IMQ_duSnd(IMQ_DataDesc dataDesc, QU_Head *duHead, Int 	 size)
{
    IMQ_Info *quInfo;
    imq_Elem *elem;
    Ptr locData;
    Ptr dataIndex;
    DU_View du;

    TM_TRACE((IMQ_modCB, TM_ENTER,
	"IMQ_duSnd(simu): dataDesc=0x%x  size=%d\n",
	dataDesc, size));

    quInfo = (IMQ_Info *)dataDesc;

    elem = (imq_Elem *) SF_memGet(sizeof(*elem));

    dataIndex = locData = (Ptr) SF_memGet(size);

    du = (DU_View) duHead->first;
    while (du != (DU_View)duHead) {
	DU_View duNext;
	
	duNext = du->next;
	QU_REMOVE(du);
	OS_copy(dataIndex, DU_data(du), DU_size(du));
	dataIndex += DU_size(du);
	DU_free(du);
	du = duNext;
    }	
    elem->data = locData;
    elem->size = size;
    QU_INIT(elem);
    QU_INSERT(elem, &quInfo->elemQuHead);

    return 0;
}


/*<
 * Function:
 * Description:
 *	See MSGOP, msgrcv
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Ptr
IMQ_dataRcv(IMQ_DataDesc dataDesc, Int	 *size	/* Out */)
{
    IMQ_Info *quInfo;
    imq_Elem *elem;
    Ptr retVal;

    quInfo = (IMQ_Info *)dataDesc;

    elem = quInfo->elemQuHead.first; 
    if (elem !=  (imq_Elem *) &quInfo->elemQuHead) {
	QU_REMOVE(elem);
	retVal = elem->data;
	*size =  elem->size;
	free(elem);
    } else {
	retVal = (Ptr)0;
	*size =  0;
    }

    TM_TRACE((IMQ_modCB, TM_ENTER,
	    "IMQ_dataRcv: retVal=%d dataDesc=0x%x data=%s size=%d\n",
	    retVal, dataDesc, TM_prAddr(retVal), *size));

    return ( retVal );
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC int
IMQ_dataRcvComplete(IMQ_DataDesc dataDesc, Ptr data)
{
    free(data);
    return 0;
}


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC Void
IMQ_dataDelete(IMQ_DataDesc dataDesc)
{
    TM_TRACE((IMQ_modCB, TM_ENTER,
	    "IMQ_dataDelete(simu): dataDesc=0x%x\n", dataDesc));
}


/*<
 * Function:    IMQ_blockQuAdd
 *
 * Description: Add block queue	(just for compatibity with upq_bsd)
 *
 * Arguments:	-
 *
 * Returns: 	-
 *
>*/

Void *
IMQ_blockQuAdd(IMQ_PrimDesc quDesc)
{
    /* Dummy function: Just for compatibility reasons */
    return (Void *)NULL;
}


/*<
 * Function:    Remove block queue
 *
 * Description: Remove block queue    (just for compatibity with upq_bsd)
 *
 * Arguments:	-
 *
 * Returns: 	-
 *
>*/

Void
IMQ_blockQuRemove(IMQ_PrimDesc quDesc)
{
    /* Dummy function: Just for compatibility reasons */
}
