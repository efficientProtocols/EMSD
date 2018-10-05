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


#include "estd.h"
#include "tm.h"
#include "strfunc.h"
#include "strlocal.h"
#include "eh.h"


/*<
 * Function:    STR_alloc
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

ReturnCode
STR_alloc(OS_Uint16 len,
	  STR_String * phString)
{
    Str_StringStruct *	pString;

    /* Allocate the string structure and additional memory */
    if ((pString = OS_alloc(sizeof(Str_StringStruct) + len)) == NULL)
    {
	EH_problem("STR_alloc: OS_alloc failed!\n");
	return ResourceError;
    }

    /* This is to be the first reference */
    pString->refCount = 1;

    /* It's not an attached string */
    pString->status = Str_Status_LocallyAllocated;

    /* We know the maximum length */
    pString->maxLen = len;

    /* The string currently has no data in it */
    pString->len = 0;

    /* Our data area immediately follows the structure */
    pString->p = (unsigned char *) (pString + 1);

    /* Give 'em what they came for */
    *phString = pString;

    return Success;
}


/*<
 * Function:    STR_attachString
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

ReturnCode
STR_attachString(OS_Uint16 maxLen,
		 OS_Uint16 len,
		 unsigned char * pOctets,
		 OS_Boolean toBeFreed,
		 STR_String * phString)
{
    Str_StringStruct *    pString;

    /* Allocate the string structure and additional memory */
    if ((pString = OS_alloc(sizeof(Str_StringStruct))) == NULL)
    {
	EH_problem("STR_attachString: OS_alloc failed!\n");
	return ResourceError;
    }

    /* This is to be the first reference */
    pString->refCount = 1;

    /* It's an attached string.  Should we free it later? */
    if (toBeFreed)
    {
	pString->status = Str_Status_AttachedDynamic;
    }
    else
    {
	pString->status = Str_Status_AttachedStatic;
    }

    /* We know the maximum length */
    pString->maxLen = maxLen;

    /* We also know the current length of the string */
    pString->len = len;

    /* Our data area was given to us */
    pString->p = (unsigned char *) pOctets;

    /* Give 'em what they came for */
    *phString = pString;

    return Success;
}


/*<
 * Function:    STR_attachZString
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

ReturnCode
STR_attachZString(char * pZString,
		  OS_Boolean toBeFreed,
		  STR_String * phString)
{
    Str_StringStruct *    pString;

    /* Allocate the string structure and additional memory */
    if ((pString = OS_alloc(sizeof(Str_StringStruct))) == NULL)
    {
	EH_problem("STR_attachZString: OS_alloc failed!\n");
	return ResourceError;
    }

    /* This is to be the first reference */
    pString->refCount = 1;

    /* It's an attached string.  Should we free it later? */
    if (toBeFreed)
    {
	pString->status = Str_Status_AttachedDynamic;
    }
    else
    {
	pString->status = Str_Status_AttachedStatic;
    }

    /* We don't know the maximum length */
    pString->maxLen = STR_ZSTRING;

    /* We don't know the current length of the string */
    pString->len = STR_ZSTRING;

    /* Our data area was given to us */
    pString->p = (unsigned char *) pZString;

    /* Give 'em what they came for */
    *phString = pString;

    return Success;
}


/*<
 * Function:    STR_addReference
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

void
STR_addReference(STR_String hString)
{
    Str_StringStruct *	pString = hString;

    /* Increment the reference count */
    ++pString->refCount;
}


/*<
 * Function:    STR_free
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

void
STR_free(STR_String hString)
{
    Str_StringStruct *	pString = hString;

    /* Ignore null pointers */
    if (pString == NULL)
    {
	EH_problem("STR_free: Null pointer to me!\n");
	return;
    }

    /* Decrement the reference count.  Has it gone to zero? */
    if (--pString->refCount == 0)
    {
	/* Yup.  Do we need to free the data memory? */
	if (pString->status == Str_Status_AttachedDynamic)
	{
	    /* Yes.  Free it */
	    OS_free(pString->p);
	}

	/* Free the string structure */
	OS_free(pString);
    }
}


/*<
 * Function:    STR_assignString
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

ReturnCode
STR_assignString(STR_String hString,
		 OS_Uint16 len,
		 unsigned char * pOctets)
{
    Str_StringStruct *	pString = hString;

    /* If the specified length is greater than max len, error. */
    if (pString->maxLen != STR_ZSTRING && len > pString->maxLen)
    {
	EH_problem("STR_assignString: Specified length greater than max len!\n");
	return ResourceError;
    }

    /* Copy the data */
    OS_copy(pString->p, pOctets, len);

    /* Assign the length */
    pString->len = len;

    return Success;
}


/*<
 * Function:    STR_replaceString
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

void
STR_replaceString(STR_String * phDest,
		  STR_String hSrc)
{
    Str_StringStruct *	  pSrc = hSrc;
    Str_StringStruct *    pDest = *(Str_StringStruct **) phDest;

    /* Free the old string */
    STR_free(pDest);

    /* Assign the new string to the old pointer */
    *phDest = pSrc;

    /* Up the reference count */
    ++pSrc->refCount;
}


/*<
 * Function:    STR_appendString
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

ReturnCode
STR_appendString(STR_String hString,
		 OS_Uint16 len,
		 unsigned char * pOctets)
{
    Str_StringStruct *	pString = hString;

    /* If the specified length is greater than max len, error. */
    if (pString->maxLen > 0 && pString->len + len > pString->maxLen)
    {
	EH_problem("STR_appendString: Specified length greater than max len!\n");
	return ResourceError;
    }

    /* Copy the data */
    OS_copy(pString->p + pString->len, pOctets, len);

    /* Increment the length */
    pString->len += len;

    return Success;
}


/*<
 * Function:    STR_assignZSTring
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

ReturnCode
STR_assignZString(STR_String * phString,
		  char * pZString)
{
    OS_Uint16	        len = strlen(pZString);
    Str_StringStruct *	pString = *phString;
    ReturnCode	        rc;

    /* If there's no String allocated, allocate one now */
    if (pString == NULL)
    {
	if ((rc = STR_alloc(len + 1, phString)) != Success)
	{
	    EH_problem("STR_assignZString: STR_alloc failed!\n");
	    return rc;
	}

	pString = *phString;
    }
    else
    {
	/* If the string length is greater than max len, error. */
	if (pString->maxLen > 0 && len > (OS_Uint16) pString->maxLen)
	{
	    EH_problem("STR_assignZString: String length greater than max len!\n");
	    return ResourceError;
	}
    }

    /* Copy the data */
    strcpy((char *) pString->p, pZString);

    /* We don't care about the length of Z-Strings */
    pString->len = STR_ZSTRING;

    return Success;
}


/*<
 * Function:    STR_stringStart
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

unsigned char *
STR_stringStart(STR_String hString)
{
    Str_StringStruct *	pString = hString;

    /* Return the beginning of the data area */
    return pString->p;
}


/*<
 * Function:    STR_stringEnd
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

unsigned char *
STR_stringEnd(STR_String hString)
{
    Str_StringStruct *	pString = hString;
    
    /* Do we know the length of the data? */
    if (pString->len != STR_ZSTRING)
    {
	/* Yup.  Just return a pointer to the end. */
	return pString->p + pString->len;
    }

    /* Determine the length of the Z-String */
    return pString->p + strlen((char *) pString->p);
}


/*<
 * Function:    STR_getMaxLength
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

OS_Uint16
STR_getMaxLength(STR_String hString)
{
    Str_StringStruct *	pString = hString;

    return pString->maxLen;
}



/*<
 * Function:    STR_getStringLength
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

OS_Uint16
STR_getStringLength(STR_String hString)
{
    Str_StringStruct *	pString = hString;

    /* Do we know the length of the data? */
    if (pString->len != STR_ZSTRING)
    {
	/* Yup.  Just give 'em the length. */
	return pString->len;
    }

    /* Determine the length of the Z-String */
    return (OS_Uint16) strlen((char *) pString->p);
}



/*<
 * Function:    STR_setStringLength
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

ReturnCode
STR_setStringLength(STR_String hString,
		    OS_Uint16 len)
{
    Str_StringStruct *	pString = hString;

    /* Set the length, if it doesn't exceed the maximum */
    if (len > (OS_Uint16) pString->maxLen)
    {
	EH_problem("STR_setStringLength: Requested length is greater than max!\n");
	return FAIL_RC(STR_RC_MaxLengthExceeded,
		       ("STR_setStringLength: max length exceeded"));
    }

    pString->len = len;
    return Success;
}


/*<
 * Function:    STR_bitStringGetBit
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

OS_Boolean
STR_bitStringGetBit(STR_String hString,
		    OS_Uint16 bit)
{
    Str_StringStruct *	pString = hString;

    /*
     * If the bit number is beyond the length of the string, just
     * assume that the bit is not set.
     */
    if (bit >= (OS_Uint16) pString->len)
    {
	EH_problem("STR_bitStringGetBit: "
		   "Bit number is beyond the length of the string!\n");

	return FALSE;
    }

    return pString->p[bit];
}


/*<
 * Function:    STR_bitStringSetBit
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

ReturnCode
STR_bitStringSetBit(STR_String hString,
		    OS_Uint16 bit,
		    OS_Boolean value)
{
    OS_Uint16	    	i;
    Str_StringStruct *	pString = hString;

    /*
     * If the bit is within the current length of the string, just
     * set the bit accordingly.
     */
    if (bit < pString->len)
    {
	pString->p[bit] = value;
	return Success;
    }

    /*
     * If the bit fits within maximum length of the bit string, fill
     * in all bits up to the bit to be specified with FALSE, and
     * then set the specified bit.
     */
    if (bit < pString->maxLen)
    {
	/* Set all intervening bits to FALSE */
	for (i = pString->len; i < bit + 1; i++)
	{
	    pString->p[i] = FALSE;
	}

	/* Set this bit's value */
	pString->p[bit] = value;

	/* Modify the length of the bit string */
	pString->len = bit + 1;

	return Success;
    }

    /* It doesn't fit, and we currently don't support resizing */
    return STR_RC_MaxLengthExceeded;
}



/*<
 * Function:    STR_dump
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

void
STR_dump(unsigned char * p,
	 OS_Uint16 len,
	 OS_Uint16 indentLevel)
{
    unsigned char * 		q;
    char 			string[4];
    char 			asciiBuf[9];
    char * 			ascii;
    int 			cnt;
    static char			hex[] = "0123456789ABCDEF";
#ifdef TM_ENABLED
    extern FILE *		tmFile;
    FILE *			hOutput = tmFile;
#elif defined(OS_TYPE_MSDOS) || defined(OS_VARIANT_Windows) || defined(OS_VARIANT_WinCE)
    FILE *			hOutput = NULL;
#else
    FILE *			hOutput = stdout;
#endif /* TM_ENABLED */

    for (ascii = asciiBuf;
	 ascii < asciiBuf + sizeof(asciiBuf) - 1;
	 ascii++)
    {
	*ascii = ' ';
    }
    
    *ascii = '\0';
    ascii = asciiBuf;
    
    if (hOutput != NULL)
    {
	fprintf(hOutput, "%*s", indentLevel, "");
    }

    cnt = 0;
    q = p + len;
    while (p < q)
    {
	if ((cnt & 0x07) == 0)
	{
	    if (cnt > 0)
	    {
		if (hOutput != NULL)
		{
		    fprintf(hOutput, " %s", asciiBuf);
		}
		for (ascii = asciiBuf;
		     ascii < asciiBuf + sizeof(asciiBuf) - 1;
		     ascii++)
		{
		    *ascii = ' ';
		}

		*ascii = '\0';
		ascii = asciiBuf;
		
		if (hOutput != NULL)
		{
#if defined(OS_VARIANT_WinCE)
		    fprintf(hOutput, "\n%s", "");
#else
		    fprintf(hOutput, "\n%*s", indentLevel, "");
#endif /* OS_VARIANT_WinCE */
		}
	    }
	}

	if (isprint(*p))
	    *ascii++ = *p;
	else
	    *ascii++ = '.';
	
	string[0] = hex[(*p >> 4) & 0x0f];
	string[1] = hex[*p & 0x0f];
	string[2] = ' ';
	string[3] = 0;
	if (hOutput != NULL)
	{
	    fprintf(hOutput, string);
	}
	cnt++;
	p++;
    }

    while (ascii != asciiBuf + sizeof(asciiBuf) - 1)
    {
	ascii++;
	if (hOutput != NULL)
	{
	    fprintf(hOutput, "   ");
	}
    }

    if (hOutput != NULL)
    {
	fprintf(hOutput, " %s", asciiBuf);
	fputc('\n', hOutput);
    }
}
