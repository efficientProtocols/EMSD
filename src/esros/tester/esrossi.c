/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File name: esrossi.c
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
static char rcs[] = "$Id: esrossi.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>

#include "estd.h"
#include "ui.h"
#include "addr.h"
#include "tm.h"
#include "sch.h"


#ifdef TM_ENABLED
extern TM_ModuleCB *SCH_modCB; /* for compiler bug fix */
#endif

extern FILE *xfopen();
extern int errno;

struct {
	FILE	*fp;
	int	line;
	char	name[256];
} files[10];

extern FILE *yyin;
extern int yydebug;
extern int yychar;

char strval[4096];
char logfile[256];
char fname[256] = "standard input";
char *directory = NULL;
int ignore_faults = 0;		
jmp_buf errlbl;

N_SapAddr parseaddr;

FILE *logfp = stderr;

int verbose = 1;
int cflag   = 0;
int egflag  = 0;
static int indexVal = -1;

int line    = 1;

int yyparse();
Void G_exit();
Void log();
int hexval();

Void
TSI_parse()
{
	for (;;) {
#ifdef TM_ENABLED
               	TM_ModuleCB *SCH_modCBKeep = SCH_modCB; /* Compiler bug fix */
#endif

		if (setjmp(errlbl)) {
/*
			if (!cflag)
				tabort();
*/
			fclose(yyin);
			while (indexVal >= 0) {
				fclose(files[indexVal].fp);
				indexVal--;
			}
		}
#ifdef TM_ENABLED
               	SCH_modCB = SCH_modCBKeep; /* Compiler bug fix */
#endif

		if (directory) {
		    UI_printf("Current directory for scenarios is:\n%s\n\n", directory);
		}
		UI_printf("Enter scenario file name: ");
		if (UI_gets(fname, sizeof(fname)) == (String)0)
			break;
		if (fname[0] == 0)
			continue;
		if ((yyin = xfopen(fname, "r")) == NULL) {
			fprintf(stderr, "Cannot open scenario file (%s)\n", fname);
			continue;
		}
		indexVal = -1;
		line = 1;
		yyparse();
	}
	printf("\nEOF\n");

	/* Do any Neccassary Clean Up */
	G_exit(0);
}

Void
yyerror(char *s)
{
	log("%s, line %u: %s next token=%u\n", fname, line, s, yychar);
	longjmp(errlbl, 1);
}

Void
do_log(char *s)
{
	if (logfile[0]) {
		fclose(logfp);
		logfile[0] = 0;
	}
	if (*s == 0) {
		logfp = stderr;
		return;
	} 

	strcpy (logfile, s); /* check this part */
		
	if ((logfp = fopen(logfile, "w")) == NULL) {
		log("warning: could not create logfile (%s)\n", logfile);
		logfile[0] = 0;
	}
}

Void
log(a, b, c, d, e, f, g, h)
char *a;
int  b, c, d, e, f, g, h;
{
	if (verbose) {
		UI_printf(a, b, c, d, e, f, g, h);
		UI_printf("\n");
	}
	if (logfile[0]) {
		fprintf(logfp, a, b, c, d, e, f, g, h);   /* test this */
		fprintf(logfp, "\n");			/* test this */
	}
}

Void
plog(a, b, c, d, e, f, g, h)
char *a;
int b, c, d, e, f, g, h;
{
	if (verbose)
		printf(a, b, c, d, e, f, g, h);
	if (logfile[0])
		fprintf(logfp, a, b, c, d, e, f, g, h);
}

Void
fatal(a, b, c, d, e, f)
char *a;
int  b, c, d, e, f;
{
	printf("Fatal error in ");
	printf(a, b, c, d, e, f);
	printf("\n");
	if (logfile[0]) {
		fprintf(logfp, "Fatal error in ");
		fprintf(logfp, a, b, c, d, e, f);
		fprintf(logfp, "\n");
		fclose(logfp);
	}
	longjmp(errlbl, 1);
}

Void
include(char *s)
{
	register FILE *fp;

	indexVal++;
	files[indexVal].fp = yyin;
	files[indexVal].line = line;
	strcpy(files[indexVal].name, fname);
	if ((fp = xfopen(s, "r")) == NULL) {
		indexVal--;
		printf("%s: cannot open\n", s);
		longjmp(errlbl, 1);
	}
	strcpy(fname, s);
	line = 1;
	yyin = fp;
}

Void
path(char *s)
{
	directory = strdup(s);
}

Void
delay(int n)
{
	sleep(n);
}

int
yywrap()
{
	fclose(yyin);
	if (indexVal < 0)
		return(1);

	yyin = files[indexVal].fp;
	line = files[indexVal].line;
	strcpy(fname, files[indexVal].name);
	indexVal--;
	return(0);
}

int
xtoi(register char *p)
{
	register int n;

	n = 0;
	while (hexval(*p) >= 0)
		n = n * 16 + hexval(*p++);
	return(n);
}

int
hexval(register int c)
{
	if (c >= '0' && c <= '9')
		return(c - '0');
	if (c >= 'a' && c <= 'f')
		return(c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return(c - 'A' + 10);
	return(-1);
}

FILE *xfopen(char *fname, char *mode)
{
	register FILE *fp;
	char buffer[256];

	if ((fp = fopen(fname, mode)) != NULL)
		return(fp);
	if (directory == NULL)
		return(NULL);
	strcpy(buffer, directory);
	strcat(buffer, "/");
	strcat(buffer, fname);
	return(fopen(buffer, mode));
}
