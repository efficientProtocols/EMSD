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
 * File:	strflds.c
 *
 * Description:
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: strflds.c,v 1.8 1996/11/11 00:12:08 fletch Exp $";
#endif /*}*/

/* #includes */
#include "estd.h"
 


/*<
 * Function:	PF_strIgnore
 *
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
>*/
PUBLIC Char * 
PF_strIgnore(Char *inStr, Char *ignores)
{
    char *pFound;
    int   c;

    /* Ignore every thing up until the first character that is not in
     * Ignores.
     */
    do {
        if ( (c = *inStr++) == '\0' ) {
	    /* So there was nothing there but ignores */
	    break;
        }
    } while ( (pFound = strchr(ignores, c)) );
    return ( --inStr );
}





/*<
 * Function: PF_strNextFld
 *
 * Description:	
 *  Given a a pointer to the input string, prs_str copies contents of 
 *  i_str to o_str until any of charachters in p_seps is detected.
 *  -) All initial seperators are ignored.
 *  -)A '\0' is written to o_str instead of the seperator.
 *  -) contents of p_i_str points to the character following the seperator
 *      unless the seperator had been a '\0' in which case p_i_str points to
 *      it.
 *  -) single quote ('\'') is not copied to output, what is enclosed in 
 *      single quotes is immune to seperator detection.
 *
 * Returns:
 *  detected seperator.
 *
 * 
>*/
PUBLIC Char 
PF_strNextFld(char **p_i_str, char *o_str, char *p_seps)
                        /* pointer to input string */
                        /* pointer to output string */
               
{
    char * i_str   = NULL;
    char * p_fnd   = NULL;
    int    c       = 0;

    i_str = * p_i_str;

    /* Skipp over initial seperators */
    do {
        if ( (c = *i_str++) == '\0' ) {
            break;
        }
    } while ( (p_fnd = strchr(p_seps, c)) );

    while ((p_fnd = strchr(p_seps, c)) == NULL) {
        int completed;
        switch (c) {
        case '\'':
            completed = FALSE;
            while ( (c = *i_str++) ) {
                if (c == '\'') {
                    completed = TRUE;
                    break;
                } else {
                    *o_str++ = c;
                }
            }
            if (!completed) {
                --i_str;
#if 0
                fprintf (stderr, "Unmatched \'\n");
#endif
            }
            break;
        default:
            *o_str++ = c;
            break;
        }
        c = *i_str++;
    }
    if ( *p_fnd == '\0') {
        --i_str;
    }
    *o_str = '\0';
    *p_i_str = i_str;
    return (*p_fnd);
}



/*<
 * Function: 	str2Flds
 *
 * Description: Gets an array of arguments from a string
 *
 * Parameters:
 *
 * Returns: 	Number of arguments detected.
 * 
>*/
PUBLIC int 
PF_strToFlds(char *str, char **flds, int maxflds, char *seps)
          
                  /* Where to put the parsed fields */
                 /* Maximum number of arguments expected */
            
{
    int argc;
    char  *str2;
    char **strPtr;
    char parseStr();
    char sep_fnd;
    
    /* One can not reliably take &str */
    str2 = str;
    strPtr = &str2;

    argc  = 0;

    do {
        sep_fnd = PF_strNextFld(strPtr, str, seps);
        flds[argc++] = str;
        str = *strPtr;
    } while ((argc < maxflds) && sep_fnd);

    if ( ! *flds[argc-1] ) {
	/* There had only been seperators.
	 * As far as application is concerned nothing is there
	 */
	--argc;
    }
    
    return argc;
}




/*<
 * Function: 	PS_strToArgs
 *
 * Description: Gets an array of arguments from a string
 *
 * Parameters:
 *
 * Returns: 	Number of arguments detected.
 * 
>*/
PUBLIC int 
PF_strToArgs(char *str, char **argv, int maxarg)
          
                  /* pointer to an array of strings containing the arguments */
                /* Maximum number of arguments expected */
{
    return (PF_strToFlds(str, argv, maxarg, " "));
}

