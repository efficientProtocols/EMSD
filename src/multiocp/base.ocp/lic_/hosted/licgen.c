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
 * File name: licgen.c
 *
 * Description: License File Generator
 *
 * Functions:
 *
 */

#include <stdio.h>
#include <string.h>

#include <pf.h>

#include "lic.h"

void
fgets_sans_newline(char *str, int len, FILE *stream)
{
    int	inlen;

    fgets(str, len, stream);
    inlen = strlen(str);

    if ( str[inlen - 1] == '\n' )
	str[inlen - 1] = '\0';
}

int
main()
{
    char licensee[256];
    char startDate[256];
    char endDate[256];
    char hostID[256];
    char *hash;

    fprintf(stdout, "Enter Licensee: ");
    fgets_sans_newline(licensee, sizeof(licensee), stdin);
    fprintf(stdout, "Licensee is <%s>\n", licensee);
    
    fprintf(stdout, "Enter Start Date: ");
    fgets_sans_newline(startDate, sizeof(startDate), stdin);
    fprintf(stdout, "Start Date is <%s>\n", startDate);

    fprintf(stdout, "Enter End Date: ");
    fgets_sans_newline(endDate, sizeof(endDate), stdin);
    fprintf(stdout, "End Date is <%s>\n", endDate);

    fprintf(stdout, "Enter Host ID: ");
    fgets_sans_newline(hostID, sizeof(hostID), stdin);
    fprintf(stdout, "Host ID is <%s>\n", hostID);

    LIC_computeHash(licensee, startDate, endDate, hostID, &hash);
    fprintf(stdout, "Hash is <%s>\n", hash);

    if ( LIC_checkHash(licensee, startDate, endDate, hostID, hash) == Success ) {
	printf("checks out using LIC_checkHash()!\n");
    }
    else {
	printf("does not check out using LIC_checkHash()!\n");
	exit(1);
    }

    exit(0);
}
