/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File: pdupr.c
 *
 * Description: Print PDU.
 *
 * Functions: tm_pduPr (char *str, DU_View du, char *p, int maxLog)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: pdupr.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "invoke.h"
#include "du.h"
#include "eropfsm.h"
#include "nm.h"
#include "layernm.h"
#include "byteordr.h"
#include "eropdu.h"

#include "local.h"

#ifdef TM_ENABLED
static char *typeStr[5] = {
    	"inv",		/* 0x00 */
	"res",		/* 0x01 */
	"err",		/* 0x02 */
	"ack",		/* 0x03 */
	"fai",		/* 0x04 */
};
#endif


/*<
 * Function:    tm_pduPr
 *
 * Description: Print PDU contents.
 *
 * Arguments:   Message string, pointer to data unit, print addr, length.
 *
 * Returns:     None.
 *
>*/

#ifdef TM_ENABLED
LOCAL Void
tm_pduPr(tm_ModInfo *modInfo, TM_Mask mask, char *str, DU_View du, int maxLog)
{
    extern FILE *tmFile;
    extern logPdu();

    char *duData, *duDataStart;
    Int  duSize;
    unsigned char c;
    Int   length;
    Bool  pduCut = 0;
    LgInt lastLogAddr;

    if ( !(modInfo->mask & mask) ) {
	return;
    }

    TM_TRACE((EROP_modCB, TM_ENTER,
	    "tm_pduPr:  type=(%s): duAddr=%lu  dataAddr=%lu  DUsize=%d\n",
	    str, TM_prAddr((Ptr) du), TM_prAddr((Ptr) DU_data(du)), DU_size(du)));

    /* PCI, Byte1 , PDU-TYPE + remEsroSapSel or encodingType ... */

    fprintf(tmFile, "------------------------------------------------------------------------\n");

    duDataStart = duData = DU_data(du);
    duSize = DU_size(du);

    BO_get1(c, duData);

    switch (c & 0x07) {

    case INVOKE_PDU:

	fprintf (tmFile, "%s %s  ", str, typeStr[c & 0x07]);
	/* PCI, BYTE-1 */
	fprintf (tmFile, "Rem=%d  ",  (c & 0xF0) >> 4);	
	fprintf (tmFile, "Loc=%d  ", ((c & 0xF0) >> 4) - 1);

	/* PCI Byte-2 */
	BO_get1(c, duData);
	fprintf (tmFile, "Ref=%u  ", c);	

	/* PCI, Byte-3 , operationValue + EncodingType */
	BO_get1(c, duData);  

	fprintf (tmFile, "OpVal=%d  ", c & 0x3F);
	fprintf (tmFile, "Encod=%d\n", (c & 0xC0) >> 6);

	break;

    case RESULT_PDU:
	fprintf (tmFile, "%s %s  ", str, typeStr[c & 0x07]);
	/* PCI, BYTE-1 */
	fprintf (tmFile, "Encod=%d  ", (c & 0xC0) >> 6);

	/* PCI Byte-2 */
	BO_get1(c, duData);
	fprintf (tmFile, "RefNo=%u\n", c);	

	break;

    case ERROR_PDU:

	fprintf (tmFile, "%s %s  ", str, typeStr[c & 0x07]);
	/* PCI, BYTE-1 */
	fprintf (tmFile, "\nEncod=%d  ", (c & 0xC0) >> 6);

	/* PCI Byte-2 */
	BO_get1(c, duData);
	fprintf (tmFile, "RefNo=%u  ", c);	

	/* PCI Byte-3 */
	BO_get1(c, duData);
	fprintf (tmFile, "ErrVal=%u\n", c);	

	break;

    case ACK_PDU:

	fprintf (tmFile, "%s %s  ", str, typeStr[c & 0x07]);
	/* PCI Byte-2 */
	BO_get1(c, duData);
	fprintf (tmFile, "RefNo=%u\n", c);	

        return;

    case FAILURE_PDU:

	fprintf (tmFile, "%s %s  ", str, typeStr[c & 0x07]);
	BO_get1(c, duData);
	fprintf (tmFile, "RefNo=%u\n", c);	

	break;

    default:
	fprintf (tmFile, "Unknown PDU type %s\n", str );
	break;
    }

    if ( (length = duSize) > maxLog ) {
	length = maxLog;
	pduCut = 1;
    }

    lastLogAddr = TM_hexDump(EROP_modCB, TM_PDUIN | TM_PDUOUT, 
                             str, DU_data(du), length-1);

    if ( pduCut ) {
	fprintf (tmFile, "%05ld: ...hex dump is cut...\n", lastLogAddr);
    }

} /* tm_pduPr() */
#endif

