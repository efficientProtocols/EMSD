/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of Neda-EMSD. An implementation of RFC-2524.
 *
 *Neda-EMSD is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with Neda-EMSD; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
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

/*
 * 
 * History:
 *
 */

#ifndef __RC_H__
#define	__RC_H__

/*
 * Generic Return Codes applicable to all modules.  Each module will define
 * its own return codes to supplement these.
 */
typedef enum
{
    Success		= 0,
    Fail		= 1,
    ProgrammerError	= 2,
    UnsupportedOption	= 3,
    ResourceError	= 4,

    /* For backwards compatibility */
    OldFail		= -1
} ReturnCode;


#ifndef NO_SUCC_FAIL_RESULTS

/*
 * These macros make it easier to write code that returns error
 * codes and diagnostic messages, without cluttering the code with
 * it.
 */

/* Declare _returnCodeDebug someplace if you define this */
extern ReturnCode _returnCodeDebug(char * pFormat, ...);

/* You can use this declaration if you like... */

# define	RC_DEBUG_ENABLE					\
	char RC_buf[2048];					\
	void * RC_hTM = NULL;					\
								\
	int							\
	RC_xprintf(char * pFormat, ...)				\
	{							\
	    va_list	    pArgs;				\
								\
	    va_start(pArgs, pFormat);				\
	    vsprintf(RC_buf + strlen(RC_buf), pFormat, pArgs);	\
	    return 0;						\
	}

extern int RC_xprintf(char * pFormat, ...);
extern char RC_buf[];
extern void * RC_hTM;

# define	_returnCodeDebug(debugParamList, rc)	\
	(RC_xprintf("%s: ", __applicationName),		\
	 RC_xprintf debugParamList,			\
	 RC_xprintf(": ReturnCode=0x%04x\n", rc),	\
	 tm_loc = tm_here(__FILE__, __LINE__),		\
	 tm_trace(RC_hTM, TM_ENTER, RC_buf),		\
	 rc)

# define	FAIL_RC(rc, dbgParamList)		\
	(_returnCodeDebug(dbgParamList, rc))

# define	RETURN_IF_FAIL(val, dbgParamList)	\
{							\
    ReturnCode		 rc;				\
    if ((rc = (val)) != Success)			\
    {							\
	return _returnCodeDebug(dbgParamList, rc);	\
    }							\
}

#else	/* not SUCC_FAIL_DEBUG */

# define	RC_DEBUG_ENABLE						\

# define	FAIL_RC(rc, dbgParamList)	(rc)

# define	RETURN_IF_FAIL(val, dbgParamList)	\
{							\
    ReturnCode		 rc;				\
    if ((rc = (val)) != Success)			\
	return rc;					\
}

#endif	/* SUCC_FAIL_DEBUG */


#define	RC_CALL(rc, funcCall, debugParamList)	\
{						\
    if ((rc = (funcCall)) != Success)		\
    {						\
	return FAIL_RC(rc, debugParamList);	\
    }						\
}


#endif /* __RC_H__ */
