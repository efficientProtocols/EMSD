#include "estd.h"
#include "tm.h"
#include "tmr.h"
#include "buf.h"
#include "strfunc.h"
#include "mm.h"

#define	MM_AGENT_PORT	2223
int	port = MM_AGENT_PORT;

void doRegisterModule(void);
void doRegisterManagableObject(void);
void doRegisterDestination(void);
void doGetThreshold(void);
void doSetThreshold(void);
void doGetValue(void);
void doSetValue(void);
void doIncrementValue(void);
void doStartTimer(void);
void doStopTimer(void);
void doLogMessage(void);
void doGetObjectNotifyMask(void);
void doSetObjectNotifyMask(void);
void doGetDestinationMasks(void);
void doSetDestinationMasks(void);
signed long getModuleNum(void);
signed long getObjectNum(void);
signed long getDestinationNum(void);
void screenAlert(char * pDestinationName,
		 char * pApplicationEntityInstanceName,
		 char * pModuleName,
		 char * pManagableObjectName,
		 char * pDescription,
		 MM_EventType eventType,
		 ...);
char * rcString(ReturnCode rc);
ReturnCode initAgent(void);
void addManager(void);
void processAgent(void);
void agentErrorResponse(OS_Uint8 operationValue, ReturnCode rc);
ReturnCode
issueOperation(void ** phOperation,
	       MM_RemoteOperationType operationType,
	       MM_RemoteOperationValue  operationValue,
	       void ** phBuf);
static void
valueChanged(void * hUserData,
	     void * hManagableObject,
	     MM_Value * pValue);



char *		__applicationName;

char		applicationEntityInstanceName[] = "mmtest";
char		idMsg[] = "This is the identification message for " ;
void * 		hApplicationEntityInstance;

int 			    numModules = 0;
struct
{
    void * 			handle;
    char * 			pName;
} modules[100] = { { NULL } };

int 			    numObjects = 0;
struct
{
    MM_ManagableObjectType 	objectType;
    void * 			handle;
    char * 			pName;
} objects[100] = { { NULL } };

int 			    numDestinations = 0;
struct
{
    void * 			handle;
    char * 			pName;
    void		     (* pfAlert)(char * pDestinationName,
					 char * pApplicationEntityInstanceName,
					 char * pModuleName,
					 char * pManagableObjectName,
					 char * pDescription,
					 MM_EventType eventType,
					 ...);
} destinations[2] =
  {
      { NULL, NULL, MM_agentDestination },
      { NULL, NULL, screenAlert }
  };





int		listenFd = -1;
int		messageFd = -1;

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



int
main(int argc, char * argv[])
{
    int		    i;
    ReturnCode	    rc;
    signed long	    command;
    fd_set	    readFds;
    struct timeval  timeval;
    static void	 (* pfCommands[])(void) =
    {
	doRegisterModule,
	doRegisterManagableObject,
	doRegisterDestination,
	doGetThreshold,
	doSetThreshold,
	doGetValue,
	doSetValue,
	doIncrementValue,
	doStartTimer,
	doStopTimer,
	doLogMessage,
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
	    printf("usage: %s [-T <tm flags>] ...\n", argv[0]);
	    break;
	}
    }

    argc -= optind;
    argv += optind;

    /* There are no more arguments to this program. */
    if (argc != 0)
    {
	goto usage;
    }

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    /* Register this application entity instance */
    if ((rc = MM_entityInit(applicationEntityInstanceName,
			    NULL, &hApplicationEntityInstance)) != Success)
    {
	printf("MM_init() failed, reason %s\n", rcString(rc));
	return 1;
    }

    /* Initialize the MM agent code */
    if ((rc = MM_agentInit(issueOperation)) != Success)
    {
	printf("MM_agentInit() failed, reason %s\n", rcString(rc));
	return 1;
    }

    /* Initialize the test agent */
    if ((rc = initAgent()) != Success)
    {
	printf("Could not initialize agent, reason %s\n", rcString(rc));
	return 1;
    }

    for (;;)
    {
	if ((rc =
	     TMR_processQueue(NULL)) != Success)
	{
	    printf("TMR_processQueue() failed, reason %s\n", rcString(rc));
	    return 1;
	}

	if ((rc =
	     MM_processEvents(hApplicationEntityInstance, NULL)) != Success)
	{
	    printf("MM_processEvents() failed, reason %s\n", rcString(rc));
	    return 1;
	}

	printf("Command options:\n");

#define	p(s)	printf("\t%s\n", s)

	p("0.  Exit");
	p("1.  Register a module");
	p("2.  Register a managable object");
	p("3.  Register a destination");
	p("4.  Get a threshold");
	p("5.  Set a threshold");
	p("6.  Get a value");
	p("7.  Set a value");
	p("8.  Increment a value");
	p("9.  Start a timer");
	p("10. Stop a timer.");
	p("11. Log a message");
	p("12. Get Object's Notify Mask");
	p("13. Set Object's Notify Mask");
	p("14. Get Destination's Masks");
	p("15. Set Destination's Masks");

#undef p

	printf("Enter a command number: ");

	for (;;)
	{
	    /* Set up to wait for keyboard input or from elsewhere */
	    FD_ZERO(&readFds);
	    FD_SET(fileno(stdin), &readFds);

	    if (listenFd >= 0)
	    {
		FD_SET(listenFd, &readFds);
	    }

	    if (messageFd >= 0)
	    {
		FD_SET(messageFd, &readFds);
	    }
	    
	    timeval.tv_usec = 0;
	    timeval.tv_sec = 1;

	    /* Wait for input from somewhere */
	    i = select(FD_SETSIZE, &readFds, NULL, NULL, &timeval);

	    if (i < 0)
	    {
		printf("select failed, errno=%d\n", errno);
		return 1;
	    }
	    else if (i == 0)
	    {
		if ((rc =
		     TMR_processQueue(NULL)) != Success)
		{
		    printf("TMR_processQueue() failed, reason %s\n",
			   rcString(rc));
		    return 1;
		}
		
		if ((rc =
		     MM_processEvents(hApplicationEntityInstance,
				      NULL)) != Success)
		{
		    printf("MM_processEvents() failed, reason %s\n",
			   rcString(rc));
		    return 1;
		}

		continue;
	    }
	    else
	    {
		break;
	    }
	}
		
	if (listenFd >= 0 && FD_ISSET(listenFd, &readFds))
	{
	    addManager();
	}

	if (messageFd >= 0 && FD_ISSET(messageFd, &readFds))
	{
	    processAgent();
	}

	if (! FD_ISSET(fileno(stdin), &readFds))
	{
	    continue;
	}

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

	(* pfCommands[command])();
    }

    return 0;
}


void doRegisterModule(void)
{
    char	    moduleName[32];
    ReturnCode	    rc;

    printf("+++ Register a module\n");

    if (numModules == DIMOF(modules))
    {
	printf("--- No more module handles available.\n");
	return;
    }

    printf("Enter the module name: ");

    if (GETSTRING(moduleName) == NULL)
    {
	printf("--- Invalid module name.\n");
	return;
    }

    if ((rc = MM_registerModule(hApplicationEntityInstance,
				moduleName,
				&modules[numModules].handle)) != Success)
    {
	printf("--- MM_registerModule() failed, reason %s\n", rcString(rc));
	return;
    }

    modules[numModules].pName = OS_allocStringCopy(moduleName);

    ++numModules;

    printf("--- Module (%s) registered.\n", moduleName);
}


void doRegisterManagableObject(void)
{
    int 			moduleNum;
    char 			objectName[32];
    char 			objectId[128];
    char			notify[8];
    MM_ManagableObjectType 	objectType;
    unsigned long 		ul;
    OS_Uint32 			notifyMask;
    OS_Uint8 			accessByName;
    ReturnCode 			rc;

    printf("+++ Register a managable object\n");

    if (numModules == 0)
    {
	printf("--- No modules have yet been registered.\n");
	return;
    }

    if (numObjects == DIMOF(objects))
    {
	printf("--- No more managable object handles available.\n");
	return;
    }

    if ((moduleNum = getModuleNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    printf("Object Types:\n");

#define	p(s)	printf("\t%s\n", s)

    p("1. Signed Counter");
    p("2. Unsigned Counter");
    p("3. Signed Guage");
    p("4. Unsigned Guage");
    p("5. String");
    p("6. Timer");
    p("7. Log");

#undef p

    printf("Select an object type: ");
    objectType = GETUL(ul);
    if (objectType < 1 || objectType > 7)
    {
	printf("--- Invalid object type.\n");
	return;
    }
    --objectType;

    printf("Enter the managable object name: ");
    if (GETSTRING(objectName) == NULL)
    {
	printf("--- Invalid object name.\n");
	return;
    }

    printf("Enter the managable object identification string: ");
    if (GETSTRING(objectId) == NULL)
    {
	printf("--- Invalid object identification string.\n");
	return;
    }

    printf("Enter the managable object initial notification mask: 0x");
    notifyMask = GETULX(ul);

    printf("Enter the managable object accessByName mode: 0x");
    accessByName = GETULX(ul);

    printf("Notify when value changed [y]? ");
    if (GETSTRING(notify) == NULL || *notify == 'y' || *notify == 'Y')
    {
	strcpy(notify, "y");
    }
    else
    {
	strcpy(notify, "n");
    }

    if ((rc =
	 MM_registerManagableObject(modules[moduleNum].handle,
				    objectType,
				    objectName,
				    objectId,
				    accessByName,
				    notifyMask,
				    *notify == 'y' ? valueChanged : NULL,
				    *notify == 'y' ? (void *)numObjects : NULL,
				    &objects[numObjects].handle)) != Success)
    {
	printf("--- MM_registerModule() failed, reason %s\n", rcString(rc));
	return;
    }

    objects[numObjects].pName = OS_allocStringCopy(objectName);
    objects[numObjects].objectType = objectType;

    ++numObjects;

    printf("--- Managable object (%s) registered.\n", objectName);
}


void doRegisterDestination(void)
{
    unsigned long   ul;
    OS_Uint32	    notifyMask;
    OS_Uint32	    eventMask;
    OS_Uint8	    accessByName;
    char	    destinationName[32];
    char	    destinationId[64];
    ReturnCode	    rc;

    printf("+++ Register destination\n");

    if (numDestinations == DIMOF(destinations))
    {
	printf("--- No more destination handles available.\n");
	return;
    }

    printf("Enter the destination name: ");
    if (GETSTRING(destinationName) == NULL)
    {
	printf("--- Invalid destination name.\n");
	return;
    }

    printf("Enter the destination identification string: ");
    if (GETSTRING(destinationId) == NULL)
    {
	printf("--- Invalid destination identification string.\n");
	return;
    }

    printf("Enter the destination initial notification mask: 0x");
    notifyMask = GETULX(ul);

    printf("Enter the destination initial event mask: 0x");
    eventMask = GETULX(ul);

    printf("Enter the managable object accessByName mode: 0x");
    accessByName = GETULX(ul);

    if ((rc =
	 MM_registerDestination(hApplicationEntityInstance,
				destinationName,
				destinationId,
				accessByName,
				notifyMask,
				eventMask,
				destinations[numDestinations].pfAlert,
				&destinations[numDestinations].handle)) !=
	Success)
    {
	printf("--- MM_registerModule() failed, reason %s\n", rcString(rc));
	return;
    }

    destinations[numDestinations].pName = OS_allocStringCopy(destinationName);

    ++numDestinations;

    printf("--- Destination (%s) registered\n", destinationName);
}


void doGetThreshold(void)
{
    int 			objectNum;
    MM_Value			value;
    unsigned long 		sl;
    ReturnCode 			rc;
    MM_ThresholdType 		thresholdType;

    printf("+++ Get Threshold\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    printf("Threshold Types:\n");
    printf("\t1. Minimum Threshold\n");
    printf("\t2. Maximum Threshold\n");
    printf("\t3. Minimum Absolute\n");
    printf("\t4. Maximum Absolute\n");
    printf("Select the threshold type to be retrieved: ");
    switch(GETSL(sl))
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
	printf("--- Invalid threshold type number specified.\n");
	return;
    }

    if ((rc = MM_getThresholdByHandle(objects[objectNum].handle,
				      thresholdType,
				      &value)) != Success)
    {
	printf("--- MM_getThresholdByValue() failed, reason %s\n",
	       rcString(rc));
	return;
    }

    if (value.type == MM_ValueType_SignedInt)
    {
	printf("--- MM_getThresholdByValue() "
	       "got signed threshold value %ld\n",
	       value.un.signedInt);
    }
    else
    {
	if ((rc = MM_getThresholdByHandle(objects[objectNum].handle,
					  thresholdType,
					  &value)) != Success)
	{
	    printf("--- MM_getThresholdByValue() failed, reason %s\n",
		   rcString(rc));
	    return;
	}

	printf("--- MM_getThresholdByValue() "
	       "got unsigned threshold value %lu\n",
	       value.un.unsignedInt);
    }
}


void doSetThreshold(void)
{
    int 			objectNum;
    signed long 		sl;
    unsigned long 		ul;
    MM_Value			value;
    ReturnCode 			rc;
    MM_ThresholdType 		thresholdType;
    MM_ManagableObjectType	objectType;

    printf("+++ Get Threshold\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    printf("Threshold Types:\n");
    printf("\t1. Minimum Threshold\n");
    printf("\t2. Maximum Threshold\n");
    printf("\t3. Minimum Absolute\n");
    printf("\t4. Maximum Absolute\n");
    printf("Select the threshold type to be set: ");
    switch(GETSL(sl))
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
	printf("--- Invalid threshold type number specified.\n");
	return;
    }

    objectType = objects[objectNum].objectType;
    if (objectType == MM_ManagableObjectType_CounterSigned ||
	objectType == MM_ManagableObjectType_GaugeSigned)
    {
	printf("Enter the threshold value (signed): ");
	value.type = MM_ValueType_SignedInt;
	value.un.signedInt = GETSL(sl);

	if ((rc = MM_setThresholdByHandle(objects[objectNum].handle,
					  thresholdType,
					  &value)) != Success)
	{
	    printf("--- MM_setThresholdByValue() failed, reason %s\n",
		   rcString(rc));
	    return;
	}
    }
    else
    {
	printf("Enter the threshold value (unsigned): ");
	value.type = MM_ValueType_UnsignedInt;
	value.un.unsignedInt = GETUL(ul);

	if ((rc = MM_setThresholdByHandle(objects[objectNum].handle,
					  thresholdType,
					  &value)) != Success)
	{
	    printf("--- MM_setThresholdByValue() failed, reason %s\n",
		   rcString(rc));
	    return;
	}
    }

    printf("--- Set Threshold done.\n");
}


void doGetValue(void)
{
    int 		objectNum;
    MM_Value		value;
    ReturnCode 		rc;

    printf("+++ Get Value\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    if ((rc =
	 MM_getValueByHandle(objects[objectNum].handle, &value)) != Success)
    {
	printf("--- MM_getValueByHandle() failed, reason %s\n", rcString(rc));
	return;
    }

    switch(objects[objectNum].objectType)
    {
    case MM_ManagableObjectType_CounterSigned:
    case MM_ManagableObjectType_GaugeSigned:
	printf("--- Got signed value %ld\n", value.un.signedInt);
	break;
	
    case MM_ManagableObjectType_CounterUnsigned:
    case MM_ManagableObjectType_GaugeUnsigned:
	printf("--- Got unsigned value %lu\n", value.un.unsignedInt);
	break;
	
    case MM_ManagableObjectType_String:
	printf("--- Got string value (%s)\n", value.un.string);
	break;
	
    case MM_ManagableObjectType_Timer:
    case MM_ManagableObjectType_Log:
	printf("--- ERROR: how did we get here?\n");
	break;
    }
}


void doSetValue(void)
{
    int 		objectNum;
    MM_Value		value;
    signed long		sl;
    unsigned long	ul;
    char		buf[256];
    ReturnCode 		rc;

    printf("+++ Set Value\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    switch(objects[objectNum].objectType)
    {
    case MM_ManagableObjectType_CounterSigned:
    case MM_ManagableObjectType_GaugeSigned:
	printf("Enter a new signed value for this object: ");
	value.type = MM_ValueType_SignedInt;
	value.un.signedInt = GETSL(sl);
	break;
	
    case MM_ManagableObjectType_CounterUnsigned:
    case MM_ManagableObjectType_GaugeUnsigned:
	printf("Enter a new unsigned value for this object: ");
	value.type = MM_ValueType_UnsignedInt;
	value.un.unsignedInt = GETUL(ul);
	break;
	
    case MM_ManagableObjectType_String:
	printf("Enter a new string value for this object: ");
	value.type = MM_ValueType_String;
	value.un.string = GETSTRING(buf);
	break;
	
    case MM_ManagableObjectType_Timer:
    case MM_ManagableObjectType_Log:
	break;
    }
    
    if ((rc =
	 MM_setValueByHandle(objects[objectNum].handle, &value)) != Success)
    {
	printf("--- MM_setValueByHandle() failed, reason %s\n", rcString(rc));
	return;
    }

    printf("--- Set Value done.\n");
}


void doIncrementValue(void)
{
    int 		objectNum;
    OS_Sint32 		incrementBy;
    signed long		sl;
    ReturnCode 		rc;

    printf("+++ Increment Value\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    printf("By what value should the object be incremented (signed): ");
    incrementBy = GETSL(sl);

    if ((rc =
	 MM_incrementValue(objects[objectNum].handle, incrementBy)) != Success)
    {
	printf("--- MM_incrementValue() failed, reason %s\n", rcString(rc));
	return;
    }

    printf("--- Increment Value done.\n");
}


void doStartTimer(void)
{
    int 		objectNum;
    OS_Sint32 		milliseconds;
    signed long		sl;
    ReturnCode 		rc;

    printf("+++ Start Timer\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    printf("Timer value (in milliseconds): ");
    milliseconds = GETSL(sl);

    if ((rc = MM_startTimerByHandle(objects[objectNum].handle,
				    milliseconds)) != Success)
    {
	printf("--- MM_startTimerByHandle() failed, reason %s\n",
	       rcString(rc));
	return;
    }

    printf("--- Timer started.\n");
}


void doStopTimer(void)
{
    int 		objectNum;
    ReturnCode 		rc;

    printf("+++ Stop Timer\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    if ((rc = MM_stopTimerByHandle(objects[objectNum].handle)) != Success)
    {
	printf("--- MM_stopTimerByHandle() failed, reason %s\n", rcString(rc));
	return;
    }

    printf("--- Timer stoped.\n");
}


void doLogMessage(void)
{
    int 		objectNum;
    ReturnCode 		rc;
    OS_Uint32		currentDateTime;
    char		buf[128];

    printf("+++ Log Message\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    (void) OS_currentDateTime(NULL, &currentDateTime);

    if ((rc =
	 MM_logMessage(objects[objectNum].handle,
		       buf, "%s: Log message test.",
		       OS_printableDateTime(&currentDateTime))) != Success)
    {
	printf("--- MM_logMessage() failed, reason %s\n", rcString(rc));
	return;
    }

    printf("--- Log Message done.\n");
}


void doGetObjectNotifyMask(void)
{
    int 		objectNum;
    ReturnCode 		rc;
    OS_Uint32		notifyMask;

    printf("+++ Get Object Notify Mask\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    if ((rc =
	 MM_getManagableObjectNotifyMaskByHandle(objects[objectNum].handle,
						 &notifyMask)) != Success)
    {
	printf("--- MM_getManagableObjectNotifyMaskByHandle() failed, "
	       "reason %s\n", rcString(rc));
	return;
    }

    printf("--- Get Object Notify Mask = 0x%lx\n", (unsigned long) notifyMask);
}


void doSetObjectNotifyMask(void)
{
    int 		objectNum;
    unsigned long	ul;
    OS_Uint32		notifyMask;
    ReturnCode 		rc;

    printf("+++ Set Object Notify Mask\n");

    if (numObjects == 0)
    {
	printf("--- No objects have yet been registered.\n");
	return;
    }

    if ((objectNum = getObjectNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    printf("Enter a new notify mask: 0x");
    notifyMask = GETULX(ul);

    if ((rc =
	 MM_setManagableObjectNotifyMaskByHandle(objects[objectNum].handle,
						 notifyMask)) != Success)
    {
	printf("--- MM_setManagableObjectNotifyMaskByHandle() failed, "
	       "reason %s\n", rcString(rc));
	return;
    }

    printf("--- Set Object Notify Mask done.\n");
}


void doGetDestinationMasks(void)
{
    int 		destNum;
    ReturnCode 		rc;
    OS_Uint32		notifyMask;
    OS_Uint32		eventMask;

    printf("+++ Get Destination Notify Mask\n");

    if (numDestinations == 0)
    {
	printf("--- No destinations have yet been registered.\n");
	return;
    }

    if ((destNum = getDestinationNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    if ((rc =
	 MM_getDestinationMasksByHandle(destinations[destNum].handle,
					&notifyMask,
					&eventMask)) != Success)
    {
	printf("--- MM_getDestinationNotifyMaskByHandle() failed, "
	       "reason %s\n", rcString(rc));
	return;
    }

    printf("--- Get Destination: NotifyMask=0x%lx  EventMask=0x%lx\n",
	   (unsigned long) notifyMask, (unsigned long) eventMask);
}


void doSetDestinationMasks(void)
{
    int 		destNum;
    ReturnCode 		rc;
    unsigned long	ul;
    OS_Uint32		notifyMask;
    OS_Uint32		eventMask;

    printf("+++ Set Destination Notify Mask\n");

    if (numDestinations == 0)
    {
	printf("--- No destinations have yet been registered.\n");
	return;
    }

    if ((destNum = getDestinationNum()) < 0)
    {
	printf("--- Operation aborted.\n");
	return;
    }

    printf("Enter the new notify mask: 0x");
    notifyMask = GETULX(ul);

    printf("Enter the new event mask: 0x");
    eventMask = GETULX(ul);

    if ((rc =
	 MM_setDestinationMasksByHandle(destinations[destNum].handle,
					notifyMask,
					eventMask)) != Success)
    {
	printf("--- MM_setDestinationNotifyMaskByHandle() failed, "
	       "reason %s\n", rcString(rc));
	return;
    }

    printf("--- Set Destination done.\n");
}


signed long
getModuleNum(void)
{
    signed long	    moduleNum;

    printf("Modules:\n");
    for (moduleNum=0; moduleNum<numModules; moduleNum++)
    {
	printf("\t%ld. %s\n", moduleNum + 1, modules[moduleNum].pName);
    }

    printf("Select a module number: ");
    if (GETSL(moduleNum) <= 0 || moduleNum > numModules)
    {
	printf("Invalid module number.\n");
	return -1;
    }

    return moduleNum - 1;
}


signed long
getObjectNum(void)
{
    signed long	    objectNum;

    printf("Objects:\n");
    for (objectNum=0; objectNum<numObjects; objectNum++)
    {
	printf("\t%ld. %s\n", objectNum + 1, objects[objectNum].pName);
    }

    printf("Select an object number: ");
    if (GETSL(objectNum) <= 0 || objectNum > numObjects)
    {
	printf("Invalid object number.\n");
	return -1;
    }

    return objectNum - 1;
}


signed long
getDestinationNum(void)
{
    signed long	    destNum;

    printf("Destinations:\n");
    for (destNum=0; destNum<numDestinations; destNum++)
    {
	printf("\t%ld. %s\n", destNum + 1, destinations[destNum].pName);
    }

    printf("Select a destination number: ");
    if (GETSL(destNum) <= 0 || destNum > numDestinations)
    {
	printf("Invalid destination number.\n");
	return -1;
    }

    return destNum - 1;
}


void screenAlert(char * pDestinationName,
		 char * pApplicationEntityInstanceName,
		 char * pModuleName,
		 char * pObjectName,
		 char * pDescription,
		 MM_EventType eventType,
		 ...)
{
    va_list		pArgs;
    MM_Value		value1;
    MM_Value		value2;
    char *		p;

    printf("\n\nvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n\n");
    printf("===> EVENT to Destination: (%s)\n", pDestinationName);
    printf("\tApplication Entity Instance Name: (%s)\n",
	   pApplicationEntityInstanceName);
    printf("\tModule Name: (%s)\n", pModuleName);
    printf("\tObject Name: (%s)\n", pObjectName);
    printf("\tObject Description: (%s)\n", pDescription);

    va_start(pArgs, eventType);

    switch(eventType)
    {
    case MM_EventType_MaxThresholdExceededSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Sint32"
	 *           - the value which caused the event by exceeding the
	 *             threshold, as an "OS_Sint32"
	 *
	 */
	value1.un.signedInt = va_arg(pArgs, OS_Sint32);
	value2.un.signedInt = va_arg(pArgs, OS_Sint32);
	printf("Maximum Threshold Exceeded (Signed): "
	       "threshold=%ld  value=%ld\n",
	       value1.un.signedInt, value2.un.signedInt);
	break;

    case MM_EventType_MaxThresholdExceededUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Uint32"
	 *           - the value which caused the event by exceeding the
	 *             threshold, as an "OS_Uint32"
	 *
	 */
	value1.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	value2.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	printf("Maximum Threshold Exceeded (Unsigned): "
	       "threshold=%lu  value=%lu\n",
	       value1.un.unsignedInt, value2.un.unsignedInt);
	break;

    case MM_EventType_MinThresholdExceededSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Sint32"
	 *           - the value which caused the event by exceeding the
	 *             threshold, as an "OS_Sint32"
	 *
	 */
	value1.un.signedInt = va_arg(pArgs, OS_Sint32);
	value2.un.signedInt = va_arg(pArgs, OS_Sint32);
	printf("Minimum Threshold Exceeded (Signed): "
	       "threshold=%ld  value=%ld\n",
	       value1.un.signedInt, value2.un.signedInt);
	break;
	
    case MM_EventType_MinThresholdExceededUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Uint32"
	 *           - the value which caused the event by exceeding the
	 *             threshold, as an "OS_Uint32"
	 *
	 */
	value1.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	value2.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	printf("Minimum Threshold Exceeded Unsigned: "
	       "threshold=%lu  value=%lu\n",
	       value1.un.unsignedInt, value2.un.unsignedInt);
	break;

    case MM_EventType_MaxThresholdReenteredSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Sint32"
	 *           - the value which caused the event by reentering the
	 *             threshold, as an "OS_Sint32"
	 *
	 */
	value1.un.signedInt = va_arg(pArgs, OS_Sint32);
	value2.un.signedInt = va_arg(pArgs, OS_Sint32);
	printf("Maximum Threshold Reentered (Signed): "
	       "threshold=%ld  value=%ld\n",
	       value1.un.signedInt, value2.un.signedInt);
	break;

    case MM_EventType_MaxThresholdReenteredUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Uint32"
	 *           - the value which caused the event by reentering the
	 *             threshold, as an "OS_Uint32"
	 *
	 */
	value1.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	value2.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	printf("Maximum Threshold Reentered Unsigned: "
	       "threshold=%lu  value=%lu\n",
	       value1.un.unsignedInt, value2.un.unsignedInt);
	break;

    case MM_EventType_MinThresholdReenteredSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Sint32"
	 *           - the value which caused the event by reentering the
	 *             threshold, as an "OS_Sint32"
	 *
	 */
	value1.un.signedInt = va_arg(pArgs, OS_Sint32);
	value2.un.signedInt = va_arg(pArgs, OS_Sint32);
	printf("Minimum Threshold Reentered (Signed): "
	       "threshold=%ld  value=%ld\n",
	       value1.un.signedInt, value2.un.signedInt);
	break;

    case MM_EventType_MinThresholdReenteredUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Uint32"
	 *           - the value which caused the event by reentering the
	 *             threshold, as an "OS_Uint32"
	 *
	 */
	value1.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	value2.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	printf("Minimum Threshold Reentered Unsigned: "
	       "threshold=%lu  value=%lu\n",
	       value1.un.unsignedInt, value2.un.unsignedInt);
	break;

    case MM_EventType_TimerExpired:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     No optional parameters are passed.
	 */
	printf("Timer expired\n");
	break;
	
    case MM_EventType_LogMessage:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 *
	 *		- the log message string, as a "char *"
	 */
	p = va_arg(pArgs, char *);
	printf("Log Message: (%s)\n", p);
	break;
	
    case MM_EventType_ValueChangedSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 * 
	 *           - the new value which caused the event, as an "OS_Sint32"
	 */
	value1.un.signedInt = va_arg(pArgs, OS_Sint32);
	printf("Value Changed (Signed): new value=%ld\n", value1.un.signedInt);
	break;
	
    case MM_EventType_ValueChangedUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 * 
	 *           - the new value which caused the event, as an "OS_Uint32"
	 */
	value1.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	printf("Value Changed (Unsigned): new value=%lu\n",
	       value1.un.unsignedInt);
	break;

    case MM_EventType_ValueChangedString:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 * 
	 *           - the new value which caused the event, as a "char *"
	 */
	p = va_arg(pArgs, char *);
	printf("Value Changed (String): new value=(%s)\n", p);
	break;
	
    case MM_EventType_ManagableObjectChange:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 * This event is only generated once per call to
	 * MM_processEvents().  This allows for a number of managable
	 * objects to be created, with only one event telling the manager
	 * application to re-read its list of managable object names.
	 *
	 *     No optional parameters are passed.
	 */
	printf("Managable Object List Changed\n");
	break;
    }

    printf("\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n");

    va_end(pArgs);
}


char *
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


ReturnCode
initAgent(void)
{
    struct sockaddr_in 		agent;
					     
    if ((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("opening socket");
	return Fail;
    }

    agent.sin_family = AF_INET;
    agent.sin_addr.s_addr = INADDR_ANY;
    agent.sin_port = ntohs(port);

    if (bind(listenFd, (struct sockaddr *) &agent, sizeof(agent)) < 0)
    {
	perror("binding socket");
	return Fail;
    }

    listen(listenFd, 1);

    return Success;
}


void
addManager(void)
{
    if ((messageFd = accept(listenFd, 0, 0)) < 0)
    {
	perror("accept");
    }
}


void
processAgent(void)
{
    int 			len;
    char 			buf[4096];
    OS_Uint8			invokeId;
    OS_Uint8 			operationType;
    OS_Uint8 			operationValue;
    STR_String			hString = NULL;
    void *			hRequestBuf = NULL;
    ReturnCode			rc;

    /* Read the message */
    if ((len = read(messageFd, buf, sizeof(buf))) < 0)
    {
	perror("reading message");
	close(messageFd);
	messageFd = -1;
    }
    else if (len == 0)
    {
	/* Connection closed. */
	close(messageFd);
	messageFd = -1;
    }
    else
    {
	/* Attach the data memory to one of our string handles */
	if ((rc = STR_attachString(len, len, buf, FALSE, &hString)) != Success)
	{
	    printf("STR_AttachString() failed, reason %s\n", rcString(rc));
	    goto cleanUp;
	}
	
	/* Allocate a buffer to which we'll attach this string */
	if ((rc = BUF_alloc(0, &hRequestBuf)) != Success)
	{
	    printf("BUF_alloc() failed, reason %s\n", rcString(rc));
	    goto cleanUp;
	}
	
	/* Attach the string to a buffer so we can parse it */
	if ((rc = BUF_appendChunk(hRequestBuf, hString)) != Success)
	{
	    printf("BUF_appendChunk() failed, reason %s\n", rcString(rc));
	    goto cleanUp;
	}

	/*
	 * Get the invoke id (ignored in this simple implementation),
	 * operation type, and operation value.
	 */
	if ((rc = BUF_getOctet(hRequestBuf, &invokeId)) != Success ||
	    (rc = BUF_getOctet(hRequestBuf, &operationType)) != Success ||
	    (rc = BUF_getOctet(hRequestBuf, &operationValue)) != Success)
	{
	    printf("BUF_getOctet() failed, reason %s\n", rcString(rc));
	    goto cleanUp;
	}

	/* Make sure we got a REQUEST */
	if (operationType != MM_RemOpType_Request)
	{
	    printf("Unexpected operation type: (%d)\n", operationType);
	    goto cleanUp;
	}

	MM_agentProcessEvent(operationType,
			     operationValue,
			     (void **) &invokeId,
			     hRequestBuf);
    }

  cleanUp:

    if (hString != NULL)
    {
	STR_free(hString);
    }
	
    if (hRequestBuf != NULL)
    {
	BUF_free(hRequestBuf);
    }

    return;
}

    

ReturnCode
issueOperation(void ** phOperation,
	       MM_RemoteOperationType operationType,
	       MM_RemoteOperationValue  operationValue,
	       void ** phBuf)
{
    char *			pMemStart = NULL;
    char *			pMem;
    char *			p;
    void *			hBuf;
    OS_Uint8			invokeId = *(OS_Uint8 *) phOperation;
    OS_Uint16			chunkLen;
    ReturnCode			rc;

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
	printf("Could not add operation value to operation\n");
	return rc;
    }

    /* Add the operation type to the PDU */
    if ((rc = BUF_addOctet(hBuf, operationType)) != Success)
    {
	printf("Could not add operation type to operation\n");
	return rc;
    }

    /* Add the invoke id (8 bits in this simple implementation) to the PDU */
    if ((rc = BUF_addOctet(hBuf, invokeId)) != Success)
    {
	printf("Could not add invoke id to operation\n");
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
    if (write(messageFd, pMemStart, BUF_getBufferLength(hBuf)) < 0)
    {
	printf("Could not write result\n");
	OS_free(pMemStart);
	return Fail;
    }

    OS_free(pMemStart);

    return Success;
}


static void
valueChanged(void * hUserData,
	     void * hManagableObject,
	     MM_Value * pValue)
{
    printf("\n\nvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n\n");
    printf("===> Value for object %d changed to ", (int) hUserData);

    switch(pValue->type)
    {
    case MM_ValueType_SignedInt:
	printf("%ld\n", pValue->un.signedInt);
	break;

    case MM_ValueType_UnsignedInt:
	printf("%lu\n", pValue->un.unsignedInt);
	break;

    case MM_ValueType_String:
	printf("%s\n", pValue->un.string);
	break;
    }

    printf("\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n");
}
