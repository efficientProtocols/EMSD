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
 * File name: imq.c  (InterModule Queue)
 *
 * Description:
 *   IMQ_ Inter Module Queue Interface.
 *   A simple interface based on QU_ module, 
 *   Address and size of data is put/get on some named queue.
 *
 *   Primitives provided by this module are:
 *	IMQ_init
 *	imq_exit
 *	IMQ_nameCreate
 *	IMQ_keyCreate
 *	IMQ_primCreate
 *	IMQ_primDelete
 *	IMQ_primSnd
 *	IMQ_primRcv
 *	IMQ_dataCreate
 *	IMQ_dataDelete
 *	IMQ_dataSnd
 *	IMQ_dataRcv
 *
 * Functions:
 *   IMQ_init(void)
 *   imq_exit(Int code)
 *   IMQ_nameCreate(void)
 *   IMQ_keyCreate(String name, Int subID)
 *   IMQ_primGet(IMQ_Key key)
 *   IMQ_primSnd(IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size)
 *   IMQ_primRcv(IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size)
 *   IMQ_primDelete(IMQ_PrimDesc primDesc)
 *   IMQ_dataGet(IMQ_Key key, Int size)
 *   IMQ_dataSnd(IMQ_DataDesc dataDesc, Ptr data, Int size)
 *   IMQ_duSnd(IMQ_DataDesc dataDesc, QU_Head *duHead, Int size)
 *   IMQ_dataRcv(IMQ_DataDesc dataDesc, Int *size)
 *   IMQ_dataRcvComplete(IMQ_DataDesc dataDesc, char *data)
 *   IMQ_dataDelete(IMQ_DataDesc dataDesc)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: imq.c,v 1.17 1996/11/07 02:57:29 kamran Exp $";
#endif /*}*/

#include  "estd.h"
#include  "eh.h"
#include  "queue.h"
#include  "imq.h"
#include  "tm.h"
#include  "target.h"

extern int errno;
extern unlink();

TM_ModuleCB *IMQ_modCB;

TM_ModuleCB *DU_modCB;
extern DU_Pool *G_duMainPool;
#define DU_MALLOC TM_BIT10

/* 
 * Set the bit corresponding to each signal that should be 
 * blocked for delivery.
 */
static int sigMask;


/*<
 * Function:    IMQ_init
 *
 * Description: Initialize InterModule Queue.
 *
 * Arguments:   None.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 * 
>*/

PUBLIC SuccFail 
IMQ_init(void)
{
    static Bool virgin = TRUE;

    sigMask = sigmask(SIGALRM) | sigmask(SIGUSR1) | sigmask(SIGUSR2);

    if ( !virgin ) {
	return 0;
    }
    virgin = FALSE;

#ifdef TM_ENABLED
    if (TM_OPEN(IMQ_modCB, "IMQ_") == NULL) {
	EH_problem("IMQ_init: TM_open IMQ_ failed");
	return ( FAIL );
    }

    TM_TRACE((IMQ_modCB, TM_ENTER, "***** Blocking upq_bsd *****\n"));
#endif

    return ( SUCCESS );
}


/*<
 * Function:    IMQ_nameCreate
 *
 * Description: Create a queue name. A file with the name /tmp/SPnnXXXXXX, 
 *              in which nn is the queue number incremented for each new queue.
 *
 * Arguments:   None.
 *
 * Returns:     Pointer to queue name (file name).
 *
>*/

PUBLIC String
IMQ_nameCreate(void)
{
    static Int chnlNu = 0;
    static Char name[128];
    String retVal;

    retVal = (String)0;
    sprintf(name, "/tmp/SP%02dXXXXXX", chnlNu);
    mktemp(name);

    ++chnlNu;
    retVal = name;

#ifdef TM_ENABLED
    TM_trace(IMQ_modCB, TM_ENTER, "IMQ_nameCreate: name=%s\n", name);
#endif
    return ( retVal );
}


/*<
 * Function:    IMQ_keyCreate
 *
 * Description: Create key.
 *
 * Arguments:   Name, sub ID.
 *
 * Returns:     Key.
 *
>*/

PUBLIC IMQ_Key
IMQ_keyCreate(String name, Int subID)
{
    static Char  newName[128];
    IMQ_Key key;

    sprintf(newName, "%s.%02d", name, subID);
    key = newName;

    TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_keyCreate:  key=%s, name=%s, id=%d\n",
	    key, name, subID));

    return ( key );
}


/*<
 * Function:    IMQ_primGet
 *
 * Description: Get primitive.
 *
 * Arguments:   Key.
 *
 * Returns:     Primitive description.
 * 
>*/

PUBLIC IMQ_PrimDesc
IMQ_primGet(IMQ_Key key)
{
    Int retVal;
    struct sockaddr_un addr;
    struct sockaddr_un fromAddr;
    Int fromLen;
    Int sock;
    Int len;
    Int origMask;
    Int statusFlags;

    TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_primGet:    key=%s\n", key));

    /* Common for Calling and Called */
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, key);

    origMask = sigblock(sigMask);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sigsetmask(origMask);
    if (sock < 0) {
	perror("opening stream socket");
        EH_problem("IMQ_primGet: socket function failed");
	return (IMQ_PrimDesc)NULL;
    }

    origMask = sigblock(sigMask);

#ifdef SOL2
    TM_TRACE((IMQ_modCB, TM_ENTER, "\n***Compiled WITH SOL2 directive*** \n"));

    retVal = bind(sock, (struct sockaddr *) &addr, 
             strlen(addr.sun_path) + sizeof(addr.sun_family));
#else /* not SOL2 */
    TM_TRACE((IMQ_modCB, TM_ENTER, 
	     "\n***Compiled WITHOUT SOL2 directive*** \n"));

    retVal   = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
#endif /* SOL2 */

    sigsetmask(origMask);
    if (retVal == SUCCESS) {
	origMask = sigblock(sigMask);

	retVal = listen(sock, 50);    /* 50 */

	sigsetmask(origMask);
	if (retVal) {
	    EH_problem("IMQ_primGet: listen function failed");
	    perror("IMQ_primGet:listen");
	    return (IMQ_PrimDesc)NULL;
	}
	while (TRUE) {
#ifdef FUTURE
            Int statusFlags = 0;
#endif
	    origMask = sigblock(sigMask);
	    fromLen = sizeof(fromAddr);
	    TM_TRACE((IMQ_modCB, TM_ENTER, 
		     "IMQ_primGet:    accept(%d) on %s\n",
		     sock, addr.sun_path));
#ifdef FUTURE
            if ((statusFlags = fcntl(sock, F_GETFL, 0)) < 0) {
		perror ("fcntl: ");
		EH_problem("IMQ_primGet: fcntl failed");
	    }

	    statusFlags |= FNDELAY;
            if (fcntl (sock, F_SETFL, statusFlags) < 0) {
		perror ("fcntl: ");
		EH_problem("IMQ_primGet: fcntl failed");
	  	return -1;
	    }
#endif /* FUTURE */

	    TM_TRACE((IMQ_modCB, TM_ENTER, 
		     "\nIMQ_primGet: Before accept, fd=%d\n", sock));
	    retVal = accept(sock, (struct sockaddr *) &fromAddr, &fromLen);

	    TM_TRACE((IMQ_modCB, TM_ENTER, 
		      "\nIMQ_primGet: After  accept, fd=%d\n", sock));

	    sigsetmask(origMask);
	    if (retVal < 0) {
		EH_problem("IMQ_primGet: accept function failed");

	        TM_TRACE((IMQ_modCB, TM_ENTER, 
		     "IMQ_primGet:    accept(%d) failed, retVal=%d, errno=%d\n",
		     sock, retVal, errno));

		perror("accept stream socket");
	        return (IMQ_PrimDesc)NULL;
	    } else {
		break;
	    }
	}
    } else {
	/* So Bind Failed */
	if (errno != EADDRINUSE) {
            EH_problem("IMQ_primGet: bind failed");
	    perror("IMQ_primGet: binding stream socket");
	    return (IMQ_PrimDesc)NULL;
	}

	TM_TRACE((IMQ_modCB, TM_ENTER, 
		  "IMQ_primGet:    connect(%d) to %s\n",
		  sock, addr.sun_path));

	origMask = sigblock(sigMask);

	TM_TRACE((IMQ_modCB, TM_ENTER, 
		  "IMQ_primGet: Before connect, fd=%d\n", sock));

#ifdef NORACE /* Used when there is race */
	sleep(2);		/* Avoid the race condition */
#endif

#ifdef SOL2
        len = strlen(addr.sun_path) + sizeof(addr.sun_family);
#else
        len = sizeof(addr);
#endif

	if (connect(sock, (struct sockaddr *) &addr, len) < 0) {
	    perror("IMQ_primGet: connect stream socket");
            EH_problem("IMQ_primGet: connect function failed");
	    return (IMQ_PrimDesc)NULL;
	}

        TM_TRACE((IMQ_modCB, TM_ENTER, 
	          "IMQ_primGet: After  connect, fd=%d\n", sock));

	sigsetmask(origMask);

/*   	unlink(addr.sun_path); */

	if ((statusFlags = fcntl(sock, F_GETFL, 0)) < 0) {
	    perror ("fcntl: ");
	    EH_problem("IMQ_primGet: fcntl failed");
	}

	statusFlags |= FNDELAY;
        if (fcntl (sock, F_SETFL, statusFlags) < 0) {
	    perror ("fcntl: ");
	    EH_problem("IMQ_primGet: fcntl failed");
	    return -1;
	}

	retVal = sock;
    }

    TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_primGet:    retVal=%d\n", retVal));

    return ( (IMQ_PrimDesc)retVal );
}


/*<
 * Function:    IMQ_primSnd
 * 
 * Description: Send primitive.
 *
 * Arguments:   Primitive description, primitive data, size.
 *
 * Returns:     0 on successfule completion, otherwise a negative error value.
 *
>*/

PUBLIC Int
IMQ_primSnd(IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size)
{
    Int retVal;
    Int origMask;

    TM_TRACE((IMQ_modCB, TM_ENTER,
	     "IMQ_primSnd:    primDesc=%d type=0x%x size=%d\n", 
             primDesc, data->mtype, size));

    if (primDesc <= 0) {
	return (FAIL);
    }

    origMask = sigblock(sigMask);
    if ((retVal = write(primDesc, data, size)) < 0) {
	EH_problem("IMQ_primSnd: write failed");
	perror("IMQ_primSnd: write");
    }
    sigsetmask(origMask);
    return ( retVal );
}


/*<
 * Function:    IMQ_primRcv
 * 
 * Description: Receive primitive.
 *
 * Arguments:   Primitive descriptor, primitive data, size.
 *
 * Returns:     Number of bytes read, or a negative error number.
 *
>*/

PUBLIC Int
IMQ_primRcv(IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size)
{
    Int retVal;
    extern int errno;
    Int origMask;
    
    TM_TRACE((IMQ_modCB, TM_ENTER,
	      "IMQ_primRcv:    Before read: primDesc=%d size=%d\n",
	      primDesc, size));

    origMask = sigblock(sigMask);
    if ((retVal = read(primDesc, data, size)) < 0) {

        TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_primRcv:    After read: ERROR\n"));

	switch (errno) {
	case EINTR:
	    perror("IMQ_primRcv: read:");
	    return (-1);

	case ECONNABORTED:
	    perror("IMQ_primRcv: read:");
	    IMQ_primDelete(primDesc);
	    break;

	case EWOULDBLOCK:
	    perror("IMQ_primRcv: read:");
             /* tried to read a non-blocking empty socket. 
		perror: Rsource temporarily unavailable */
	    break;

	default:
	    perror("IMQ_primRcv: read:");
            EH_fatal("IMQ_primRcv: read failed");
	}

    }

    TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_primRcv:    After read: SUCCESS \n"));

    sigsetmask(origMask);

    if (retVal == 0) {
	retVal = -1;
    }

    TM_TRACE((IMQ_modCB, TM_ENTER,
	      "IMQ_primRcv:    # of bytes=%d primDesc=%d type=0x%x size=%d\n",
	      retVal, primDesc, data->mtype, size));

    return ( retVal );
}


/*<
 * Function:    IMQ_primDelete
 *
 * Description: Delete primitive.
 *
 * Arguments:   Primitive descriptor.
 *
 * Returns:     None.
 *
>*/

PUBLIC Void
IMQ_primDelete(IMQ_PrimDesc primDesc)
{
    TM_TRACE((IMQ_modCB, TM_ENTER, 
	      "IMQ_primDelete: primDesc=0x%x\n", primDesc));
}


/*<
 * Function:    IMQ_dataGet
 *
 * Description: Get data.
 *
 * Arguments:   Key.
 *
 * Returns:     Data descriptor.
 * 
>*/

PUBLIC IMQ_DataDesc
IMQ_dataGet(IMQ_Key key, Int size)
{
    Int retVal;
    struct sockaddr_un addr;
    struct sockaddr_un fromAddr;
    Int fromLen;
    Int len;
    Int sock;
    Int origMask;

    TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_dataGet:    key=%s, size=%d\n", 
             key, size));

    /* Common for Calling and Called */
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, key);

    origMask = sigblock(sigMask);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sigsetmask(origMask);
    if (sock < 0) {
        EH_problem("IMQ_dataGet: socket function failed");
	perror("IMQ_dataGet: opening strean socket");
        return (IMQ_DataDesc)NULL;
    }

    origMask = sigblock(sigMask);

#ifdef SOL2
    len = strlen(addr.sun_path) + sizeof(addr.sun_family);
#else
    len = sizeof(addr);
#endif

    retVal = bind(sock, (struct sockaddr *) &addr, len);

    sigsetmask(origMask);

    if (retVal == SUCCESS) {

	TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_dataGet:    Create\n"));

	origMask = sigblock(sigMask);
	retVal = listen(sock, 1);
	sigsetmask(origMask);
	if (retVal) {
            EH_problem("IMQ_dataGet: listen function failed");
	    perror("IMQ_dataGet: listen");
            return (IMQ_DataDesc)NULL;
	}
	while (TRUE) {
	    origMask = sigblock(sigMask);
	    fromLen  = sizeof(fromAddr);
	    retVal   = accept(sock, (struct sockaddr *)&fromAddr, &fromLen);
	    sigsetmask(origMask);
	    if (retVal < 0) {
                EH_problem("IMQ_dataGet: accept function failed");
		printf("sock=%d, retVal=%d, errno=%d\n", sock, retVal, errno);
		perror("IMQ_dataGet: accept stream socket");
                return (IMQ_DataDesc)NULL;
	    } else {
		break;
	    }
	}
    } else {
	/* So Bind Failed */
	if (errno != EADDRINUSE) {
            EH_problem("IMQ_dataGet: bind failed");
	    perror("IMQ_dataGet: binding stream socket");
            return (IMQ_DataDesc)NULL;
	}
	TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_dataGet:    Connect\n"));

	origMask = sigblock(sigMask);
#ifdef NORACE /* Used when there is race */
	sleep(2);		/* Avoid the race condition */
#endif
#ifdef SOL2
  	len = strlen(addr.sun_path) + sizeof(addr.sun_family);
#else
       	len = sizeof(addr);
#endif
	while (connect(sock, (struct sockaddr *) &addr, len) < 0) {
	    perror("connect stream socket");
	}
	sigsetmask(origMask);

	unlink(addr.sun_path);
	retVal = sock;
    }

    TM_TRACE((IMQ_modCB, TM_ENTER, "IMQ_dataGet:    retVal=%d\n", retVal));

    return ( (IMQ_DataDesc)retVal );
}



/*<
 * Function:    IMQ_dataSnd
 *
 * Description: Send data.
 *
 * Arguments:   Data descriptor, data, size.
 *
 * Returns:     Number of bytes sent, or a negative error value.
 * 
>*/
PUBLIC Int
IMQ_dataSnd(IMQ_DataDesc dataDesc, Ptr data, Int size)
{
    Int retVal;
    Int origMask;

    TM_TRACE((IMQ_modCB, TM_ENTER,
	      "IMQ_dataSnd:    dataDesc=0x%x data=0x%x size=%d\n", 
              dataDesc, data, size));

    origMask = sigblock(sigMask);
    if ((retVal = write(dataDesc, data, size)) < 0) {
	EH_problem("IMQ_dataSnd: write failed");
	perror("write");
    }
    sigsetmask(origMask);

    return ( retVal );
}


/*<
 * Function:    IMQ_duSnd
 *
 * Description: Send data unit.
 *
 * Arguments:   Data description, head of the queue, size.
 *
 * Returns:     Number of bytes sent, or a negative error value.
 * 
>*/

PUBLIC Int
IMQ_duSnd(IMQ_DataDesc dataDesc, QU_Head *duHead, Int size)
{
    Int retVal;
    Int origMask;
    DU_View du;

    TM_TRACE((IMQ_modCB, TM_ENTER,
	      "IMQ_duSnd:      dataDesc=0x%x size=%d\n", dataDesc, size));

    du = (DU_View) duHead->first;
    while (du != (DU_View)duHead) {
	DU_View duNext;
	
	duNext = du->next;
	QU_remove((QU_Element *)du);

	origMask = sigblock(sigMask);
	if ((retVal = write(dataDesc, DU_data(du), DU_size(du))) < 0) {
	    EH_problem("IMQ_duSnd: write failed");
	    perror("IMQ_duSnd: write");
	}
	sigsetmask(origMask);

	DU_free(du);

        TM_TRACE((DU_modCB, DU_MALLOC, "IMQ_duSnd: DU_free: du=0x%lx\n", du));

	du = duNext;
    }	

    return ( retVal );
}


/*<
 * Function:    IMQ_dataRcv
 *
 * Description: Receive data.
 *
 * Arguments:   Data descriptor, size.
 *
 * Returns:     Pointer to data buffer.
 * 
>*/

PUBLIC char *
IMQ_dataRcv(IMQ_DataDesc dataDesc, Int *size)
                      
   	       		/* Out */
{
    Int howMany;
    extern int errno;
#define MAXBUF 4096
    static char  dataBuf[MAXBUF];
    Int origMask;

    origMask = sigblock(sigMask);
    if ((howMany = read(dataDesc, dataBuf, sizeof(dataBuf))) < 0) {
	EH_problem("IMQ_datgaRcv: read failed");
	perror("read");
        return (char *)NULL;
    }
    sigsetmask(origMask);
    *size = howMany;

    TM_TRACE((IMQ_modCB, TM_ENTER,
	      "IMQ_dataRcv:    dataDesc=0x%x size=%d\n", dataDesc, *size));

    return ( dataBuf );
}


/*<
 * Function:    IMQ_dataRcvComplete
 *
 * Description: Complete receive data.
 *
 * Arguments:   Data descriptor, data.
 *
 * Returns:     Number of bytes received, or a negative error value.
 *
>*/

PUBLIC Int
IMQ_dataRcvComplete(IMQ_DataDesc dataDesc, char *data)
{
    /* In this case there is nothing to do */
    return (SUCCESS);
}


/*<
 * Function:    IMQ_dataDelete
 *
 * Description: Delete data.
 *
 * Arguments:   Data descriptor.
 *
 * Returns:     None.
 *
>*/

PUBLIC Void
IMQ_dataDelete(IMQ_DataDesc dataDesc)
{
}
