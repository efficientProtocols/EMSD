/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of ESRO. An implementation of RFC-2188.
 *
 *ESRO is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *ESRO is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with ESRO; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
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
 * File: extfuncs.h
 *
 * Module; Tester.
 *
 * Description: External function definitions.
 * 
-*/

/*
 * Author: Mohsen Banan. Tweaked by Mark Mc
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: extfuncs.h,v 1.2 2002/10/25 19:37:45 mohsen Exp $";
#endif /*}*/

Void sapbind(int sap, int functionalUnit);
Void saprelease(int sap);
int doEsrosQueue();
int qcheck();
int invokereq(int remoteSap, int remotePort, N_SapAddr *remoteAddr, int
              operationVal, int encodingType, Byte *invokeParameter,
	      unsigned long userInvokeRef);
int errorreq(int encodingType, int errorValue, Byte *value, 
	     unsigned long userInvokeRef);
int resultreq(int encodingType, Byte *value,
	      unsigned long userInvokeRef);
int invokeind(int operationValue, int encodingType, char *value);
int failureind(int failureValue, unsigned long userInvokeRef);
int errorconf(unsigned long userInvokeRef);
int resultconf(unsigned long userInvokeRef);
int resultind(int encodingType, char *value, unsigned long userInvokeRef);
int errorind(int encodingType, int errorValue, char *value, 
	     unsigned long userInvokeRef);
Void rawevent(int n);
Void badevent();
Void badOpValue(char *eventName, int actual, int expected);
Void badEncodingType(char *eventName, int actual, int expected);
Void badData(char *eventName,  char *actualData, int actualLen, char *expected);
Void badFailureValue(char *eventName, int actual, int expected);
Void badErrorValue(char *eventName, int actual, int expected);
Void badUserInvokeRef(char *eventName, ESRO_UserInvokeRef actual, 
		      ESRO_UserInvokeRef expected);
Void yyerror(char *s);
int yylex();
Void log();
Void do_log();
Void include(char *s);
int xtoi(register char *p);
Void TSI_parse();
Void delay(int n);
Void path(char *s);

Void G_init(void);
Void G_exit(Int code);
Void G_usage(void);
Void G_sigIntr(Int unUsed);

Void udp_pc_inhibit( Int , Int );
Void udp_pc_lossy( Int , Int );
