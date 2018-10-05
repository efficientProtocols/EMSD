/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of Neda-EMSD. An implementation of RFC-2524.
 * 
 * Neda-EMSD is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Neda-EMSD; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:
 *
 */

#include "estd.h"
#include "config.h"
#include <sys/wait.h>

char *		    __applicationName;

int
main(int argc, char * argv[])
{
    int			c;
    int			pid;
    int			status;
    int			messageLength = 32;
    int			repeatCount = 1;
    void *		hConfig;
    char *		pFrom = NULL;
    char * 		pConfigDir;
    char *		pSection;
    char *		pQueueDir;
    char 		fileName[OS_MAX_FILENAME_LEN];
    char *		pMailer;
    ReturnCode		rc;
    enum
    {
	NotSpecified,
	Submit,
	Deliver
    }			submitOrDeliver = NotSpecified;

    /* Save the application name */
    __applicationName = argv[0];

    /* Initialize the OS module */
    if (OS_init() != Success)
    {
	fprintf(stderr,
		"%s: Could not initialize OS module",
		__applicationName);
    }

    /* Initialize the OS memory allocation debug routines */
    OS_allocDebugInit(NULL);

    /* See if there's a configuration direction in the environment */
    pConfigDir = getenv("EMSD_CONFIG_DIR");

    /* Make sure we know if we've been given the Mailer location */
    pMailer = NULL;

    /* Get the options from the command line */
    while ((c = getopt(argc, argv, "sdc:m:n:l:f:")) != EOF)
    {
        switch (c)
	{
	case 's':		/* Submit */
	    /* Make they haven't previously specified this */
	    if (submitOrDeliver != NotSpecified)
	    {
		goto usage;
	    }
	    submitOrDeliver = Submit;
	    break;

	case 'd':
	    /* Make they haven't previously specified this */
	    if (submitOrDeliver != NotSpecified)
	    {
		goto usage;
	    }
	    submitOrDeliver = Deliver;
	    break;
	    
	case 'c':		/* specify configuration directory */
	    pConfigDir = optarg;
	    break;

	case 'm':		/* Mailer location */
	    pMailer = optarg;
	    break;

	case 'n':		/* Repeat count */
#ifdef OS_TYPE_UNIX
	    repeatCount = atoi(optarg);
#else
	    fprintf(stderr,
		    "%s: "
		    "Warning: Repeat Count ignored; "
		    "Only used on Unix.",
		    __applicationName);
#define	fork()	(0)
#define	wait(n)
#endif
	    break;
	    
	case 'l':		/* Message Length */
	    messageLength = atoi(optarg);
	    break;

	case 'f':		/* From */
	    pFrom = optarg;
	    break;

	default:
	  usage:
	    fprintf(stderr,
		    "usage: "
		    "%s -s|-d -f <Sender> [<Optional Args>] <recipient> ...\n"
		    "\tOptional Args:\n"
		    "\t\t[-n <Repeat Count>]\n"
		    "\t\t[-c <Config Dir>]\n",
		    __applicationName);
	    return 1;
	}
    }

    /* Did they specify a Submit or Deliver flag? */
    if (submitOrDeliver == NotSpecified)
    {
	goto usage;
    }

    /* Did they specify a sender? */
    if (pFrom == NULL)
    {
	goto usage;
    }

    /* Generate the name of the configuration file */
    if (submitOrDeliver == Submit)
    {
	sprintf(fileName, "%s%c%s",
		pConfigDir, *OS_DIR_PATH_SEPARATOR, "emsd-ua.ini");
    }
    else
    {
	sprintf(fileName, "%s%c%s",
		pConfigDir, *OS_DIR_PATH_SEPARATOR, "emsd-mts.ini");
    }

    /* First, get a handle to the configuration data */
    if ((rc = CONFIG_open(fileName, &hConfig)) != Success)
    {
	fprintf(stderr,
		"%s: Could not open configuration file\n\t(%s)\n\t"
		"reason 0x%04x\n",
		__applicationName, fileName, rc);
	return 1;
    }

    /* Determine the configuration Section name, for new-message directory */
    if (submitOrDeliver == Submit)
    {
	pSection = "Submission";
    }
    else
    {
	pSection = "Delivery";
    }

    /* Get the new-message directory */
    if ((rc = CONFIG_getString(hConfig,
			       pSection,
			       "New Message Directory",
			       &pQueueDir)) != Success)
    {
	fprintf(stderr,
		"%s: Configuration parameter\n\t%s/%s\n"
		"\tnot found, reason 0x%04x\n",
		__applicationName,
		pSection,
		"New Message Directory",
		rc);
	return 1;
    }

    /*
     * We know that there were at least two mandatory flag parameters which
     * have been specified (meaning that the original argc must have been at
     * least three), so we're going to cheat and use this same argument list
     * for our call to the EMSD Mailer.  Update the argument start pointer to
     * three positions prior to the location of the recipient list.  We'll
     * then insert the mailer name and the arguments to the mailer in those
     * locations.
     */
    argv += optind - 3;

    /* Add the mailer name */
    argv[0] = "EMSD Mailer";

    /* Add the spool directory argument */
    argv[1] = "-s";
    argv[2] = pQueueDir;

    /* If no mailer was specified... */
    if (pMailer == NULL)
    {
	/*
	 * ... make an assumption about the directory structure.  Normally,
	 * the structure has
	 *
	 *         ${EMSD_RUNTIME_BASE}/config
	 *                 and
	 *         ${EMSD_RUNTIME_BASE}/bin
	 *
	 * So, we'll assume this.
	 */
	sprintf(fileName, "%s%c%s%c%s%c%s",
		pConfigDir,
		*OS_DIR_PATH_SEPARATOR,
		"..",
		*OS_DIR_PATH_SEPARATOR,
		"bin",
		*OS_DIR_PATH_SEPARATOR,
		"emsd-mailer");

	/* Point to the mailer name that we generated. */
	pMailer = fileName;
    }

    /*
     * We're now going to execute the mailer the number of times
     * specified in the repeat count.
     */
    while (repeatCount-- > 0)
    {
	/* Fork and execute the mailer */
	if ((pid = fork()) == 0)
	{
	    /*
	     * This is the child.  Execute the mailer.
	     */
	    if (execv(pMailer, argv) < 0)
	    {
		fprintf(stderr,
			"%s: Could not execute mailer (errno=%d):\n\t%s\n",
			__applicationName, errno, pMailer);
		return 1;
	    }
	}
	else if (pid < 0)
	{
	    fprintf(stderr,
		    "%s: Couldn't fork to execute mailer (errno=%d)\n",
		    __applicationName, errno);
	    return 1;
	}
	else
	{
	    wait((void *) &status);
	    if (status != 0)
	    {
		fprintf(stderr,
			"%s: Mailer returned exit code %d\n",
			__applicationName, status);
		return status;
	    }
	}
    }

    return 0;
}
