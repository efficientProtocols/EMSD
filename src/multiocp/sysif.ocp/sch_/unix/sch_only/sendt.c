/*
 *  
 *  Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
 * 
 */

/*+
 * File name: send.c 
 *
 * Description: Simple send
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sendt.c,v 1.3 1999/09/30 00:05:05 mohsen Exp $";
#endif /*}*/


#include "estd.h"
#include "oe.h"

#include "tm.h"
#include "eh.h"
#include "du.h"

#include "byteordr.h"
#include "addr.h"

#include "sap.h"
#include "nm.h"

#include "sf.h"
#include "target.h"
#include "udp_if.h"

#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/file.h>
#include "/usr/ucbinclude/sys/signal.h"
#include  <sys/errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "inetaddr.h"

extern int errno;

extern int atoi();
extern void bzero();
extern char *strcat();

Void
main(int argc, char *argv[])
{
char *Usage = "Usage: %s sender_no\n";

    int i;

    struct sockaddr_in sockAddr;
    struct sockaddr_in destAddr;

    int sock;
    Int retVal;
    Int senderPort = 3400;
    Int destPort   = 2002;
    Int senderNo;
    char inStr[100] = "";
    char msg[100] = "Sender ";
    char csendNo[4] = " : ";
    char cmsgNo[4] = " : ";
    int remote = 1;
    char sendName[30] = "Remote sender";
    struct sockaddr_un name;
    Int nameLength;

    if (argc != 2)	/* check if the sender number is given in comm. */
    {
	printf (Usage, argv[0]);
	exit (1);
    }

    if ((senderNo = atoi(argv[1])) < 0) /* get sender no from command line */
    {
	printf ("\nEnter a 0 or positive sender no");
	exit (11);
    }

    if (!strcmp(argv[0], "sendloc")) {
	remote = 0;
        strcpy (sendName, "Local sender");
    }
    
    senderPort += senderNo;

    if (remote == 1) {
        if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
  	    perror("opening stream socket");
	    exit (13);
        }
        bzero((char *) &sockAddr, sizeof(sockAddr));
    
        sockAddr.sin_family      = AF_INET;
        sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sockAddr.sin_port = htons(senderPort);

        bzero((char *) &destAddr, sizeof(destAddr));
        destAddr.sin_family      = AF_INET;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_port 	     = htons(destPort);

        printf ("\nSender %d: src port=%d   des port=%d", 
	    senderNo, senderPort, destPort);

        if ((retVal = bind(sock, (struct sockaddr *)&sockAddr, 
                           sizeof(sockAddr))) != 0) {
	    perror("Bind: binding stream socket");
 	    exit (14);
        }
    }
    else {
        if ( (sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
  	    perror("opening stream socket");
	    exit (13);
        }
        bzero((char *)&name, sizeof(name));
        name.sun_family = AF_UNIX;
        strcpy(name.sun_path, "/tmp/SPT");
#ifdef SOL2
        nameLength = sizeof(name.sun_family) + strlen(name.sun_path);
#else
        nameLength = sizeof(name);
#endif
/*
        if (bind(sock, (struct sockaddr *) &name, nameLength) < 0) {
	   perror("binding name to datagram socket");
	   exit(1);
        }
*/
    }

    csendNo[0] = senderNo + '0';

    for (i = 0; i < 10; i++) {
	strcpy (msg, sendName);
	strcat (msg, csendNo);
        cmsgNo[0]  = i + '0';
	strcat (msg, " -> Message ");
	strcat (msg, cmsgNo);

        printf ("\nEnter data to transmit or simply press ENTER: ");
        gets(inStr);
	strcat (msg, inStr);

        if (remote == 1)
            if ((retVal = sendto(sock, msg, strlen(msg) + 1, 0, 
       		    	     (struct sockaddr *)&destAddr,
	       		     sizeof(destAddr))) != strlen(msg) + 1) 
		printf ("\nsendto retval(%d) != size(%d)", retVal, strlen(msg));
 	    else
		printf ("\n>%s< sent", msg);	
        else
            if (sendto(sock, (char *) msg, strlen(msg) + 1, 0,
		(struct sockaddr *) &name, sizeof(name)) < 0) 
	              perror("sending datagram message");
            else
		      printf ("\n>%s< sent", msg);	
    }

    printf ("\nremote=%d", remote);
    strcpy (msg, "9999");
    if (remote == 1)
        if ((retVal = sendto(sock, msg, strlen(msg) + 1, 0, 
       		    	     (struct sockaddr *)&destAddr,
       			     sizeof(destAddr))) != strlen(msg) + 1) 
	    printf ("\nsendto retval(%d) != size(%d)\n", retVal, strlen(msg));
        else
 	    printf ("\n>%s< sent\n", msg);	
    else
        if (sendto(sock, (char *) msg, strlen(msg) + 1, 0,
		(struct sockaddr *) &name, sizeof(name)) < 0) 
	perror("sending datagram message");
    
} /* main() */

