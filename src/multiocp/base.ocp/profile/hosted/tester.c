#include "estd.h"
#include "profile.h"


#define	ATTR_TYPE_STRING	(0x00000001)
#define	ATTR_TYPE_NUMBER	(0x00000002)
#define	ATTR_TYPE_IP_ADDR	(0x00000003)

typedef struct Attributes
{
    char * 	    pAttributeName;
    OS_Uint32	    attributeType;
} Attributes;

Attributes attributes[] =
{
    {
	"Full Name",
	ATTR_TYPE_STRING
    },
    {
	"Email-AUA",
	ATTR_TYPE_STRING
    },
    {
	"NNP-Subscriber Error Corrected",
	ATTR_TYPE_NUMBER
    },
    {
	"Default MTS",
	ATTR_TYPE_IP_ADDR
    },
    {
	"EMSD-AUA",
	ATTR_TYPE_STRING
    },
    {
	"Comment",
	ATTR_TYPE_STRING | PROFILE_MULTIVALUE
    },
    {
	NULL,
	NULL
    }
};

Attributes *	pAttributes;

char *		__applicationName;

FORWARD OS_Boolean
compare(char * pValue1, char * pValue2, OS_Uint32 type);

FORWARD void
printValues(void * hProfile,
	    void * hSearch);



int
main(int argc, char * argv[])
{
    ReturnCode	    rc;
    void *	    hProfile;
    void *	    hSearch;

    __applicationName = argv[0];

    /* Open the profile */
    if ((rc = PROFILE_open("tester.pro", &hProfile)) != Success)
    {
	fprintf(stderr,
		"%s: Could not open profile 'profile.ini', reason 0x%x\n",
		__applicationName, rc);
	return 1;
    }

    /* Add the attribute names and types */
    for (pAttributes = attributes;
	 pAttributes->pAttributeName != NULL;
	 pAttributes++)
    {
	if ((rc = PROFILE_addAttribute(hProfile,
				       pAttributes->pAttributeName,
				       pAttributes->attributeType,
				       compare)) != Success)
	{
	    fprintf(stderr,
		    "%s: addAttribute failed for attribute name (%s), "
		    "reason 0x%x\n",
		    __applicationName, pAttributes->pAttributeName, rc);
	    return 1;
	}
    }

    /* Find the profile for a particular user */
    if (PROFILE_searchByAttribute(hProfile,
				  attributes[0].pAttributeName,
				  "Kamran Ghane",
				  &hSearch) != Success)
    {
	fprintf(stderr,
		"%s: PROFILE_searchByAttribute could not locate "
		"Full Name = Kamran Ghane\n",
		__applicationName);
	return 1;
    }

    /* Now that we've found the correct profile, print its data. */
    printValues(hProfile, hSearch);
    
    /* Find profile for a particular user based on multi-valued attribute */
    if (PROFILE_searchByAttribute(hProfile,
				  "Comment",
				  "second entry",
				  &hSearch) != Success)
    {
	fprintf(stderr,
		"%s: PROFILE_searchByAttribute could not locate "
		"Comment <n> = second entry\n",
		__applicationName);
	return 1;
    }

    /* Now that we've found the correct profile, print its data. */
    printValues(hProfile, hSearch);

    /* Find the profile where any attribute of a type matches a value */
    if (PROFILE_searchByType(hProfile,
			     ATTR_TYPE_STRING,
			     "1.201.003.3@emsd.neda.com",
			     &hSearch) != Success)
    {
	fprintf(stderr,
		"%s: PROFILE_searchByType could not locate string "
		"(1.201.003.3@emsd.neda.com)\n",
		__applicationName);
	return 1;
    }

    /* Now that we've found the correct profile, print its data. */
    printValues(hProfile, hSearch);

    return 0;
}


OS_Boolean
compare(char * pValue1, char * pValue2, OS_Uint32 type)
{
    int		    i;
    struct
    {
	int 		len;
	long 		num[4];
    } values[2];

    type &= ~PROFILE_MULTIVALUE;

    if (type == ATTR_TYPE_NUMBER)
    {
	values[0].num[0] = atol(pValue1);
	values[0].num[1] = atol(pValue2);

	return (values[0].num[0] == values[0].num[1]);
    }
    else if (type == ATTR_TYPE_STRING)
    {
	return (strcmp(pValue1, pValue2) == 0);
    }
    else if (type == ATTR_TYPE_IP_ADDR)
    {
	values[0].len = sscanf(pValue1, "%ld.%ld.%ld.%ld",
			       &values[0].num[0], &values[0].num[1],
			       &values[0].num[2], &values[0].num[3]);
	values[1].len = sscanf(pValue1, "%ld.%ld.%ld.%ld",
			       &values[1].num[0], &values[1].num[1],
			       &values[1].num[2], &values[1].num[3]);

	if (values[0].len != values[1].len)
	{
	    return FALSE;
	}
	
	for (i = 0; i < values[0].len; i++)
	{
	    if (values[0].num[i] != values[1].num[i])
	    {
		return FALSE;
	    }
	}

	return TRUE;
    }
    else
    {
	/* unknown type */
	return FALSE;
    }
}


void
printValues(void * hProfile,
	    void * hSearch)
{
    ReturnCode	    rc;
    Attributes *    pAttributes;
    char *	    pValue;
    char	    attributeName[256];
    int		    i;
    
    for (pAttributes = attributes;
	 pAttributes->pAttributeName != NULL;
	 pAttributes++)
    {
	i = 0;

	do
	{
	    if (pAttributes->attributeType & PROFILE_MULTIVALUE)
	    {
		sprintf(attributeName, "%s %d",
			pAttributes->pAttributeName, ++i);
	    }
	    else
	    {
		strcpy(attributeName, pAttributes->pAttributeName);
	    }

	    if ((rc = PROFILE_getAttributeValue(hProfile,
						hSearch,
						attributeName,
						&pValue)) == Success)
	    {
		printf("%s = %s\n", attributeName, pValue);
	    }
	    else
	    {
		if ((pAttributes->attributeType & PROFILE_MULTIVALUE) == 0 ||
		    i == 1)
		{
		    printf("%s = <not found>\n", attributeName);
		}
	    }
	} while (rc == SUCCESS &&
		 (pAttributes->attributeType & PROFILE_MULTIVALUE));
    }
}
