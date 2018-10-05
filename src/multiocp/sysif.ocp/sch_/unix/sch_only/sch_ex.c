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
 * File name: sch_ex.c
 *
 * Description: Sample program for Secheduler
 *		Doesn't use the ESROS modules
 *		Opens two sockets: one host local AF_UNIX for local sender,
 *				   one internet   AF_INET for remote sender.
 *
-*/

#ifdef SCCS_VER	/*{*/
static char sccs[] = "%W%    Released: %G%";
#endif /*}*/

#include <stdio.h>

#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "estd.h"
#include "tm.h"
#include "getopt.h"
#include "g.h"
#include "eh.h"
#include "sf.h"
#include "sch.h"

#include "udp_if.h"
#include "inetaddr.h"

extern bzero();
extern unlink();
extern udp_receive();

Void SSP_init(), SSP_sapCreate();
Void USER_init();

PUBLIC DU_Pool *G_duMainPool;

PUBLIC G_Env G_env;

#define MAXBFSZ 1528

int stepwise = 0;

Void
main(Int argc, String argv[])
{
    Int c;
    Int retVal;

    TM_init();

    while ((c = getopt(argc, argv, "T:t:S:s")) != EOF) {
	switch ( c ) {
	case 'T':
	case 't':
	    TM_setUp(optarg);
	    break;
	case 'S':
	case 's':
	    stepwise = 1;
	    break;
	case '?':
	default:
	    fprintf(stderr, "\nUsage: %s [-t] [-s]\n", argv[0]);
	    exit(1);
	}
    }

    G_duMainPool = DU_buildPool (MAXBFSZ, 20, 40);

    G_env.hardReset = FALSE;
    G_env.softReset = FALSE;

    while ( !G_env.hardReset ) {
	SCH_init(K_schQuLen);
	SSP_init();
	USER_init();
	TM_validate();

	while ( !G_env.softReset ) {

            if (stepwise == 1) {
		printf("\nIn step mode");
		getchar();
	    }

	    if ((retVal = SCH_block()) < 0)
	   	fprintf(stderr, 
			"main: SCH_block: interrupt or error, return value=%d", 
			retVal);
	    
	    SCH_run();
	}

	SCH_term();
    }

    exit(0);
}

/* 
 * USER_ module.
 */

Void 
userEventPrim()
{
    printf("SSP Event Primitive, invoked through the scheduler\n");
    G_env.softReset = G_env.hardReset = TRUE;
}

Void USER_init()
{
    SSP_sapCreate(userEventPrim);
}

Int
udp_recv(Int *sockfd)
{
    unsigned char buff[2000];
    Int retVal;
    struct sockaddr_in srcAddr;
    int srcAddrsiz;

    if ((retVal = recvfrom (*sockfd, buff, 1000, 0, 
		(struct sockaddr *)&srcAddr, &srcAddrsiz)) < 0)
	    printf ("\nrecvfrom failed\n");
    else {
    	    buff[retVal + 1]='\0';
   	    printf("\nUDP received: >%s<\n", buff);

    	    SCH_submit ((Void *)udp_recv, (Ptr)sockfd, (SCH_Event)*sockfd, 
			"Receive UDP");
         }
    return (SUCCESS);
}

Int
invokeReq(Int *sockfd)
{
    unsigned char buff[2000];
    Int retVal;
    struct sockaddr_in srcAddr;
    int srcAddrsiz;

    if ((retVal = recvfrom (*sockfd, buff, 1000, 0, 
		(struct sockaddr *)&srcAddr, &srcAddrsiz)) < 0)
	    printf ("\ninvokeReq: recvfrom failed");
    else {
    	    buff[retVal + 1]='\0';
   	    printf("\nInovokeReq received: >%s<\n", buff);

    	    SCH_submit ((Void *)invokeReq, (Ptr)sockfd, (SCH_Event)*sockfd, 
			"Receive UDP");
         }
    return (SUCCESS);
}

/*
 * Some Service Provider (SSP_) Module.
 */
Void 
SSP_init()
{
    /* Initialization could have been done here */
}

Void 
SSP_sapCreate(Void (*eventPrim)())
{
    struct sockaddr_un name;
    Int nameLength;
    struct sockaddr_in srcAddr;
    struct sockaddr_in locAddr;

    static int sock;
    static int sock2;

/*----- Remote SAP -----*/

    bzero((char *) &srcAddr, sizeof(srcAddr));
    bzero((char *) &locAddr, sizeof(locAddr));
    locAddr.sin_family      = AF_INET;
    locAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    locAddr.sin_port 	    = htons(2002);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {	printf ("\nsocket failed");
        exit(2);
    }

    if (bind(sock, (struct sockaddr *)&locAddr, sizeof(locAddr)) < 0)
    { 
       perror("\nbind failed");
       exit(1);
    }

    SCH_submit ((Void *)udp_recv, (Ptr)&sock, (SCH_Event)sock, "Recieve UDP");

/*----- Local SAP ------*/

    if ((sock2 = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    	printf ("\nsocket failed");
        exit(2);
    }
    if (unlink("/tmp/SPT") == SUCCESS) {	
	EH_unixProblem("PSQ_init: Removing Left Over File");
    }
    bzero((char *)&name, sizeof(name));
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, "/tmp/SPT");
#ifdef SOL2
    nameLength = sizeof(name.sun_family) + strlen(name.sun_path);
#else
    nameLength = sizeof(name);
#endif

    if (bind(sock2, (struct sockaddr *) &name, nameLength) < 0) {
	perror("binding name to datagram socket");
	exit(1);
    }

    SCH_submit ((Void *)invokeReq, (Ptr)&sock2, (SCH_Event)sock2, "Receive request");

}

