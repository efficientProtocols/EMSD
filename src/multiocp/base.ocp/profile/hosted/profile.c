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


#include "profile.h"

/* #include "port.h" DJL */
#include "cfg.h"



/*
 * PROFILE_open()
 *
 * Prepare to begin using a profile library.
 *
 * Parameters:
 *
 *         pProfileName --
 *             Name of the profile library.  In some portations, this may be a
 *             file name.
 *
 *         phProfile --
 *             Pointer to a location in which a handle for this profile
 *             library will be placed.
 *
 * Returns:
 *
 *     Success
 *     ResourceError
 *     <depending upon the implementation, other non-Success values>
 */
ReturnCode
PROFILE_open(char * pProfileName,
	     void ** phProfile)
{
    return profile_open(pProfileName, phProfile);
}


/*
 * PROFILE_close()
 *
 * Release resources allocated on behalf of the user, for accessing a profile
 * library.
 *
 * Parameters:
 *
 *         hProfile --
 *             Handle to a profile library, previously provided by
 *             PROFILE_open().
 *
 * Returns:
 *
 *     Nothing.
 */
void
PROFILE_close(void * hProfile)
{
    profile_close(hProfile);
}


/*
 * PROFILE_addAttribute()
 *
 * Add an attribute to the list of attributes which this application is
 * concerned with.  Not all attributes in the profile library need be added;
 * only those of concern by the application making this call.  These
 * attributes are the ones that will be searchable by
 * PROFILE_searchByAttribute() and PROFILE_searchByType().
 *
 * There is one exception to the above rule.  The "key" attribute must be
 * added, and must be the very first attribute added.
 *
 * Parameters:
 *
 *         hProfile --
 *             Handle to a profile library, previously provided by
 *             PROFILE_open().
 *
 *         pAttributeName --
 *             Name of an attribute in a profile within this profile library.
 *
 *         type --
 *             Caller-defined value defining the type of the attribute.  If
 *             multiple attributes have the same type, then
 *             PROFILE_searchByType() will search all of those attributes for
 *             a particular value.
 *
 *             Note: The caller-defined portion of this value is the low-order
 *                   31 bits.  The high-order bit is reserved for OR-ing with
 *                   PROFILE_MULTIVALUE.  See its comment, above.
 *
 *         pfEqual --
 *             Pointer to a function which is called, during searches, to
 *             compare the specified search value against the attribute value
 *             in the profile.
 *
 * Returns:
 *
 *     Success
 *     Fail
 */
ReturnCode
PROFILE_addAttribute(void * hProfile,
		     char * pAttributeName,
		     OS_Uint32 type,
		     OS_Boolean (* pfEqual)(char * pValue1,
					    char * pValue2,
					    OS_Uint32 type))
{
    return profile_addAttribute(hProfile, pAttributeName, type, pfEqual);
}


/*
 * PROFILE_searchByAttribute()
 *
 * Search the specified profile library for a particular attribute matching a
 * specified value.
 *
 * Parameters:
 *
 *         hProfile --
 *             Handle to a profile library, previously provided by
 *             PROFILE_open().
 *
 *         pAttributeName --
 *             Name of the attribute which is to be compared to the search
 *             attribute value.
 *
 *         pSearchValue --
 *             Attribute value to be searched for.
 *
 *         phSearch --
 *             Pointer to a location in which a Search Handle is placed upon
 *             successful completion of this function.  This Search Handle may
 *             be passed to PROFILE_getData() to obtain other attributes from
 *             the profile found in this search.
 *
 * Returns:
 *
 *     Success
 *     Fail
 */
ReturnCode
PROFILE_searchByAttribute(void * hProfile,
			  char * pAttributeName,
			  char * pSearchValue,
			  void ** phSearch)
{
    return profile_searchByAttribute(hProfile, pAttributeName,
				     pSearchValue, phSearch);
}


/*
 * PROFILE_searchByType()
 *
 * Search the specified profile library for any attribute with the specified
 * type, matching a specified value.
 *
 * Parameters:
 *
 *         hProfile --
 *             Handle to a profile library, previously provided by
 *             PROFILE_open().
 *
 *         type --
 *             Value specifying the attribute types to be scanned for the
 *             specified search value.
 *
 *         pSearchValue --
 *             Attribute value to be searched for.
 *
 *         phSearch --
 *             Pointer to a location in which a Search Handle is placed upon
 *             successful completion of this function.  This Search Handle may
 *             be passed to PROFILE_getData() to obtain other attributes from
 *             the profile found in this search.
 *
 * Returns:
 *
 *     Success
 *     Fail
 */
ReturnCode
PROFILE_searchByType(void * hProfile,
		     OS_Uint32 type,
		     char * pSearchValue,
		     void ** phSearch)
{
    return profile_searchByType(hProfile, type, pSearchValue, phSearch);
}


/*
 * PROFILE_getAttributeValue()
 *
 * Obtain attribute values from the profile determined by
 * PROFILE_searchByAttribute() or PROFILE_searchByType().
 *
 * Parameters:
 *
 *         hProfile --
 *             Handle to a profile library, previously provided by
 *             PROFILE_open().
 *
 *         hSearch --
 *             Search Handle, previously provided by
 *             PROFILE_searchByAttribute() or PROFILE_searchByType().
 *
 *         pAttributeName --
 *             Name of the attribute whose value is desired.
 *
 *         ppValue --
 *             Pointer to the location in which a pointer to the character
 *             string representation of the requested value is placed.
 *
 * Returns:
 *
 *     Success
 *     Fail
 */
ReturnCode
PROFILE_getAttributeValue(void * hProfile,
			  void * hSearch,
			  char * pAttributeName,
			  char ** ppValue)
{
    return profile_getData(hProfile, hSearch, pAttributeName, ppValue);
}
