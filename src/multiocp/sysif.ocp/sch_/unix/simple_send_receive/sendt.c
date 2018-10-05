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
 * File name: sendt.c 
 *
 * Description: A simple sender (doesn't use OCP)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sendt.c,v 1.4 1999/09/30 00:05:07 mohsen Exp $";
#endif /*}*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include "/usr/ucbinclude/sys/signal.h"
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

extern int errno;
extern bzero();

int
main(int argc, char *argv[])
{
char *Usage = "Usage: %s sender_no\n";

    int i;

    struct sockaddr_in sockAddr;
    struct sockaddr_in destAddr;

    int sock;
    int retVal;
    int senderPort = 3400;
    int destPort   = 2002;
    int senderNo;
    char inStr[100] = "";
    char msg[100] = "Sender ";
    char csendNo[4] = " : ";
    char cmsgNo[4] = " : [";

    if (argc != 2) {	/* check if the sender number is given in comm. */
	printf (Usage, argv[0]);
	exit (1);
    }

    if ((senderNo = atoi(argv[1])) < 0) { /* get sender no from command line */
	printf ("\nEnter a 0 or positive sender no");
	exit (11);
    }

    senderPort += senderNo;

    if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("opening strean socket");
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

    if ((retVal = bind(sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr))) 
        != 0) {
	    perror("Bind: binding stream socket");
 	    exit (14);
    }

    csendNo[0] = senderNo + '0';

    for (i = 0; i < 10; i++) {
	strcpy (msg, "Sender ");
	strcat (msg, csendNo);
        cmsgNo[0]  = i + '0';
	strcat (msg, " -> Message ");
	strcat (msg, cmsgNo);

        printf ("\nEnter data to transmit or simply press ENTER: ");
        gets(inStr);
	strcat (msg, inStr);

        if ((retVal = sendto(sock, msg, strlen(msg) + 1, 0, 
       		    	     (struct sockaddr *)&destAddr,
	       		     sizeof(destAddr))) != strlen(msg) + 1) {
		printf ("\nsendto retval(%d) != size(%d)", retVal, strlen(msg));
	} else {
		printf ("\n>%s< sent", msg);	
	}

    }

    strcpy (msg, "9999");
    if ((retVal = sendto(sock, msg, strlen(msg) + 1, 0, 
   		    	(struct sockaddr *)&destAddr,
       			sizeof(destAddr))) != strlen(msg) + 1) {
	printf ("\nsendto retval(%d) != size(%d)\n", retVal, strlen(msg));
    } else {
	printf ("\n>%s< sent\n", msg);	
    }

    exit(0);

} /* main() */

