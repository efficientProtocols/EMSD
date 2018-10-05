#include "estd.h"
#include "tm.h"
#include "tmr.h"
#include "buf.h"
#include "strfunc.h"
#include "mm.h"

/*
 * These macros are to avoid all-together too much typing!
 *
 * (It also helps it look like Erik's code! :-)
 */
#define	e	applicationEntities[entity]
#define	d	e.destinations[dest]
#define	m	e.modules[module]
#define	o	m.objects[object]


#define	MM_AGENT_PORT	2223
int	port = MM_AGENT_PORT;

#define	MAX_APPLICATION_ENTITIES		2
#define	MAX_DESTINATIONS_PER_APPL_ENTITY	50
#define	MAX_MODULES_PER_APPL_ENTITY		50
#define	MAX_OBJECTS_PER_MODULE			50

char *		__applicationName;

int			    numApplicationEntities;
struct
{
    char *			pName;

    int 			numDestinations;
    struct
    {
	char * 			    pName;
	char * 			    pDescription;
    } destinations[MAX_DESTINATIONS_PER_APPL_ENTITY];

    int 			numModules;
    struct
    {
	char * 			    pName;

	int 			    numObjects;
	struct
	{
	    MM_ManagableObjectType 	objectType;
	    char * 			pName;
	    char *			pDescription;
	} objects[MAX_OBJECTS_PER_MODULE];

    } modules[MAX_MODULES_PER_APPL_ENTITY];
} applicationEntities[MAX_APPLICATION_ENTITIES] = { { NULL } };


typedef struct PendingOperation
{
    QU_ELEMENT;

    OS_Uint8			invokeId;
    MM_RemoteOperationType	operationType;
    MM_RemoteOperationValue	operationValue;
    void *			hBuf;
} PendingOperation;

static QU_Head		pendingOperations = QU_INITIALIZE(pendingOperations);



int		agentFd = -1;

char		__get_buf[1024];

#define	GETSTRING(buf)				\
(						\
    fgets(buf, sizeof(buf), stdin),		\
    buf[strlen(buf) - 1] = '\0',		\
    (strlen(buf) == 0 ? NULL : buf)		\
)

#define	GETUL(ul)				\
(						\
    fgets(__get_buf, sizeof(__get_buf), stdin),	\
    __get_buf[strlen(__get_buf) - 1] = '\0',	\
    sscanf(__get_buf, "%lu", &ul),		\
    ul						\
)

#define	GETULX(ul)				\
(						\
    fgets(__get_buf, sizeof(__get_buf), stdin),	\
    __get_buf[strlen(__get_buf) - 1] = '\0',	\
    sscanf(__get_buf, "%lx", &ul),		\
    ul						\
)

#define	GETSL(sl)				\
(						\
    fgets(__get_buf, sizeof(__get_buf), stdin),	\
    __get_buf[strlen(__get_buf) - 1] = '\0',	\
    sscanf(__get_buf, "%ld", &sl),		\
    sl						\
)




ReturnCode
remOpSend(void ** phOperation,
	  MM_RemoteOperationType operationType,
	  MM_RemoteOperationValue operationValue,
	  void ** phBuf);

ReturnCode
remOpRecv(void ** phOperation,
	  MM_RemoteOperationType * pOperationType,
	  MM_RemoteOperationValue * pOperationValue,
	  void ** phBuf);

static void
doBuildLists(void);

static void
doGetValue(void);

static void
doSetValue(void);

static void
doGetThreshold(void);

static void
doSetThreshold(void);

static void
doStartTimer(void);

static void
doStopTimer(void);

static void
doGetObjectNotifyMask(void);

static void
doSetObjectNotifyMask(void);

static void
doGetDestinationMasks(void);

static void
doSetDestinationMasks(void);

static int
getEntity(void);

static int
getDestination(int * pEntity);

static int
getModule(int * pEntity);

static int
getObject(int * pEntity,
	  int * pModule);

static ReturnCode
managerInit(char * pHostName);

static void
getEvent(void);

static char *
rcString(ReturnCode rc);




int
main(int argc, char * argv[])
{
    int		    i;
    ReturnCode	    rc;
    signed long	    command;
    fd_set	    readFds;
    static void	 (* pfCommands[])(void) =
    {
	doGetValue,
	doSetValue,
	doGetThreshold,
	doSetThreshold,
	doStartTimer,
	doStopTimer,
	doGetObjectNotifyMask,
	doSetObjectNotifyMask,
	doGetDestinationMasks,
	doSetDestinationMasks
    };
    
    __applicationName = argv[0];

    TM_INIT();

    while ((i = getopt(argc, argv, "T:p:")) != EOF)
    {
        switch (i)
	{
	case 'T':
	    TM_SETUP(optarg);
	    break;

	case 'p':
	    port = atoi(optarg);
	    break;

	default:
	  usage:
	    printf("usage: %s [-T <tm flags>] ... <agent host name>\n",
		   __applicationName);
	    return 1;
	}
    }

    argc -= optind;
    argv += optind;

    /* There must be one more argument (the agent host name) */
    if (argc != 1)
    {
	goto usage;
    }

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    /* Register this application entity instance */
    if ((rc = MM_managerInit(remOpSend, remOpRecv)) != Success)
			     
    {
	printf("MM_managerInit() failed, reason %s\n", rcString(rc));
	return 1;
    }

    /* Connect to our agent */
    if ((rc = managerInit(argv[0])) != Success)
    {
	printf("Could not connect to agent, reason %s\n", rcString(rc));
	return 1;
    }

    /*
     * Build the lists of application entities, destinations, modules,
     * and objects.
     */
    for (;;)
    {
	printf("Command options:\n");

#define	p(s)	printf("\t%s\n", s)

	p("0.  Exit");
	p("1.  Get a value");
	p("2.  Set a value");
	p("3.  Get a threshold");
	p("4.  Set a threshold");
	p("5.  Start a timer");
	p("6.  Stop a timer.");
	p("7.  Get Object's Notify Mask");
	p("8.  Set Object's Notify Mask");
	p("9.  Get Destination's Masks");
	p("10. Set Destination's Masks");

#undef p

	printf("Enter a command number: ");

	for (;;)
	{
	    /* Set up to wait for keyboard input or from elsewhere */
	    FD_ZERO(&readFds);

	    /* First, see if there are pending operations. */
	    if (! QU_EQUAL(QU_FIRST(&pendingOperations), &pendingOperations))
	    {
		/* There are.  Don't block. */
		FD_SET(agentFd, &readFds);
		break;
	    }

	    /* Now set up to wait for keyboard input */
	    FD_ZERO(&readFds);
	    FD_SET(fileno(stdin), &readFds);

	    /* Set up to wait for agent input */
	    if (agentFd >= 0)
	    {
		FD_SET(agentFd, &readFds);
	    }

	    /* Wait (indefinitely) for input from somewhere */
	    i = select(FD_SETSIZE, &readFds, NULL, NULL, NULL);

	    if (i < 0)
	    {
		printf("select failed, errno=%d\n", errno);
		return 1;
	    }
	    else
	    {
		break;
	    }
	}
		
	if (FD_ISSET(fileno(stdin), &readFds))
	{
	    GETSL(command);

	    /* Decrement command to make it zero-relative */
	    --command;

	    /* Make sure it's a legal command.  If they entered '0', exit */
	    if (command < 0)
	    {
		return 0;
	    }

	    if (command >= DIMOF(pfCommands))
	    {
		printf("Invalid command number.\n");
		continue;
	    }

	    doBuildLists();

	    (* pfCommands[command])();
	}

	if (agentFd >= 0 && FD_ISSET(agentFd, &readFds))
	{
	    getEvent();
	}
    }

    return 0;
}


ReturnCode
remOpSend(void ** phOperation,
	  MM_RemoteOperationType operationType,
	  MM_RemoteOperationValue operationValue,
	  void ** phBuf)
{
    char *			pMemStart = NULL;
    char *			pMem;
    char *			p;
    void *			hBuf;
    OS_Uint8			invokeId;
    OS_Uint16			chunkLen;
    ReturnCode			rc;
    static unsigned long	requestInvokeId = 0;

    /* If we weren't provided with a buffer, allocate one */
    if (*phBuf == NULL)
    {
	if ((rc = BUF_alloc(0, phBuf)) != Success)
	{
	    printf("Could not allocate empty buffer\n");
	    return rc;
	}
    }

    /* Get a handle to the buffer */
    hBuf = *phBuf;

    /* Add the operation value to the PDU */
    if ((rc = BUF_addOctet(hBuf, operationValue)) != Success)
    {
	printf("Could not add operation value to result\n");
	return rc;
    }

    /* Add the operation type to the PDU */
    if ((rc = BUF_addOctet(hBuf, operationType)) != Success)
    {
	printf("Could not add operation type to result\n");
	return rc;
    }

    /* Add the invoke id (8 bits in this simple implementation) to the PDU */
    if (*phOperation == NULL)
    {
	invokeId = requestInvokeId++;
	*phOperation = (void *) ((OS_Uint32) invokeId);
    }
    else
    {
	invokeId = (OS_Uint8) ((OS_Uint32) *phOperation);
    }
    if ((rc = BUF_addOctet(hBuf, invokeId)) != Success)
    {
	printf("Could not add invoke id to result\n");
	return rc;
    }

    /* Allocate contiguous memory the size of our buffer contents */
    if ((pMemStart =
	 OS_alloc(BUF_getBufferLength(hBuf))) == NULL)
    {
	printf("Could not allocate contiguous memory\n");
	return ResourceError;
    }
	
    /* For each buffer segment... */
    for (pMem = pMemStart; ; pMem += chunkLen)
    {
	/* ... get the segment data, ... */
	chunkLen = 0;
	if ((rc = BUF_getChunk(hBuf,
			       &chunkLen,
			       (unsigned char **) &p)) != Success)
	{
	    if (rc == BUF_RC_BufferExhausted)
	    {
		/* Normal loop exit; no more segments. */
		break;
	    }
		
	    printf("BUF_getChunk failed, reason %s\n", rcString(rc));
	    OS_free(pMemStart);
	    return rc;
	}
	    
	/* ... and copy it into our contiguous memory. */
	OS_copy(pMem, (unsigned char *) p, chunkLen);
    }

    /* Write the PDU */
    if (write(agentFd, pMemStart, BUF_getBufferLength(hBuf)) < 0)
    {
	printf("Could not write result\n");
	OS_free(pMemStart);
	return Fail;
    }

    OS_free(pMemStart);

    return Success;
}


ReturnCode
remOpRecv(void ** phOperation,
	  MM_RemoteOperationType * pOperationType,
	  MM_RemoteOperationValue * pOperationValue,
	  void ** phBuf)
{
    int 			len;
    char 			buf[4096];
    OS_Uint8			invokeId;
    OS_Uint8			requestedInvokeId = 0;
    OS_Uint8 			operationType;
    OS_Uint8 			operationValue;
    STR_String			hString = NULL;
    void *			hBuf = NULL;
    ReturnCode			rc = Success;
    PendingOperation *		pPendingOperation;

    /* If they asked for any ol' operation and we have some pending... */
    pPendingOperation = QU_FIRST(&pendingOperations);
    if (*phOperation == NULL &&
	! QU_EQUAL(pPendingOperation, &pendingOperations))
    {
	/* ... remove it from the queue, ... */
	QU_REMOVE(pPendingOperation);

	/* ... give 'em what they came for, ... */
	*phOperation = (void *) ((OS_Uint32) pPendingOperation->invokeId);
	*pOperationType = pPendingOperation->operationType;
	*pOperationValue = pPendingOperation->operationValue;
	*phBuf = pPendingOperation->hBuf;

	/* ... and free the pending operation. */
	OS_free(pPendingOperation);

	return Success;
    }

    /* If they asked for a specific operation, and we have some pending... */
    if (*phOperation != NULL)
    {
	/* Find out what the requested invoke id was */
	requestedInvokeId = (OS_Uint8) ((OS_Uint32) *phOperation);

	/* Search the queue for this operation id */
	for (pPendingOperation = QU_FIRST(&pendingOperations);
	     ! QU_EQUAL(pPendingOperation, &pendingOperations);
	     pPendingOperation = QU_NEXT(pPendingOperation))
	{
	    /* Is this the one we're looking for? */
	    if (pPendingOperation->invokeId == requestedInvokeId)
	    {
		/* Yup.  Remove it from the queue. */
		QU_REMOVE(pPendingOperation);

		/* Give 'em what they came for */
		*phOperation =
		    (void *) ((OS_Uint32) pPendingOperation->invokeId);
		*pOperationType = pPendingOperation->operationType;
		*pOperationValue = pPendingOperation->operationValue;
		*phBuf = pPendingOperation->hBuf;

		/* ... and free the pending operation. */
		OS_free(pPendingOperation);

		return Success;
	    }
	}
    }

    /* Ok, it wasn't pending.  Wait for it. */
    for (;;)
    {
	/* Read the message */
	if ((len = read(agentFd, buf, sizeof(buf))) < 0)
	{
	    perror("reading message");
	    close(agentFd);
	    exit(1);
	}
	else if (len == 0)
	{
	    /* Connection closed. */
	    close(agentFd);
	    exit(0);
	}
	else
	{
	    /* Attach the data memory to one of our string handles */
	    if ((rc = STR_attachString(len, len, buf,
				       FALSE, &hString)) != Success)
	    {
		printf("STR_AttachString() failed, reason %s\n", rcString(rc));
		goto cleanUp;
	    }
	
	    /* Allocate a buffer to which we'll attach this string */
	    if ((rc = BUF_alloc(0, &hBuf)) != Success)
	    {
		printf("BUF_alloc() failed, reason %s\n", rcString(rc));
		goto cleanUp;
	    }
	
	    /* Attach the string to a buffer so we can parse it */
	    if ((rc = BUF_appendChunk(hBuf, hString)) != Success)
	    {
		printf("BUF_appendChunk() failed, reason %s\n", rcString(rc));
		goto cleanUp;
	    }

	    /*
	     * Get the invoke id (ignored in this simple implementation),
	     * operation type, and operation value.
	     */
	    if ((rc = BUF_getOctet(hBuf, &invokeId)) != Success ||
		(rc = BUF_getOctet(hBuf, &operationType)) != Success ||
		(rc = BUF_getOctet(hBuf, &operationValue)) != Success)
	    {
		printf("BUF_getOctet() failed, reason %s\n", rcString(rc));
		goto cleanUp;
	    }

	    /*
	     * If we were asked for any ol' operation, or if this received
	     * operation matches a specific one they asked for, give it to
	     * 'em.
	     */

	    if (*phOperation == NULL || requestedInvokeId == invokeId)
	    {
		*pOperationType = operationType;
		*pOperationValue = operationValue;
		*phBuf = hBuf;
		hBuf = NULL;
		*phOperation = (void *) ((OS_Uint32) invokeId);

		/* Get outta the loop. */
		break;
	    }
	    else
	    {
		/* This isn't a requested operation.  Enqueue it. */
		if ((pPendingOperation =
		     OS_alloc(sizeof(PendingOperation))) == NULL)
		{
		    /* Ignore it. */
		    rc = ResourceError;
		    goto cleanUp;
		}

		QU_INIT(pPendingOperation);

		pPendingOperation->invokeId = invokeId;
		pPendingOperation->operationType = operationType;
		pPendingOperation->operationValue = operationValue;
		pPendingOperation->hBuf = hBuf;

		QU_INSERT(pPendingOperation, &pendingOperations);
	    }
	}
    }

  cleanUp:

    if (hString != NULL)
    {
	STR_free(hString);
    }
	
    if (hBuf != NULL)
    {
	BUF_free(hBuf);
    }

    return rc;
}


static void
doBuildLists(void)
{
    ReturnCode			rc;
    int				entity;
    int				dest;
    int				module;
    int				object;
    char *			pList;
    char *			pStart;

    /* Free each of the strings currently in the lists */
    for (entity = 0; entity < numApplicationEntities; entity++)
    {
	OS_free(e.pName);

	for (dest = 0; dest < e.numDestinations; dest++)
	{
	    OS_free(d.pName);
	    OS_free(d.pDescription);
	}

	for (module = 0; module < e.numModules; module++)
	{
	    OS_free(m.pName);

	    for (object = 0; object < m.numObjects; object++)
	    {
		OS_free(o.pName);
		OS_free(o.pDescription);
	    }

	    /* Reset the object list counter */
	    m.numObjects = 0;
	}

	/* Reset the destination and module list counters */
	e.numDestinations = 0;
	e.numModules = 0;
    }

    /* Reset the application entity list counter */
    numApplicationEntities = 0;

    /* Get the list of application entities */
    if ((rc = MM_getApplicationEntityInstanceList(&pList)) != Success)
    {
	printf("MM_getApplicationEntityInstaceList() failed, reason %s\n",
	       rcString(rc));
	exit(1);
    }

    /*
     * Scan the list and separate it into its constituent strings.
     * Each string is terminated by a null character.  The end of the
     * list has two nulls in a row.
     */
    for (pStart = pList; strlen(pStart) > 0; pStart += strlen(pStart) + 1)
    {
	/* Add this string to the list */
	if ((applicationEntities[numApplicationEntities].pName =
	     OS_allocStringCopy(pStart)) == NULL)
	{
	    printf("Parse of application entity list failed, reason %s\n",
		   rcString(ResourceError));
	    exit(1);
	}

	/* We've added an application entity */
	++numApplicationEntities;
    }

    /* Free the list we were supplied with. */
    OS_free(pList);

    /* Get the list of modules in each application entity */
    for (entity = 0; entity < numApplicationEntities; entity++)
    {
	/* Get this application entity's module list */
	if ((rc = MM_getModuleList(e.pName, &pList)) != Success)
	{
	    printf("MM_getModuleList failed, reason %s\n", rcString(rc));
	    exit(1);
	}

	/*
	 * Scan the list and separate it into its constituent strings.
	 * Each string is terminated by a null character.  The end of
	 * the list has two nulls in a row.
	 */
	for (pStart = pList; strlen(pStart) > 0; pStart += strlen(pStart) + 1)
	{
	    /* Add this string to the list */
	    if ((e.modules[e.numModules].pName =
		 OS_allocStringCopy(pStart)) == NULL)
	    {
		printf("Parse of module list failed, reason %s\n",
		       rcString(ResourceError));
		exit(1);
	    }

	    /* We've added a module */
	    ++e.numModules;
	}

	/* Free the list we were supplied with. */
	OS_free(pList);
    }

    /* Get the list of objects in each module */
    for (entity = 0; entity < numApplicationEntities; entity++)
    {
	for (module = 0; module < e.numModules; module++)
	{
	    /* Get this module's managable object list */
	    if ((rc = MM_getManagableObjectList(e.pName,
						m.pName, &pList)) != Success)
	    {
		printf("MM_getManagableObjectList failed, reason %s\n",
		       rcString(rc));
		exit(1);
	    }

	    /*
	     * Scan the list and separate it into its constituent
	     * strings.  Each string is terminated by a null
	     * character.  The end of the list has two nulls in a row.
	     */

	    for (pStart = pList;
		 strlen(pStart) > 0;
		 pStart += strlen(pStart) + 1)
	    {
		object = m.numObjects;

		/* Add this string to the list */
		if ((o.pName =
		     OS_allocStringCopy(pStart)) == NULL)
		{
		    printf("Parse of managable object list failed, "
			   "reason %s\n",
			   rcString(ResourceError));
		    exit(1);
		}

		/* Get this object's type and description */
		if ((rc =
		     MM_getManagableObjectInfo(e.pName,
					       m.pName,
					       o.pName,
					       &o.objectType,
					       &o.pDescription)) != Success)
		{
		    printf("MM_getManagableObjectInfo() failed, reason %s\n",
			   rcString(rc));
		    exit(1);
		}
		
		/* We've added a managable object */
		++m.numObjects;
	    }

	    /* Free the list we were supplied with. */
	    OS_free(pList);
	}
    }
}


static void
doGetValue(void)
{
    int				entity;
    int				module;
    int				object;
    ReturnCode			rc;
    MM_Value			value;

    /* Get the indexes we need */
    if ((object = getObject(&entity, &module)) < 0)
    {
	return;
    }

    /* Make the request */
    if ((rc = MM_getValueByName(e.pName, m.pName, o.pName, &value)) != Success)
    {
	printf("MM_getValueByName() failed, reason %s\n", rcString(rc));
    }

    switch(value.type)
    {
    case MM_ValueType_SignedInt:
	printf("Current value (signed) is %ld\n", value.un.signedInt);
	break;

    case MM_ValueType_UnsignedInt:
	printf("Current value (unsigned) is %lu\n", value.un.unsignedInt);
	break;

    case MM_ValueType_String:
	printf("Current value (string) is \"%s\"\n", value.un.string);
	OS_free(value.un.string);
	break;
    }
}


static void
doSetValue(void)
{
    int				entity;
    int				module;
    int				object;
    ReturnCode			rc;
    MM_Value			value;
    signed long			sl;
    unsigned long		ul;
    char			string[128];

    /* Get the indexes we need */
    if ((object = getObject(&entity, &module)) < 0)
    {
	return;
    }

    switch(o.objectType)
    {
    case MM_ManagableObjectType_CounterSigned:
    case MM_ManagableObjectType_GaugeSigned:
	printf("Enter the new value (signed): ");
	sl = GETSL(sl);
	value.type = MM_ValueType_SignedInt;
	value.un.signedInt = sl;
	break;

    case MM_ManagableObjectType_CounterUnsigned:
    case MM_ManagableObjectType_GaugeUnsigned:
	printf("Enter the new value (unsigned): ");
	ul = GETUL(ul);
	value.type = MM_ValueType_UnsignedInt;
	value.un.unsignedInt = ul;
	break;

    case MM_ManagableObjectType_String:
	printf("Enter the new value (string): ");
	GETSTRING(string);
	value.type = MM_ValueType_String;
	value.un.string = string;
	break;

    case MM_ManagableObjectType_Timer:
    case MM_ManagableObjectType_Log:
	/* Illegal request. Just to test the remote interface, pass garbage */
	value.type = MM_ValueType_UnsignedInt;
	value.un.unsignedInt = 23;
	break;
    }

    /* Make the request */
    if ((rc = MM_setValueByName(e.pName, m.pName, o.pName, &value)) != Success)
    {
	printf("MM_setValueByName() failed, reason %s\n", rcString(rc));
	return;
    }

    printf("Done.\n");
}


static void
doGetThreshold(void)
{
    int				entity;
    int				module;
    int				object;
    signed long			sl;
    ReturnCode			rc;
    MM_ThresholdType		thresholdType;
    MM_Value			value;

    /* Get the indexes we need */
    if ((object = getObject(&entity, &module)) < 0)
    {
	return;
    }

    /* Which threshold is requested */
    printf("Threshold Types:\n");
    printf("%6d. Abort request\n", 0);
    printf("%6d. Minimum Threshold\n", 1);
    printf("%6d. Maximum Threshold\n", 2);
    printf("%6d. Minimum Absolute\n", 3);
    printf("%6d. Maximum Absolute\n", 4);
    printf("Select a threshold type: ");
    sl = GETSL(sl);

    switch(sl)
    {
    case 1:
	thresholdType = MM_ThresholdType_MinimumThreshold;
	break;

    case 2:
	thresholdType = MM_ThresholdType_MaximumThreshold;
	break;

    case 3:
	thresholdType = MM_ThresholdType_MinimumAbsolute;
	break;

    case 4:
	thresholdType = MM_ThresholdType_MaximumAbsolute;
	break;

    default:
	return;
    }
	
    /* Make the request */
    if ((rc = MM_getThresholdByName(e.pName, m.pName,
				    o.pName, thresholdType,
				    &value)) != Success)
    {
	printf("MM_getThresholdByName() failed, reason %s\n", rcString(rc));
    }
    
    switch(value.type)
    {
    case MM_ValueType_SignedInt:
	printf("Current %s value (signed) is %ld\n",
	       (thresholdType == MM_ThresholdType_MaximumThreshold ?
		"Maximum Threshold" :
		(thresholdType == MM_ThresholdType_MinimumThreshold ?
		 "Minimum Threshold" :
		 (thresholdType == MM_ThresholdType_MaximumAbsolute ?
		  "Maximum Absolute" :
		  (thresholdType == MM_ThresholdType_MinimumAbsolute ?
		   "Minimum Absolute" :
		   "UNKNOWN THRESHOLD TYPE")))),
	       value.un.signedInt);
	break;

    case MM_ValueType_UnsignedInt:
	printf("Current %s value (unsigned) is %lu\n",
	       (thresholdType == MM_ThresholdType_MaximumThreshold ?
		"Maximum Threshold" :
		(thresholdType == MM_ThresholdType_MinimumThreshold ?
		 "Minimum Threshold" :
		 (thresholdType == MM_ThresholdType_MaximumAbsolute ?
		  "Maximum Absolute" :
		  (thresholdType == MM_ThresholdType_MinimumAbsolute ?
		   "Minimum Absolute" :
		   "UNKNOWN THRESHOLD TYPE")))),
	       value.un.unsignedInt);
	break;

    default:
	break;
    }
}


static void
doSetThreshold(void)
{
    int				entity;
    int				module;
    int				object;
    ReturnCode			rc;
    MM_ThresholdType		thresholdType;
    MM_Value			value;
    signed long			sl;
    unsigned long		ul;

    /* Get the indexes we need */
    if ((object = getObject(&entity, &module)) < 0)
    {
	return;
    }

    /* Which threshold is requested */
    printf("Threshold Types:\n");
    printf("%6d. Abort request\n", 0);
    printf("%6d. Minimum Threshold\n", 1);
    printf("%6d. Maximum Threshold\n", 2);
    printf("%6d. Minimum Absolute (illegal from manager)\n", 3);
    printf("%6d. Maximum Absolute (illegal from manager)\n", 4);
    printf("Select a threshold type: ");
    sl = GETSL(sl);

    switch(sl)
    {
    case 1:
	thresholdType = MM_ThresholdType_MinimumThreshold;
	break;

    case 2:
	thresholdType = MM_ThresholdType_MaximumThreshold;
	break;

    case 3:
	thresholdType = MM_ThresholdType_MinimumAbsolute;
	break;

    case 4:
	thresholdType = MM_ThresholdType_MaximumAbsolute;
	break;

    default:
	return;
    }
	
    switch(o.objectType)
    {
    case MM_ManagableObjectType_CounterSigned:
    case MM_ManagableObjectType_GaugeSigned:
	printf("Enter the new threshold (signed): ");
	sl = GETSL(sl);
	value.type = MM_ValueType_SignedInt;
	value.un.signedInt = sl;
	break;

    case MM_ManagableObjectType_CounterUnsigned:
    case MM_ManagableObjectType_GaugeUnsigned:
	printf("Enter the new threshold (unsigned): ");
	ul = GETUL(ul);
	value.type = MM_ValueType_UnsignedInt;
	value.un.unsignedInt = ul;
	break;

    case MM_ManagableObjectType_String:
    case MM_ManagableObjectType_Timer:
    case MM_ManagableObjectType_Log:
	/* Illegal request. Just to test the remote interface, pass garbage */
	value.type = MM_ValueType_UnsignedInt;
	value.un.unsignedInt = 23;
	break;
    }

    /* Make the request */
    if ((rc = MM_setThresholdByName(e.pName, m.pName,
				    o.pName, thresholdType,
				    &value)) != Success)
    {
	printf("MM_setThresholdByName() failed, reason %s\n", rcString(rc));
	return;
    }
    
    printf("Done.\n");
}


static void
doStartTimer(void)
{
    int				entity;
    int				module;
    int				object;
    ReturnCode			rc;
    unsigned long		ul;
    OS_Uint32			milliseconds;

    /* Get the indexes we need */
    if ((object = getObject(&entity, &module)) < 0)
    {
	return;
    }

    printf("Enter timer value (milliseconds): ");
    milliseconds = GETUL(ul);

    /* Make the request */
    if ((rc = MM_startTimerByName(e.pName, m.pName,
				  o.pName, milliseconds)) != Success)
    {
	printf("MM_startTimerByName() failed, reason %s\n", rcString(rc));
	return;
    }

    printf("Done.\n");
}


static void
doStopTimer(void)
{
    int				entity;
    int				module;
    int				object;
    ReturnCode			rc;

    /* Get the indexes we need */
    if ((object = getObject(&entity, &module)) < 0)
    {
	return;
    }

    /* Make the request */
    if ((rc = MM_stopTimerByName(e.pName, m.pName, o.pName)) != Success)
    {
	printf("MM_stopTimerByName() failed, reason %s\n", rcString(rc));
	return;
    }

    printf("Done.\n");
}


static void
doGetObjectNotifyMask(void)
{
    int				entity;
    int				module;
    int				object;
    ReturnCode			rc;
    OS_Uint32			notifyMask;

    /* Get the indexes we need */
    if ((object = getObject(&entity, &module)) < 0)
    {
	return;
    }

    /* Make the request */
    if ((rc = MM_getManagableObjectNotifyMaskByName(e.pName,
						    m.pName,
						    o.pName,
						    &notifyMask)) != Success)
    {
	printf("MM_getValueByName() failed, reason %s\n", rcString(rc));
    }

    printf("Object's notify mask is 0x%lx\n", notifyMask);
}


static void
doSetObjectNotifyMask(void)
{
    int				entity;
    int				module;
    int				object;
    ReturnCode			rc;
    unsigned long		ul;
    OS_Uint32			notifyMask;

    /* Get the indexes we need */
    if ((object = getObject(&entity, &module)) < 0)
    {
	return;
    }

    printf("Enter the object's new notify mask: 0x");
    notifyMask = GETULX(ul);

    /* Make the request */
    if ((rc = MM_setManagableObjectNotifyMaskByName(e.pName,
						    m.pName,
						    o.pName,
						    notifyMask)) != Success)
    {
	printf("MM_setManagableObjectNotifyMasksByName() failed, "
	       "reason %s\n", rcString(rc));
	return;
    }
    
    printf("Done.\n");
}


static void
doGetDestinationMasks(void)
{
    int				entity;
    int				dest;
    OS_Uint32			notifyMask;
    OS_Uint32			eventMask;
    ReturnCode			rc;

    /* Get the indexes we need */
    if ((dest = getDestination(&entity)) < 0)
    {
	return;
    }

    /* Make the request */
    if ((rc = MM_getDestinationMasksByName(e.pName,
					   d.pName,
					   &notifyMask,
					   &eventMask)) != Success)
    {
	printf("MM_getDestinationMasksByName() failed, reason %s\n",
	       rcString(rc));
	return;
    }

    printf("Notify Mask = 0x%lx,  Event Mask = 0x%lx\n",
	   notifyMask, eventMask);
}


static void
doSetDestinationMasks(void)
{
    int				entity;
    int				dest;
    OS_Uint32			notifyMask;
    OS_Uint32			eventMask;
    unsigned long		ul;
    ReturnCode			rc;

    /* Get the indexes we need */
    if ((dest = getDestination(&entity)) < 0)
    {
	return;
    }

    printf("Enter the destination's new notify mask: 0x");
    notifyMask = GETULX(ul);

    printf("Enter the destination's new event mask: 0x");
    eventMask = GETULX(ul);

    /* Make the request */
    if ((rc = MM_getDestinationMasksByName(e.pName,
					   d.pName,
					   &notifyMask,
					   &eventMask)) != Success)
    {
	printf("MM_getDestinationMasksByName() failed, reason %s\n",
	       rcString(rc));
	return;
    }

    printf("Done.\n");
}


static int
getEntity(void)
{
    int				entity;
    long 			sl;

    printf("Application Entities:\n");
    printf("%6d. Abort request.\n", 0);

    for (entity = 0; entity < numApplicationEntities; entity++)
    {
	printf("%6d. %s\n", entity + 1, e.pName);
    }

    printf("Select an application entity: ");
    sl = GETSL(sl);

    return (int) (sl - 1);
}


static int
getDestination(int * pEntity)
{
    int				dest;
    int				entity;
    long			sl;

    if ((entity = getEntity()) < 0)
    {
	return -1;
    }

    printf("Destinations:\n");
    printf("%6d. Abort request.\n", 0);

    for (dest = 0; dest < e.numDestinations; dest++)
    {
	printf("%6d. %s\n", dest + 1, d.pName);
    }

    printf("Select a destination: ");
    GETSL(sl);

    *pEntity = entity;
    return (int) (sl - 1);
}


static int
getModule(int * pEntity)
{
    int				module;
    int				entity;
    long			sl;

    if ((entity = getEntity()) < 0)
    {
	return -1;
    }

    printf("Modules:\n");
    printf("%6d. Abort request.\n", 0);

    for (module = 0; module < e.numModules; module++)
    {
	printf("%6d. %s\n", module + 1, m.pName);
    }

    printf("Select a module: ");
    GETSL(sl);

    *pEntity = entity;
    return (int) (sl - 1);
}


static int
getObject(int * pEntity,
	  int * pModule)
{
    int				object;
    int 			entity;
    int				module;
    long			sl;

    if ((module = getModule(&entity)) < 0)
    {
	return -1;
    }

    printf("Managable Objects:\n");
    printf("%6d. Abort request.\n", 0);

    for (object = 0; object < m.numObjects; object++)
    {
	printf("%6d. %s (%s)\n", object + 1, o.pName, o.pDescription);
    }

    printf("Select a managable object: ");
    GETSL(sl);

    *pEntity = entity;
    *pModule = module;
    return (int) (sl - 1);
}


static ReturnCode
managerInit(char * pHostName)
{
    struct sockaddr_in 		agent;
    struct hostent *		hp;
					     
    if ((agentFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("opening socket");
	return Fail;
    }

    agent.sin_family = AF_INET;

    if ((hp = gethostbyname(pHostName)) == NULL)
    {
	printf("Could not resolve host name (%s)\n", pHostName);
	return Fail;
    }
    OS_copy(&agent.sin_addr, hp->h_addr, hp->h_length);
    agent.sin_port = ntohs(port);

    if (connect(agentFd, (struct sockaddr *) &agent, sizeof(agent)) < 0)
    {
	perror("connecting socket");
	return Fail;
    }

    return Success;
}


static void
getEvent(void)
{
    MM_RemoteOperationType	operationType;
    MM_RemoteOperationValue	operationValue;
    MM_Event *			pEvent = NULL;
    ReturnCode			rc;
    void *			hOperation;
    void *			hBuf;

    /* This will handle asynchronous events, once they're handled remotely. */

    hOperation = NULL;

    if ((rc = remOpRecv(&hOperation,
			&operationType,
			&operationValue,
			&hBuf)) != Success)
    {
	printf("Remote Operation Received failed, reason %s\n",
	       rcString(rc));
	goto cleanUp;
    }

    if ((rc = MM_remoteOperationEvent(operationValue,
				      hBuf,
				      &pEvent)) != Success)
    {
	printf("Processing of remote operation failed, reason %s\n",
	       rcString(rc));
	goto cleanUp;
    }


    printf("\n\n===> EVENT\n"
	   "\tApplication Entity Instance Name = (%s)\n"
	   "\tModule Name = (%s)\n"
	   "\tObject Name = (%s)\n",
	   pEvent->pApplicationEntityInstanceName,
	   pEvent->pModuleName,
	   pEvent->pObjectName);

    switch(pEvent->type)
    {
    case MM_EventType_MaxThresholdExceededSigned:
	printf("\tEvent Type = MaxThresholdExceededSigned\n"
	       "\tThreshold = %ld\n"
	       "\tNew Value = %ld\n",
	       pEvent->un.maxThresholdExceededSigned.threshold,
	       pEvent->un.maxThresholdExceededSigned.newValue);
	break;
	
    case MM_EventType_MaxThresholdExceededUnsigned:
	printf("\tEvent Type = MaxThresholdExceededUnsigned\n"
	       "\tThreshold = %lu\n"
	       "\tNew Value = %lu\n",
	       pEvent->un.maxThresholdExceededUnsigned.threshold,
	       pEvent->un.maxThresholdExceededUnsigned.newValue);
	break;
	
    case MM_EventType_MinThresholdExceededSigned:
	printf("\tEvent Type = MinThresholdExceededSigned\n"
	       "\tThreshold = %ld\n"
	       "\tNew Value = %ld\n",
	       pEvent->un.minThresholdExceededSigned.threshold,
	       pEvent->un.minThresholdExceededSigned.newValue);
	break;
	
    case MM_EventType_MinThresholdExceededUnsigned:
	printf("\tEvent Type = MinThresholdExceededUnsigned\n"
	       "\tThreshold = %lu\n"
	       "\tNew Value = %lu\n",
	       pEvent->un.minThresholdExceededUnsigned.threshold,
	       pEvent->un.minThresholdExceededUnsigned.newValue);
	break;
	
    case MM_EventType_MaxThresholdReenteredSigned:
	printf("\tEvent Type = MaxThresholdReenteredSigned\n"
	       "\tThreshold = %ld\n"
	       "\tNew Value = %ld\n",
	       pEvent->un.maxThresholdReenteredSigned.threshold,
	       pEvent->un.maxThresholdReenteredSigned.newValue);
	break;
	
    case MM_EventType_MaxThresholdReenteredUnsigned:
	printf("\tEvent Type = MaxThresholdReenteredUnsigned\n"
	       "\tThreshold = %lu\n"
	       "\tNew Value = %lu\n",
	       pEvent->un.maxThresholdReenteredUnsigned.threshold,
	       pEvent->un.maxThresholdReenteredUnsigned.newValue);
	break;
	
    case MM_EventType_MinThresholdReenteredSigned:
	printf("\tEvent Type = MinThresholdReenteredSigned\n"
	       "\tThreshold = %ld\n"
	       "\tNew Value = %ld\n",
	       pEvent->un.minThresholdReenteredSigned.threshold,
	       pEvent->un.minThresholdReenteredSigned.newValue);
	break;
	
    case MM_EventType_MinThresholdReenteredUnsigned:
	printf("\tEvent Type = MinThresholdReenteredUnsigned\n"
	       "\tThreshold = %lu\n"
	       "\tNew Value = %lu\n",
	       pEvent->un.minThresholdReenteredUnsigned.threshold,
	       pEvent->un.minThresholdReenteredUnsigned.newValue);
	break;
	
    case MM_EventType_TimerExpired:
	printf("\tEvent Type = TimerExpired\n");
	break;
	
    case MM_EventType_LogMessage:
	printf("\tEvent Type = LogMessage:\n"
	       "\tMessage = (%s)\n",
	       pEvent->un.logMessage.pMessage);
	break;
	
    case MM_EventType_ValueChangedSigned:
	printf("\tEvent Type = ValueChangedSigned\n"
	       "\tNew Value = %ld\n",
	       pEvent->un.valueChangedSigned.newValue);
	break;
	
    case MM_EventType_ValueChangedUnsigned:
	printf("\tEvent Type = ValueChangedUnsigned\n"
	       "\tNew Value = %lu\n",
	       pEvent->un.valueChangedUnsigned.newValue);
	break;
	
    case MM_EventType_ValueChangedString:
	printf("\tEvent Type = ValueChangedString\n"
	       "\tNew Value = (%s)\n",
	       pEvent->un.valueChangedString.pNewValue);
	break;
	
    case MM_EventType_ManagableObjectChange:
	printf("\tEvent Type = ManagableObjectChange\n");
	break;
    }

  cleanUp:

    if (pEvent != NULL)
    {
	OS_free(pEvent);
    }
}


static char *
rcString(ReturnCode rc)
{
    static char		buf[64];

    switch(rc)
    {
    case Success:
	return "Success";
	
    case Fail:
	return "Fail";
	
    case ProgrammerError:
	return "ProgrammerError";
	
    case UnsupportedOption:
	return "UnsupportedOption";
	
    case ResourceError:
	return "ResourceError";
	
    case MM_RC_ThresholdNotSupported:
	return "ThresholdNotSupported";
	
    case MM_RC_MinimumThresholdNotSupported:
	return "MinimumThresholdNotSupported";
	
    case MM_RC_IncrementNotSupported:
	return "IncrementNotSupported";
	
    case MM_RC_WrongObjectType:
	return "WrongObjectType";
	
/*    case MM_RC_WrongValueType:
	return "WrongValueType";
	*/
    case MM_RC_NoSuchManagableObject:
	return "NoSuchManagableObject";
	
    case MM_RC_NoSuchModule:
	return "NoSuchModule";
	
    case MM_RC_NoSuchApplicationEntityInstance:
	return "NoSuchApplicationEntityInstance";
	
    case MM_RC_NoSuchDestination:
	return "NoSuchDestination";
	
    case MM_RC_ByNameReadAccessDenied:
	return "ByNameReadAccessDenied";
	
    case MM_RC_ByNameWriteAccessDenied:
	return "ByNameWriteAccessDenied";
	
    case MM_RC_NameAlreadyInUse:
	return "NameAlreadyInUse";
	
    case MM_RC_UnrecognizedRemoteOperationValue:
	return "UnrecognizedRemoteOperationValue";
	
    case MM_RC_UnexpectedRemoteOperationType:
	return "UnexpectedRemoteOperationType";
	
    case MM_RC_CommunicationFailure:
	return "CommunicationFailure";
	
    default:
	sprintf(buf, "\n\tUnrecognized return code: module %u, value %u\n",
		((unsigned int) rc >> 10) & 0x1f,
		(unsigned int) rc & 0x3ff);
	return buf;
    }
}
