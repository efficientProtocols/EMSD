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
 * Description: Simple send program
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sendt.c,v 1.4 1999/09/30 00:05:14 mohsen Exp $";
#endif /*}*/

#include <stdlib.h>
#include <string.h>
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

#include "estd.h"
#include "oe.h"
#include "tm.h"
#include "eh.h"
#include "du.h"
#include "byteordr.h"
#include "sap.h"
#include "nm.h"
#include "sf.h"
#include "addr.h"
#include "inetaddr.h"
#include "target.h"
#include "udp_if.h"

extern int errno;

#define SENDER_PORT  3400
#define DEST_PORT    2002

PUBLIC DU_Pool *G_duMainPool;

Int
main(int argc, char *argv[])
{
    char *Usage = "Usage: %s sender_no\n";

    int i;

    UDP_SapDesc locSapDesc;		/* UDP local SAP descriptor */
    T_SapSel  locTsapSel;		/* Local Transport SAP selector */
    T_SapSel  remTsapSel;		/* Remote Transport SAP Selector */
    N_SapAddr remNsapAddr = {4, {198, 62, 92, 10} };

    Int senderPort = SENDER_PORT;
    Int destPort   = DEST_PORT;

    Int retVal;

    Int senderNo;
    char inStr[100];
    char msg[200]   = "Remote Sender  ";
    Int strLen;

    DU_View du;

    if (argc != 2)	/* check if the sender number is given */
    {
	printf (Usage, argv[0]);
	exit (1);
    }

    if ((senderNo = atoi(argv[1])) < 0) /* get sender no from command line */
    {
	printf ("\nEnter a 0 or positive sender no");
	exit (11);
    }

    senderPort += senderNo;

    TM_init();		/* Initialize Trace Module */
    UDP_init(5);		/* Initialize UDP Module   */

    INET_portNuToTsapSel (senderPort, &locTsapSel);
    INET_portNuToTsapSel (destPort,   &remTsapSel);

    UDP_sapUnbind (&locTsapSel);

    if ( (locSapDesc = UDP_sapBind (&locTsapSel, (Void *)NULL)) <= 0) {
	EH_fatal("UDP_sapBind failed");
    }

    G_duMainPool = DU_buildPool(MAXBFSZ, 20, 40);
    du = DU_alloc(G_duMainPool, 50);

    strLen = strlen (msg);
    msg[strLen - 1] = senderNo + '0';
    strcat (msg, " => Message   : [");
    strLen = strlen (msg);

    printf ("\nSender %d: src port=%d   des port=%d", 
	    senderNo, senderPort, destPort);

    for (i = 0; i < 10; i++) {
        msg[strLen - 5]  = i + '0';
        msg[strLen]  = '\0';

        printf ("\nEnter data to transmit or simply press ENTER: ");
        gets(inStr);
	strcat (msg, inStr);
	strcpy (DU_data(du), msg);

            if ((retVal = UDP_dataReq((UDP_SapDesc)locSapDesc, 
					&remTsapSel, 
					&remNsapAddr, 
					(DU_View) du)) != 0) 
		printf ("\nUDP_dataReq failed");
 	    else
		printf ("\n%s]", msg);	
    }

    strcpy (msg, "9999");
    strcpy (DU_data(du), msg);

    if ((retVal = UDP_dataReq((UDP_SapDesc) locSapDesc, 
				&remTsapSel, 
				&remNsapAddr, 
				(DU_View) du)) != 0)
		printf ("\nUDP_dataReq failed");
    else
 	printf ("\n\nLast message: [%s]\n\n", msg);	

    exit (0);

} /* main() */

