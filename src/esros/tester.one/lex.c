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

#include <stdio.h>
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
#ifndef __cplusplus
# define output(c) (void)putc(c,yyout)
#else
# define lex_output(c) (void)putc(c,yyout)
#endif

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
	int yyback(int *, int);
	int yyinput(void);
	int yylook(void);
	void yyoutput(int);
	int yyracc(int);
	int yyreject(void);
	void yyunput(int);
	int yylex(void);
#ifdef YYLEX_E
	void yywoutput(wchar_t);
	wchar_t yywinput(void);
#endif
#ifndef yyless
	void yyless(int);
#endif
#ifndef yywrap
	int yywrap(void);
#endif
#ifdef LEXDEBUG
	void allprint(char);
	void sprint(char *);
#endif
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void exit(int);
#ifdef __cplusplus
}
#endif

#endif
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
#ifndef __cplusplus
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#else
# define lex_input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#endif
#define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;


# line 4 "lex.l"
/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */


# line 14 "lex.l"
/*+
 * File name: lex.l
 *
 * Description:
 *
-*/


# line 21 "lex.l"
/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: lex.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "tm.h"
#include "addr.h"
#include "esro.h"
#include "extfuncs.h"

#ifdef MSDOS
#include "ytab.h"
#else
#include "y.tab.h"
#endif

extern int line;
extern int yylval;
extern char strval[];
extern N_SapAddr parseaddr;

# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
#ifdef __cplusplus
/* to avoid CC and lint complaining yyfussy not being used ...*/
static int __lex_hack = 0;
if (__lex_hack) goto yyfussy;
#endif
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:

# line 50 "lex.l"
      return(RESERVED2);
break;
case 2:

# line 51 "lex.l"
      return(RESERVED3);
break;
case 3:

# line 52 "lex.l"
   	return(BASIC);
break;
case 4:

# line 53 "lex.l"
         return(PACKED);
break;
case 5:

# line 54 "lex.l"
      return(TRANSFAIL);
break;
case 6:

# line 55 "lex.l"
return(LOCRESOURCE);
break;
case 7:

# line 56 "lex.l"
return(USERNOTRESP);
break;
case 8:

# line 57 "lex.l"
return(REMRESOURCE);
break;
case 9:

# line 58 "lex.l"
	return(EVENT);
break;
case 10:

# line 59 "lex.l"
	return(FAULTS);
break;
case 11:

# line 60 "lex.l"
	return(IGNOREIT);
break;
case 12:

# line 61 "lex.l"
	return(NOTICE);
break;
case 13:

# line 62 "lex.l"
	return(DELAY);
break;
case 14:

# line 63 "lex.l"
	return(PATH);
break;
case 15:

# line 64 "lex.l"
      return(SHELL_CMD);
break;
case 16:

# line 65 "lex.l"
	return(LOG);
break;
case 17:

# line 66 "lex.l"
	return(LOGFILE);
break;
case 18:

# line 67 "lex.l"
	return(QUIET);
break;
case 19:

# line 68 "lex.l"
	return(VERBOSE);
break;
case 20:

# line 69 "lex.l"
	return(INCLUDE);
break;
case 21:

# line 70 "lex.l"
		return(ERRORIND);
break;
case 22:

# line 71 "lex.l"
		return(ERRORREQ);
break;
case 23:

# line 72 "lex.l"
		return(ERRORCONF);
break;
case 24:

# line 73 "lex.l"
                return(SAPBIND);
break;
case 25:

# line 74 "lex.l"
             return(SAPRELEASE);
break;
case 26:

# line 75 "lex.l"
return(INVOKEREQ);
break;
case 27:

# line 76 "lex.l"
return(INVOKEIND);
break;
case 28:

# line 77 "lex.l"
return(RESULTREQ);
break;
case 29:

# line 78 "lex.l"
return(RESULTIND);
break;
case 30:

# line 79 "lex.l"
return(RESULTCONF);
break;
case 31:

# line 80 "lex.l"
return(FAILUREIND);
break;
case 32:

# line 81 "lex.l"
return(UDP_PC_INHIBIT);
break;
case 33:

# line 82 "lex.l"
return(UDP_PC_LOSSY);
break;
case 34:

# line 83 "lex.l"
return(SEND);
break;
case 35:

# line 84 "lex.l"
return(RECEIVE);
break;
case 36:

# line 86 "lex.l"
		{
			strncpy(strval, yytext+1, yyleng-2);
			strval[yyleng-2] = '\0';
			return(STRING);
			}
break;
case 37:

# line 91 "lex.l"
            {
                        parseaddr.len = 4;
                        parseaddr.addr[0] = atoi(strtok(yytext,"."));
                        parseaddr.addr[1] = atoi(strtok(NULL,"."));
                        parseaddr.addr[2] = atoi(strtok(NULL,"."));
                        parseaddr.addr[3] = atoi(strtok(NULL,"."));
                        return(IPADDR);
                       }
break;
case 38:

# line 99 "lex.l"
	{
			yylval = xtoi(&yytext[2]);
			return(NUMBER);
			}
break;
case 39:

# line 103 "lex.l"
		{
			yylval = atoi(yytext);
			return(NUMBER);
			}
break;
case 40:

# line 107 "lex.l"
		line++;
break;
case 41:

# line 108 "lex.l"
		;
break;
case -1:
break;
default:
(void)fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] = {
0,

41,
0,

40,
0,

39,
0,

39,
0,

36,
0,

38,
0,

16,
0,

14,
0,

34,
0,

3,
0,

13,
0,

9,
0,

18,
0,

10,
0,

11,
0,

12,
0,

4,
0,

37,
0,

20,
0,

17,
0,

35,
0,

24,
0,

19,
0,

1,
0,

2,
0,

15,
0,

5,
0,

25,
0,

6,
0,

8,
0,

33,
0,

7,
0,

22,
0,

26,
0,

28,
0,

32,
0,

21,
0,

27,
0,

29,
0,

23,
0,

31,
0,

30,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,3,	1,4,	
5,5,	22,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
5,5,	5,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,3,	0,0,	1,5,	
0,0,	22,22,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	5,22,	0,0,	44,71,	
99,125,	1,6,	1,7,	1,7,	
1,7,	1,7,	1,7,	1,7,	
1,7,	1,7,	1,7,	5,5,	
6,23,	0,0,	6,7,	6,7,	
6,7,	6,7,	6,7,	6,7,	
6,7,	6,7,	6,7,	6,7,	
0,0,	0,0,	0,0,	7,23,	
5,5,	7,7,	7,7,	7,7,	
7,7,	7,7,	7,7,	7,7,	
7,7,	7,7,	7,7,	23,44,	
23,44,	23,44,	23,44,	23,44,	
23,44,	23,44,	23,44,	23,44,	
23,44,	68,96,	8,25,	1,8,	
11,29,	1,9,	1,10,	1,11,	
9,26,	12,30,	1,12,	15,34,	
17,36,	1,13,	21,43,	1,14,	
12,31,	1,15,	1,16,	1,17,	
1,18,	1,19,	1,20,	1,21,	
10,27,	13,32,	14,33,	16,35,	
10,28,	18,37,	19,40,	20,41,	
25,46,	18,38,	26,47,	27,48,	
18,39,	28,49,	6,24,	30,52,	
31,53,	33,57,	35,60,	34,58,	
37,64,	29,50,	20,42,	24,45,	
24,45,	24,45,	24,45,	24,45,	
24,45,	24,45,	24,45,	24,45,	
24,45,	29,51,	32,55,	31,54,	
34,59,	36,61,	32,56,	38,65,	
24,45,	24,45,	24,45,	24,45,	
24,45,	24,45,	39,66,	36,62,	
40,67,	41,68,	42,69,	43,70,	
46,72,	36,63,	47,73,	48,74,	
49,75,	50,76,	51,77,	52,78,	
53,79,	54,80,	55,81,	56,82,	
57,83,	58,84,	59,85,	60,86,	
61,87,	62,88,	63,89,	64,91,	
24,45,	24,45,	24,45,	24,45,	
24,45,	24,45,	65,93,	66,94,	
67,95,	69,97,	70,98,	72,100,	
73,101,	74,102,	63,90,	64,92,	
71,99,	71,99,	71,99,	71,99,	
71,99,	71,99,	71,99,	71,99,	
71,99,	71,99,	75,103,	76,104,	
77,105,	78,106,	79,107,	80,108,	
81,109,	82,110,	83,111,	84,112,	
86,113,	87,114,	88,115,	89,116,	
90,117,	91,118,	92,119,	94,120,	
95,121,	96,122,	97,123,	98,124,	
102,102,	104,129,	105,130,	106,131,	
107,132,	108,133,	109,134,	110,135,	
111,136,	112,137,	114,138,	115,139,	
116,140,	117,141,	118,142,	119,143,	
120,144,	121,145,	122,146,	123,147,	
124,148,	126,150,	127,151,	102,102,	
125,149,	125,149,	125,149,	125,149,	
125,149,	125,149,	125,149,	125,149,	
125,149,	125,149,	128,152,	129,153,	
132,154,	133,133,	134,157,	135,158,	
138,159,	139,160,	140,161,	142,165,	
141,141,	143,166,	144,167,	145,168,	
146,146,	147,171,	148,172,	150,173,	
151,174,	152,175,	153,153,	155,177,	
156,178,	157,179,	160,180,	161,181,	
133,133,	162,182,	163,183,	164,184,	
166,185,	167,186,	168,187,	141,141,	
169,188,	170,189,	171,190,	146,146,	
173,191,	174,192,	175,193,	176,194,	
177,195,	153,153,	178,196,	179,197,	
180,198,	181,199,	181,200,	182,201,	
183,202,	184,203,	185,204,	186,205,	
187,206,	188,207,	102,126,	189,208,	
190,209,	191,210,	192,211,	193,212,	
102,127,	194,213,	195,214,	196,215,	
197,216,	198,217,	201,218,	202,219,	
203,220,	102,128,	204,221,	207,222,	
208,223,	209,224,	210,225,	211,226,	
212,227,	213,228,	214,229,	215,230,	
216,231,	217,232,	218,233,	219,234,	
220,235,	222,236,	223,237,	224,238,	
225,239,	226,240,	227,241,	228,242,	
229,243,	230,244,	233,245,	234,246,	
235,247,	133,155,	141,162,	236,248,	
239,249,	240,250,	242,251,	243,252,	
141,163,	244,253,	133,156,	245,254,	
146,169,	246,255,	247,256,	146,170,	
248,257,	141,164,	153,176,	249,258,	
250,259,	251,260,	252,261,	254,262,	
255,263,	258,264,	259,265,	260,266,	
261,267,	262,268,	263,269,	264,270,	
266,271,	267,272,	268,273,	269,274,	
270,275,	271,276,	273,277,	277,278,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+1,	0,		0,	
yycrank+0,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+-11,	0,		0,	
yycrank+14,	0,		yyvstop+5,
yycrank+29,	0,		yyvstop+7,
yycrank+1,	0,		0,	
yycrank+3,	0,		0,	
yycrank+6,	0,		0,	
yycrank+3,	0,		0,	
yycrank+2,	0,		0,	
yycrank+10,	0,		0,	
yycrank+11,	0,		0,	
yycrank+10,	0,		0,	
yycrank+6,	0,		0,	
yycrank+7,	0,		0,	
yycrank+28,	0,		0,	
yycrank+12,	0,		0,	
yycrank+27,	0,		0,	
yycrank+9,	0,		0,	
yycrank+-3,	yysvec+5,	yyvstop+9,
yycrank+39,	0,		0,	
yycrank+95,	0,		0,	
yycrank+13,	0,		0,	
yycrank+22,	0,		0,	
yycrank+17,	0,		0,	
yycrank+32,	0,		0,	
yycrank+36,	0,		0,	
yycrank+25,	0,		0,	
yycrank+37,	0,		0,	
yycrank+55,	0,		0,	
yycrank+21,	0,		0,	
yycrank+40,	0,		0,	
yycrank+33,	0,		0,	
yycrank+58,	0,		0,	
yycrank+28,	0,		0,	
yycrank+49,	0,		0,	
yycrank+65,	0,		0,	
yycrank+71,	0,		0,	
yycrank+57,	0,		0,	
yycrank+69,	0,		0,	
yycrank+57,	0,		0,	
yycrank+1,	yysvec+23,	0,	
yycrank+0,	yysvec+24,	yyvstop+11,
yycrank+67,	0,		0,	
yycrank+77,	0,		0,	
yycrank+64,	0,		0,	
yycrank+66,	0,		0,	
yycrank+69,	0,		0,	
yycrank+70,	0,		0,	
yycrank+68,	0,		0,	
yycrank+72,	0,		0,	
yycrank+70,	0,		0,	
yycrank+68,	0,		0,	
yycrank+81,	0,		yyvstop+13,
yycrank+79,	0,		0,	
yycrank+78,	0,		0,	
yycrank+82,	0,		0,	
yycrank+86,	0,		0,	
yycrank+87,	0,		0,	
yycrank+75,	0,		0,	
yycrank+89,	0,		0,	
yycrank+93,	0,		0,	
yycrank+98,	0,		0,	
yycrank+91,	0,		0,	
yycrank+90,	0,		0,	
yycrank+2,	0,		0,	
yycrank+87,	0,		0,	
yycrank+104,	0,		0,	
yycrank+160,	0,		0,	
yycrank+104,	0,		0,	
yycrank+83,	0,		0,	
yycrank+91,	0,		0,	
yycrank+102,	0,		0,	
yycrank+102,	0,		0,	
yycrank+104,	0,		0,	
yycrank+107,	0,		0,	
yycrank+105,	0,		0,	
yycrank+116,	0,		0,	
yycrank+123,	0,		0,	
yycrank+120,	0,		0,	
yycrank+127,	0,		0,	
yycrank+126,	0,		0,	
yycrank+0,	0,		yyvstop+15,
yycrank+112,	0,		0,	
yycrank+124,	0,		0,	
yycrank+129,	0,		0,	
yycrank+117,	0,		0,	
yycrank+124,	0,		0,	
yycrank+128,	0,		0,	
yycrank+133,	0,		0,	
yycrank+0,	0,		yyvstop+17,
yycrank+127,	0,		0,	
yycrank+121,	0,		0,	
yycrank+125,	0,		0,	
yycrank+128,	0,		0,	
yycrank+128,	0,		0,	
yycrank+2,	yysvec+71,	0,	
yycrank+0,	0,		yyvstop+19,
yycrank+0,	0,		yyvstop+21,
yycrank+231,	0,		0,	
yycrank+0,	0,		yyvstop+23,
yycrank+127,	0,		0,	
yycrank+127,	0,		0,	
yycrank+142,	0,		0,	
yycrank+144,	0,		0,	
yycrank+144,	0,		0,	
yycrank+131,	0,		0,	
yycrank+139,	0,		0,	
yycrank+147,	0,		0,	
yycrank+149,	0,		0,	
yycrank+0,	0,		yyvstop+25,
yycrank+132,	0,		0,	
yycrank+136,	0,		0,	
yycrank+134,	0,		0,	
yycrank+137,	0,		0,	
yycrank+144,	0,		0,	
yycrank+147,	0,		0,	
yycrank+161,	0,		0,	
yycrank+155,	0,		0,	
yycrank+159,	0,		0,	
yycrank+148,	0,		0,	
yycrank+145,	0,		0,	
yycrank+216,	0,		0,	
yycrank+150,	0,		0,	
yycrank+152,	0,		0,	
yycrank+173,	0,		0,	
yycrank+174,	0,		0,	
yycrank+0,	0,		yyvstop+27,
yycrank+0,	0,		yyvstop+29,
yycrank+175,	0,		0,	
yycrank+268,	0,		0,	
yycrank+167,	0,		0,	
yycrank+178,	0,		0,	
yycrank+0,	0,		yyvstop+31,
yycrank+0,	0,		yyvstop+33,
yycrank+179,	0,		0,	
yycrank+170,	0,		0,	
yycrank+181,	0,		0,	
yycrank+275,	0,		0,	
yycrank+183,	0,		0,	
yycrank+184,	0,		0,	
yycrank+187,	0,		0,	
yycrank+190,	0,		0,	
yycrank+279,	0,		0,	
yycrank+173,	0,		0,	
yycrank+189,	0,		0,	
yycrank+0,	yysvec+125,	yyvstop+35,
yycrank+181,	0,		0,	
yycrank+192,	0,		0,	
yycrank+180,	0,		0,	
yycrank+285,	0,		0,	
yycrank+0,	0,		yyvstop+37,
yycrank+185,	0,		0,	
yycrank+195,	0,		0,	
yycrank+180,	0,		0,	
yycrank+0,	0,		yyvstop+39,
yycrank+0,	0,		yyvstop+41,
yycrank+181,	0,		0,	
yycrank+199,	0,		0,	
yycrank+190,	0,		0,	
yycrank+192,	0,		0,	
yycrank+202,	0,		0,	
yycrank+0,	0,		yyvstop+43,
yycrank+207,	0,		0,	
yycrank+196,	0,		0,	
yycrank+201,	0,		0,	
yycrank+198,	0,		0,	
yycrank+198,	0,		0,	
yycrank+196,	0,		0,	
yycrank+0,	0,		yyvstop+45,
yycrank+210,	0,		0,	
yycrank+208,	0,		0,	
yycrank+197,	0,		0,	
yycrank+205,	0,		0,	
yycrank+216,	0,		0,	
yycrank+205,	0,		0,	
yycrank+205,	0,		0,	
yycrank+206,	0,		0,	
yycrank+271,	0,		0,	
yycrank+213,	0,		0,	
yycrank+224,	0,		0,	
yycrank+212,	0,		0,	
yycrank+211,	0,		0,	
yycrank+227,	0,		0,	
yycrank+220,	0,		0,	
yycrank+225,	0,		0,	
yycrank+216,	0,		0,	
yycrank+231,	0,		0,	
yycrank+228,	0,		0,	
yycrank+235,	0,		0,	
yycrank+234,	0,		0,	
yycrank+237,	0,		0,	
yycrank+233,	0,		0,	
yycrank+222,	0,		0,	
yycrank+241,	0,		0,	
yycrank+242,	0,		0,	
yycrank+0,	0,		yyvstop+47,
yycrank+0,	0,		yyvstop+49,
yycrank+240,	0,		0,	
yycrank+238,	0,		0,	
yycrank+227,	0,		0,	
yycrank+245,	0,		0,	
yycrank+0,	0,		yyvstop+51,
yycrank+0,	0,		yyvstop+53,
yycrank+242,	0,		0,	
yycrank+233,	0,		0,	
yycrank+234,	0,		0,	
yycrank+236,	0,		0,	
yycrank+254,	0,		0,	
yycrank+237,	0,		0,	
yycrank+248,	0,		0,	
yycrank+255,	0,		0,	
yycrank+254,	0,		0,	
yycrank+255,	0,		0,	
yycrank+256,	0,		0,	
yycrank+253,	0,		0,	
yycrank+260,	0,		0,	
yycrank+259,	0,		0,	
yycrank+0,	0,		yyvstop+55,
yycrank+263,	0,		0,	
yycrank+241,	0,		0,	
yycrank+251,	0,		0,	
yycrank+255,	0,		0,	
yycrank+249,	0,		0,	
yycrank+250,	0,		0,	
yycrank+268,	0,		0,	
yycrank+271,	0,		0,	
yycrank+254,	0,		0,	
yycrank+0,	0,		yyvstop+57,
yycrank+0,	0,		yyvstop+59,
yycrank+256,	0,		0,	
yycrank+274,	0,		0,	
yycrank+257,	0,		0,	
yycrank+270,	0,		0,	
yycrank+0,	0,		yyvstop+61,
yycrank+0,	0,		yyvstop+63,
yycrank+279,	0,		0,	
yycrank+272,	0,		0,	
yycrank+0,	0,		yyvstop+65,
yycrank+281,	0,		0,	
yycrank+263,	0,		0,	
yycrank+265,	0,		0,	
yycrank+274,	0,		0,	
yycrank+269,	0,		0,	
yycrank+270,	0,		0,	
yycrank+272,	0,		0,	
yycrank+275,	0,		0,	
yycrank+281,	0,		0,	
yycrank+277,	0,		0,	
yycrank+289,	0,		0,	
yycrank+0,	0,		yyvstop+67,
yycrank+298,	0,		0,	
yycrank+291,	0,		0,	
yycrank+0,	0,		yyvstop+69,
yycrank+0,	0,		yyvstop+71,
yycrank+292,	0,		0,	
yycrank+288,	0,		0,	
yycrank+294,	0,		0,	
yycrank+289,	0,		0,	
yycrank+285,	0,		0,	
yycrank+291,	0,		0,	
yycrank+292,	0,		0,	
yycrank+0,	0,		yyvstop+73,
yycrank+293,	0,		0,	
yycrank+295,	0,		0,	
yycrank+301,	0,		0,	
yycrank+297,	0,		0,	
yycrank+298,	0,		0,	
yycrank+299,	0,		0,	
yycrank+0,	0,		yyvstop+75,
yycrank+299,	0,		0,	
yycrank+0,	0,		yyvstop+77,
yycrank+0,	0,		yyvstop+79,
yycrank+0,	0,		yyvstop+81,
yycrank+301,	0,		0,	
yycrank+0,	0,		yyvstop+83,
0,	0,	0};
struct yywork *yytop = yycrank+411;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
  0,   1,   1,   1,   1,   1,   1,   1, 
  1,   9,  10,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  9,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
 48,  48,  48,  48,  48,  48,  48,  48, 
 48,  48,   1,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  65,  65,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  65,  65,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	Copyright (c) 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
#if defined(__cplusplus) || defined(__STDC__)
int yylook(void)
#else
yylook()
#endif
{
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
#ifndef __cplusplus
			*yylastch++ = yych = input();
#else
			*yylastch++ = yych = lex_input();
#endif
			if(yylastch > &yytext[YYLMAX]) {
				fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
				exit(1);
			}
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
#ifndef __cplusplus
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
#else
		yyprevious = yytext[0] = lex_input();
		if (yyprevious>0)
			lex_output(yyprevious);
#endif
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
#if defined(__cplusplus) || defined(__STDC__)
int yyback(int *p, int m)
#else
yyback(p, m)
	int *p;
#endif
{
	if (p==0) return(0);
	while (*p) {
		if (*p++ == m)
			return(1);
	}
	return(0);
}
	/* the following are only used in the lex library */
#if defined(__cplusplus) || defined(__STDC__)
int yyinput(void)
#else
yyinput()
#endif
{
#ifndef __cplusplus
	return(input());
#else
	return(lex_input());
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyoutput(int c)
#else
yyoutput(c)
  int c; 
#endif
{
#ifndef __cplusplus
	output(c);
#else
	lex_output(c);
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyunput(int c)
#else
yyunput(c)
   int c; 
#endif
{
	unput(c);
	}
