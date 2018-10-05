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
 * This portation of the Profile module is implemented in terms of the CONFIG_
 * module.
 */

#include "estd.h"
#include "queue.h"
#include "config.h"
#include "profile.h"


typedef struct Profile
{
    void *	    hConfig;
    QU_Head	    attributeList;
} Profile;


typedef struct Attribute
{
    QU_ELEMENT;

    char * 	    pAttributeName;
    OS_Uint32	    type;
    OS_Boolean	 (* pfEqual)(char * pValue1,
			     char * pValue2,
			     OS_Uint32 type);
} Attribute;



/*
 * profile_open()
 *
 */
ReturnCode
profile_open(char * pProfileName,
	     void ** phProfile)
{
    ReturnCode	    rc;
    Profile *	    pProfile;

    /* Allocate a profile structure */
    if ((pProfile = OS_alloc(sizeof(Profile))) == NULL)
    {
	return ResourceError;
    }

    /* Initialize the profile's queue pointers */
    QU_INIT(&pProfile->attributeList);

    /* Open the profile configuration file */
    if ((rc = CONFIG_open(pProfileName, &pProfile->hConfig)) != Success)
    {
	OS_free(pProfile);
	return rc;
    }

    /* Give 'em what they came for */
    *phProfile = pProfile;

    return Success;
}


/*
 * profile_close()
 *
 */
void
profile_close(void * hProfile)
{
    Profile *	    pProfile = hProfile;
    Attribute *	    pAttribute;

    /* Close the configuration */
    CONFIG_close(pProfile->hConfig);

    /*
     * Free any attributes that have been allocated on behalf of this
     * profile.
     */
    while (! QU_EQUAL((pAttribute = QU_FIRST(&pProfile->attributeList)),
		      &pProfile->attributeList))
    {
	/* Remove this attribute from the queue */
	QU_REMOVE(pAttribute);

	/* Free it */
	OS_free(pAttribute);
    }

    /* Free our allocated resources */
    OS_free(pProfile);
}


/*
 * profile_addAttribute()
 *
 * In this implementation, the first attribute added becomes the "key field".
 */
ReturnCode
profile_addAttribute(void * hProfile,
		     char * pAttributeName,
		     OS_Uint32 type,
		     OS_Boolean (* pfEqual)(char * pValue1,
					    char * pValue2,
					    OS_Uint32 type))
{
    Profile *	    pProfile = hProfile;
    Attribute *	    pAttribute;

    /* Make sure the attribute name isn't too long */
    if (strlen(pAttributeName) > PROFILE_MAX_ATTRIBUTE_NAME_LEN)
    {
	return Fail;
    }

    /* Allocate an attribute structure and space for the attribute name */
    if ((pAttribute =
	 OS_alloc(sizeof(Attribute) + strlen(pAttributeName) + 1)) == NULL)
    {
	return ResourceError;
    }

    /* Initialize the queue pointers */
    QU_INIT(pAttribute);

    /* Point the attribute-name pointer to the end of the fixed structure */
    pAttribute->pAttributeName = (char *) (pAttribute + 1);

    /* Copy the attribute name */
    strcpy(pAttribute->pAttributeName, pAttributeName);

    /* Save the type */
    pAttribute->type = type;

    /* Save the compare function */
    pAttribute->pfEqual = pfEqual;

    /* Add this attribute to the profile */
    QU_INSERT(pAttribute, &pProfile->attributeList);

    /* All done. */
    return Success;
}


/*
 * profile_searchByAttribute()
 *
 */
ReturnCode
profile_searchByAttribute(void * hProfile,
			  char * pAttributeName,
			  char * pSearchValue,
			  void ** phSearch)
{
    ReturnCode	    rc;
    Profile *	    pProfile = hProfile;
    Attribute *	    pAttribute;
    char *	    pSectionName;
    char *	    pValue;
    char	    attributeName[PROFILE_MAX_ATTRIBUTE_NAME_LEN + 16];
    void *	    hSection;
    int		    sequenceId;

    /* Make sure the attribute name isn't too long */
    if (strlen(pAttributeName) > PROFILE_MAX_ATTRIBUTE_NAME_LEN)
    {
	return Fail;
    }

    /* Search for the specified attribute name in the registered attributes */
    for (pAttribute = QU_FIRST(&pProfile->attributeList);
	 ! QU_EQUAL(pAttribute, &pProfile->attributeList);
	 pAttribute = QU_NEXT(pAttribute))
    {
	/* Is this the one we're looking for? */
	if (OS_strcasecmp(pAttributeName, pAttribute->pAttributeName) == 0)
	{
	    /* Yup. */
	    break;
	}
    }

    /* Did we find the attribute name as one of our registered attributes? */
    if (QU_EQUAL(pAttribute, &pProfile->attributeList))
    {
	/* Nope.  So we therefore can't find what they're looking for. */
	return Fail;
    }

    /* If this is the first attribute name in the attribute list... */
    if (QU_EQUAL(pAttribute, QU_FIRST(&pProfile->attributeList)))
    {
	/* ...the attribute value is the Section name.  Make sure it exists. */
	hSection = NULL;
	if (CONFIG_nextSection(pProfile->hConfig,
			       &pSearchValue,
			       &hSection) == Success)
	{
	    /* It does.  Give it to 'em. */
	    *phSearch = pSearchValue;

	    return Success;
	}

	/* Attribute doesn't exist as a section name.  Oh well. */
	return Fail;
    }

    /*
     * Get each section name, and search for this attribute name in
     * each section.
     */
    hSection = NULL;
    pSectionName = NULL;
    while ((rc = CONFIG_nextSection(pProfile->hConfig,
				    &pSectionName, &hSection)) == Success)
    {
	/* Initialize the attribute sequence counter */
	sequenceId = 1;

	/* Search for the specified attribute in this section */
	do
	{
	    if (pAttribute->type & PROFILE_MULTIVALUE)
	    {
		sprintf(attributeName, "%s %d", pAttributeName, sequenceId++);
	    }
	    else
	    {
		strcpy(attributeName, pAttributeName);
	    }

	    if ((rc = CONFIG_getString(pProfile->hConfig,
				       pSectionName,
				       attributeName,
				       &pValue)) == Success)
	    {
		/* We found it.  Does it match what we're looking for? */
		if ((* pAttribute->pfEqual)(pValue, pSearchValue,
					    pAttribute->type))
		{
		    /* Yup.  Give 'em what they came for. */
		    *phSearch = pSectionName;
		    
		    return Success;
		}
	    }
	} while (rc == SUCCESS && (pAttribute->type & PROFILE_MULTIVALUE));
    }

    /* Couldn't find any more section names */
    return Fail;
}


/*
 * profile_searchByType()
 *
 */
ReturnCode
profile_searchByType(void * hProfile,
		     OS_Uint32 type,
		     char * pSearchValue,
		     void ** phSearch)
{
    ReturnCode	    rc;
    Profile *	    pProfile = hProfile;
    Attribute *	    pAttribute;
    int		    sequenceId;
    char *	    pSectionName;
    char *	    pValue;
    char	    attributeName[PROFILE_MAX_ATTRIBUTE_NAME_LEN + 16];
    void *	    hSection;

    /*
     * Get each section name, and search for any attributes of the
     * specified type in each section.
     */
    for (hSection = NULL, pSectionName = NULL;
	 CONFIG_nextSection(pProfile->hConfig, &pSectionName, &hSection) == Success;
	 pSectionName = NULL)
    {
	/*
	 * Search for the specified attribute name in the registered
	 * attributes.
	 */
	for (pAttribute = QU_FIRST(&pProfile->attributeList);
	     ! QU_EQUAL(pAttribute, &pProfile->attributeList);
	     pAttribute = QU_NEXT(pAttribute))
	{
	    /* If this attribute type isn't the type we're looking for... */
	    if ((pAttribute->type & ~PROFILE_MULTIVALUE) != (type & ~PROFILE_MULTIVALUE))
	    {
		/* ... try the next attribute. */
		continue;
	    }

	    /* If this is the first attribute... */
	    if (QU_EQUAL(pAttribute, QU_FIRST(&pProfile->attributeList)))
	    {
		/* ... the value is the section name */
		if ((* pAttribute->pfEqual)(pSectionName, pSearchValue, type))
		{
		    /* We found it.  Give 'em what they came for. */
		    *phSearch = pSectionName;
		    
		    return Success;
		}
	    }
	    else
	    {
		/* Initialize the attribute sequence counter */
		sequenceId = 1;

		/* Search for the specified attribute in this section */
		do
		{
		    if (pAttribute->type & PROFILE_MULTIVALUE)
		    {
			sprintf(attributeName, "%s %d",
				pAttribute->pAttributeName, sequenceId++);
		    }
		    else
		    {
			strcpy(attributeName, pAttribute->pAttributeName);
		    }
		    
		    /* Search for this attribute in the current section */
		    if ((rc = CONFIG_getString(pProfile->hConfig,
					       pSectionName,
					       attributeName,
					       &pValue)) == Success)
		    {
			/*
			 * We found it.  Does it match what we're
			 * looking for?
			 */
			if ((* pAttribute->pfEqual)(pValue,
						    pSearchValue, type))
			{
			    /* Yup.  Give 'em what they came for. */
			    *phSearch = pSectionName;
			    
			    return Success;
			}
		    }
		} while (rc == SUCCESS &&
			 (pAttribute->type & PROFILE_MULTIVALUE));
	    }
	}
    }

    /* Couldn't find any more section names */
    return Fail;
}


/*
 * profile_getData()
 *
 */
ReturnCode
profile_getData(void * hProfile,
		void * hSearch,
		char * pAttributeName,
		char ** ppValue)
{
    Profile *	    pProfile = hProfile;
    Attribute *	    pAttribute;
    char *	    pSectionName = hSearch;

    /* See if this attribute name is for the first attribute */
    pAttribute = QU_FIRST(&pProfile->attributeList);
    if (OS_strcasecmp(pAttributeName, pAttribute->pAttributeName) == 0)
    {
	/* It is.  That means we just want to return the section name */
	*ppValue = pSectionName;

	return Success;
    }

    /* It's not the first attribute.  Retrieve the parameter value */
    return CONFIG_getString(pProfile->hConfig,
			    pSectionName,
			    pAttributeName,
			    ppValue);
}
