/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of Neda-EMSD. An implementation of RFC-2524.
 * 
 * Neda-EMSD is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Neda-EMSD; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:
 *
 */


#include "estd.h"
#include "tm.h"
#include "asn.h"
#include "emsd.h"
#include "emsdlocal.h"



/* --------------------------------------------------------------- */

ReturnCode
EMSD_compileErrorRequest1_0(unsigned char * pCStruct,
			   OS_Boolean * pExists,
			   EMSD_ErrorRequest * pErrorReq,
			   OS_Uint8 tag,
			   QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pErrorReq->error,
		  "Error Request",
		  ("emsd_compileErrorRequest: error"));

    return Success;
}
