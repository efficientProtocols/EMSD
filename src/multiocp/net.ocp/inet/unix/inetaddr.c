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
 * File name: inetaddr.c 
 *
 * Description: 
 *
 * Functions:
 *   INET_nsapAddrToin_addr(N_SapAddr *nsapAddr, struct in_addr *in)
 *   INET_in_addrToNsapAddr(struct in_addr *in, N_SapAddr *nsapAddr)
 *   INET_portNuToTsapSel(MdInt portNu, T_SapSel *tsapSel)
 *   INET_tsapSelToPortNu(T_SapSel *tsapSel, Int *portNu)
 *   INET_tsapSelToPortNu(T_SapSel *tsapSel, Int *portNu)
 *   INET_gethostid(OS_Ubits32 *hostid);
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: inetaddr.c,v 1.18 2002/03/22 08:30:55 mohsen Exp $";
#endif /*}*/

/* #includes */

#include "estd.h"
#include "eh.h"
#include "tm.h"
#include "pf.h"
#include "addr.h"
#include "bs.h"
#include "os.h"

/* static prototypes */

#if defined(OS_VARIANT_Windows) || defined(OS_VARIANT_WinCE)
static unsigned long int GetHostID();
#endif


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

PUBLIC SuccFail
INET_nsapAddrToin_addr(N_SapAddr *nsapAddr, struct in_addr *in)
{
    if ( nsapAddr->len !=4 ) {
	return FAIL;
    }

    BS_memCopy((Byte *) nsapAddr->addr, (Byte *) in, 4);

    return SUCCESS;
}




/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

PUBLIC SuccFail
INET_in_addrToNsapAddr(struct in_addr *in, N_SapAddr *nsapAddr)
{

    BS_memCopy((Byte *) in, (Byte *) nsapAddr->addr, 4);
    nsapAddr->len = 4;

    return SUCCESS;
}


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

SuccFail
INET_portNuToTsapSel(MdInt portNu, T_SapSel *tsapSel)
{
    /* This has not been tested on CPU archs other than Sparc */
    /* tested on 80x86 - shane 9/95 */

    portNu = htons(portNu);

    BS_memCopy((Byte *) &portNu, (Byte *) tsapSel->addr, 2);
    tsapSel->len = 2;

    return SUCCESS;
}


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

SuccFail
INET_tsapSelToPortNu(T_SapSel *tsapSel, Int *portNu)
{
    /* This has not been tested on CPU archs other than Sparc */
    /* tested on 80x86 - shane 9/95 */

    MdInt twoBytes;

    BS_memCopy((Byte *) tsapSel->addr, (Byte *) &twoBytes, 2);

    *portNu = ntohs(twoBytes);

    return SUCCESS;
}
    

/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments: buf should be at least 11 to be able to fit an 
 * unsigned null-terminated 32bit integer in decimal 
 *
 * Returns: 
 *
>*/

SuccFail
INET_gethostid(char *buf, size_t buflen)
{
    if ( buflen < 12 ) return FAIL;

#if defined(OS_VARIANT_Windows) || defined(OS_VARIANT_WinCE)

    sprintf(buf, "%ld", GetHostID()); 

#elif defined(OS_TYPE_MSDOS)

    return FAIL;

#else

#if 0  
    NOTYET, LINUX PORT
    sysinfo(SI_HW_SERIAL, buf, buflen);
#else
    return FAIL;
#endif

#endif

    return SUCCESS;
}

    
#if defined(OS_VARIANT_Windows) || defined(OS_VARIANT_WinCE)

#include "windows.h"
#include "windowsx.h"
#include "winsock.h"
#include "dos.h"


static unsigned long int 
GetHostID()
{
    char szLclHost [1024];
    LPHOSTENT lpstHostent;
    SOCKADDR_IN stLclAddr;
    SOCKADDR_IN stRmtAddr;
    int nAddrSize = sizeof(SOCKADDR);
    int hSock; /* SOCKET hSock; */
    int nRet;
    stLclAddr.sin_addr.s_addr = INADDR_ANY;

    nRet = gethostname(szLclHost, 1024);
    if (nRet != SOCKET_ERROR) {
	lpstHostent = gethostbyname((LPSTR)szLclHost);
	if (lpstHostent)
	    stLclAddr.sin_addr.s_addr =
		*((u_long FAR*) (lpstHostent->h_addr));
    }

    if (stLclAddr.sin_addr.s_addr == INADDR_ANY) {
        WORD version;
        WSADATA sockdata;
        char *p = (char *) &version;
        *p++ = 1; *p = 1;	/* version 1.1 */
        if (WSAStartup(version, &sockdata))
	{ 
	    char ebuf[100];
	    sprintf(ebuf,"WSAstartup fails returning error %d\n",
		    WSAGetLastError());
	    EH_problem(ebuf);
	    return(FAIL);
	}
	hSock = socket(AF_INET, SOCK_DGRAM, 0);
 	if (hSock != INVALID_SOCKET) {
	    stRmtAddr.sin_family = AF_INET;
	    stRmtAddr.sin_port   = htons(IPPORT_ECHO);
	    stRmtAddr.sin_addr.s_addr = inet_addr("128.127.50.1");
	    nRet = connect(hSock, (LPSOCKADDR)&stRmtAddr, sizeof(SOCKADDR));
	    
	    if (nRet != SOCKET_ERROR) {
		getsockname(hSock, (LPSOCKADDR)&stLclAddr, 
			    (int FAR*)&nAddrSize);
	    }
	    closesocket(hSock);
	}
    }
    return (stLclAddr.sin_addr.s_addr);

} /* GetHostID() */

#endif /* defined(OS_TYPE_MSDOS) && defined(OS_VARIANT_Windows) */
 

