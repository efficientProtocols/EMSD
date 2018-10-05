/*+
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)pp.h	1.1    Released: 6/13/90";
#endif /*}*/


#include <estd.h>
#include <pp.h>

#ifndef PPIF_H_
#define PPIF_H_

typedef struct PP_PortInfo {
    struct PP_PortInfo *next;
    struct PP_PortInfo *prev;
    PP_PortId portId;
    struct PP_SapInfo *sapInfo;
    struct PP_SPI_ServiceInfo *spi_serviceInfo;
    struct PP_SPI_PortInfo *spi_portInfo;
    Char dtmfBuf[PP_K_DtmfNuMax];
} PP_PortInfo;


/* Function definitions for obtain event ... all go here */

#endif
