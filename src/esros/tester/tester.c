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


# line 2 "tester.y"

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
 * File name: tester.y
 *
 * Description: ESROS scenario interpreter grammer
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: tester.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include <stdlib.h>
#include <string.h>

#include "estd.h"
#include "du.h"
#include "target.h"
#include "esrossi.h"
#include "addr.h"
#include "extfuncs.h"

extern char strval[];
extern int cflag, verbose;
extern N_SapAddr parseaddr;
extern int ignore_faults;

# define LOG 257
# define LOGFILE 258
# define QUIET 259
# define VERBOSE 260
# define INCLUDE 261
# define ERRORIND 262
# define ERRORREQ 263
# define ERRORCONF 264
# define INVOKEREQ 265
# define INVOKEIND 266
# define RESULTREQ 267
# define RESULTIND 268
# define RESULTCONF 269
# define FAILUREIND 270
# define STRING 271
# define NUMBER 272
# define SAPBIND 273
# define SAPRELEASE 274
# define IPADDR 275
# define SHELL_CMD 276
# define EVENT 277
# define ERROR0 278
# define BASIC 279
# define PACKED 280
# define RESERVED2 281
# define RESERVED3 282
# define DELAY 283
# define PATH 284
# define IGNORE 285
# define NOTICE 286
# define FAULTS 287
# define TRANSFAIL 288
# define LOCRESOURCE 289
# define USERNOTRESP 290
# define REMRESOURCE 291
# define UDP_PC_LOSSY 292
# define UDP_PC_INHIBIT 293
# define SEND 294
# define RECEIVE 295

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#else
#include <malloc.h>
#include <memory.h>
#endif

#include <values.h>

#ifdef __cplusplus

#ifndef yyerror
	void yyerror(const char *);
#endif

#ifndef yylex
#ifdef __EXTERN_C__
	extern "C" { int yylex(void); }
#else
	int yylex(void);
#endif
#endif
	int yyparse(void);

#endif
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 68
# define YYLAST 165
yytabelem yyact[]={

    39,    40,    41,    42,    38,    33,    31,    32,    26,    27,
    28,    29,    30,    34,    68,    78,    35,    36,    82,    77,
    37,    53,    97,    87,    88,    52,    43,    44,    45,    46,
    64,    65,    66,    67,    96,    47,    48,    59,    95,    94,
    80,    81,    92,    84,    55,    56,    57,    58,    91,    75,
    71,    70,    69,    50,    99,    76,    74,    73,    72,    86,
    51,    54,    90,    79,     2,    63,    49,    25,    24,    23,
    22,    21,    20,    19,    18,    17,    16,    15,    14,    13,
    12,    11,    10,     9,     8,     7,     6,     5,     4,     3,
     1,    60,     0,    61,     0,    62,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    83,    85,     0,     0,     0,     0,     0,     0,
    89,     0,     0,     0,     0,    93,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    98,     0,     0,     0,     0,
   100,     0,     0,   101,     0,     0,     0,     0,   102,     0,
     0,     0,     0,   104,   103 };
yytabelem yypact[]={

  -257,  -257,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -219,  -251,  -235,  -235,
-10000000,  -235,-10000000,  -235,  -258,  -220,  -221,  -222,  -213,  -214,
  -215,-10000000,-10000000,  -223,  -216,  -268,  -272,  -254,  -254,-10000000,
  -229,  -235,-10000000,-10000000,  -248,-10000000,-10000000,-10000000,-10000000,-10000000,
  -248,  -230,  -230,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -233,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -234,
-10000000,-10000000,-10000000,  -238,  -253,  -248,-10000000,-10000000,  -217,-10000000,
  -248,-10000000,-10000000,  -248,-10000000,-10000000,-10000000,  -251,-10000000,-10000000,
-10000000,-10000000,  -235,  -248,-10000000 };
yytabelem yypgo[]={

     0,    90,    64,    89,    88,    87,    86,    85,    84,    83,
    82,    81,    80,    79,    78,    77,    76,    75,    74,    73,
    72,    71,    70,    69,    68,    67,    60,    61,    59,    65,
    62,    63 };
yytabelem yyr1[]={

     0,     1,     1,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,    12,    13,    20,    21,
    22,    23,    14,     3,    27,    27,    27,    27,    27,    29,
    29,    29,    29,    29,    26,    26,    30,    30,     4,     8,
     5,     6,    10,    11,     7,     9,    28,    28,    15,    16,
    17,    18,    19,    24,    25,    31,    31,    31 };
yytabelem yyr2[]={

     0,     3,     5,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     7,     5,     5,     5,
     5,     5,     5,    15,     3,     3,     3,     3,     2,     3,
     3,     3,     3,     2,     3,     2,     3,     2,     9,     9,
     7,     7,     9,     5,     3,     3,     2,     5,     5,     5,
     5,     3,     3,     7,     7,     3,     3,     2 };
yytabelem yychk[]={

-10000000,    -1,    -2,    -3,    -4,    -5,    -6,    -7,    -8,    -9,
   -10,   -11,   -12,   -13,   -14,   -15,   -16,   -17,   -18,   -19,
   -20,   -21,   -22,   -23,   -24,   -25,   265,   266,   267,   268,
   269,   263,   264,   262,   270,   273,   274,   277,   261,   257,
   258,   259,   260,   283,   284,   285,   286,   292,   293,    -2,
   272,   -26,   276,   272,   -27,   279,   280,   281,   282,   272,
   -27,   -27,   -27,   -29,   288,   289,   290,   291,   272,   272,
   272,   272,   271,   271,   271,   272,   271,   287,   287,   -31,
   294,   295,   272,   -31,   272,   -27,   -28,   271,   272,   -28,
   -30,   278,   272,   -30,   272,   272,   272,   275,   -28,   271,
   -28,   -28,   -26,   -27,   -28 };
yytabelem yydef[]={

     0,    -2,     1,     3,     4,     5,     6,     7,     8,     9,
    10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,     0,     0,     0,     0,
    54,     0,    55,     0,     0,     0,     0,     0,     0,     0,
     0,    61,    62,     0,     0,     0,     0,     0,     0,     2,
     0,     0,    44,    45,     0,    34,    35,    36,    37,    38,
     0,     0,     0,    53,    39,    40,    41,    42,    43,     0,
    27,    32,    58,    59,    60,    28,    29,    30,    31,     0,
    65,    66,    67,     0,     0,     0,    50,    56,     0,    51,
     0,    46,    47,     0,    26,    63,    64,     0,    48,    57,
    49,    52,     0,     0,    33 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"LOG",	257,
	"LOGFILE",	258,
	"QUIET",	259,
	"VERBOSE",	260,
	"INCLUDE",	261,
	"ERRORIND",	262,
	"ERRORREQ",	263,
	"ERRORCONF",	264,
	"INVOKEREQ",	265,
	"INVOKEIND",	266,
	"RESULTREQ",	267,
	"RESULTIND",	268,
	"RESULTCONF",	269,
	"FAILUREIND",	270,
	"STRING",	271,
	"NUMBER",	272,
	"SAPBIND",	273,
	"SAPRELEASE",	274,
	"IPADDR",	275,
	"SHELL_CMD",	276,
	"EVENT",	277,
	"ERROR0",	278,
	"BASIC",	279,
	"PACKED",	280,
	"RESERVED2",	281,
	"RESERVED3",	282,
	"DELAY",	283,
	"PATH",	284,
	"IGNORE",	285,
	"NOTICE",	286,
	"FAULTS",	287,
	"TRANSFAIL",	288,
	"LOCRESOURCE",	289,
	"USERNOTRESP",	290,
	"REMRESOURCE",	291,
	"UDP_PC_LOSSY",	292,
	"UDP_PC_INHIBIT",	293,
	"SEND",	294,
	"RECEIVE",	295,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"scenario : command_line",
	"scenario : scenario command_line",
	"command_line : INVOKE.request",
	"command_line : INVOKE.indication",
	"command_line : RESULT.request",
	"command_line : RESULT.indication",
	"command_line : RESULT.confirmation",
	"command_line : ERROR.request",
	"command_line : ERROR.confirmation",
	"command_line : ERROR.indication",
	"command_line : FAILURE.indication",
	"command_line : sapbind",
	"command_line : saprelease",
	"command_line : rawevent",
	"command_line : include",
	"command_line : log",
	"command_line : logfile",
	"command_line : quiet",
	"command_line : verbose",
	"command_line : delay",
	"command_line : path",
	"command_line : ignore",
	"command_line : notice",
	"command_line : udp_pc.lossy",
	"command_line : udp_pc.inhibit",
	"sapbind : SAPBIND NUMBER NUMBER",
	"saprelease : SAPRELEASE NUMBER",
	"delay : DELAY NUMBER",
	"path : PATH STRING",
	"ignore : IGNORE FAULTS",
	"notice : NOTICE FAULTS",
	"rawevent : EVENT NUMBER",
	"INVOKE.request : INVOKEREQ NUMBER NUMBER IPADDR operationval encodingtype data",
	"encodingtype : BASIC",
	"encodingtype : PACKED",
	"encodingtype : RESERVED2",
	"encodingtype : RESERVED3",
	"encodingtype : NUMBER",
	"failurevalue : TRANSFAIL",
	"failurevalue : LOCRESOURCE",
	"failurevalue : USERNOTRESP",
	"failurevalue : REMRESOURCE",
	"failurevalue : NUMBER",
	"operationval : SHELL_CMD",
	"operationval : NUMBER",
	"errorvalue : ERROR0",
	"errorvalue : NUMBER",
	"INVOKE.indication : INVOKEIND operationval encodingtype data",
	"ERROR.request : ERRORREQ encodingtype errorvalue data",
	"RESULT.request : RESULTREQ encodingtype data",
	"RESULT.indication : RESULTIND encodingtype data",
	"ERROR.indication : ERRORIND encodingtype errorvalue data",
	"FAILURE.indication : FAILUREIND failurevalue",
	"RESULT.confirmation : RESULTCONF",
	"ERROR.confirmation : ERRORCONF",
	"data : STRING",
	"data : NUMBER STRING",
	"include : INCLUDE STRING",
	"log : LOG STRING",
	"logfile : LOGFILE STRING",
	"quiet : QUIET",
	"verbose : VERBOSE",
	"udp_pc.lossy : UDP_PC_LOSSY direction NUMBER",
	"udp_pc.inhibit : UDP_PC_INHIBIT direction NUMBER",
	"direction : SEND",
	"direction : RECEIVE",
	"direction : NUMBER",
};
#endif /* YYDEBUG */
/*
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 */

#pragma ident	"@(#)yaccpar	6.12	93/06/07 SMI"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yynewmax * sizeof(type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()		yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**	If i<255, i itself is the token.  If i>255 but the neither 
**	of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
	int first = 0;
	int last = YYNMBCHARS - 1;
	int mid;
	wchar_t j;

	if(i&0x60000000){/*Must convert to a token. */
		if( yymbchars[last].character < i ){
			return i;/*Giving up*/
		}
		while ((last>=first)&&(first>=0)) {/*Binary search loop*/
			mid = (first+last)/2;
			j = yymbchars[mid].character;
			if( j==i ){/*Found*/ 
				return yymbchars[mid].tvalue;
			}else if( j<i ){
				first = mid + 1;
			}else{
				last = mid -1;
			}
		}
		/*No entry in the table.*/
		return i;/* Giving up.*/
	}else{/* i is already a token. */
		return i;
	}
}
#else/*!YYNMBCHARS*/
#define YYLEX()		yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
	hacks to please C++ and lint - goto's inside switch should never be
	executed; yypvt is set to 0 to avoid "used before set" warning.
*/
	static int __yaccpar_lint_hack__ = 0;
	switch (__yaccpar_lint_hack__)
	{
		case 1: goto yyerrlab;
		case 2: goto yynewstate;
	}
	yypvt = 0;
#endif

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
			yyerror("yacc initialization error");
			YYABORT;
		}
	}
#endif

	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */
	goto yystack;	/* moved from 6 lines above to here to please C++ */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			int yynewmax;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				char *newyys = (char *)YYNEW(int);
				char *newyyv = (char *)YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
				yyerror( "yacc stack overflow" );
				YYABORT;
			}
			yymaxdepth = yynewmax;

			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
			skip_init:
				yynerrs++;
				/* FALLTHRU */
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 63 "tester.y"
{
		    } break;
case 2:
# line 66 "tester.y"
{
		    } break;
case 26:
# line 97 "tester.y"
{sapbind(yypvt[-1], yypvt[-0]);} break;
case 27:
# line 100 "tester.y"
{saprelease(yypvt[-0]);} break;
case 28:
# line 103 "tester.y"
{ delay(yypvt[-0]); } break;
case 29:
# line 106 "tester.y"
{path(strval); } break;
case 30:
# line 109 "tester.y"
{ ignore_faults = 1; } break;
case 31:
# line 112 "tester.y"
{ ignore_faults = 0; } break;
case 32:
# line 115 "tester.y"
{ rawevent(yypvt[-0]); } break;
case 33:
# line 120 "tester.y"
{
				if (!cflag)
					invokereq(yypvt[-5],yypvt[-4],&parseaddr,yypvt[-2],yypvt[-1],strval);
				} break;
case 34:
# line 126 "tester.y"
{yyval = 0;} break;
case 35:
# line 128 "tester.y"
{yyval = 1;} break;
case 36:
# line 130 "tester.y"
{yyval = 2;} break;
case 37:
# line 132 "tester.y"
{yyval = 3;} break;
case 39:
# line 137 "tester.y"
{yyval = 0;} break;
case 40:
# line 139 "tester.y"
{yyval = 1;} break;
case 41:
# line 141 "tester.y"
{yyval = 2;} break;
case 42:
# line 143 "tester.y"
{yyval = 3;} break;
case 44:
# line 148 "tester.y"
{ yyval = 2; } break;
case 46:
# line 153 "tester.y"
{yyval = 0;} break;
case 48:
# line 161 "tester.y"
{
				if (!cflag)
					invokeind(yypvt[-2],yypvt[-1],strval);
				} break;
case 49:
# line 167 "tester.y"
{
				if (!cflag)
					errorreq(yypvt[-2],yypvt[-1],strval);
				} break;
case 50:
# line 173 "tester.y"
{
				if (!cflag)
					resultreq(yypvt[-1],strval);
				} break;
case 51:
# line 179 "tester.y"
{
				if (!cflag)
					resultind(yypvt[-1],strval);
				} break;
case 52:
# line 185 "tester.y"
{
				if (!cflag)
					errorind(yypvt[-2],yypvt[-1],strval);
				} break;
case 53:
# line 191 "tester.y"
{
				if (!cflag)
					failureind(yypvt[-0]);
				} break;
case 54:
# line 197 "tester.y"
{
				if (!cflag)
					resultconf();
				} break;
case 55:
# line 203 "tester.y"
{
				if (!cflag)
					errorconf();
				} break;
case 57:
# line 210 "tester.y"
{
			int i,n;
			char *p;
			char tmp[4096];
			n = strlen(strval);
			strcpy(tmp,strval);
			for (i = 0, p = strval; i < yypvt[-1]; i++,p += n)
			    {
				strncpy(p,tmp,n);
			    }
			*p = 0;
                        } break;
case 58:
# line 225 "tester.y"
{
			include(strval);
			} break;
case 59:
# line 232 "tester.y"
{
			if (!cflag)
				log(strval);
			} break;
case 60:
# line 239 "tester.y"
{
			if (!cflag)
				do_log(strval);
			} break;
case 61:
# line 246 "tester.y"
{
			if (!cflag)
				verbose = 0;
			} break;
case 62:
# line 253 "tester.y"
{
			if (!cflag)
				verbose = 1;
			} break;
case 63:
# line 264 "tester.y"
{ 
			if (!cflag)
			  udp_pc_lossy(yypvt[-1], yypvt[-0]); 
			} break;
case 64:
# line 274 "tester.y"
{ 
			if (!cflag)
			  udp_pc_inhibit(yypvt[-1], yypvt[-0]); 
			} break;
case 65:
# line 281 "tester.y"
{yyval = 0;} break;
case 66:
# line 283 "tester.y"
{yyval = 1;} break;
	}
	goto yystack;		/* reset registers in driver code */
}

