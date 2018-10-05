%{

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
 * 
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: tester.y,v 1.2 2002/10/25 19:37:45 mohsen Exp $";
#endif /*}*/

#include <stdlib.h>
#include <string.h>

#include "estd.h"
#include "du.h"
#include "target.h"
#include "esrossi.h"
#include "addr.h"
#include "esro.h"
#include "extfuncs.h"

extern char strval[];
extern int cflag, verbose;
extern N_SapAddr parseaddr;
extern int ignore_faults;

%}

%token LOG LOGFILE QUIET VERBOSE INCLUDE  ERRORIND ERRORREQ ERRORCONF 
%token INVOKEREQ INVOKEIND RESULTREQ RESULTIND RESULTCONF FAILUREIND
%token STRING NUMBER
%token SAPBIND SAPRELEASE IPADDR SHELL_CMD EVENT ERROR0
%token BASIC PACKED RESERVED2 RESERVED3 
%token DELAY PATH IGNORE NOTICE FAULTS
%token TRANSFAIL LOCRESOURCE USERNOTRESP REMRESOURCE
%token UDP_PC_LOSSY UDP_PC_INHIBIT
%token SEND RECEIVE


%start scenario

%%

scenario	: command_line
		    {
		    }
		| scenario command_line
		    {
		    }
		;

command_line	: INVOKE.request
		| INVOKE.indication
		| RESULT.request
		| RESULT.indication
		| RESULT.confirmation
		| ERROR.request
		| ERROR.confirmation
		| ERROR.indication
		| FAILURE.indication
                | sapbind
		| saprelease
		| rawevent
		| include
		| log
		| logfile
		| quiet
		| verbose
		| delay
		| path
		| ignore
		| notice
                | udp_pc.lossy
                | udp_pc.inhibit
		;


sapbind         : SAPBIND NUMBER NUMBER
                        {sapbind($2, $3);}

saprelease      : SAPRELEASE NUMBER
                        {saprelease($2);}

delay	:	DELAY NUMBER
			{ delay($2); }

path	: 	PATH STRING
				{path(strval); }

ignore	:	IGNORE FAULTS
				{ ignore_faults = 1; }

notice : 	NOTICE FAULTS
				{ ignore_faults = 0; }

rawevent	:	EVENT NUMBER
				{ rawevent($2); }

/* invoke remotesap remoteport remoteIP encodingtype parameter userinvokeref */

INVOKE.request	: INVOKEREQ NUMBER NUMBER IPADDR operationval encodingtype data NUMBER
				{
				if (!cflag)
					invokereq($2,$3,&parseaddr,$5,$6,strval,$8);
				}

encodingtype    : BASIC 
                      {$$ = 0;} 
                | PACKED 
                      {$$ = 1;} 
                | RESERVED2 
                      {$$ = 2;}
                | RESERVED3 
                      {$$ = 3;}
				| NUMBER
				;

failurevalue    : TRANSFAIL 
                      {$$ = 0;} 
                | LOCRESOURCE 
                      {$$ = 1;} 
                | USERNOTRESP 
                      {$$ = 2;}
                | REMRESOURCE 
                      {$$ = 3;}
				| NUMBER
				;

operationval	:	SHELL_CMD
				{ $$ = 2; }
				| NUMBER
		;

errorvalue      : ERROR0 
                      {$$ = 0;} 
				| NUMBER
				;


/* invokeindication opval encodingtype parameter */

INVOKE.indication	: INVOKEIND operationval encodingtype data 
				{
				if (!cflag)
					invokeind($2,$3,strval);
				}

ERROR.request	:	ERRORREQ encodingtype errorvalue data NUMBER
				{
				if (!cflag)
					errorreq($2,$3,strval,$5);
				}

RESULT.request	: RESULTREQ encodingtype data NUMBER
				{
				if (!cflag)
					resultreq($2,strval,$4);
				}

RESULT.indication	: RESULTIND encodingtype data NUMBER
				{
				if (!cflag)
					resultind($2,strval,$4);
				}

ERROR.indication	: ERRORIND encodingtype errorvalue data NUMBER
				{
				if (!cflag)
					errorind($2,$3,strval,$5);
				}

FAILURE.indication	: FAILUREIND failurevalue NUMBER
				{
				if (!cflag)
					failureind($2,$3);
				}

RESULT.confirmation	:	RESULTCONF NUMBER
				{
				if (!cflag)
					resultconf($2);
				}

ERROR.confirmation	:	ERRORCONF NUMBER
				{
				if (!cflag)
					errorconf($2);
				}

data           :    STRING
               |    NUMBER STRING
                        {
			int i,n;
			char *p;
			char tmp[4096];
			n = strlen(strval);
			strcpy(tmp,strval);
			for (i = 0, p = strval; i < $1; i++,p += n)
			    {
				strncpy(p,tmp,n);
			    }
			*p = 0;
                        }   


include		: INCLUDE STRING
			{
			include(strval);
			}
		;


log		: LOG STRING
			{
			if (!cflag)
				log(strval);
			}
		;

logfile		: LOGFILE STRING
			{
			if (!cflag)
				do_log(strval);
			}
		;

quiet		: QUIET
			{
			if (!cflag)
				verbose = 0;
			}
		;

verbose		: VERBOSE
			{
			if (!cflag)
				verbose = 1;
			}
		;

/* udp_pc lossy [send | receive] percent
   ex. - udp_pc lossy receive 12
*/

udp_pc.lossy  : UDP_PC_LOSSY direction NUMBER 
			{ 
			if (!cflag)
			  udp_pc_lossy($2, $3); 
			}
 
/* udp_pc inhibit [send | receive] next 
   ex. - udp_pc inhibit send 2
*/

udp_pc.inhibit  : UDP_PC_INHIBIT direction NUMBER 
			{ 
			if (!cflag)
			  udp_pc_inhibit($2, $3); 
			} 

/* direction must agree with udp_pc.h */
direction    : SEND 
                      {$$ = 0;} 
             | RECEIVE 
                      {$$ = 1;}
	     | NUMBER
	     ;
