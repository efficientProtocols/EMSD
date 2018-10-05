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
 * Author: Derrell Lipman
 * History:
 *
 */

/*
 * RCS Revision: $Id: configw.h,v 1.1 1996/08/27 23:02:03 kamran Exp $
 */

#ifndef _CONFIGW_H_	/*{*/
#define _CONFIGW_H_

/* Maximum length of a parameter, including any macro expansion */
#define	CONFIGW_MAX_PARAMETER_LEN	1024

typedef enum
{
    CONFIGW_Permanent,
    CONFIGW_ThisExecution,
    CONFIGW_Transient
} CONFIGW_Permanance;

/*
 * CONFIGW_init()
 */
ReturnCode
CONFIGW_init(void);


/*
 * CONFIGW_openFile()
 *
 * Parameters:
 *
 *         pFileName --
 *             Name of the configuration file to be used.
 *
 *         phConfig --
 *             Pointer to a location in which the returned configuration
 *             handle is to be placed.
 *
 * Returns:
 *         Success
 *	   ResourceError	-- Out of memory
 *	   UnsupportedOption	-- Unsupported/unrecognized configuration data
 *         OS_RC_NoSuchFile	-- Couldn't open the configuration file
 */
ReturnCode
CONFIGW_open(char * pFileName,
	     void ** phConfig);

/*
 * CONFIGW_close()
 *
 * Parameters:
 *
 *         hConfig --
 *             Configuration handle previously returned by CONFIGW_openFile.
 *
 * Returns:
 *         Nothing.
 */
void
CONFIGW_close(void * hConfig);

/*
 * CONFIGW_getNumber()
 *
 * Parameters:
 *
 *         hConfig --
 *             Configuration handle previously returned by CONFIGW_openFile.
 *
 *         pSectionName --
 *             Name of the configuration file section containing the parameter
 *             to be retrieved.  Section names have white space stripped from
 *             them.
 *
 *         pTypeName --
 *             Name of the parameter type to search for, in the specified
 *             section.  Type names have white space stripped from them.
 *
 *         pValue --
 *             Pointer to the location to put the returned Number.
 *
 * Returns:
 *         Success
 *         Fail
 */
ReturnCode
CONFIGW_getNumber(void * hConfig,
		  char * pSectionName,
		  char * pTypeName,
		  OS_Uint32 * pValue);


/*
 * CONFIGW_getString()
 *
 * Parameters:
 *
 *         hConfig --
 *             Configuration handle previously returned by CONFIGW_openFile.
 *
 *         pSectionName --
 *             Name of the configuration file section containing the parameter
 *             to be retrieved.  Section names have white space stripped from
 *             them.
 *
 *         pTypeName --
 *             Name of the parameter type to search for, in the specified
 *             section.  Type names have white space stripped from them.
 *
 *         ppValue --
 *             Pointer to the location to put a pointer to the returned
 *             parameter value.
 *
 * Returns:
 *         Success
 *         Fail
 *
 * Note:
 *         The returned Value pointer points to static data.  Do not modify
 *         the data pointed to by the returned parameter.
 */
ReturnCode
CONFIGW_getString(void * hConfig,
		  char * pSectionName,
		  char * pTypeName,
		  char ** ppValue);



/*
 * CONFIGW_setNumber()
 *
 * Parameters:
 *
 *         hConfig --
 *             Configuration handle previously returned by CONFIGW_openFile.
 *
 *         pSectionName --
 *             Name of the configuration file section containing the parameter
 *             to be retrieved
 *
 *         pTypeName --
 *             Name of the parameter type to search for, in the specified
 *             section.
 *
 *         value --
 *             Numeric value to be assigned to the specified Section/Type.
 *
 *         permanance --
 *             If CONFIGW_Permanent, rewrite the configuration file with this
 *             new value assigned to the specified Section/Type.  If
 *             CONFIGW_ThisExecution, this value is "local" to the current
 *             run-time environment, and returns to the value saved in the
 *             file upon program termination or upon a call to CONFIGW_close().
 *             If CONFIGW_Transient, this value is "local to the current
 *             run-time environment, and returns to the value saved in the
 *             file upon re-reading of the configuration file (not yet
 *             implemented), or upon a call to CONFIGW_close().
 *
 * Returns:
 *         Success
 *         ResourceError
 */
ReturnCode
CONFIGW_setNumber(void * hConfig,
		  char * pSectionName,
		  char * pTypeName,
		  OS_Uint32 value,
		  CONFIGW_Permanance permanance);


/*
 * CONFIGW_setString()
 *
 * Parameters:
 *
 *         hConfig --
 *             Configuration handle previously returned by CONFIGW_openFile.
 *
 *         pSectionName --
 *             Name of the configuration file section containing the parameter
 *             to be retrieved
 *
 *         pTypeName --
 *             Name of the parameter type to search for, in the specified
 *             section.
 *
 *         pValue --
 *             String value to be assigned to the specified Section/Type.
 *
 *         permanance --
 *             If CONFIGW_Permanent, rewrite the configuration file with this
 *             new value assigned to the specified Section/Type.  If
 *             CONFIGW_ThisExecution, this value is "local" to the current
 *             run-time environment, and returns to the value saved in the
 *             file upon program termination or upon a call to CONFIGW_close().
 *             If CONFIGW_Transient, this value is "local to the current
 *             run-time environment, and returns to the value saved in the
 *             file upon re-reading of the configuration file (not yet
 *             implemented), or upon a call to CONFIGW_close().
 *
 * Returns:
 *         Success
 *         Fail
 */
ReturnCode
CONFIGW_setString(void * hConfig,
		  char * pSectionName,
		  char * pTypeName,
		  char * pValue,
		  CONFIGW_Permanance permanance);


/*
 * CONFIGW_nextParameter()
 *
 * Parameters:
 *
 *         hConfig --
 *             Configuration handle previously returned by CONFIGW_openFile.
 *
 *         pSectionName --
 *             Name of the configuration file section containing the
 *             parameters to be retrieved
 *
 *         ppTypeName --
 *             Pointer to the location to place a pointer to the character
 *             string containing the parameter's type name
 *
 *         ppValue --
 *             Pointer to the location to place a poitner to the character
 *             string containing the parameter's value
 *
 *         phParameter --
 *             Handle indicating where to begin the search.  If the location
 *             pointed to by this parameter contains NULL
 *                 (i.e. *phParameter == NULL)
 *             then the first parameter in the specified section will be
 *             returned.  Upon return from this function, *phParameter is
 *             updated with a new value.  If this value of phParameter is
 *             passed to this function again (with the same section name), the
 *             next parameter in the specified section will be returned.
 *
 * Returns:
 *         Success
 *         Fail
 */
ReturnCode
CONFIGW_nextParameter(void * hConfig,
		      char * pSectionName,
		      char ** ppTypeName,
		      char ** ppValue,
		      void ** phParameter);


/*
 * CONFIGW_nextSection()
 *
 * Parameters:
 *
 *         hConfig --
 *             Configuration handle previously returned by CONFIGW_openFile.
 *
 *         ppSectionName --
 *             Pointer to the location to place a pointer to the character
 *             string containing the section name
 *
 *         phSection --
 *             Handle indicating where to begin the search.  If the location
 *             pointed to by this parameter contains NULL
 *                 (i.e. *phSection == NULL)
 *             then the first section in the configuration file will be
 *             returned.  Upon return from this function, *phSection is
 *             updated with a new value.  If this value of phSection is passed
 *             to this function again (with the same section name), the next
 *             section in the configuration file will be returned.
 *
 * Returns:
 *         Success
 *         Fail
 */
ReturnCode
CONFIGW_nextSection(void * hConfig,
		    char ** ppSectionName,
		    void ** phSection);

#endif	/*}*/

