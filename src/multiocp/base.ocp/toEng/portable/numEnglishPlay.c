/*+
 * Description:
 *	DO_ Dial Out Module.
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)main.c	1.1    Released: 8/8/87";
#endif /*}*/

/* #includes */
#include  "eh.h"
#include  "estd.h"
#include  "getopt.h"
#include <signal.h>
#include <sys/types.h>
#include <time.h>

#include "tm.h"
#include "pp.h"
#include "vm.h"
#include "pf.h"
#include "spa.h"
#include "queue.h"
#include "seq.h"


/*
 * Deliver the message
 */
SuccFail
SPA_numEnglishPlay( PP_PortDesc portDesc, LgInt num)
{
    Char buf[512];
    SPA_PlayResult playResult;


#if 0
    RANGE CHECKING MAY BE NEEDED

    if (PF_getLgInt(G_env.systemId, &systemId,
		    (LgInt)0, (LgInt)0, (LgInt)999999) != SUCCESS) {
	EH_problem("");
	return (FAIL);
    }
#endif


    if (! PF_intToDigitEnglish(buf, ENDOF(buf), num)) {
	EH_problem("");
	return (FAIL);
    }

    playResult = SPA_wordSeqPlayN(portDesc, buf, 0);
    if (playResult == FAIL) {
	return (FAIL);
    }

    return (SUCCESS);
}

