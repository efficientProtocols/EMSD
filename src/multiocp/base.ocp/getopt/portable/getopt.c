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
 * File name: getopt.c
 *
 * Description: getopt - get option letter from argv
 *
 */

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: getopt.c,v 1.5 1996/11/07 02:45:54 kamran Exp $";
#endif /*}*/

#include <stdio.h>
#include <string.h>

char  *optarg;		/* Global argument pointer. */
int   optind = 0;	/* Global argv index. */

static char  *scan = NULL;	/* Private scan pointer. */



/*<
 * Function:    getopt
 *
 * Description: Get options.
 *
 * Arguments:	argc, argv, option.
 *
 * Returns: 
 *
>*/

int
getopt(int argc, char *const*argv, const char *optstring)
{
	register char c;
	register char *place;

	optarg = NULL;

	if (scan == NULL || *scan == '\0') {
		if (optind == 0)
			optind++;
	
		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		if (strcmp(argv[optind], "--")==0) {
			optind++;
			return(EOF);
		}
	
		scan = argv[optind]+1;
		optind++;
	}

	c = *scan++;
	place = strchr(optstring, c);

	if (place == NULL || c == ':') {
		fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
		return('?');
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			optarg = scan;
			scan = NULL;
		} else if (optind < argc) {
			optarg = argv[optind];
			optind++;
		} else {
			fprintf(stderr, "%s: -%c argument missing\n", argv[0], c);
			return('?');
		}
	}

	return(c);
}
