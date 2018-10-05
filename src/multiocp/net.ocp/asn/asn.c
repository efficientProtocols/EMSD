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


#include "estd.h"
#include "tm.h"
#include "eh.h"
#include "asn.h"
#include "asnlocal.h"

TM_ModuleCB *		asn_hModCB;

static struct
{
    ReturnCode (* pfFormat)(ASN_TableEntry *  pTab,
			    void *            hBuf,
			    unsigned char *   pCStruct,
			    OS_Uint32 *       pFormattedLength,
			    OS_Uint8          depth);

    ReturnCode (* pfParse)(ASN_TableEntry *   pTab,
			   void *             hBuf,
			   unsigned char *    pCStruct,
			   OS_Uint32 *        pduLength,
			   OS_Uint8           depth);
} encodingRules[] =
{
    { asn_berFormat, asn_berParse }	/* Basic encoding rules */
};



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

ReturnCode
ASN_init(void)
{
    /* Give ourselves a trace handle */
    if (TM_OPEN(asn_hModCB, "ASN") == NULL)
    {
	EH_problem("Could not open ASN trace");
	return Fail;
    }

    return Success;
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

ReturnCode
ASN_newTableEntry(ASN_TableEntry **   ppTab,
		  ASN_TableEntryType  type,
		  OS_Uint8            itemTag,
		  QU_Head *           pQ,
		  unsigned char *     pBase,
		  void *              pExists,
		  void *              pData,
		  char *              pDebugMessage)
{
    ASN_TableEntry *	pTab;

    TM_TRACE((asn_hModCB, ASN_TRACE_DETAIL, 
	     "New table: 0x%02x 0x%02x %3.3s '%s'",
	      itemTag, type, pExists ? "Opt" : "Man", pDebugMessage));

    /* Allocate a new table entry */
    if ((pTab = OS_alloc(sizeof(ASN_TableEntry))) == NULL)
    {
	return ResourceError;
    }

    /* Initialize the table entry's queue pointers */
    QU_INIT(pTab);

    /* Assign the type and tag */
    pTab->type = type;
    pTab->tag = itemTag;

    /* Save the pointer to the debug-assistance message */
    pTab->pDebugMessage = pDebugMessage;

    /* Determine the offset of the exists pointer from the base */
    if (pExists == NULL)
    {
	pTab->existsOffset = ASN_MANDATORY;
    }
    else
    {
	pTab->existsOffset =
	    (unsigned char *) pExists - (unsigned char *) pBase;
    }

    /* Determine the offset of the data pointer from the base */
    if (pData == NULL)
    {
	pTab->dataOffset = ASN_NODATA;
    }
    else
    {
	pTab->dataOffset = 
	    (unsigned char *) pData - (unsigned char *) pBase;
    }

    /* Determine the default maximum and minimum values or sizes */
    switch(type)
    {
    case ASN_Type_Integer:
	pTab->minimum = 0x00000000;
	pTab->maximum = 0xffffffff;
	break;

    case ASN_Type_BitString:
	pTab->minimum = 0;
	pTab->maximum = 31;
	break;

    default:
	/* Nothing to do in other case.  Prevent compile warnings */
	;
    }

    /* Default to infinite data length allowed */
    pTab->maxDataLength = 0xffffffff;

    /* Initialize the table data queue head */
    QU_INIT(&pTab->tableList);

    /* Insert this table entry onto its queue */
    QU_INSERT(pTab, pQ);

    /* Give 'em a pointer to the new table entry */
    *ppTab = pTab;

    return Success;
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

ReturnCode
ASN_format(ASN_EncodingRules  encRules,
	   void *             hTab,
	   void *             hBuf,
	   void *             pCStruct,
	   OS_Uint32 *        pFormattedLength)
{
    ASN_TableEntry *	pTab = hTab;

    TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT,
	      "Format ASN.1 PDU: <<%s>>", pTab->pDebugMessage));
    /* Make sure we got a valid encoding rules value */
    if (encRules > ASN_EncodingRules_Max)
    {
	return ASN_RC_InvalidEncodingRules;
    }

    /* Make sure there are elements to be formatted */
    if (QU_EQUAL(pTab, QU_LAST(pTab)))
    {
	/* There's nothing to be formatted.  We're done here. */
	return Success;
    }

    /* Format, using the selected encoding rules */
    return (* encodingRules[encRules].pfFormat)(QU_LAST(pTab),
						hBuf,
						pCStruct,
						pFormattedLength,
						0);
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

ReturnCode
ASN_parse(ASN_EncodingRules  encRules,
	  void *             hTab,
	  void *             hBuf,
	  void *             pCStruct,
	  OS_Uint32 *        pduLength)
{
    ASN_TableEntry *	pTab = hTab;

    TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, "Parse ASN.1 PDU: <<%s>>", pTab->pDebugMessage));
    /* Make sure we got a valid encoding rules value */
    if ((unsigned int) encRules > (unsigned int) ASN_EncodingRules_Max)
    {
	return ASN_RC_InvalidEncodingRules;
    }

    /* Make sure there are elements to be parsed */
    if (QU_EQUAL(pTab, QU_FIRST(pTab)))
    {
	/* There's nothing to be formatted.  We're done here. */
	return Success;
    }

    /* Parse, using the selected encoding rules */
    return (*encodingRules[encRules].pfParse)(QU_FIRST(pTab), hBuf, pCStruct, pduLength, 0);
}
