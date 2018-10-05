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
#include "asn.h"
#include "strfunc.h"

static void
printTree(ASN_TableEntry * pTab,
	  unsigned char * pCStruct,
	  ASN_DebugPrint printValues,
	  int indentLevel,
	  char * pMsg);

static char *
type(ASN_TableEntryType type);

/*******************************************************************/

/*
 * Print the ASN Parse Tree.
 */
void
ASN_printTree(void * hTab,
	      void * pCStruct,
	      ASN_DebugPrint printValues,
	      char * pMsg)
{
    ASN_TableEntry *	pTab = QU_FIRST(hTab);
    
    printTree(pTab, (unsigned char *) pCStruct, 
	      printValues, 0, pMsg);
}


static void
printTree(ASN_TableEntry * pTab,
	  unsigned char * pCStruct,
	  ASN_DebugPrint printValues,
	  int indentLevel,
	  char * pMsg)
{
    ASN_TableEntry * 	pChildTab;
    OS_Uint32		integer;
    OS_Uint32		choice = 0xffffffff;
    OS_Uint32		thisChoice;
    OS_Boolean		recurse = TRUE;
    OS_Boolean		printThisValue;
    STR_String		hString;
    int			numBits;
    int			i;

    /* See if we're omiting optional elements */
    if (printValues == ASN_DebugPrint_Existing)
    {
	/* We are.  See if this is an optional element */
	if (pTab->existsOffset != ASN_MANDATORY)
	{
	    /* It is. See if it exists */
	    if (! *(OS_Boolean *) (pCStruct + pTab->existsOffset))
	    {
		/* It doesn't.  Get outta here. */
		return;
	    }
	}
    }
    
    printf("%*s", indentLevel, "");

    if (pMsg != NULL)
    {
	printf("%s: ", pMsg);
    }

    printf("\"%s\" ", pTab->pDebugMessage);
    printf("%s ", type(pTab->type));
    printf("[%x] ", pTab->tag);
    printf("(%s%s) ",
	   pTab->existsOffset == ASN_MANDATORY ? "M" : "O-",
	   pTab->existsOffset == ASN_MANDATORY ? "" :
	   (*(OS_Boolean *) (pCStruct + pTab->existsOffset) ?
	    "present" : "not present"));

    if (printValues == ASN_DebugPrint_NoValues)
    {
	goto skipValue;
    }

    /* If this element is optional, ... */
    if (pTab->existsOffset != ASN_MANDATORY)
    {
	/* ... and it doesn't exist, ... */
	if (! *(OS_Boolean *) (pCStruct + pTab->existsOffset))
	{
	    /* ... make sure we don't print it or recurse into it. */
	    recurse = FALSE;
	    printValues = ASN_DebugPrint_NoValues;
	    goto skipValue;
	}
    }

    switch(pTab->type)
    {
    case ASN_Type_Integer:
    case ASN_Type_BcdString:	/* HACK */
	integer =
	    *(OS_Uint32 *) (pCStruct +
			    pTab->dataOffset) & 0xffffffff;
	printf("Val=0x%lx", integer);
	break;

    case ASN_Type_BitString:
	hString = (STR_String) (pCStruct + pTab->dataOffset);
	if (hString == NULL)
	{
	    printf("<NULL>");
	}
	else
	{
	    numBits = STR_getStringLength(hString);
	    
	    printf("Val=");
	    for (i = 0; i < numBits; i++)
	    {
		printf("%c",
		       STR_bitStringGetBit(hString, i) ? '1' : '0');
	    }
	}
	break;

    case ASN_Type_OctetString:
	hString = *(STR_String *) (pCStruct + pTab->dataOffset);
	if (hString == NULL)
	{
	    printf("<NULL>\n");
	}
	else
	{
	    integer = STR_getStringLength(hString);
	    if (integer == 0xffff)
	    {
		printf("ZString ");
		integer =
		    (OS_Uint32) strlen((char *) STR_stringStart(hString));
	    }
	    printf("Len=%lu\n", integer);
	    STR_dump(STR_stringStart(hString),
		     (OS_Uint16) integer, indentLevel + 4);
	}
	break;

    case ASN_Type_SequenceOf:
	integer = *(OS_Uint32 *) (pCStruct + pTab->dataOffset);
	printf("Count=%lu\n", integer);
	break;

    case ASN_Type_Choice:
	choice = *(OS_Uint32 *) (pCStruct + pTab->dataOffset);
	printf("Choice #%lu\n", choice + 1);
	break;

    default:
	break;
    }

 skipValue:
    printf("\n");
    fflush(stdout);

    if (recurse)
    {
	for (thisChoice = 0, pChildTab = QU_FIRST(&pTab->tableList);
	     ! QU_EQUAL(pChildTab, &pTab->tableList);
	     thisChoice++, pChildTab = QU_NEXT(pChildTab))
	{
	    /* Initially assume that we'll print this value */
	    printThisValue = printValues;

	    /* If we're dealing with a choice... */
	    if (choice != 0xffffffff)
	    {
		/* ... and this isn't the selected choice element... */
		if (thisChoice != choice)
		{
		    /* ... and we're not to print non-existing elements... */
		    if (printValues == ASN_DebugPrint_Existing)
		    {
			/* don't recurse. See ya. */
			return;
		    }
		    else
		    {
			/* otherwise, make sure it doesn't get printed */
			printThisValue = ASN_DebugPrint_NoValues;
		    }
		}
	    }
	    
	    if (choice == 0xffffffff || ! printValues || thisChoice == choice)
	    {
		printThisValue = printValues;
	    }
	    else
	    {
		printThisValue = ASN_DebugPrint_NoValues;
	    }

	    printTree(pChildTab, pCStruct,
		      printThisValue, indentLevel + 2, pMsg);
	}
    }
}


static char *
type(ASN_TableEntryType type)
{
    static char	    buf[32];

    switch(type)
    {
    case ASN_Type_None:
	return "None";
	
    case ASN_Type_Integer:
	return "Integer";
	
    case ASN_Type_BitString:
	return "BitString";
	
    case ASN_Type_OctetString:
	return "OctetString";
	
    case ASN_Type_BcdString:	/* HACK! */
	return "BcdString (HACK)";
	
    case ASN_Type_Sequence:
	return "Sequence";
	
    case ASN_Type_SequenceOf:
	return "SequenceOf";
	
    case ASN_Type_Choice:
	return "Choice";
	
    case ASN_Type_Buffer:
	return "Buffer";
	
    default:
	sprintf(buf, "<UNKNOWN TYPE: %d>", type);
	return buf;
    }
}
