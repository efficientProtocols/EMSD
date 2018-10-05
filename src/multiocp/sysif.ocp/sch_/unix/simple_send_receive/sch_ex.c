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
 * Description: A simple receiver with socket (doesn't use OCP).
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */


#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: sch_ex.c,v 1.2 1996/07/06 01:13:20 mohsen Exp $";
#endif /*}*/

#include <stdio.h>
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

#include "estd.h"
#include "tm.h"
#include "getopt.h"
#include "g.h"
#include "eh.h"
#include "sch.h"
#include "udp_if.h"
#include "inetaddr.h"

extern errno;
extern bzero();

Int
main(Int argc, String argv[])
{
    char buff[100];
    int sock;
    struct sockaddr_in srcAddr;
    struct sockaddr_in locAddr;
    int srcAddrsiz;
    fd_set readFdSet;
    Int     maxFd = 10;
    Int activeFds;
    Int retVal;

    FD_ZERO(&readFdSet);

    bzero((char *) &srcAddr, sizeof(srcAddr));
    bzero((char *) &locAddr, sizeof(locAddr));
    locAddr.sin_family      = AF_INET;
    locAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    locAddr.sin_port 	    = htons(2002);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {	
	perror ("\nsocket failed");
        exit(1);
    }

    FD_SET(sock, &readFdSet);

    if (bind(sock, (struct sockaddr *)&locAddr, sizeof(locAddr)) < 0) {   
	perror("\nbind failed");
        exit(2);
    }

    for(; ;) {
	printf ("\nBefore select:\n");
	if ((activeFds = select(maxFd, &readFdSet, (fd_set *)0, (fd_set *)0,
            (struct timeval *)0)) < 0) {
	    switch (errno) {
    	        case EINTR: 
		    printf ("\nselect: signal EINTR seen");
		    break;
	        case EBADF: 
		    printf ("\nselect: signal EBADF seen");
		    break;
	    	default:
		    perror("SCH_block: select failed: ");
		    return -3;
  	    }
	} else {
            if (FD_ISSET(sock, &readFdSet)) {
    	        if ((retVal = recvfrom (sock, buff, 1000, 0, 
				(struct sockaddr *)&srcAddr, &srcAddrsiz)) < 0) {
		    printf ("\nrecvfrom failed");
	        } else {
   	    	    buff[retVal + 1]='\0';
    	    	    printf("\nData received: %s]", buff);
	        }
	    } else {
		printf ("\nSelect exited without activity on sock");
	    }
	}

        if (!strncmp(buff, "9999", 4)) {
   	    break;
	}
    }

    putchar('\n'); 
    exit(0);

} /* main() */

