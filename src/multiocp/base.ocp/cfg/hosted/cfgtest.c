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

#include "estd.h"
#include "tm.h"
#include "eh.h"
#include "config.h"


/*<
 * Function:    main 
 *
 * Description: Config test
 *
 * Arguments:   argc, argv.
 *
 * Returns:     None.
 *
>*/

void
main(int argc, char * argv[])
{
    ReturnCode	    rc;
    void *	    hConfig;
    void *	    hNext;
    char *	    pType;
    char *	    pValue;
    char *	    pSection;
    OS_Uint32	    val;
    int		    c;
    char *	    pStr;
    char *	    pFileName = "cfgtest.ini";
    enum
    {
	String,
	Number,
	ListParameters,
	ListSections
    }		    option = String;

    OS_allocDebugInit(NULL);

    EH_init();
    TM_init();

    if (CONFIG_init() != Success)
    {
	fprintf(stderr, "CONFIG_init() failed.\n");
	exit(1);
    }

    while ((c = getopt(argc, argv, "f:nps")) != EOF)
    {
        switch (c)
	{
	case 'f':
	    pFileName = optarg;
	    break;

	case 'n':
	    option = Number;
	    break;

	case 'p':
	    option = ListParameters;
	    break;

	case 's':
	    option = ListSections;
	    break;

	default:
	  usage:
	    fprintf(stderr,
		    "usage:\n"
		    "\tcfgtest [-n] [-f <Config_file>] <Section> <Type>\n"
		    "\tcfgtest -p [-f <Config_file>] <Section>\n"
		    "\tcfgtest -s [-f <Config_file>]\n");
	    exit(1);
	}
    }

    argc -= optind;
    argv += optind;

    switch (option)
    {
    case String:
    case Number:
	if (argc != 2)
	{
	    goto usage;
	}
	break;

    case ListSections:
	if (argc != 0)
	{
	    goto usage;
	}
	break;

    case ListParameters:
	if (argc != 1)
	{
	    goto usage;
	}
	break;
    }

    if ((rc = CONFIG_open(pFileName, &hConfig)) != Success)
    {
	fprintf(stderr, "CONFIG_open() failed, reason 0x%x\n", rc);
	exit(1);
    }

    switch (option)
    {
    case ListParameters:
	printf("Section [%s]\n", argv[0]);

	/* Initialize parameter handle to find first parameter in section */
	hNext = NULL;

	/* For each parameter in the section... */
	while ((rc = CONFIG_nextParameter(hConfig,
					  argv[0],
					  &pType,
					  &pValue,
					  &hNext)) == Success)
	{
	    /* ... print its type and value. */
	    printf("    (%s) = (%s)\n", pType, pValue);
	}
	break;

    case ListSections:
	printf("Sections\n");

	/* Initialize parameter handle to find first section */
	hNext = NULL;

	/* We're not looking for any particular section */
	pSection = NULL;

	/* For  section... */
	while ((rc = CONFIG_nextSection(hConfig,
					&pSection,
					&hNext)) == Success)
	{
	    /* ... print its name. */
	    printf("    [%s]\n", pSection);

	    /* Again, we're not looking for any particular section */
	    pSection = NULL;
	}
	break;
	
    case Number:
	if ((rc = CONFIG_getNumber(hConfig,
				   argv[0], argv[1], &val)) != Success)
	{
	    fprintf(stderr, "CONFIG_getNumber failed, reason 0x%x\n", rc);
	    exit(1);
	}

	printf("[%s]/%s = (%lu)\n", argv[0], argv[1], val);
	break;

    case String:
	if ((rc = CONFIG_getString(hConfig,
				   argv[0], argv[1], &pStr)) != Success)
	{
	    fprintf(stderr, "CONFIG_getString failed, reason 0x%x\n", rc);
	    exit(1);
	}

	printf("[%s]/%s = (%s)\n", argv[0], argv[1], pStr);

	break;
    }

    EH_problem("Testing EH_*() exception file.  Ignore this problem.");

    CONFIG_close(hConfig);

    exit(0);
}
