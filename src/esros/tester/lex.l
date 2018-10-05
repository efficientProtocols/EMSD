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
 * File name: lex.l
 *
 * Description:
 *
-*/

/*
 * 
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: lex.l,v 1.2 2002/10/25 19:37:45 mohsen Exp $";
#endif /*}*/

#include <stdlib.h>
#include <string.h>

#include "y.tab.h"
#include "estd.h"
#include "tm.h"
#include "addr.h"
#include "esro.h"
#include "extfuncs.h"

extern int line;
extern int yylval;
extern char strval[];
extern N_SapAddr parseaddr;

%}

%%
reserved2       return(RESERVED2);
reserved3       return(RESERVED3);
basic    	return(BASIC);
packed          return(PACKED);
transfail       return(TRANSFAIL);
locresource	return(LOCRESOURCE);
usernotresp	return(USERNOTRESP);
remresource	return(REMRESOURCE); 
event		return(EVENT);
faults		return(FAULTS);
ignore		return(IGNORE);
notice		return(NOTICE);
delay		return(DELAY);
path		return(PATH);
shell_cmd       return(SHELL_CMD);
log		return(LOG);
logfile		return(LOGFILE);
quiet		return(QUIET);
verbose		return(VERBOSE);
include		return(INCLUDE);
error[ \t]*indication			return(ERRORIND);
error[ \t]*request			return(ERRORREQ);
error[ \t]*confirmation			return(ERRORCONF);
sapbind                 return(SAPBIND);
saprelease              return(SAPRELEASE);
invoke[ \t]*request	return(INVOKEREQ);
invoke[ \t]*indication	return(INVOKEIND);
result[ \t]*request	return(RESULTREQ);
result[ \t]*indication	return(RESULTIND);
result[ \t]*confirmation return(RESULTCONF);
failure[ \t]*indication	return(FAILUREIND);
udp_pc[ \t]*inhibit	return(UDP_PC_INHIBIT);
udp_pc[ \t]*lossy	return(UDP_PC_LOSSY);
send	return(SEND);
receive	return(RECEIVE);

\".*\"			{
			strncpy(strval, yytext+1, yyleng-2);
			strval[yyleng-2] = '\0';
			return(STRING);
			}
[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+             {
                        parseaddr.len = 4;
                        parseaddr.addr[0] = atoi(strtok(yytext,"."));
                        parseaddr.addr[1] = atoi(strtok(NULL,"."));
                        parseaddr.addr[2] = atoi(strtok(NULL,"."));
                        parseaddr.addr[3] = atoi(strtok(NULL,"."));
                        return(IPADDR);
                       }
0x[0-9a-fA-F]+		{
			yylval = xtoi(&yytext[2]);
			return(NUMBER);
			}
[0-9]+			{
			yylval = atoi(yytext);
			return(NUMBER);
			}
[\n]			line++;
[ \t]			;
