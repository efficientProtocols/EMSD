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

/*
 * Author: Derrell Lipman
 * History:
 *
 */

#ifndef __ASN_H__
#define	__ASN_H__

#include "estd.h"
#include "queue.h"
#include "strfunc.h"

/*
 * Trace flags
 */
#define ASN_TRACE_ERROR    (1 << 0) /* Errors */
#define ASN_TRACE_WARNING  (1 << 1) /* Warnings */
#define ASN_TRACE_INIT	   (1 << 2) /* Initialization messages */
#define ASN_TRACE_FORMAT   (1 << 3) /* ASN.1 encoding */
#define ASN_TRACE_PARSE    (1 << 4) /* ASN.1 decoding */
#define ASN_TRACE_DATA     (1 << 5) /* Values of encoded/decoded elements */
#define ASN_TRACE_DETAIL   (1 << 6) /* Various details usually not wanted */

/*
 * Encoding rules for which we have provided formatters and parsers.
 */
typedef enum
{
    ASN_EncodingRules_Basic	= 0,

    /* Maximum legal value for an "int" of this type (validation) */
    ASN_EncodingRules_Max	= ASN_EncodingRules_Basic
} ASN_EncodingRules;

typedef OS_Uint8	ASN_ChoiceSelector;
typedef OS_Uint32	ASN_Count;

typedef STR_String	ASN_BitString;

#define	ASN_TAG_CONTEXT(n)	(0x80 + (n))
#define	ASN_TAG_APPL(n)		(0x40 + (n))


/*
 * Tag modifier indicates that an explicit flag should be added in
 * addition to the one specified in a CHOICE.
 */
#define	ASN_EXPLICIT		(0x20)

enum ASN_UniversalTags
{
    ASN_UniversalTag_Boolean				= 0x01,
    ASN_UniversalTag_Integer				= 0x02,
    ASN_UniversalTag_BitString				= 0x03,
    ASN_UniversalTag_OctetString			= 0x04,
    ASN_UniversalTag_Null				= 0x05,
    ASN_UniversalTag_ObjectIdentifier			= 0x06,
    ASN_UniversalTag_ObjectDescriptor			= 0x07,
    ASN_UniversalTag_External				= 0x08,
    ASN_UniversalTag_Real				= 0x09,
    ASN_UniversalTag_Enumerated				= 0x0a,
    ASN_UniversalTag_Sequence				= 0x10,
    ASN_UniversalTag_Set				= 0x11,
    ASN_UniversalTag_NumericString			= 0x12,
    ASN_UniversalTag_PrintableString			= 0x13,
    ASN_UniversalTag_TeletexString			= 0x14,
    ASN_UniversalTag_VideotexString			= 0x15,
    ASN_UniversalTag_IA5String				= 0x16,
    ASN_UniversalTag_GraphicString			= 0x19,
    ASN_UniversalTag_VisibleString			= 0x1a,
    ASN_UniversalTag_GeneralString			= 0x1b
};


/* Success/Fail Return Codes for the ASN module */
typedef enum
{
    ASN_RC_WrongTag				= (1 | ModId_Asn),
    ASN_RC_MissingElement			= (2 | ModId_Asn),
    ASN_RC_InvalidSize				= (3 | ModId_Asn),
    ASN_RC_InvalidLengthOfLength		= (4 | ModId_Asn),
    ASN_RC_FormatErrorInPDU			= (5 | ModId_Asn),
    ASN_RC_InvalidEncodingRules			= (6 | ModId_Asn),
    ASN_RC_IllegalData				= (7 | ModId_Asn)
} ASN_ReturnCode;





typedef enum
{
    ASN_Type_None		= 0,
    ASN_Type_Integer,
    ASN_Type_BitString,
    ASN_Type_OctetString,
    ASN_Type_BcdString,	      /* An octet string with a different terminator */
    ASN_Type_Sequence,
    ASN_Type_SequenceOf,
    ASN_Type_Choice,
    ASN_Type_Buffer
} ASN_TableEntryType;


typedef struct ASN_TableEntry
{
    QU_ELEMENT;
    ASN_TableEntryType	    type;
    OS_Uint8		    tag;
    OS_Uint32		    minimum;
    OS_Uint32		    maximum;
    OS_Uint32		    existsOffset;
    OS_Uint32		    dataOffset;
    OS_Uint32		    maxDataLength;
    OS_Uint32		    elementSize;
    char *		    pDebugMessage;
    struct
    {
	QU_HEAD;
    } tableList;
} ASN_TableEntry;

/* Internal representation, in table entry, if no Exists pointer */
#define	ASN_MANDATORY	0xffffffff

/* Internal representation, in table entry, if no Data pointer */
#define	ASN_NODATA	0xffffffff

/* Flags for debug printing of the ASN tree */
typedef enum ASN_DebugPrint
{
    ASN_DebugPrint_NoValues,
    ASN_DebugPrint_Existing,
    ASN_DebugPrint_All
} ASN_DebugPrint;


ReturnCode
ASN_init(void);

ReturnCode
ASN_format(ASN_EncodingRules encRules,
	   void * pTab,
	   void * hBuf,
	   void * pCStruct,
	   OS_Uint32 * pFormattedLength);


ReturnCode
ASN_parse(ASN_EncodingRules encRules,
	  void * pTab,
	  void * hBuf,
	  void * pCStruct,
	  OS_Uint32 * pduLength);


ReturnCode
ASN_newTableEntry(ASN_TableEntry **  ppTab,
		  ASN_TableEntryType type,
		  OS_Uint8           itemTag,
		  QU_Head *          pQ,
		  unsigned char *    pBase,
		  void *             pExists,
		  void *             pData,
		  char *             pDebugMessage);

void
ASN_printTree(void * hTab,
	      void * pCStruct,
	      ASN_DebugPrint printValues,
	      char * pMsg);


#define	ASN_NEW_TABLE(pTab, type, itemTag, pQ, pCStruct, pExists,	\
		      pData, pMessage, debugParamList)			\
									\
	RETURN_IF_FAIL(ASN_newTableEntry(&pTab,				\
					 type,				\
					 itemTag,			\
					 pQ,				\
		       (unsigned char *) pCStruct,			\
				(void *) pExists,			\
			        (void *) pData,				\
					 pMessage),			\
		       debugParamList)

#endif /* __ASN_H__ */



