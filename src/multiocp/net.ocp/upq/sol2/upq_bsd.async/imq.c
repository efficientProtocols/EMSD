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
 * Functions:
 *   IMQ_init(Int nuOfElems)
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
static char rcs[] = "$Id: imq.c,v 1.29 1997/01/15 19:49:41 kamran Exp $";
#endif /*}*/

#include  "estd.h"		/* Extended stdio.h */
#include  "eh.h"		/* Exception Handler */
#include  "queue.h"		/* Queue */
#include  "imq.h"		/* Inter-module Queue */
#include  "tm.h"		/* Trace Module */
#include  "sf.h"		/* System Facilities */
#include  "byteordr.h"		/* Byte Order */
#include  "sch.h"		/* Scheduler */

#include  "target.h"		/* Target Environment */

#define BUFFER    8192
#define MAX_BLOCK 16384
#define MAX_INCOMING_CONNETCIONS_BACKLOG    50

#ifndef MSDOS
extern int errno;
#endif
extern unlink();

#ifdef TM_ENABLED
extern TM_ModuleCB *DU_modCB;		/* Data Unit Module tracing handler*/
#define DU_MALLOC TM_BIT10		/* malloc tracing */

TM_ModuleCB *IMQ_modCB;			/* Inter module queue tracing handler*/
#define IMQ_BLOCKQUEUE TM_BIT10		/* Block mode queue tracing enalbled */
#endif

extern DU_Pool *G_duMainPool;		/* Data Unit main pool */

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
    IMQ_PrimDesc descriptor;
} IMQ_Info;

typedef struct imq_InfoHead {
    IMQ_Info *first;
    IMQ_Info *last;
} imq_InfoHead;

static imq_InfoHead availQu;		/* Available Queue */
static imq_InfoHead activeQu;		/* Active Queue */

#if TM_ENABLED
static int nuOfAvailQus = IMQ_QUEUES;	/* Number of available queues */
#endif
/* 
 * Set the bit corresponding to each signal that should be 
 * blocked for delivery.
 */
static int sigMask;			/* Signal mask */

IMQ_Info *imq_descToQuInfo(IMQ_PrimDesc sock);


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
IMQ_init(Int nuOfElems)
{
    static Bool virgin = TRUE;
    Int i;
    IMQ_Info *elem;

    sigMask = sigmask(SIGALRM) | sigmask(SIGUSR1) | sigmask(SIGUSR2);

    if ( !virgin ) {
	return 0;
    }
    virgin = FALSE;

#ifdef TM_ENABLED
    if (TM_OPEN(IMQ_modCB, "IMQ_") == NULL) {
	EH_problem("IMQ_init (ASYNC): TM_open IMQ_ failed");
	return ( FAIL );
    }

    TM_TRACE((IMQ_modCB, IMQ_TRACE_INIT, "***** NON-Blocking upq_bsd *****\n"));
#endif
   
    QU_INIT(&availQu);
    QU_INIT(&activeQu);
    for (i = 0; i < nuOfElems; i++) {
       	if ((elem=(IMQ_Info *) SF_memGet(sizeof(*elem))) == (IMQ_Info *)0) {
	    EH_fatal("IMQ_init: Out of memory, SF_memGet failed\n");
        }
        QU_INIT(elem);
        QU_INSERT(elem, &availQu);
    }

    return ( SUCCESS );

} /* IMQ_init() */


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
    String retVal = (String)0;

    sprintf(name, "/tmp/SP%02dXXXXXX", chnlNu);
    mktemp(name);

    ++chnlNu;
    retVal = name;

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_nameCreate (ASYNC): name=%s\n", name));

    return ( retVal );

} /* IMQ_nameCreate() */


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

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_keyCreate (ASYNC):  key=%s, name=%s, id=%d\n",
	     key, name, subID));

    return ( key );

} /* IMQ_keyCreate() */


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
IMQ_acceptConn(IMQ_Key key)
{
    Int retVal;
    struct sockaddr_un addr;
    struct sockaddr_un fromAddr;
    int fromLen;
    int sock;
    Int IMQ_origMask;
    Int statusFlags = 0;

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_acceptConn:    key=%s\n", key));

    /* Common for Calling and Called */
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, key);

    IMQ_origMask = sigblock(sigMask);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sigsetmask(IMQ_origMask);
    if (sock < 0) {
	perror("opening stream socket");
        EH_problem("IMQ_acceptConn (ASYNC): socket function failed");
	return (IMQ_PrimDesc)NULL;
    }


    IMQ_origMask = sigblock(sigMask);

#ifdef SOL2
    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "\n***Compiled WITH SOL2 directive*** \n"));

    retVal   = bind(sock, (struct sockaddr *) &addr,
		    strlen(addr.sun_path) + sizeof(addr.sun_family));
#else
    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "\n***Compiled WITHOUT SOL2 directive*** \n"));

    retVal   = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
#endif
/*    sigsetmask(IMQ_origMask); */
    if (retVal != SUCCESS) {
/*  if (errno != EADDRINUSE) { */
 	EH_problem("IMQ_acceptConn (ASYNC): bind failed\n");
	perror("IMQ_acceptConn (ASYNC): binding stream socket");
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	         "\nIMQ_acceptConn (ASYNC): bind failed\n"));
        close(sock);
        SCH_allTaskDelete(sock, NULL, NULL);
#if 0
        shutdown(sock, 2);
#endif
	return (IMQ_PrimDesc)NULL;
    }
/*    IMQ_origMask = sigblock(sigMask); */

    retVal = listen(sock, MAX_INCOMING_CONNETCIONS_BACKLOG);  

    sigsetmask(IMQ_origMask);
    if (retVal) {
	EH_problem("IMQ_primGet (ASYNC): listen function failed");
	perror("IMQ_primGet (ASYNC):listen");
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	         "\nIMQ_acceptConn (ASYNC): listen function failed\n"));
        close(sock);
        SCH_allTaskDelete(sock, NULL, NULL);
#if 0
        shutdown(sock, 2);
#endif
	return (IMQ_PrimDesc)NULL;
    }
    IMQ_origMask = sigblock(sigMask);
    fromLen  = sizeof(fromAddr);

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_primGet (ASYNC):    accept(%d) on %s\n",
    	     sock, addr.sun_path));

    if ((statusFlags = fcntl(sock, F_GETFL, 0)) < 0) {
 	perror ("fcntl: ");
	EH_problem("IMQ_primGet (ASYNC): fcntl failed");
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	         "\nIMQ_acceptConn (ASYNC): fcntl function failed\n"));
        close(sock);
        SCH_allTaskDelete(sock, NULL, NULL);
#if 0
        shutdown(sock, 2);
#endif
	return (IMQ_PrimDesc)NULL;  /* return error_value */
    }

    statusFlags |= FNDELAY;
    if (fcntl (sock, F_SETFL, statusFlags) < 0) {
	perror ("fcntl: ");
	EH_problem("IMQ_primGet (ASYNC): fcntl failed");
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	         "\nIMQ_acceptConn (ASYNC): fcntl function failed\n"));
        close(sock);
        SCH_allTaskDelete(sock, NULL, NULL);
#if 0
        shutdown(sock, 2);
#endif
	return (IMQ_PrimDesc)NULL;  /* return error_value */
    }
    sigsetmask(IMQ_origMask);

    return (IMQ_PrimDesc)sock;

} /* IMQ_acceptConnect () */


/*<
 * Function:    IMQ_connect
 *
 * Description: Connect to socket.
 *
 * Arguments:   Queue descriptor.
 *
 * Returns:     
 * 
>*/

IMQ_PrimDesc
IMQ_connect(IMQ_PrimDesc queue)
{
    int retVal;
    struct sockaddr_un fromAddr;
    int fromLen;

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_connect (ASYNC): Before accept, fd=%d\n", queue));

    fromLen  = sizeof(fromAddr);

    retVal = accept(queue, (struct sockaddr *) &fromAddr, &fromLen);

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_connect (ASYNC): After  accept, fd=%d\n", queue));

    if (retVal < 0) {
	EH_problem("IMQ_primGet (ASYNC): accept function failed");

        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	         "IMQ_primGet (ASYNC):    accept(%d) failed, "
		 "retVal=%d, errno=%d\n",
	         queue, retVal, errno));

	perror("accept stream socket");

	close(queue);
        SCH_allTaskDelete(queue, NULL, NULL);
#if 0
	shutdown(queue, 2);
#endif
        return (IMQ_PrimDesc)NULL;
    } 

    if (IMQ_blockQuAdd(retVal) == NULL) {

        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	         "IMQ_primGet (ASYC): IMQ_blockQuAdd failed"));
	close(queue);
        SCH_allTaskDelete(queue, NULL, NULL);
#if 0
	shutdown(queue, 2);
#endif
	return (IMQ_PrimDesc) NULL;
    }

    close(queue);
    SCH_allTaskDelete(queue, NULL, NULL);
#if 0
    shutdown(queue, 2);
#endif

    return (IMQ_PrimDesc) retVal;

} /* IMQ_connect() */


/*<
 * Function:    IMQ_clientConnect
 *
 * Description: Connect client.
 *
 * Arguments:   Key.
 *
 * Returns:     Primitive description.
 * 
>*/


PUBLIC IMQ_PrimDesc
IMQ_clientConnect(IMQ_Key key)
{
    Int retVal;
    struct sockaddr_un addr;
    Int sock;
    Int len;
    Int IMQ_origMask;

#if TM_ENABLED
    static Int dotCounter = 0;
#endif

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_clientConn (ASYNC):    key=%s\n", key));

    /* Common for Calling and Called */
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, key);

    IMQ_origMask = sigblock(sigMask);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sigsetmask(IMQ_origMask);
    if (sock < 0) {
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
		 "IMQ_primGet (ASYNC): socket function failed"));
	perror("opening stream socket");
        EH_problem("IMQ_primGet (ASYNC): socket function failed");
	return (IMQ_PrimDesc)NULL;
    }

    IMQ_origMask = sigblock(sigMask);

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_primGet (ASYNC):    connect(%d) to %s...\n",
	     sock, addr.sun_path));

#ifdef NORACE 		/* Set NORACE when there is race condition */
    sleep(2);	/* Avoid the race condition */
#endif

#ifdef SOL2		/* SUN Solaris 2 */
    len = strlen(addr.sun_path) + sizeof(addr.sun_family);
#else
    len = sizeof(addr);
#endif

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	     "IMQ_primGet (ASYNC): Trying to connect, fd=%d\n", sock));

    /* 	Synchronous: Wait for connection to be established. Client supposedly 
	doesn't have much to do without connection to server */
    while (connect(sock, (struct sockaddr *) &addr, len) < 0) {
#if TM_ENABLED
    	if (++dotCounter > 2000) {
	    extern FILE *tmFile;
    	    fprintf(tmFile, ".");
	    dotCounter = 0;
    	}
#endif
    }

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     "IMQ_primGet (ASYNC): Connected, fd=%d\n", sock));

    sigsetmask(IMQ_origMask);

/*  	unlink(addr.sun_path); */

    {
        Int statusFlags = 0;

	if ((statusFlags = fcntl(sock, F_GETFL, 0)) < 0) {
            TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     	     "IMQ_primGet (ASYNC): fcntl failed"));

	    perror ("fcntl: ");
	    EH_problem("IMQ_primGet (ASYNC): fcntl failed");
	    close(sock);
	    return (IMQ_PrimDesc)NULL;
	}

	statusFlags |= FNDELAY;
        if (fcntl (sock, F_SETFL, statusFlags) < 0) {
            TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     	     "IMQ_primGet (ASYNC): fcntl failed"));
	    perror ("fcntl: ");
	    EH_problem("IMQ_primGet (ASYNC): fcntl failed");
	    close(sock);
	    return (IMQ_PrimDesc)NULL;
	}
    }

    retVal = sock;

    if (IMQ_blockQuAdd(sock) == NULL) {
        TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	     	 "IMQ_primGet (ASYNC): IMQ_blockQuAdd failed"));
    	close(sock);
	return (IMQ_PrimDesc) NULL;
    }

    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION, 
	      "IMQ_primGet (ASYNC):    retVal=%d\n", retVal));

    return ( (IMQ_PrimDesc)retVal );

} /* IMQ_clientConnect(IMQ_Key key) */


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
    Int retVal;					/* Return Value */
    Int IMQ_origMask;				/* Original signal mask */
    unsigned char ob[BUFFER], *outBuf = ob;

    if (primDesc == 0) {
	return (FAIL);
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
		 "IMQ_primSnd: Invalid primitive (socket) desciptor"));
    }

#ifdef TM_ENABLED
{
    struct tmps {
	long mtype;
	int type;
    };
    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	     "IMQ_primSnd (ASYNC):    primDesc=%d mtype=0x%x size=%d\n", 
             primDesc, data->mtype, size, ((struct tmps *)data)->type));
}
#endif

    IMQ_origMask = sigblock(sigMask);
 
    BO_put4(outBuf, size);		/* BO_put4 increments outBuf pointer */
    OS_copy(outBuf, (char *)data, size);

    if ((retVal = write(primDesc, ob, size+4)) < 0) {
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
		 "IMQ_primSnd: write failed. RetVal=%d", retVal));
	EH_problem("IMQ_primSnd (ASYNC): write failed");
	perror("IMQ_primSnd (ASYNC): write");
	return -2;
    }
    if (retVal != size+4) {
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
		 "IMQ_primSnd: write failed to write %d bytes\n", size+4));
	EH_problem("IMQ_primSnd: write failed to write total bytes!\n");
    }

    sigsetmask(IMQ_origMask);

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	     "IMQ_primSnd (ASYNC):    primDesc=%d  "
	     "bytes_written=%d  input size=%d + 4 (block size header)\n", 
	     primDesc, retVal, size));

    return ( retVal-4 );
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
    Int retVal;		/* Return value of this function */
    Int readRetVal;	/* Return value of 'read' function */
    Int noOfBytes;	/* Number of bytes */
    extern int errno;	/* Error number */
    Int IMQ_origMask;	/* Original interrupt mask */
    IMQ_Info *quInfo;	/* Queue info structure */
    imq_Elem *elem;	/* Queue element */
    Ptr locData;	/* Local data */
    Int blockSize = 0;	/* Block size */
    Int bytes;		/* Number of bytes */

    unsigned char dataBuf[BUFFER+1], *buf = dataBuf;	/* Data buffer */
    
    if (primDesc == 0) {
        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR,
	         "IMQ_primRcv (ASYNC):    File descriptor 0 not supported\n"));
	EH_problem("IMQ_primRcv: File descriptor (stdin) not supported");
	return -2;
    }

    IMQ_origMask = sigblock(sigMask);
  
    quInfo = imq_descToQuInfo(primDesc);

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	    "IMQ_primRcv (ASYNC):    Before read: primDesc=%d size=%d\n",
	    primDesc, size));

    for (buf = dataBuf, readRetVal = noOfBytes = read(primDesc, buf, BUFFER);
	 noOfBytes > 0;
	 noOfBytes = read(primDesc, buf, BUFFER)) {

	TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
		 "IMQ_primRcv (ASYNC):    After read: SUCCESS  "
		 "noOfBytes=%d (fd=%d)\n", 
		 noOfBytes, primDesc));

      	for (; noOfBytes > 0; noOfBytes -= (blockSize+4)) {

	    BO_get4(blockSize, buf);

            if ((locData = (Ptr) SF_memGet(blockSize)) == 0) {
	        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	    	         "IMQ_primRcv: Out of memory, SF_memGet failed"));
	    	EH_problem("IMQ_primRcv: Out of memory, SF_memGet failed");
	    	return (FAIL);
	    }

   	    if (blockSize > noOfBytes) {

	    	if (blockSize > MAX_BLOCK) {
	            TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	    	             "IMQ_primRcv: Block size is too large"));
	            EH_problem("IMQ_primRcv (ASYNC): block size "
			       "is too large\n")
	  	    return (FAIL);
	    	}

	        OS_copy(locData, buf, noOfBytes - 4);

	    	if ((bytes = read(primDesc, locData + noOfBytes - 4, 
				  blockSize - noOfBytes + 4)) 
		    != blockSize - noOfBytes + 4) {	 
	            TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	    	             "IMQ_primRcv: Block size doesn't match"));
		    EH_problem("IMQ_primRcv (ASYNC): Block size doesn't match "
			       "the available data size\n");
	  	    return (FAIL);
	        }

	    } else {
	    	OS_copy(locData, buf, blockSize);
	    }
	
	    if ((elem = (imq_Elem *) SF_memGet(sizeof(*elem))) == 0) {
	        TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
	    	         "IMQ_primRcv: Out of memory, SF_memGet failed"));
	        EH_problem("IMQ_primRcv: Out of memory, SF_memGet failed");
	        return (FAIL);
	    }

	    elem->data = locData;
	    elem->size = blockSize;
	    QU_INIT(elem);
	    QU_INSERT(elem, &quInfo->elemQuHead);
	    buf += (blockSize);

        } /* for */

        buf = dataBuf; 

    } /* for */

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
	     "IMQ_primRcv (ASYNC):    After read loop: (fd=%d) "
	     "readRetVal=%d\n", 
	     primDesc, readRetVal));

    if (readRetVal < 0) {

      TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
		"IMQ_primRcv (ASYNC):    After failed read: (fd=%d)\n", 
		primDesc));

      switch (errno) {
      case EINTR:
	perror("IMQ_primRcv (ASYNC): read:");
	retVal = -1;
	break;

      case ECONNABORTED:
        TM_TRACE((IMQ_modCB, IMQ_TRACE_WARNING, 
		 "IMQ_primRcv (ASYNC): Connection aborted\n"));
	perror("IMQ_primRcv (ASYNC): read:");
	IMQ_primDelete(primDesc, NULL);
	return -2;

      case EWOULDBLOCK:
	/* perror("IMQ_primRcv (ASYNC): (It's OK): read:");
	   tried to read a non-blocking empty socket. 
	   perror: Rsource temporarily unavailable */

        TM_TRACE((IMQ_modCB, IMQ_TRACE_WARNING, 
		 "IMQ_primRcv (ASYNC): Nonblocking empty socket (OK)\n"));

	retVal = -1;
	break;

      default:
	perror("IMQ_primRcv (ASYNC): read failed:");
        TM_TRACE((IMQ_modCB, IMQ_TRACE_WARNING, 
		 "IMQ_primRcv (ASYNC): read failed. Errono=%d\n", errno));
	retVal = -1;   /* retVal = -2? */
      }
    }

    if ((elem = quInfo->elemQuHead.first) != (imq_Elem *) &quInfo->elemQuHead) {

        TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
		 "IMQ_primRcv (ASYNC): Data found in imq queue\n"));

	if (size < elem->size) {
	    EH_problem("IMQ_primRcv: buffer smaller than data size\n");
            TM_TRACE((IMQ_modCB, IMQ_TRACE_ERROR, 
		     "IMQ_primRcv: size=%d  elem->size=%d\n", 
		     size, elem->size));
	    retVal = -1;
	}
	QU_REMOVE(elem);
	OS_copy(data, elem->data, elem->size);
	retVal = elem->size;
	SF_memRelease(elem->data);
	SF_memRelease(elem);

    } else {
	retVal = -1;
	errno = 0;
    }	

    sigsetmask(IMQ_origMask);

    if (readRetVal == 0) {
	retVal = -2;
    }

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	    "IMQ_primRcv (ASYNC):    Before return: # of bytes=%d "
	    "primDesc=%d mtype=0x%x size=%d\n",
	    retVal, primDesc, data->mtype, size));

    return ( retVal );

} /* IMQ_primRcv() */


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
IMQ_primDelete(IMQ_PrimDesc primDesc, char *chnlName)
{
    TM_TRACE((IMQ_modCB, IMQ_TRACE_VALIDATION,
	    "IMQ_primDelete: primDesc=0x%x\n", primDesc));
#if 0
    shutdown((int)primDesc, 2);
    if (chnlName != NULL) {
    	unlink(IMQ_keyCreate(chnlName, 0));
    	unlink(IMQ_keyCreate(chnlName, 1));
    }
#endif

    close(primDesc);
    SCH_allTaskDelete(primDesc, NULL, NULL);
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
    Int sock;
    Int len;
    Int IMQ_origMask;

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	     "IMQ_dataGet (ASYNC):    key=%s, size=%d\n", 
             key, size));

    /* Common for Calling and Called */
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, key);

    IMQ_origMask = sigblock(sigMask);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sigsetmask(IMQ_origMask);
    if (sock < 0) {
        EH_problem("IMQ_dataGet (ASYNC): socket function failed");
	perror("IMQ_dataGet (ASYNC): opening strean socket");
        return (IMQ_DataDesc)NULL;
    }

    IMQ_origMask = sigblock(sigMask);

#ifdef SOL2
    len = strlen(addr.sun_path) + sizeof(addr.sun_family);
#else
    len = sizeof(addr);
#endif

    retVal = bind(sock, (struct sockaddr *) &addr, len);

    sigsetmask(IMQ_origMask);

    if (retVal == SUCCESS) {

        TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
		 "IMQ_dataGet (ASYNC):    Create\n"));

	IMQ_origMask = sigblock(sigMask);
	retVal = listen(sock, 1);
	sigsetmask(IMQ_origMask);
	if (retVal) {
            EH_problem("IMQ_dataGet (ASYNC): listen function failed");
	    perror("IMQ_dataGet (ASYNC): listen");
            return (IMQ_DataDesc)NULL;
	}
	while (TRUE) {
	    IMQ_origMask = sigblock(sigMask);
	    fromLen  = sizeof(fromAddr);
	    retVal   = accept(sock, (struct sockaddr *)&fromAddr, &fromLen);
	    sigsetmask(IMQ_origMask);
	    if (retVal < 0) {
                EH_problem("IMQ_dataGet (ASYNC): accept function failed");
		printf("sock=%d, retVal=%d, errno=%d\n", sock, retVal, errno);
		perror("IMQ_dataGet (ASYNC): accept stream socket");
                return (IMQ_DataDesc)NULL;
	    } else {
		break;
	    }
	}
    } else {
	/* So Bind Failed */
	if (errno != EADDRINUSE) {
            EH_problem("IMQ_dataGet (ASYNC): bind failed");
	    perror("IMQ_dataGet (ASYNC): binding stream socket");
            return (IMQ_DataDesc)NULL;
	}

        TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
		 "IMQ_dataGet (ASYNC):    Connect\n"));

	IMQ_origMask = sigblock(sigMask);

#ifdef DJL /* Define DJL if there is race condition */
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
	sigsetmask(IMQ_origMask);

	unlink(addr.sun_path);
	retVal = sock;
    }

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	     "IMQ_dataGet (ASYNC):    retVal=%d\n", retVal));

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
    Int IMQ_origMask;

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	"IMQ_dataSnd (ASYNC):    dataDesc=0x%x data=0x%x size=%d\n", 
        dataDesc, data, size));

    IMQ_origMask = sigblock(sigMask);
    if ((retVal = write(dataDesc, data, size)) < 0) {
	EH_problem("IMQ_dataSnd (ASYNC): write failed");
	perror("write");
    }
    sigsetmask(IMQ_origMask);

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
    Int retVal = 0;
    Int IMQ_origMask;
    DU_View du;

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	     "IMQ_duSnd (ASYNC):      dataDesc=0x%x size=%d\n", dataDesc, size));

    du = (DU_View) duHead->first;
    while (du != (DU_View)duHead) {
	DU_View duNext;
	
	duNext = du->next;
	QU_REMOVE(du);

	IMQ_origMask = sigblock(sigMask);
	if ((retVal = write(dataDesc, DU_data(du), DU_size(du))) < 0) {
	    EH_problem("IMQ_duSnd (ASYNC): write failed");
	    perror("IMQ_duSnd (ASYNC): write");
	}
	sigsetmask(IMQ_origMask);

	DU_free(du);

        TM_TRACE((DU_modCB, DU_MALLOC, 
		 "IMQ_duSnd (ASYNC): DU_free: du=0x%lx\n", du));

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
    Int IMQ_origMask;

    IMQ_origMask = sigblock(sigMask);
    if ((howMany = read(dataDesc, dataBuf, sizeof(dataBuf))) < 0) {
	EH_problem("IMQ_datgaRcv (ASYNC): read failed");
	perror("read");
        return (char *)NULL;
    }
    sigsetmask(IMQ_origMask);
    *size = howMany;

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL,
	     "IMQ_dataRcv (ASYNC):    dataDesc=0x%x size=%d\n", dataDesc, *size));

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


/*<
 * Function:    imq_descToQuIndo
 *
 * Description: Get queue info for a give descriptor.
 *
 * Arguments:	Descriptor
 *
 * Returns: 	Queue Info.
 *
>*/

IMQ_Info *
imq_descToQuInfo(IMQ_PrimDesc quDesc)
{
    IMQ_Info *elem;

    for (elem = activeQu.first;
	    elem != (IMQ_Info *)&activeQu; 
	    elem = elem->next) {
	if (quDesc == elem->descriptor) {
	    /* we Got it */
	    break;
	}
    }

    if (elem == (IMQ_Info *)&activeQu) {
      return (IMQ_Info *)NULL;
    }

    return elem;
}


/*<
 * Function:    IMQ_blcokQuAdd
 *
 * Description: Add a block queue.
 *
 * Arguments:	Socket descriptor.
 *
 * Returns: 
 *
>*/

Void *
IMQ_blockQuAdd(IMQ_PrimDesc quDesc)
{
    IMQ_Info *elem;

    for (elem = activeQu.first;
   	 elem != (IMQ_Info *)&activeQu; 
	 elem = elem->next) {
	if (quDesc == elem->descriptor) {
            TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
		     "IMQ_blockQuAdd (ASYNC): qu found: elem=%x\n", &elem));
	    /* we Got it */
	    break;
	}
    }

    if (elem == (IMQ_Info *)&activeQu) {
	/* So it is not already active and must be created */
	if ( (elem = availQu.first) == (IMQ_Info *) &availQu ) {
	    EH_problem("IMQ_primGet: Out of Queues\n");
	    return (Void *)NULL;
	}
        elem->descriptor = quDesc;
	QU_INIT(&elem->elemQuHead);
	QU_MOVE(elem, &activeQu);

        TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
	         "IMQ_blockQuAdd (ASYNC): qu added: elem=%x  "
		 "elemQuHead=%x first=%x\n", 
		 elem, &elem->elemQuHead, elem->elemQuHead.first));
    } 

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
	     "IMQ_blockQuAdd (ASYNC): elem=%x   elemQuHead=%x  first=%x\n", 
	     elem, &elem->elemQuHead, elem->elemQuHead.first));

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
	     "IMQ_blockQuAdd: IMQ queue ADDED: %d    availQus=%d\n",
	     quDesc, --nuOfAvailQus));

    return (Void *) elem;

} /* IMQ_blockQuAdd() */



/*<
 * Function:    IMQ_blockQuRemove
 *
 * Description: Remove block queue.
 *
 * Arguments:	Queue (Socket)
 *
 * Returns: 	None.
 *
>*/

Void
IMQ_blockQuRemove(IMQ_PrimDesc quDesc)
{
    IMQ_Info *quInfo;

    OS_EnterAtomic();
    quInfo = imq_descToQuInfo(quDesc);
    OS_LeaveAtomic();

    QU_MOVE(quInfo, &availQu);

    TM_TRACE((IMQ_modCB, IMQ_TRACE_DETAIL, 
	     "IMQ_blockQuRemove: IMQ queue REMOVED: %d  availQus=%d\n", 
	     quDesc, ++nuOfAvailQus));

} /* IMQ_blockQuRemove() */
