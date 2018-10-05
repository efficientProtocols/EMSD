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
 * Configuration File Maintenance
 *
 * This module provides a set of functions for reading and writing
 * configuration data.  The data can be maintained in a file, or can be set on
 * a temporary basis for use during an iteration of the program, or until the
 * data is re-read from the file.
 *
 *
 *
 * NOT YET IMPLEMENTED:
 *
 * This module does not yet write to the configuration file.  The file should
 * be created by hand, for now.  For this reason, the PERMANANCE values have
 * no meaning at this time.
 */

#include "estd.h"
#include "tm.h"
#include "eh.h"
#include "queue.h"
#include "config.h"

static void * hTM;

typedef struct
{
    char *	    pFileName;
    QU_Head	    sections;
    QU_Head 	    macros;
} ConfigFile;

typedef struct
{
    QU_ELEMENT;

    char *	    pSectionName;
    QU_Head	    parameters;
} Section;

typedef struct
{
    QU_ELEMENT;

    char * 		pTypeName;
    char *		pDefinition;
    char * 		pValue;
    CONFIGW_Permanance	permanance;
} Parameter;

typedef struct
{
    QU_ELEMENT;

    char *	        pMacroName;
    char *		pDefinition;
    char *		pExpansion;
} Macro;

static ReturnCode
processFile(char * pFileName,
	    ConfigFile * pConfig,
	    Section * pSection);

static ReturnCode
newSection(void * hConfig,
	   char * pSectionName,
	   Section ** ppSection);

static ReturnCode
newParameter(ConfigFile * pConfig,
	     Section * pSection,
	     char * pTypeName,
	     char * pDefinition,
	     char * pSaveDefinition,
	     CONFIGW_Permanance permanance);

static ReturnCode
newMacro(ConfigFile * pConfig,
	 char * pMacroName,
	 char * pExpansion);

static ReturnCode
expand(ConfigFile * pConfig,
       char * pInput,
       char * pOutput,
       char * pOutputEnd);

static ReturnCode
findCreateSetParameter(void * hConfig,
		       char * pSectionName,
		       char * pTypeName,
		       char * pDefinition,
		       CONFIGW_Permanance permanance);

static ReturnCode
findParameter(void * hConfig,
	     char * pSectionName,
	     char * pTypeName,
	     Section ** ppSection,
	     Parameter ** ppParameter);

void
stripWhiteSpace(char ** ppText);

static ReturnCode
saveConfig(ConfigFile * pConfig);



/*<
 * Function:    CONFIGW_init()
 *
 * Description: 
 *
 * Arguments:   None.
 *
 * Returns:     
 *
>*/

ReturnCode
CONFIGW_init(void)
{
    if (TM_OPEN(hTM, "CFG") == NULL)
    {
	return Fail;
    }

    return Success;
}


/*
 * Function:	CONFIGW_open()
 *
 * Description: Config write open.
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
	     void ** phConfig)
{
    ReturnCode	    rc = Success;
    ConfigFile *    pConfig;

    /* Allocate a ConfigFile structure for this file */
    if ((pConfig =
	 OS_alloc(sizeof(ConfigFile) + strlen(pFileName) + 1)) == NULL)
    {
	return ResourceError;
    }

    /* Point to the memory allocated for the file name */
    pConfig->pFileName = (char *) (pConfig + 1);

    /* Copy the file name */
    strcpy(pConfig->pFileName, pFileName);

    /* Initialize the queue head of section names */
    QU_INIT(&pConfig->sections);

    /* Initialize the queue head of macro expansions */
    QU_INIT(&pConfig->macros);

    /* Process this file */
    rc = processFile(pFileName, pConfig, NULL);

    /* If there was an error, free the Config structure */
    if (rc != Success)
    {
	/* Free everything up */
	CONFIGW_close(pConfig);
    }
    else
    {
	/* Give 'em what they came for */
	*phConfig = pConfig;
    }

    return rc;
}



/*
 * Function:	CONFIGW_close()
 *
 * Description: Config write close.
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
CONFIGW_close(void * hConfig)
{
    ConfigFile *    pConfig = hConfig;
    Section *	    pSection;
    Parameter *	    pParameter;
    Macro *	    pMacro;

    /* For each section... */
    for (pSection = QU_FIRST(&pConfig->sections);
	 ! QU_EQUAL(pSection, &pConfig->sections);
	 pSection = QU_FIRST(&pConfig->sections))
    {
	/* For each parameter in this section... */
	for (pParameter = QU_FIRST(&pSection->parameters);
	     ! QU_EQUAL(pParameter, &pSection->parameters);
	     pParameter = QU_FIRST(&pSection->parameters))
	{
	    /* Remove this parameter from its queue */
	    QU_REMOVE(pParameter);

	    /* Free it */
	    OS_free(pParameter);
	}

	/* Remove this section from its queue */
	QU_REMOVE(pSection);

	/* Free it */
	OS_free(pSection);
    }

    /* For each macro... */
    for (pMacro = QU_FIRST(&pConfig->macros);
	 ! QU_EQUAL(pMacro, &pConfig->macros);
	 pMacro = QU_FIRST(&pConfig->macros))
    {
	/* Remove this macro from its queue */
	QU_REMOVE(pMacro);

	/* Free it */
	OS_free(pMacro);
    }

    /* Free the ConfigFile structure itself */
    OS_free(pConfig);
}




/*
 * Function:	CONFIGW_getNumber()
 *
 * Description: 
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
		  OS_Uint32 * pValue)
{
    Section *	    pSection;
    Parameter *	    pParameter;

    /* Look for the specified parameter */
    if (findParameter(hConfig, pSectionName, pTypeName,
		      &pSection, &pParameter) != Success)
    {
	/* Parameter Section/Type not found. */
	return Fail;
    }

    /* Convert the parameter value to a number, and give it to 'em. */
    *pValue = atol(pParameter->pValue);

    return Success;
}

/*
 * Function:	CONFIGW_getString()
 *
 * Description: Config write get string.
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
		  char ** ppValue)
{
    Section *	    pSection;
    Parameter *	    pParameter;

    /* Look for the specified parameter */
    if (findParameter(hConfig, pSectionName, pTypeName,
		      &pSection, &pParameter) != Success)
    {
	/* Parameter Section/Type not found. */
	return Fail;
    }

    /* Give 'em the pointer to the value string */
    *ppValue = pParameter->pValue;

    return Success;
}



/*
 * Function:	CONFIGW_setNumber()
 *
 * Description: 
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
		  CONFIGW_Permanance permanance)
{
    char	    buf[64];

    /* Create a string of the value */
    sprintf(buf, "%lu", value);

    /*
     * Find (or create, if necessary) the specified parameter structure, and
     * set its value.
     */
    return findCreateSetParameter(hConfig, pSectionName,
				  pTypeName, buf, permanance);
}



/*
 * Function:	CONFIGW_setString()
 *
 * Description: 
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
		  CONFIGW_Permanance permanance)
{
    /*
     * Find (or create, if necessary) the specified parameter structure, and
     * set its value.
     */
    return findCreateSetParameter(hConfig, pSectionName,
				  pTypeName, pValue, permanance);
}



/*
 * Function:	CONFIGW_nextParameter()
 *
 * Description: 
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
		      void ** phParameter)
{
    ConfigFile *    pConfig = hConfig;
    Section *	    pSection;
    Parameter *	    pParameter;

    /* Yup.  For each section... */
    for (pSection = QU_FIRST(&pConfig->sections);
	 ! QU_EQUAL(pSection, &pConfig->sections);
	 pSection = QU_NEXT(pSection))
    {
	/* Is this the section name we're looking for? */
	if (OS_strcasecmp(pSectionName, pSection->pSectionName) == 0)
	{
	    /* Yup.  Save a pointer to it */
	    break;
	}
    }

    /* If we didn't find the section, let 'em know. */
    if (QU_EQUAL(pSection, &pConfig->sections))
    {
	return Fail;
    }

    /* If we were given a starting point... */
    if (*phParameter != NULL)
    {
	/* ... then use it. */
	pParameter = *phParameter;
    }
    else
    {
	/* Otherwise, use the beginning of the queue */
	pParameter = QU_FIRST(&pSection->parameters);
    }

    /* Is there anything left on this queue? */
    if (QU_EQUAL(pParameter, &pSection->parameters))
    {
	/* No.  Let 'em know. */
	return Fail;
    }

    /* Give 'em the parameter data */
    *ppTypeName = pParameter->pTypeName;
    *ppValue = pParameter->pValue;

    /* Point to the next element on the queue, for next time */
    *phParameter = QU_NEXT(pParameter);

    return Success;
}



/*
 * Function:	CONFIGW_nextSection()
 *
 * Description: 
 *
 * Parameters:
 *
 *         hConfig --
 *             Configuration handle previously returned by CONFIGW_openFile.
 *
 *         ppSectionName --
 *             Pointer to the location to place a pointer to the character
 *             string containing the section name.  If, upon entry to this
 *             function, the location pointed to by this variable is not NULL,
 *             it is assumed to be a section name to search for.
 *
 *         phSection --
 *             Handle indicating where to begin the search.  If the location
 *             pointed to by this parameter contains NULL
 *                 (i.e. *phSection == NULL)
 *             then the first section in the configuration file will be
 *             returned.  Upon return from this function, *phSection is
 *             updated with a new value.  If this value of phSection is passed
 *             to this function again, the next section in the configuration
 *             file will be returned.
 *
 * Returns:
 *         Success
 *         Fail
 */
ReturnCode
CONFIGW_nextSection(void * hConfig,
		    char ** ppSectionName,
		    void ** phSection)
{
    ConfigFile *    pConfig = hConfig;
    Section *	    pSection = *phSection;
    char *	    pSectionName = *ppSectionName;

    /* Is this a request for the first section? */
    if (pSection == NULL)
    {
	/* Yup.  Point to the first section. */
	pSection = QU_FIRST(&pConfig->sections);
    }

    /* Is there anything left on this queue? */
    if (QU_EQUAL(pSection, &pConfig->sections))
    {
	return Fail;
    }

    /* Did they specify a particular section name to search for? */
    if (pSectionName != NULL)
    {
	/* Yup.  Find it. */
	for (;
	     ! QU_EQUAL(pSection, &pConfig->sections);
	     pSection = QU_NEXT(pSection))
	{
	    if (OS_strcasecmp(pSectionName, pSection->pSectionName) == 0)
	    {
		/* We found it. */
		break;
	    }
	}

	/* Did we find it? */
	if (QU_EQUAL(pSection, &pConfig->sections))
	{
	    /* Nope.  Let 'em know. */
	    return Fail;
	}
    }

    /* Give 'em the section name */
    *ppSectionName = pSection->pSectionName;

    /* Point to the next element on the queue, for next time */
    *phSection = QU_NEXT(pSection);

    return Success;
}



/*<
 * Function:    processFile.
 *
 * Description: Process file.
 *
 * Arguments:  
 *
 * Returns:   
 *
>*/


static ReturnCode
processFile(char * pFileName,
	    ConfigFile * pConfig,
	    Section * pSection)
{
    int		    i;
    FILE *	    hFd;
    char *	    p;
    char *	    pTypeName;
    char *	    pSectionName;
    char	    buf[512];
    OS_Boolean	    bMacro;
    long	    fileLocation;
    ReturnCode	    rc = Success;
    static int	    nestCount = 0;
    static int	    maxNest = 20;

    /* Open the specified configuration file */
    if ((hFd = fopen(pFileName, "r")) == NULL)
    {
	OS_free(pConfig);
	return OS_RC_NoSuchFile;
    }

    /*
     * Read the configuration file.
     */
    TM_TRACE((hTM, TM_ENTER, "Reading configuration file (%s)", pFileName));
    while (OS_fileGetString(p = buf, sizeof(buf), hFd) != NULL)
    {
	/* Strip leading and trailing white space from the line */
	stripWhiteSpace(&p);

	/* If there's nothing on the line... */
	if (*p == '\0')
	{
	    /* ... get the next line. */
	    continue;
	}

	/* Are we looking at a comment? */
	if (*p == '#')
	{
	    /* Yup.  Do nothing. */
	    continue;
	}

	/* Are we looking at an internal command? */
	if (*p == '!')
	{
	    /* Yup.  Determine which command it is. */
	    ++p;
	    stripWhiteSpace(&p);

	    if (OS_strncasecmp(p, "include", 7) == 0 &&
		isspace(p[7]))
	    {
		/*
		 * It's an INCLUDE command.  Get the included file name.
		 */

		/* Make sure we're not at the maximum recursion level */
		if (nestCount >= maxNest)
		{
		    /* We are.  They probably included themselves */
		    TM_TRACE((hTM, TM_ENTER,
			      "Excessive include file nesting"));
		    rc = UnsupportedOption;
		    goto Done;
		}

		/* Advance past INCLUDE command and strip white space */
		p += 8;
		stripWhiteSpace(&p);

		/* Save out location in the current file */
		fileLocation = ftell(hFd);

		/* Close the current file so we don't run out of descriptors */
		fclose(hFd);

		/* Indicate that we're going one more nesting level */
		++nestCount;

/* DONT RECURSIVELY INCLUDE FILE.  JUST INDICATE THAT IT'S TO BE INCLDUED */

		/* Read the included config file */
		rc = processFile(p, pConfig, pSection);
		
		/* We're back to our previous nesting level */
		--nestCount;

		/* Was the inclusion successful? */
		if (rc != Success)
		{
		    /* Nope. */
		    TM_TRACE((hTM, TM_ENTER,
			      "Error processing included file (%s)", p));
		    goto Done;
		}

		/* Re-open the previous file */
		if ((hFd = fopen(pFileName, "r")) == NULL)
		{
		    rc = OS_RC_NoSuchFile;
		    goto Done;
		}

		/* Seek back to the previous location within the file */
		(void) fseek(hFd, fileLocation, SEEK_SET);
	    }
	    else if (OS_strncasecmp(p, "trace", 5) == 0 &&
		     isspace(p[5]))
	    {
		/*
		 * It's a TRACE command
		 */

		/* Move past the TRACE command */
		p += 6;

		/* Strip white space */
		stripWhiteSpace(&p);

/* DONT DO IT; JUST SAVE REQUEST */

		/* The remainder of the line is a TM set-up string */
		if (TM_setUp(p) != SUCCESS)
		{
		    TM_TRACE((hTM, TM_ENTER,
			      "Invalid trace string (%s)", p));
		    rc = Fail;
		    goto Done;
		}
	    }
	    else if (OS_strncasecmp(p, "traceFile", 9) == 0 &&
		     isspace(p[9]))
	    {
		/*
		 * It's a TRACEFILE command
		 */

		/* Move past the TRACEFILE command */
		p += 10;

		/* Strip white space */
		stripWhiteSpace(&p);

/* DONT DO IT; JUST SAVE REQUEST */

		/* Tell TM what file to send its output to */
		if (TM_config(p) != SUCCESS)
		{
		    TM_TRACE((hTM, TM_ENTER,
			      "Could not set trace output file to (%s)", p));
		    rc = Fail;
		    goto Done;
		}
	    }
	    else if (OS_strncasecmp(p, "exceptionFile", 13) == 0 &&
		     isspace(p[13]))
	    {
		/*
		 * It's an EXCEPTIONFILE command
		 */

		/* Move past the EXCEPTION command */
		p += 10;

		/* Strip white space */
		stripWhiteSpace(&p);

/* DONT DO IT; JUST SAVE REQUEST */

		/* Tell TM what file to send its output to */
		if (EH_config(p) != SUCCESS)
		{
		    TM_TRACE((hTM, TM_ENTER,
			      "Could not set exception output file to (%s)",
			      p));
		    rc = Fail;
		    goto Done;
		}
	    }
	    else if (OS_strncasecmp(p, "nest", 4) == 0 &&
		     isspace(p[4]) &&
		     (i = atoi(&p[5])) > 0)
	    {
/* DONT DO IT; JUST SAVE REQUEST */

		/* It's a NEST command.  Set the maximum nesting level */
		maxNest = i;
	    }
	    else if (OS_strncasecmp(p, "print", 5) == 0 &&
		     isspace(p[5]))
	    {
/* DONT DO IT; JUST SAVE REQUEST */

		/* It's a PRINT command.  Just print the remainder of line */
		puts(p + 6);
	    }
	    else
	    {
		/* That's all of the internal commands we support. */
		TM_TRACE((hTM, TM_ENTER,
			  "Unrecognized internal command (%s)", p));
		rc = UnsupportedOption;
		goto Done;
	    }

	    /* Nothing more to do. */
	    continue;
	}

	/* Are we looking at a Section Name? */
	if (*p == '[')
	{
	    /* Yup.  Save the section name. */
	    pSectionName = p + 1;

	    /* Scan to the end of the section name */
	    for (p = pSectionName; *p && *p != ']'; p++)
		;

	    /* If we didn't find a ']', it's an invalid section name. */
	    if (*p != ']')
	    {
		TM_TRACE((hTM, TM_ENTER,
			  "Unterminated Section name (%s)", pSectionName));
		rc = UnsupportedOption;
		goto Done;
	    }

	    /* Null terminate at end of section name */
	    *p = '\0';

	    /* Add a new section */
	    if ((rc = newSection(pConfig, pSectionName, &pSection)) != Success)
	    {
		goto Done;
	    }

	    continue;
	}

	if (*p == '%')	/* Are we looking at a Macro Definition? */
	{
	    /* Yup.  Flag it. */
	    bMacro = TRUE;

	    /* Update pointer to character following '%', ... */
	    ++p;

	    /* ... and strip white space again. */
	    stripWhiteSpace(&p);
	}
	else
	{
	    bMacro = FALSE;
	}

	/*
	 * We've found a "Parameter = Value" pair or a macro definition
	 */
	    
	/* Point to the Type Name */
	pTypeName = p;

	/* Scan to the end of the type name */
	for (; *p && *p != '='; p++)
	    ;

	/* If there's no data on the line, just skip this line. */
	if (*p == '\0')
	{
	    continue;
	}

	/* If we didn't find a '=', it's an invalid type name. */
	if (*p != '=')
	{
	    rc = UnsupportedOption;
	    goto Done;
	}

	/* Null terminate at end of type name */
	*p = '\0';

	if (bMacro)
	{
	    if ((rc = newMacro(pConfig, pTypeName, p+1)) != Success)
	    {
		TM_TRACE((hTM, TM_ENTER, "newMacro() failed\n"));
		goto Done;
	    }
	}
	else
	{
	    /*
	     * If it's a parameter definition and there's no current
	     * section pointer, we can't do this.
	     */
	    if (pSection == NULL)
	    {
		rc = UnsupportedOption;
		goto Done;
	    }
	    
	    if ((rc =
		 newParameter(pConfig,
			      pSection,
			      pTypeName,
			      p + 1,
			      NULL,
			      CONFIGW_Transient)) != Success)
	    {
		TM_TRACE((hTM, TM_ENTER, "newParameter() failed\n"));
		goto Done;
	    }
	}
    }

  Done:
    /* Close the file */
    fclose(hFd);
    
    return rc;
}



/*<
 * Function:    newSection
 *
 * Description: New section
 *
 * Arguments:   
 *
 * Returns:    
 *
>*/


static ReturnCode
newSection(void * hConfig,
	   char * pSectionName,
	   Section ** ppSection)
{
    ConfigFile *    pConfig = hConfig;
    Section *	    pSection;

    /* Strip white space from the section name */
    stripWhiteSpace(&pSectionName);

    /* Allocate a Section Name structure */
    if ((pSection =
	 OS_alloc(sizeof(Section) + strlen(pSectionName) + 1)) == NULL)
    {
	return ResourceError;
    }

    /* Initialize our queue pointers */
    QU_INIT(pSection);

    /* Point to the memory allocated for the section name */
    pSection->pSectionName = (char *) (pSection + 1);

    /* Copy the section name */
    strcpy(pSection->pSectionName, pSectionName);
    
    /* Initialize the parameters queue head */
    QU_INIT(&pSection->parameters);

    /* Insert this section onto the config's section queue */
    QU_INSERT(pSection, &pConfig->sections);

    /* Give 'em what they came for */
    *ppSection = pSection;

    return Success;
}


/*<
 * Function:    newParameter
 *
 * Description: New parameter
 *
 * Arguments:   
 *
 * Returns:    
 *
>*/

static ReturnCode
newParameter(ConfigFile * pConfig,
	     Section * pSection,
	     char * pTypeName,
	     char * pDefinition,
	     char * pSaveDefinition,
	     CONFIGW_Permanance permanance)
{
    ReturnCode	    rc;
    int		    len;
    char	    buf[CONFIGW_MAX_PARAMETER_LEN];
    Parameter *	    pParameter;

    /* Strip white space from the type name */
    stripWhiteSpace(&pTypeName);

    /* Strip white space from the Value */
    stripWhiteSpace(&pDefinition);

    /* Get the length of the value string */
    len = strlen(pDefinition);

    /*
     * If the value string is quoted, strip the quotes.  Otherwise,
     * look for expansions in the value string.
     */
    if (len < 2 || *pDefinition != '"' || pDefinition[len - 1] != '"')
    {
	/* Expand any macros */
	if ((rc = expand(pConfig, pDefinition, buf,
			 buf + CONFIGW_MAX_PARAMETER_LEN)) != Success)
	{
	    return rc;
	}
    }
    else
    {
	strncpy(buf, pDefinition + 1, len - 2);
	buf[len-2] = '\0';
    }

    /* Determine which definition to save -- the new one or a passed-in one */
    if (pSaveDefinition == NULL)
    {
	/* Use the new one, since we weren't provided with one. */
	pSaveDefinition = pDefinition;
    }

    /* Allocate a Parameter structure */
    if ((pParameter =
	 OS_alloc(sizeof(Parameter) +
		  strlen(pTypeName) + 1 +
		  strlen(pSaveDefinition) + 1 +
		  strlen(buf) + 1)) == NULL)
    {
	return ResourceError;
    }

    /* Initialize the Parameter's queue pointers */
    QU_INIT(pParameter);

    /* Point to the memory allocated for the type name */
    pParameter->pTypeName = (char *) (pParameter + 1);

    /* Copy the type name */
    strcpy(pParameter->pTypeName, pTypeName);

    /* Point to the memory allocated for the definition */
    pParameter->pDefinition =
	pParameter->pTypeName + strlen(pParameter->pTypeName) + 1;

    /* Copy the definition. */
    strcpy(pParameter->pDefinition, pSaveDefinition);

    /* Point to the memory allocated for the value */
    pParameter->pValue =
	pParameter->pDefinition + strlen(pParameter->pDefinition) + 1;

    /* Copy the value. */
    strcpy(pParameter->pValue, buf);

    /* Save the permanence of this parameter */
    pParameter->permanance = permanance;

    /* Insert this new parameter onto section queue */
    QU_INSERT(pParameter, &pSection->parameters);

    return Success;
}



/*<
 * Function:    newMacro
 *
 * Description: New macro
 *
 * Arguments:   
 *
 * Returns:    
 *
>*/

static ReturnCode
newMacro(ConfigFile * pConfig,
	 char * pMacroName,
	 char * pDefinition)
{
    ReturnCode	    rc;
    int		    len;
    char	    buf[CONFIGW_MAX_PARAMETER_LEN];
    Macro *	    pMacro;

    TM_TRACE((hTM, TM_ENTER, "entering newMacro\n"));

    /* Strip white space from the macro name */
    stripWhiteSpace(&pMacroName);

    /* Strip white space from the expansion */
    stripWhiteSpace(&pDefinition);

    /* Get the length of the string to be expanded */
    len = strlen(pDefinition);

    /*
     * If the value string is quoted, strip the quotes.  Otherwise,
     * look for expansions in the value string.
     */
    if (len < 2 || *pDefinition != '"' || pDefinition[len - 1] != '"')
    {
	/* Expand any macros */
	if ((rc = expand(pConfig, pDefinition, buf,
			 buf + CONFIGW_MAX_PARAMETER_LEN)) != Success)
	{
	    return rc;
	}
    }
    else
    {
	strncpy(buf, pDefinition + 1, len - 2);
	buf[len-2] = '\0';
    }

    /* Allocate a Macro structure */
    if ((pMacro =
	 OS_alloc(sizeof(Macro) +
		  strlen(pMacroName) + 1 +
		  strlen(pDefinition) + 1 +
		  strlen(buf) + 1)) == NULL)
    {
	return ResourceError;
    }

    /* Initialize the Macro's queue pointers */
    QU_INIT(pMacro);

    /* Point to the memory allocated for the type name */
    pMacro->pMacroName = (char *) (pMacro + 1);

    /* Copy the macro name */
    strcpy(pMacro->pMacroName, pMacroName);

    /* Point to the memory allocated for the definition */
    pMacro->pDefinition =
	pMacro->pMacroName + strlen(pMacro->pMacroName) + 1;

    /* Copy the definition */
    strcpy(pMacro->pDefinition, pDefinition);

    /* Point to the memory allocated for the expansion */
    pMacro->pExpansion =
	pMacro->pDefinition + strlen(pMacro->pDefinition) + 1;

    /* Copy the expansion */
    strcpy(pMacro->pExpansion, buf);

    /* Insert this new macro onto section queue */
    QU_INSERT(pMacro, &pConfig->macros);

    return Success;
}



/*<
 * Function:    expand
 *
 * Description: Expand
 *
 * Arguments:   
 *
 * Returns:    
 *
>*/

static ReturnCode
expand(ConfigFile * pConfig,
       char * pInput,
       char * pOutput,
       char * pOutputEnd)
{
    int		    len;
    char * 	    pEnd;
    char *	    pEnv;
    Macro *	    pMacro;
#ifdef OS_TYPE_UNIX
    char *	    pUserOutput = pOutput;
    char *	    pUserOutputEnd = pOutputEnd;
    char	    buf[CONFIGW_MAX_PARAMETER_LEN];

    pOutput = buf;
    pOutputEnd = buf + CONFIGW_MAX_PARAMETER_LEN;
#endif

    TM_TRACE((hTM, TM_ENTER, "Entering expand().  Do macro substitution\n"));

    /*
     * Pass 1.  Do macro substitution.
     */

    /* Copy the parameter into our buffer, expanding any macros encountered. */
    for (; *pInput != '\0'; pInput++)
    {
	/* Are we looking at a macro substitution request? */
	if (*pInput == '$' && pInput[1] == '{')
	{
	    /* Yup.  Find the end of the macro name */
	    pEnd = strchr(pInput+2, '}');

	    /* Did we find the end? */
	    if (pEnd != NULL)
	    {
		/* Yup.  Null terminate to isolate the macro name */
		*pEnd = '\0';

		/* Update our pointer to the beginning of the macro name */
		pInput += 2;

		/* Search for this macro name in the list of macros */
		for (pMacro = QU_FIRST(&pConfig->macros);
		     ! QU_EQUAL(pMacro, &pConfig->macros);
		     pMacro = QU_NEXT(pMacro))
		{
		    /* Is this the one we're looking for?  (case-sensitive) */
		    if (strcmp(pMacro->pMacroName, pInput) == 0)
		    {
			/* Yup.  Expand it if there's room in the buffer. */
			if (pOutputEnd - pOutput <
			    (len = strlen(pMacro->pExpansion)))
			{
			    /* Expansion too long */
			    return ResourceError;
			}

			/* Copy the expansion */
			strcpy(pOutput, pMacro->pExpansion);

			/* Update the output buffer pointer */
			pOutput += len;

			/* Already found the macro.  Get outta the loop. */
			break;
		    }
		}

		/* Did we find a substitution? */
		if (QU_EQUAL(pMacro, &pConfig->macros))
		{
		    /* Nope.  See if there's an environment variable. */
		    if ((pEnv = getenv(pInput)) != NULL)
		    {
			/* There is.  See if there's room in the buffer. */
			if (pOutputEnd - pOutput < (len = strlen(pEnv)))
			{
			    /* Expansion too long */
			    return ResourceError;
			}

			/* Copy the expansion. */
			strcpy(pOutput, pEnv);

			/* Update the output buffer pointer */
			pOutput += len;
		    }
		    else
		    {
			TM_TRACE((hTM, TM_ENTER,
				  "No expansion for (%s)\n", pInput));

			/* No expansion for specified variable */
			return Fail;
		    }
		}

		/* Point to character following macro expansion request */
		pInput = pEnd;
	    }
	}
	else
	{
	    /* Is there room in the buffer for this character? */
	    if (pOutput >= pOutputEnd)
	    {
		/* Nope */
		return ResourceError;
	    }

	    /* Copy this character */
	    *pOutput++ = *pInput;
	}
    }

    /* Null-terminate the output */
    *pOutput = '\0';

#ifdef OS_TYPE_UNIX
    /*
     * Pass 2.  Do program-execution substitution.
     */

    TM_TRACE((hTM, TM_ENTER,
	      "expand(): Beginning program-execution substitution:\n"
	      "input = (%s)\n", buf));

    /* Copy the parameter into our buffer, running specified programs. */
    for (pInput = buf, pOutput = pUserOutput, pOutputEnd = pUserOutputEnd;
	 *pInput != '\0';
	 pInput++)
    {
	if (*pInput == '`' && (pEnd = strchr(pInput + 1, '`')) != NULL)
	{
	    FILE *	    hCommand;
	    char	    cmdBuf[CONFIGW_MAX_PARAMETER_LEN];

	    /* We found a command to run.  Null terminate it. */
	    *pEnd = '\0';

	    /* Point to the beginning of the command */
	    ++pInput;

	    /* Open a pipe to the specified command */
	    if ((hCommand = popen(pInput, "r")) == NULL)
	    {
		TM_TRACE((hTM, TM_ENTER,
			  "Could not popen(%s, \"r\")\n",
			  pInput));
		return Fail;
	    }

	    /* Get the command's output */
	    if (fgets(cmdBuf, sizeof(cmdBuf), hCommand) == NULL)
	    {
		/* No output?  Just give 'em an empty string */
		*cmdBuf = '\0';
	    }

	    /* We're done with this command now. */
	    (void) pclose(hCommand);

	    /* Get the length of the command output */
	    len = strlen(cmdBuf);

	    /* If the command ended with a newline... */
	    if (len > 0 && cmdBuf[len - 1] == '\n')
	    {
		/* ... then eliminate it. */
		cmdBuf[--len] = '\0';
	    }

	    /* See if there's room in the buffer for the command output. */
	    if (pOutputEnd - pOutput < (len = strlen(cmdBuf)))
	    {
		/* Expansion too long */
		return ResourceError;
	    }
	    
	    /* Copy the expansion. */
	    strcpy(pOutput, cmdBuf);
	    
	    /* Update the output buffer pointer */
	    pOutput += len;

	    /* Update the input pointer */
	    pInput = pEnd;
	}
	else
	{
	    /* Is there room in the buffer for this character? */
	    if (pOutput >= pOutputEnd)
	    {
		/* Nope */
		return ResourceError;
	    }

	    /* Copy this character */
	    *pOutput++ = *pInput;
	}
    }

    /* Null terminate. */
    *pOutput = '\0';
#endif

    TM_TRACE((hTM, TM_ENTER, "Leaving expand\n"));

    return Success;
}



/*<
 * Function:    findCreateSetParameter
 *
 * Description: 
 *
 * Arguments:   
 *
 * Returns:    
 *
>*/


static ReturnCode
findCreateSetParameter(void * hConfig,
		       char * pSectionName,
		       char * pTypeName,
		       char * pDefinition,
		       CONFIGW_Permanance permanance)
{
    ReturnCode	    rc;
    Section *	    pSection;
    Parameter *	    pParameter = NULL;
    Parameter *	    pNextParameter = NULL;
    char *	    pSaveDefinition;
    OS_Boolean	    bCreatedSection = FALSE;
    ConfigFile *    pConfig = hConfig;

    /* Find the specified parameter, if it already exists. */
    if (findParameter(pConfig, pSectionName, pTypeName,
		      &pSection, &pParameter) != Success)
    {
	/* Parameter Section/Type not found.  Was the section name found? */
	if (pSection == NULL)
	{
	    /* No.  Create it first. */
	    if ((rc = newSection(hConfig, pSectionName, &pSection)) != Success)
	    {
		return rc;
	    }

	    /* Remember that we created this section */
	    bCreatedSection = TRUE;
	}

    }
    else
    {
	/* Remove the parameter from the list.  We'll recreate it. */
	pNextParameter = QU_NEXT(pParameter);
	QU_REMOVE(pParameter);
    }

    /*
     * If it's not a permanent change and we found a parameter it's
     * replacing...
     */
    if (permanance != CONFIGW_Permanent && pParameter != NULL)
    {
	/* ... then save the original definition */
	pSaveDefinition = pParameter->pDefinition;
    }
    else
    {
	/* Otherwise, save the new definition. */
	pSaveDefinition = NULL;
    }

    /* Create the parameter. */
    if ((rc = newParameter(pConfig, pSection, pTypeName,
			   pDefinition,
			   pSaveDefinition,
			   permanance)) != Success)
    {
	goto cleanUp;
    }

    /* Rewrite the file, if the change is permanent */
    if (permanance == CONFIGW_Permanent)
    {
	if ((rc = saveConfig(pConfig)) != Success)
	{
	    goto cleanUp;
	}
    }

    /* We've recreated the parameter.  Free the old one. */
    if (pParameter != NULL)
    {
	OS_free(pParameter);
    }

    return Success;

  cleanUp:
    /* Something went wrong.  If we created the section, free it now */
    if (bCreatedSection)
    {
	QU_REMOVE(pSection);
	OS_free(pSection);
    }

    /* If we had removed an existing parameter, reinsert it */
    if (pParameter != NULL)
    {
	QU_INSERT(pParameter, pNextParameter);
    }

    return rc;
}



/*<
 * Function:    findParameter
 *
 * Description: Find parameter
 *
 * Arguments:   
 *
 * Returns:    
 *
>*/

static ReturnCode
findParameter(void * hConfig,
	     char * pSectionName,
	     char * pTypeName,
	     Section ** ppSection,
	     Parameter ** ppParameter)
{
    ConfigFile *    pConfig = hConfig;
    Section *	    pSection;
    Parameter *	    pParameter;

    /* For each section... */
    for (pSection = QU_FIRST(&pConfig->sections);
	 ! QU_EQUAL(pSection, &pConfig->sections);
	 pSection = QU_NEXT(pSection))
    {
	if (OS_strcasecmp(pSectionName, pSection->pSectionName) == 0)
	{
	    /* For each parameter in this section... */
	    for (pParameter = QU_FIRST(&pSection->parameters);
		 ! QU_EQUAL(pParameter, &pSection->parameters);
		 pParameter = QU_NEXT(pParameter))
	    {
		if (OS_strcasecmp(pTypeName, pParameter->pTypeName) == 0)
		{
		    /* We found it.  Give it to 'em */
		    *ppSection = pSection;
		    *ppParameter = pParameter;

		    return Success;
		}
	    }
	}
    }

    return Fail;
}



/*<
 * Function:    stripWhiteSpace
 *
 * Description: Strip white space characters
 *
 * Arguments:   Pointer to text.
 *
 * Returns:     None.
 *
>*/


void
stripWhiteSpace(char ** ppText)
{
    int		    len;
    char *	    p = *ppText;

    /* Strip leading white space */
    if (isspace(*p))
    {
	while (isspace(*p))
	{
	    p++;
	}

	*ppText = p;
    }

    /* Strip trailing white space */
    len = strlen(p);
    if (len > 0)
    {
	p += len - 1;
	if (isspace(*p))
	{
	    while (isspace(*p))
	    {
		p--;
	    }
	    
	    *++p = '\0';
	}
    }
}



/*<
 * Function:    saveConfig
 *
 * Description: Save config
 *
 * Arguments:   Config file handle.
 *
 * Returns:   
 *
>*/


static ReturnCode
saveConfig(ConfigFile * pConfig)
{
    ReturnCode	    rc;
    char *	    p;
    char	    tempName[OS_MAX_FILENAME_LEN];
    FILE *	    hFd;
    Macro *	    pMacro;
    Section *	    pSection;
    Parameter *	    pParam;

    /* Copy the config file name */
    strcpy(tempName, pConfig->pFileName);

    /* Find the trailing directory separator */
    for (p = tempName + strlen(tempName) - 1;
	 p != tempName;
	 p--)
    {
	if (strchr(OS_DIR_PATH_SEPARATOR, *p) != NULL)
	{
	    break;
	}
    }

    /* Did we find one? */
    if (strchr(OS_DIR_PATH_SEPARATOR, *p) != NULL)
    {
	/* Yup.  Move past it. */
	++p;
    }

    /* Create a temporary name template */
    strcpy(p, "XXXXXX.cfg");

    /* Create temporary name to which we'll rename the existing config file */
    if ((rc = OS_uniqueName(tempName)) != Success)
    {
	return rc;
    }

    /* Rename the config file to this temporary name */
    if ((rc = OS_moveFile(tempName, pConfig->pFileName)) != Success)
    {
	return rc;
    }

    /* Create the new configuration file */
    if ((hFd = fopen(pConfig->pFileName, "w")) == NULL)
    {
	(void) OS_moveFile(pConfig->pFileName, tempName);
	return OS_RC_FileCreate;
    }

    /* Write each macro to the file */
    for (pMacro = QU_FIRST(&pConfig->macros);
	 ! QU_EQUAL(pMacro, &pConfig->macros);
	 pMacro = QU_NEXT(pMacro))
    {
	fprintf(hFd, "%-30s= %s\n", pMacro->pMacroName, pMacro->pDefinition);
    }

    fprintf(hFd, "\n\n");

    /* For each section... */
    for (pSection = QU_FIRST(&pConfig->sections);
	 ! QU_EQUAL(pSection, &pConfig->sections);
	 pSection = QU_NEXT(pSection))
    {
	/* Write out the section name */
	fprintf(hFd, "[%s]\n", pSection->pSectionName);

	/* Write each parameter in this section */
	for (pParam = QU_FIRST(&pSection->parameters);
	     ! QU_EQUAL(pParam, &pSection->parameters);
	     pParam = QU_NEXT(pParam))
	{
	    fprintf(hFd, "    %-30s= %s\n",
		    pParam->pTypeName, pParam->pDefinition);
	}

	fprintf(hFd, "\n");
    }

    /* Close the file */
    fclose(hFd);

    /* Delete the old (renamed) configuration file */
    (void) OS_deleteFile(tempName);

    return Success;
}
