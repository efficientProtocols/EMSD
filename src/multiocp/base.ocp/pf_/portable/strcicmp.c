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
 * File:	strcicmp.c
 *
 * Description: String compare: ignore case.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: strcicmp.c,v 1.7 1997/10/18 23:34:25 mohsen Exp $";
#endif /*}*/

/* #includes */
#include "estd.h"


/*<
 * Function: PF_strCaseIgnoreCmp
 *
 * Description: Compre two string while ignoring the case.
 *
 * Returns:
 *	0 if ignoring the case, str1 and str2  are the same.
 *	1 otherwise.
 *
>*/
Int
PF_strCaseIgnoreCmp(String str1, String str2)
{
    Int retVal = 0;

#if defined(OS_VARIANT_WinCE)
    static WCHAR 	*wstr1;
    static WCHAR 	*wstr2;
    static Int 		wlen1;
    static Int 		wlen2;
    Int 		len1 = strlen(str1);
    Int			len2 = strlen(str2);
    wint_t		wc1;
    wint_t		wc2;
    
    /* maybe realloc WCHAR buffers */
    if ( len1 >= wlen1 ) {
	if ( wstr1 ) OS_free(wstr1);
	wstr1 = (WCHAR *)OS_alloc(sizeof(WCHAR)*(len1 + 1));
    } 
	
    if ( len2 >= wlen2 ) {
	if ( wstr2 ) OS_free(wstr2);
	wstr2 = (WCHAR *)OS_alloc(sizeof(WCHAR)*(len2 + 1));
    }

    /* make UNICODE versions of str1 and str2 */
    wsprintf(wstr1, TEXT("%S"), str1);
    wsprintf(wstr2, TEXT("%S"), str2);

    while ( (wc2 = *wstr2++) ) {
	if (iswupper(wc2)) {
	    wc2 = towlower(wc2);	
	}
	wc1 = *wstr1++;
	if (iswupper(wc1)) {
	    wc1 = towlower(wc1);
	}
	if (wc1 != wc2) {
	    retVal = wc1 - wc2;
	    break;
	}
    }
#else
    Int c1;
    Int c2;

    while ( (c2 = *str2++) ) {
	if (isupper(c2)) {
	    c2 = tolower(c2);
	}
	c1 = *str1++;
	if (isupper(c1)) {
	    c1 = tolower(c1);
	}
	if (c1 != c2) {
	    retVal = c1 - c2;
	    break;
	}
    }
#endif /* OS_VARIANT_WinCE */

    return (retVal);
}
