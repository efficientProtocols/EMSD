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
#include "asn.h"
#include "strfunc.h"
#include "buf.h"
#include "asnlocal.h"

/* Define the constructor bit */
#define	CONSTRUCTOR	(0x20)

static ReturnCode
formatLength(OS_Uint32 length,
	     void * hBuf,
	     OS_Uint8 * pLengthOfLength);

static ReturnCode
parseLength(void * hBuf,
	    OS_Uint32 * pLength,
	    OS_Uint32 * pLengthOfLength);

static int
intToBcd (OS_Uint32 integer, unsigned char *string);

static OS_Uint32
bcdToInt (unsigned char *string, int len);

/*
 * For formatting and parsing using the Basic Encoding Rules, the
 * Minimum and Maximum Value elements in the Table Entry are unused.
 * These elements will be used when other encoding rules are
 * provided, which don't waste PDU space with lengths when the
 * lengths are known.
 */



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
asn_berFormat(ASN_TableEntry *  pTab,
	      void *            hBuf,
	      unsigned char *   pCStruct,
	      OS_Uint32 *       pFormattedLength,
	      OS_Uint8          depth)
{
    int 		i                = 0;
    int 		j                = 0;
    ReturnCode 		rc               = 0;
    OS_Uint8 		x                = 0;
    OS_Uint8	        tag              = 0;
    OS_Uint8	        choiceTag        = 0;
    OS_Uint8		savedTag         = 0;
    OS_Uint8 		lengthOfLength   = 0;
    OS_Uint8	        unusedBits       = 0;
    OS_Uint16 		numBits          = 0;
    OS_Uint32 		len              = 0;
    OS_Uint32 		tempLen		 = 0;
    OS_Uint32 		integer          = 0;
    OS_Uint32 		n                = 0;
    OS_Uint32		count            = 0;
    OS_Uint32		elementSize      = 0;
    STR_String 	        string           = NULL;
    void *	        hPreFormattedBuf = NULL;
    void *		pTableList       = NULL;
    char                bcdString[24];

    OS_memSet(bcdString, '\0', sizeof(bcdString));

    depth++;		/* increment the level of recursion */

    /* If we've been asked for ASN debugging, print it now */
    TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
	     "BER Format%*d: 0x%02x 0x%02x +0x%04lx %s: '%s'",
	      depth*2, depth, pTab->tag, pTab->type, pTab->dataOffset,
	      pTab->existsOffset == ASN_MANDATORY ? "Man" : "Opt",
	      pTab->pDebugMessage));

    /* Is this element optional? */
    if (pTab->existsOffset != ASN_MANDATORY)
    {
	/* Yes.  See if the element is to be included in the PDU */
	if (! *(OS_Boolean *) (pCStruct + pTab->existsOffset))
	{
	    /* It's not.  We're adding no length to the pdu */
	    TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, "BER Format%*d: Omitted '%s'", depth*2, depth, pTab->pDebugMessage));
	    *pFormattedLength = 0;
	    return Success;
	}
    }

    /* What tag do we expect to be formatting? */
    tag = pTab->tag;

    /* See what type of element to expect */
    switch(pTab->type)
    {
    case ASN_Type_Integer:
	/* We can only encode at most 32-bit values. */
	integer =  *(OS_Uint32 *)(pCStruct + pTab->dataOffset) & 0xffffffff;
	
	/* Determine the length of the data field for this element */
	if (integer > 0xffffff)
	{
	    len = 4;
	}
	else if (integer > 0xffff)
	{
	    len = 3;
	}
	else if (integer > 0xff)
	{
	    len = 2;
	}
	else
	{
	    len = 1;
	}
	
        TM_TRACE((asn_hModCB, ASN_TRACE_DATA, "BER Format%*d: Int%ld=%08lx", 
		 depth*2, depth, len, integer));

	/* Add the value to the buffer */
	do
	{
	    /* Add the next octet of the value to the buffer */
	    RC_CALL(rc,
		    BUF_addOctet(hBuf, (OS_Uint8) (integer & 0xff)),
		    ("ASN_berFormat: octet of value"));
	    
	    /* Strip off the just-added portion of the value */
	    integer >>= 8;
	} while (integer > 0);
	break;

    case ASN_Type_BitString:
	/* Get the length of the data, based on the number of bits */
	string = *(STR_String *) (pCStruct + pTab->dataOffset);

	numBits = STR_getMaxLength(string);

        TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, "BER Format%*d: BitString%d", 
		 depth*2, depth, numBits));

	/* Make sure we're getting some bits */
	if (numBits == 0)
	{
	    /* Add the number of unused bits octet */
	    RC_CALL(rc,
		    BUF_addOctet(hBuf, (OS_Uint8) 0),
		    ("ASN_berFormat: Bit String: "
		     "add unused bits (zero)"));

	    len = 1;
	    break;
	}
	
	/* Get the number of octets of data + unused bits octet */
	len = ((numBits - 1) / 8) + 1 + 1;
	
	/* Get the number of unused bits in the last octet */
	unusedBits = (8 - (numBits % 8)) % 8;

	/* Add the data */
	for (x = 0, i = 8, j = 0; numBits > 0; numBits--)
	{
	    /* Or-in a bit if this bit-element is set */
	    x |= (STR_bitStringGetBit(string, j++) ? 1 : 0) << --i;

	    /* Have we created a complete octet? */
	    if (i == 0)
	    {
		/* Yup.  Add the octet to the buffer */
		RC_CALL(rc,
			BUF_addOctet(hBuf, (OS_Uint8) (x & 0xff)),
			("ASNBER_format: Bit String: "
			 "add octet of data"));

		/* Reset the octet value and bit number */
		x = 0;
		i = 8;
	    }
	}
	
	/* Have we added the final octet? */
	if (i != 8)
	{
	    /* Shift it left to account for unused bits */
	    x <<= unusedBits - 1;

	    /* No.  Add it now */
	    RC_CALL(rc,
		    BUF_addOctet(hBuf, (OS_Uint8) (x & 0xff)),
		    ("ASN_berFormat: Bit String: "
		     "add octet of data"));
	}

	/* Add the number of unused bits octet */
	RC_CALL(rc,
		BUF_addOctet(hBuf, unusedBits),
		("ASN_berFormat: Bit String: "
		 "add actual unused bits"));

	break;

    case ASN_Type_BcdString:
        /* internallly a OS_Uint32, externally a packed OctetString */
	/* We can only encode at most 32-bit values. */
        TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
		  "BER Format%*d: BCD OctetString, off=%08lx",
		  depth*2, depth, pTab->dataOffset));

	integer =  *(OS_Uint32 *)(pCStruct + pTab->dataOffset) & 0xffffffff;
	
        TM_TRACE((asn_hModCB, ASN_TRACE_DATA, 
		  "BER Format%*d: BCD OctetString%ld=%08lx",
		  depth*2, depth, len, integer));

	/* create the BCD string */
	len = intToBcd (integer, bcdString);

	/* prepend the string */
	for (tempLen = 1; tempLen <= len; tempLen++)
	{
	    /* Add the next octet of the value to the buffer */
	    rc = BUF_addOctet(hBuf, 
		        (OS_Uint8) (((char *)bcdString)[len-tempLen] & 0xff));
	    if (rc != Success) {
	      TM_TRACE((asn_hModCB, ASN_TRACE_ERROR, 
			"BER format: BCD addOctet failed"));
	      return (rc);
	    }
	}
	TM_TRACE((asn_hModCB, ASN_TRACE_DATA, "BER format: len=%ld", len));
	break;

    case ASN_Type_OctetString:
	/* Point to the octet string */
	string = *(STR_String *) (pCStruct + pTab->dataOffset);

	len = STR_getStringLength(string);

        TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, "BER Format%*d: OctetString%ld",
		  depth*2, depth, len));

	/* Prepend this octet string to the buffer */
	RC_CALL(rc,
		BUF_prependChunk(hBuf, string),
		("ASN_berFormat: octet string: prependChunk"));;
	break;

    case ASN_Type_Sequence:
        TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
		 "BER Format%*d: Sequence...", depth*2, depth));

	/* Initialize our length counter */
	len = 0;
	pTableList = &pTab->tableList;

	/* Process each element in the sequence */
	for (pTab = QU_LAST(pTableList);
	     ! QU_EQUAL(pTab, pTableList);
	     pTab = QU_PREV(pTab))
	{
	    /*
	     * Recursively call the formatter for this sequence
	     * element.
	     */
	    RETURN_IF_FAIL(asn_berFormat(pTab, hBuf, pCStruct, &n, depth),
			   ("asn_berFormat: element of sequence"));

	    /* Update the number of octets added so far */
	    len += n;
	}
        TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
		 "BER Format%*d: ...Sequence", depth*2, depth));

	/* Sequences are always constructed */
	tag |= CONSTRUCTOR;
	break;

    case ASN_Type_SequenceOf:
	/* Get the count of elements in the sequence-of */
	count = *(OS_Uint32 *) (pCStruct + pTab->dataOffset);

        TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT,
		  "BER Format%*d: SequenceOf[%ld] of size %ld",
		  depth*2, depth, count, pTab->elementSize));

	/* Initialize our length counter */
	len = 0;

	/* Keep track of the size of each element of the sequence-of */
	elementSize = pTab->elementSize;

	/*
	 * Get the table entry for the type of which this is a
	 * sequence of
	 */
	pTab = QU_FIRST(&pTab->tableList);

	/* Process each element in the sequence-of */
	for (i = 0; i < count; i++)
	{
	    /* Recursively call formatter for this sequence element */
	    RETURN_IF_FAIL(asn_berFormat(pTab, hBuf, pCStruct, &n, depth),
			   ("asn_berFormat: element of sequence"));

	    /* Update the number of octets added so far */
	    len += n;

	    /* Update the base of where to find the data */
	    pCStruct += elementSize;
	}

	/* If the sequence-of was empty and optional, don't add tag */
	if (pTab->existsOffset != ASN_MANDATORY && count == 0)
	{
	    /* Don't add tag or length */
	    tag = 0;
	    TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
		     "BER Format%*d: ...SequenceOf omitted", depth*2, depth));
	}
	else
	{
	    /* Sequence-of's are always constructed */
	    tag |= CONSTRUCTOR;
	    TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
		     "BER Format%*d: ...SequenceOf", depth*2, depth));
	}
	break;

    case ASN_Type_Choice:
	/* Get the indicator of which choice element this is */
	i = *(OS_Uint8 *) (pCStruct + pTab->dataOffset);

	TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
		 "BER Format%*d: Choice(%d)...", depth*2, depth, i));
	/* Save the choice's unique tag */
	choiceTag = pTab->tag;

	/* Move to that element */
	for (pTab = QU_FIRST(&pTab->tableList);
	     i != 0;
	     pTab = QU_NEXT(pTab))
	{
	    --i;
	}

	/* Save this element's tag */
	savedTag = pTab->tag;

	/* If a unique tag was requested, and it isn't EXPLICIT ... */
	if (choiceTag != 0)
	{
	    if (! (choiceTag & ASN_EXPLICIT))
	    {
		/* ... set this element's tag to the explicit one */
		pTab->tag = choiceTag;

		/* Don't add a duplicate tag later. */
		tag = 0;
	    }
	    else
	    {
		/* We'll add the explicit tag later. */
		tag = choiceTag;
	    }
	}
	else
	{
	    /* Don't add any additional tag */
	    tag = 0;
	}

	/* Process this element */
	RC_CALL(rc,
		asn_berFormat(pTab, hBuf, pCStruct, &len, depth),
		("asn_berFormat: choice"));

	/* Set the element's tag back to the original one */
	pTab->tag = savedTag;
	TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
		 "BER Format%*d: ...Choice", depth*2, depth));
	break;

    case ASN_Type_Buffer:
	hPreFormattedBuf = *(void **) (pCStruct + pTab->dataOffset);

	if ((len = BUF_getBufferLength(hPreFormattedBuf)) > 0)
	{
	    /* Prepend this octet string to the buffer */
	    RC_CALL(rc,
		    BUF_prependBuffer(hBuf, hPreFormattedBuf),
		    ("asn_berFormat: buffer: prependBuffer"));
	}
	TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
		 "BER Format%*d: Any%ld", depth*2, depth, len));

	/* Prevent a tag from being prepended */
	tag = 0;
	break;

    default:
	return FAIL_RC(ProgrammerError,
		       ("asn_berFormat: Unexpected ASN Type: 0x%x",
			pTab->type));
    }

    /*
     * If we formatted a choice with no explicit tag, don't put in
     * another tag.
     */
    if (tag != 0)
    {
	/* Add the length octets */
	RC_CALL(rc,
		formatLength(len, hBuf, &lengthOfLength),
		("asn_berFormat: length"));
	
	/* Add the tag octet */
	RC_CALL(rc,
		BUF_addOctet(hBuf, tag),
		("asn_berFormat: tag"));

	/* keep track of the number of octets that we've formatted */
	*pFormattedLength = len + lengthOfLength + 1;
    }
    else
    {
	*pFormattedLength = len;
    }

    TM_TRACE((asn_hModCB, ASN_TRACE_FORMAT, 
	     "BER Format%*d: == returns SUCCESS ===", depth*2, depth));
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
asn_berParse(ASN_TableEntry *  pTab,
	     void *           hBuf,
	     unsigned char *  pCStruct,
	     OS_Uint32 *      pduLength,
	     OS_Uint8         depth)
{
    int 		i		= 0;
    ReturnCode 		rc		= 0;
    OS_Uint8 		x		= 0;
    OS_Uint8 		thisOctet	= 0;
    OS_Uint8 		numBits		= 0;
    OS_Uint8 		unusedBits	= 0;
    OS_Uint8 		bitsThisOctet	= 0;
    OS_Uint8 * 		pChoice		= 0;
    OS_Uint16 		n		= 0;
    OS_Uint32 		len		= 0;
    OS_Uint32 		lengthOfLength	= 0;
    OS_Uint32 		integer		= 0;
    OS_Uint32 		elementSize	= 0;
    OS_Uint32 * 	pCount		= 0;
    OS_Uint32 		tempLen		= 0;
    OS_Boolean * 	pExists		= NULL;
    STR_String 		string;
    ASN_TableEntry * 	pSeqOfTab	= NULL;
    unsigned char * 	bufMem		= NULL;
    unsigned char * 	p		= NULL;
    void * 		pTableList	= NULL;
    void * 		hBufferContent	= NULL;

    depth++;		/* increment the level of recursion */

    /* If we've been asked for ASN debugging, print it now */
    TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
	      "BER Parse%*d: tag=0x%02x type=0x%02x offset=0x%04lx "
	      "len=%ld %s: '%s'",
	      depth*2, depth, pTab->tag, pTab->type, pTab->dataOffset, 
	      *pduLength, pTab->existsOffset == ASN_MANDATORY ? "Man" : "Opt",
	      pTab->pDebugMessage));

    /*
     * If we're parsing a choice with no unique tag, ignore the tag
     * for now.  We'll pick it up when we go to parse the choice
     * element.  The exception is when there is an EXPLICIT tag
     * which must be parsed prior to parsing the choice element.
     */
    if ((pTab->type != ASN_Type_Choice && pTab->tag != 0) ||
	(pTab->tag & ASN_EXPLICIT))
    {
	/* Get the tag */
	RC_CALL(rc,
		BUF_getOctet(hBuf, &x),
		("asn_berParse: get tag"));
	
	/* Is this the tag we were expecting? (ignore constructor) */
	if ((x & (~ CONSTRUCTOR)) != (pTab->tag & (~ ASN_EXPLICIT)))
	{
	    /* Put the tag back so the next element may use it */
	    BUF_ungetOctet(hBuf);
	    
	    /* Not the correct tag.  Was this element optional? */
	    if (pTab->existsOffset != ASN_MANDATORY)
	    {
		/*
		 * Yes, it was optional.  Just set the flag and get
		 * outta here
		 */
		*(OS_Boolean *) (pCStruct + pTab->existsOffset) = FALSE;
		
		/* Indicate that we've used none of aloted length */
		*pduLength = 0;

		/* If we've been asked for ASN debugging, print it now */
		TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
			  "BER Parse%*d: Optional element not found", 
			  depth*2, depth));
		return Success;
	    }
	    else
	    {
		/*
		 * We received garbage: it was the wrong tag, and
		 * not optional
		 */
		TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
			  "BER Parse%*d: Expected tag 0x%02x; got 0x%02x",
			  depth*2, depth, pTab->tag, x));
		return ASN_RC_WrongTag;
	    }
	}
	/* If this element was optional, tell 'em we got it */
	if (pTab->existsOffset != ASN_MANDATORY)
	{
	    *(OS_Boolean *) (pCStruct + pTab->existsOffset) = TRUE;
	}

	/* Get the length of this encoded element */
	RETURN_IF_FAIL(parseLength(hBuf, &len, &lengthOfLength),
		       ("asn_berParse: getting element length"));

	/* Does this data length exceed our quota? */
	if (len > *pduLength)
	{
	    return FAIL_RC(ASN_RC_FormatErrorInPDU,
			   ("asn_berParse: Element length for tag "
			    "0x%02x was %lu; quota %lu",
			    pTab->tag, len, *pduLength));
	}

	/* Initialize the length we've used so far */
	*pduLength = lengthOfLength + 1;
    }
    else
    {
	/* Initialize the length we've used so far */
	*pduLength = 0;

	/* Default the maximum length to parse */
	len = 0xffffffff;
    }

    /* See what type of element to expect */
    switch(pTab->type)
    {
    case ASN_Type_Integer:
	/* Initialize the value now, in case data length is zero */
	integer = 0;
	
	/*
	 * Make sure the length is within reason.  We support up to
	 * 32 bits
	 */
	if (len > 4)
	{
	    return FAIL_RC(ASN_RC_InvalidSize,
			   ("asn_berParse: "
			    "element length for tag 0x%02x was %lu; "
			    "max 4 for integers",
			    pTab->tag, len));
	}

	/* Keep track of length used */
	*pduLength += len;

	/* Get the value */
	for (tempLen = 0; tempLen < len; tempLen++)
	{
	    /* Get the next octet of the value */
	    RC_CALL(rc,
		    BUF_getOctet(hBuf, &thisOctet),
		    ("asn_berParse: Integer: octet of value"));
	    
	    /* Shift it into the value */
	    integer = (integer << 8) | thisOctet;
	}
	
	/* Give 'em what they came for */
	*(OS_Uint32 *) (pCStruct + pTab->dataOffset) = integer;
	TM_TRACE((asn_hModCB, ASN_TRACE_DATA, "BER Parse%*d: Int%ld=%08lx",
		  depth*2, depth, len, integer));
	break;

    case ASN_Type_BitString:
	/* There must be at least 1 octet (number of unused bits) */
	if (len < 1)
	{
	    return FAIL_RC(ASN_RC_InvalidSize,
			   ("asn_berParse: "
			    "element length for tag 0x%02x was %lu; "
			    "min 1 for bit strings",
			    pTab->tag, len));
	}

	/* Determine the number of bits */
	numBits = (len - 1) * 8;

	/* Allocate memory in which to put the data */
	RC_CALL(rc,
		STR_alloc(numBits, &string),
		("asn_berParse: alloc of bit string"));

	/* Set the length of the bit-string */
	STR_setStringLength(string, numBits);

	/* Get the number of unused bits in the last octet */
	RC_CALL(rc,
		BUF_getOctet(hBuf, &unusedBits),
		("asn_berParse: unused bits"));

        TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, "BER Parse%*d: BitString%d-%d",
                  depth*2, depth, (int)numBits, (int)unusedBits));

	/* Make sure it's a valid number of unused bits */
	if (unusedBits > 7)
	{
	    return FAIL_RC(ASN_RC_IllegalData,
			   ("asn_berParse: "
			    "Illegal number of unused bits in "
			    "bit string: %d",
			    (int) unusedBits));
	}

	/* Point to the location for bit values being parsed */
	p = STR_stringStart(string);

	/* Initialize the value now, in case data doesn't fill it */
	for (i = 0; i < numBits; i++)
	{
	    *p++ = FALSE;
	}
	    
	/* Point to the location for bit values being parsed again */
	p = STR_stringStart(string);

	/* Keep track of length used */
	*pduLength += len;

	/* Initialize our bit counter */
	numBits = 0;

	/* Get the value (if there is one) */
	if (--len != 0)
	{
	    while (len-- != 0)
	    {
		/* Get the next octet of the value */
		RC_CALL(rc,
			BUF_getOctet(hBuf, &thisOctet),
			("asn_berParseBitString: octet of value"));
	    
		/*
		 * Determine how many bits are encoded herein.  Last
		 * octet may have fewer than 8 bits.
		 */
		bitsThisOctet = (len == 0 ? 8 - unusedBits : 8);

		/* For each bit, get its value */
		for (i = bitsThisOctet - 1; i >= 0; i--)
		{
		    *p++ = (thisOctet & (1 << i) ? TRUE : FALSE);
		    numBits++;
		}
	    }
	}

	/* Give 'em their bit string */
	*(STR_String *) (pCStruct + pTab->dataOffset) = string;
	break;

    case ASN_Type_BcdString:
    case ASN_Type_OctetString:
    {
        int isBcdString = (pTab->type == ASN_Type_BcdString);

	TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
		  "BER Parse%*d: OctetString%ld %s",
		  depth*2, depth, len,
		  isBcdString ? "(BCD)" : ""));

	/* Does this data length exceed the amount of space for the data? */
	if (len > pTab->maxDataLength)
	{
	    return FAIL_RC(ASN_RC_InvalidSize,
			   ("asn_berParse: "
			    "element length for tag 0x%02x was %lu; "
			    "data size %lu",
			    pTab->tag, len, pTab->maxDataLength));
	}

	/* Keep track of our length so far */
	*pduLength = lengthOfLength + 1;

	/*
	 * Allocate memory in which to put the data.  Leave room at
	 * the end for a null-terminator.  We won't include the
	 * terminator in the length of the data, but it's there in
	 * case someone tries to access the string as
	 * null-terminated.
	 *
	 * Added support for BCD strings, which may contain null bytes
	 * and are terminated with 'xFF'.  The terminator is not part
	 * of the ASN.1 encoding. [fnh]
	 *
	 * A correct implementation of octet strings would have the length
	 * stored with the string.
	 */
	RC_CALL(rc,
		STR_alloc((OS_Uint16) len + 2, &string),
		("asn_berParseBitString: alloc of octet string"));

	/* Keep track of length used */
	*pduLength += len;

	/*
	 * Get chunks of data from buffer until we get as much as
	 * we want.
	 */
	tempLen = len;
	while (tempLen != 0)
	{
	    /* Tell 'em how much data we'd REALLY like to get */
	    n = tempLen;
	    
	    /*
	     * Get a pointer to a chunk of memory that we can copy
	     * out.
	     */
	    RC_CALL(rc,
		    BUF_getChunk(hBuf, &n, &bufMem),
		    ("asn_berParse: octet string: getChunk"));
	    
	    /* Copy it into the allocated buffer */
	    RC_CALL(rc,
		    STR_appendString(string, n, bufMem),
		    ("asn_berParseBitString: append chunk"));

	    bufMem = STR_stringStart(string);

	    /* Reduce the number of octets remaining to be gotten */
	    tempLen -= n;
	}
	
	if (isBcdString)
	{
	    /* Integer value */
	    STR_stringStart(string)[len] = 0xFF;    /* Terminate the string */
	    STR_stringStart(string)[len+1] = 0x00;  /* NULL terminate string */
	    integer = bcdToInt (STR_stringStart(string), (int) len);
	    *(OS_Uint32 *) (pCStruct + pTab->dataOffset) = integer;
	    /*** free the string ***/
	}
	else
	{
	    /* String value */
	    *(STR_String *) (pCStruct + pTab->dataOffset) = string;
	    STR_stringStart(string)[len] = 0x00;    /* Terminate the string */
	    RC_CALL(rc,
		    STR_appendString(string, 1, "\0"),
		    ("asn_berParseBitString: append chunk"));
	}
    }
    break;

    case ASN_Type_Sequence:
	TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, 
		 "BER Parse%*d: Sequence (len=%ld)...",
		  depth*2, depth, len));

	pTableList = &pTab->tableList;

	/* Keep track of length used so far */
	*pduLength += len;

	/* Process each element in the sequence */
	for (pTab = QU_FIRST(pTableList);
	     ! QU_EQUAL(pTab, pTableList) && len != 0;
	     pTab = QU_NEXT(pTab))
	{
	    tempLen = len;

	    TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
		      "BER Parse%*d: Checking for '%s'", 
		      depth*2, depth, pTab->pDebugMessage));
	    /* Recursively call parser for this sequence element */
	    RETURN_IF_FAIL(asn_berParse(pTab, hBuf, pCStruct, &tempLen, depth),
			   ("asn_berParse: element of sequence"));

	    /* Did we exceed our quota? */
	    if (tempLen > len)
	    {
	        TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
			  "BER Parse%*d: Sequence length overrun "
			  "(len=%ld, max=%ld)",
			  depth*2, depth, tempLen, len));
		return FAIL_RC(ASN_RC_InvalidSize, 
			       ("asn_berParse: sequence too long"));
	    }

	    /* Keep track of remaining length of sequence */
	    len -= tempLen;
	}
	TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
		  "BER Parse%*d: ...Sequence (len=%ld)", depth*2, depth, len));
	break;

    case ASN_Type_SequenceOf:
	TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, 
		 "BER Parse%*d: SequenceOf", depth*2, depth));

	/* Keep track of our length so far */
	*pduLength = lengthOfLength + 1;

	/* Remember where to put the number of elements we found */
	pCount = (OS_Uint32 *) (pCStruct + pTab->dataOffset);

	/* Initialize it to zero */
	*pCount = 0;

	/* Keep track of the size of each element */
        elementSize = pTab->elementSize;

	/* Keep a pointer to the sequence-of table */
	pSeqOfTab = pTab;

	/* Point to the table that this is a sequence of */
	pTab = QU_FIRST(&pTab->tableList);

	/* Keep track of length used */
	*pduLength += len;

	/* Process each element of the sequence-of */
	while (len != 0)
	{
	    /* Have we exceeded the maximum number of elements? */
	    if (*pCount >= pSeqOfTab->maxDataLength)
	    {
		return FAIL_RC(ASN_RC_InvalidSize,
			       ("asn_berParse: "
				"maximum sequence-of count 0x%lu for tag "
				"0x%02x exceeded.",
				pTab->maxDataLength, pTab->tag));
	    }

	    /* Initialize maximum element length */
	    tempLen = len;

	    /*
	     * Recursively call the parser for this sequence-of element
	     */
	    TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, 
		     "BER Parse%*d: Checking for '%s'",
		      depth*2, depth, pTab->pDebugMessage));
	    RETURN_IF_FAIL(asn_berParse(pTab, hBuf, pCStruct, &tempLen, depth),
			   ("asn_berParse: sequence of"));

	    /* Did we exceed our quota? */
	    if (tempLen > len)
	    {
		return FAIL_RC(ASN_RC_InvalidSize,
			       ("asn_berParse: "
				"sequence-of element too long"));
	    }

	    /* Keep track of remaining length of sequence */
	    len -= tempLen;

	    /* Update the base to the next element in the array */
	    pCStruct += elementSize;

	    /* Update the count of elements */
	    ++*pCount;
	}
	TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
		  "BER Parse%*d: ...SequenceOf (len=%ld)", 
		  depth*2, depth, len));
	break;

    case ASN_Type_Choice:
	TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, 
		 "BER Parse%*d: ...Choice", depth*2, depth));
	/* Keep track of our exists pointer */
	if (pTab->existsOffset == ASN_MANDATORY)
	{
	    pExists = NULL;
	}
	else
	{
	    pExists = (OS_Boolean *) (pCStruct + pTab->existsOffset);
	}

	/* Keep track of where to store the choice indicator */
	pChoice = (OS_Uint8 *) (pCStruct + pTab->dataOffset);

	/* Initialize rc incase the queue is empty */
	rc = Fail;

	/* Check each element until we find one that matches */
	pTableList = &pTab->tableList;
	for (i = 0, pTab = QU_FIRST(pTableList);
	     ! QU_EQUAL(pTab, pTableList);
	     i++, pTab = QU_NEXT(pTab))
	{
	    tempLen = len;

	    /* Recursively call the parser for this choice element */
	    TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, "BER Parse%*d: Checking '%s'",
		      depth*2, depth, pTab->pDebugMessage));
	    rc = asn_berParse(pTab, hBuf, pCStruct, &tempLen, depth);

	    /* Did we find the element we were looking for? */
	    if (rc == Success)
	    {
	        TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, "BER Parse%*d: Matched '%s'",
			  depth*2, depth, pTab->pDebugMessage));
		/* Yup.  Make sure we didn't exceed our quota */
		if (tempLen > len || (len != 0xffffffff && len != tempLen))
		{
		    return FAIL_RC(ASN_RC_InvalidSize,
				   ("asn_berParse: "
				    "choice element too long"));
		}

		/* Keep track of the length used. */
		*pduLength += tempLen;

		/* If this element was optional, let 'em know we found it */
		if (pExists != NULL)
		{
		    *pExists = TRUE;
		}

		break;
	    }

	    /* If we got an error other than WrongTag, it's fatal. */
	    if (rc != ASN_RC_WrongTag)
	    {
		return FAIL_RC(rc, ("asn_berParse: choice element"));
	    }

	    /* Otherwise, try next choice */
	}

	/* If we didn't find what we were looking for, it's an error */
	if (rc != Success)
	{
	    if (pExists == NULL)
	    {
	        TM_TRACE((asn_hModCB, ASN_TRACE_PARSE,
			  "BER Parse%*d: ...Choice Error", depth*2, depth));
		return FAIL_RC(rc, ("asn_berParse: choice element"));
	    }

	    /* Just let 'em know that we didn't find it */
	    TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, 
		     "BER Parse%*d: ...Choice omitted", depth*2, depth));
	    *pExists = FALSE;

	    break;
	}

	/* Assign the choice number of the one we found */
	*pChoice = (OS_Uint8) i;

	TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, 
		 "BER Parse%*d: ...Choice is %u", depth*2, depth, i));

	break;

    case ASN_Type_Buffer:
	/* Clone the buffer segment */
	RC_CALL(rc,
		BUF_cloneBufferPortion(hBuf,
				       BUF_REMAINDER,
				       FALSE,
				       &hBufferContent),
		("asn_berParseBitString: clone of buffer content"));

	/* Get the length of the cloned buffer */
	len = BUF_getBufferLength(hBufferContent);

        TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, 
		 "BER Parse%*d: Any%ld", depth*2, depth, len));

	/* Keep track of length used */
	*pduLength += len;

	/* Give 'em their data */
	*(void **) (pCStruct + pTab->dataOffset) = hBufferContent;
	break;

    default:
	return FAIL_RC(ProgrammerError,
		       ("asn_berParse: Unexpected ASN Type: 0x%x",
			pTab->type));
    }

    TM_TRACE((asn_hModCB, ASN_TRACE_PARSE, 
	     "BER Parse%*d: === returns SUCCESS ===", depth*2, depth));

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

static ReturnCode
formatLength(OS_Uint32 length,
	     void * hBuf,
	     OS_Uint8 * pLengthOfLength)
{
    OS_Uint8	    lengthOfLength;
    ReturnCode 	    rc;
    
    /* Add the length */
    if (length <= 127)
    {
	/* keep track of the fact that the length was one octet */
	lengthOfLength = 1;
	
	/* we can use the short form of length */
	RC_CALL(rc,
		BUF_addOctet(hBuf, (OS_Uint8) (length & 0xff)),
		("formatLength: short form"));
    }
    else
    {
	/*
	 * We have to use the long form of length.
	 */

	/*
	 * Add the length value to the PDU, and determine how many
	 * octets it takes.
	 */
	for (lengthOfLength = 0; length != 0; lengthOfLength++)
	{
	    /* Add this portion of the length */
	    RC_CALL(rc,
		    BUF_addOctet(hBuf, (OS_Uint8) (length & 0xff)),
		    ("formatLength: long form: portion"));
	    
	    /* Shift out the part we've used already */
	    length >>= 8;
	}
	
	/*
	 * Add the length of the length, while at the same time,
	 * setting the high-order bit which specifies that we're
	 * using the long form.
	 */
	RC_CALL(rc,
		BUF_addOctet(hBuf,
			     (OS_Uint8) (lengthOfLength | 0x80)),
		("formatLength: long form: length of length"));
	
	/* Keep track of length of the length (including its length) */
	++lengthOfLength;
    }
    
    /* Give 'em the length of the length */
    *pLengthOfLength = lengthOfLength;
    
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

static ReturnCode
parseLength(void * hBuf,
	    OS_Uint32 * pLength,
	    OS_Uint32 * pLengthOfLength)
{
    OS_Uint8	    x;
    OS_Uint8	    lengthOfLength;
    OS_Uint32	    dataLength;
    ReturnCode	    rc;
    
    /* Get the first octet of the length */
    RC_CALL(rc, BUF_getOctet(hBuf, &x),
	    ("parseLength: first octet of length"));
    
    /* Is this the short form or the long form of a length encoding? */
    if (x & 0x80)
    {
	/*
	 * It's the long form.
	 */
	
	/* Get length of the length, and then grab the length itself */
	lengthOfLength = (x & ~0x80);

	/* Give caller length of length + length of length of length */
	*pLengthOfLength = lengthOfLength + 1;
	
	for (dataLength = 0; lengthOfLength != 0; lengthOfLength--)
	{
	    /* Get the next octet of the length */
	    RC_CALL(rc,
		    BUF_getOctet(hBuf, &x),
		    ("parseLength: next octet of length"));
	    
	    /* OR-in this next octet */
	    dataLength = (dataLength << 8) | x;
	}
	
	/* Give 'em the total length */
	*pLength = dataLength;
    }
    else
    {
	/*
	 * It's the short form.  We already have the length.
	 */
	*pLength = x;
	*pLengthOfLength = 1;
    }
    
    return Success;
}



#define  EMSD_BcdString  unsigned char *

static int
stringToBcd (const char *str, unsigned char *pBcd);

static char *
bcdToString (EMSD_BcdString emsdAddress, int len);

/* returns length */

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

static int
intToBcd (OS_Uint32 integer, unsigned char *bcdString)
{
  int    len               = 0;
/*  int    count             = 0; */
  char   digitString[16];

  sprintf (digitString, "%lu", integer);
  len = stringToBcd (digitString, bcdString);

  return (len);
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

static OS_Uint32
bcdToInt (unsigned char *bcdString, int len)
{
  OS_Uint32        integer       = 0;         
  char *           pDigitString  = NULL;

  pDigitString = bcdToString (bcdString, len);
  integer = (OS_Uint32) atol (pDigitString);
  TM_TRACE((asn_hModCB, ASN_TRACE_DETAIL, 
	   "BCD->Int: %s->%lu", pDigitString, integer));

  return (integer);
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

static char *
bcdToString (EMSD_BcdString emsdAddress, int bcdLen)
{
    static char  str[16];

    int                   count      = 0;
    unsigned char *       pBcd       = NULL;
    unsigned char *       pStr       = NULL;

    /* 
     * The input is a BCD-encoded integer, which can be 40 digits or less
     * (The max length has been reduced to 12, to hold a 32-bit value -- fnh)
     *
     *  x01 x23 x34 x56 x78 xFF
     *  n+0 n+1 n+2 n+3 n+4 n+5   offset from the starting address
     *
     *  decimal value = 12345678
     *
     * Note that the terminating xFF is used internally to terminate the
     * octet strings, it is not subject to ASN.1 encoding nor transmitted.
     *
     * The output is a string of the format "123456780123445"
     *
     * If the length is 0, the terminator alone is used.
     */

    OS_memSet(str, '\0', sizeof(str));
    pStr = str;

    /* find the number of digits in the address */
    for (pBcd = emsdAddress; *pBcd != 0xFF; pBcd++)
      {
	*pStr++ = ((*pBcd >> 4) & 0xF) + '0';
	*pStr++ = ((*pBcd >> 0) & 0xF) + '0';
	if ((bcdLen > 0) && (++count > bcdLen))
	  {
	    TM_TRACE((asn_hModCB, ASN_TRACE_ERROR, "ERROR: No BCD terminator"));
	    break;
	  }
      }
    
    /* skip leading zeros */
    for (pStr = str; *pStr == '0'; pStr++)
      ;

    /* handle the case where the ouptut string is null */
    if (*pStr == '\0')
    {
	str[0] = '0';
	str[1] = '\0';
	pStr = str;
    }

    return (pStr);
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

static int
stringToBcd (const char *str, unsigned char *pBcd)
{
    int      len         = 0;
    char     tmp[18];
    char *   pTmp        = tmp;
    
    if (pBcd == NULL)
          return (0);
    pBcd[0] = 0x00;
    pBcd[1] = 0xff;

    if (str == NULL)
          return (0);

    OS_memSet(tmp, '\0', sizeof (tmp));

    *pTmp++ = '0';    /* add two leading zeroes in case we need them */
    *pTmp++ = '0';

    while (*str)
      {
	if (*str >= '0' && *str <= '9')
	  {
	    *pTmp++ = *str;
	  }
	str++;
      }

    /* start so there are an even number of bytes */
    pTmp = tmp;
    if (strlen(pTmp) > 2)
        pTmp++;		/* we don't need two leading zeros */
    pTmp += (strlen(pTmp) % 2) ? 1 : 0;

    TM_TRACE((asn_hModCB, ASN_TRACE_DETAIL, "Str->BCD: pTmp='%s'", pTmp));

    while (*pTmp)
      {
	*pBcd  = ((*pTmp++ - '0') << 4) & 0xF0;
	*pBcd |= ((*pTmp++ - '0') << 0) & 0x0F;
	pBcd++;
	len++;
      }

    *pBcd = 0xFF;

    return (len);
}
