#include "estd.h"
#include "tm.h"
#include "sbm.h"

int
main(int argc, char * argv[])
{
    int 		c;
    int 		i;
    int 		j;
    int			numBytes = 128;
    int			numTrans = 5;
    int			numAllocs = 1;
    char		buf[256];
    char *		__applicationName;
    void *		pMem;
    ReturnCode 		rc;
    SBM_TransInProcess	tip[100];
    SBM_Memory		mem[100];
    SBM_Transaction	trans[100];
    SBM_Transaction	transNum;

    __applicationName = argv[0];

    /* Initialize the trace module */
    TM_INIT();
    
    /* Initialize the OS module */
    if (OS_init() != Success)
    {
	printf("Could not initialize OS module");
	return 1;
    }

    /* Initialize the OS memory allocation debug routines */
    OS_allocDebugInit(NULL);

    optind = 1;
    while ((c = getopt(argc, argv, "T:n:t:a:")) != EOF)
    {
        switch (c)
	{
	case 'T':		/* enable trace module tracing */
	    TM_SETUP(optarg);
	    break;

	case 'n':
	    numBytes = atoi(optarg);
	    break;

	case 't':
	    numTrans = atoi(optarg);
	    break;

	case 'a':
	    numAllocs = atoi(optarg);
	    break;
	    
	default:
	    printf("usage: %s [-T <TM option string>]\n",
		   __applicationName);
	    return 1;
	}
    }

    if ((rc = SBM_init()) != Success)
    {
	printf("SMB_init() returned 0x%x\n", rc);
	return 1;
    }

    if ((pMem = OS_alloc(numBytes)) == NULL)
    {
	printf("OS_alloc(%d) failed\n", numBytes);
	return 1;
    }

    if ((rc = SBM_open(pMem, numBytes)) != Success)
    {
	printf("SBM_open() failed, reason 0x%x\n", rc);
	return 1;
    }

    for (i = 0; i < numTrans; i++)
    {
	if ((rc = SBM_beginTrans(1, &tip[i], &trans[i])) != Success)
	{
	    printf("SBM_beginTrans() #%d failed, reason 0x%x\n", i+1, rc);
	    return 1;
	}

	for (j = 0; j < numAllocs; j++)
	{
	    sprintf(buf, "Trans %d, Alloc %d", i, j);
	    
	    if ((rc = SBM_allocStringCopy(tip[i], buf, &mem[j])) != Success)
	    {
		printf("SBM_allocStringCopy() #%d failed, reason 0x%x\n",
		       i+1, rc);
		return 1;
	    }
	}

	if ((rc = SBM_endTrans(tip[i])) != Success)
	{
	    printf("SBM_endTrans() #%d failed, reason 0x%x\n", i+1, rc);
	    return 1;
	}
    }

    printf("\nAfter initial allocation...\n");
    for (i = 0; i < numTrans; i++)
    {
	for (j = 0; j < numAllocs; j++)
	{
	    printf("Trans[%d], Mem[%d] = \"%s\"\n",
		   i, j, (char *) SBM_getMemPointer(trans[i], mem[j]));
	}
    }

    SBM_dumpTrans();
    
    if ((rc = SBM_addToTrans(trans[0], &tip[0])) != Success)
    {
	printf("SBM_addToTrans() failed, reason 0x%x\n", rc);
	return 1;
    }

    if ((rc = SBM_allocStringCopy(tip[0],
				  "Added transaction", &mem[3])) != Success)
    {
	printf("SBM_allocStringCopy() failed, reason 0x%x\n", rc);
	return 1;
    }

    if ((rc = SBM_endTrans(tip[0])) != Success)
    {
	printf("SBM_endTrans() failed, reason 0x%x\n", rc);
	return 1;
    }

    printf("\nAfter added allocation...\n");
    for (i = 0; i < numTrans; i++)
    {
	for (j = 0; j < numAllocs + (i == 0 ? 1 : 0); j++)
	{
	    printf("Trans[%d], Mem[%d] = \"%s\"\n",
		   i, j, (char *) SBM_getMemPointer(trans[i], mem[j]));
	}
    }

    SBM_dumpTrans();
    
    if (numTrans > 2)
    {
	if ((rc = SBM_freeTrans(trans[1])) != Success)
	{
	    printf("SBM_freeTrans() failed, reason 0x%x\n", rc);
	    return 1;
	}

	printf("\nAfter free [1]...\n");
	SBM_dumpTrans();

	printf("\n");

	for (i = 0; i < numTrans; i++)
	{
	    if (i == 1)		/* skip freed one */
	    {
		continue;
	    }

	    for (j = 0; j < numAllocs; j++)
	    {
		printf("Trans[%d], Mem[%d] = \"%s\"\n",
		       i, j, (char *) SBM_getMemPointer(trans[i], mem[j]));
	    }
	}

	if ((rc = SBM_beginTrans(1, &tip[1], &trans[1])) != Success)
	{
	    printf("SBM_beginTrans() #%d failed, reason 0x%x\n", 1+1, rc);
	    return 1;
	}

	for (j = 0; j < numAllocs; j++)
	{
	    sprintf(buf, "Trans %d, Alloc %d", 1, j);
	    
	    if ((rc = SBM_allocStringCopy(tip[1], buf, &mem[j])) != Success)
	    {
		printf("SBM_allocStringCopy() #%d failed, reason 0x%x\n",
		       1+1, rc);
		return 1;
	    }
	}

	if ((rc = SBM_endTrans(tip[1])) != Success)
	{
	    printf("SBM_endTrans() #%d failed, reason 0x%x\n", 1+1, rc);
	    return 1;
	}

	printf("\nAfter realloc [1]...\n");
	SBM_dumpTrans();

	printf("\n");

	for (i = 0; i < numTrans; i++)
	{
	    for (j = 0; j < numAllocs; j++)
	    {
		printf("Trans[%d], Mem[%d] = \"%s\"\n",
		       i, j, (char *) SBM_getMemPointer(trans[i], mem[j]));
	    }
	}

	if ((rc = SBM_freeTrans(trans[0])) != Success)
	{
	    printf("SBM_freeTrans() failed, reason 0x%x\n", rc);
	    return 1;
	}

	printf("\nAfter free [0]...\n");
	SBM_dumpTrans();

	printf("\n");

	for (i = 0; i < numTrans; i++)
	{
	    if (i == 0)		/* skip freed one */
	    {
		continue;
	    }

	    for (j = 0; j < numAllocs; j++)
	    {
		printf("Trans[%d], Mem[%d] = \"%s\"\n",
		       i, j, (char *) SBM_getMemPointer(trans[i], mem[j]));
	    }
	}

	if ((rc = SBM_beginTrans(1, &tip[0], &trans[0])) != Success)
	{
	    printf("SBM_beginTrans() #%d failed, reason 0x%x\n", 1+0, rc);
	    return 1;
	}

	for (j = 0; j < numAllocs; j++)
	{
	    sprintf(buf, "Trans %d, Alloc %d", 0, j);
	    
	    if ((rc = SBM_allocStringCopy(tip[0], buf, &mem[j])) != Success)
	    {
		printf("SBM_allocStringCopy() #%d failed, reason 0x%x\n",
		       1+0, rc);
		return 1;
	    }
	}

	if ((rc = SBM_endTrans(tip[0])) != Success)
	{
	    printf("SBM_endTrans() #%d failed, reason 0x%x\n", 1+0, rc);
	    return 1;
	}

	printf("\nAfter realloc [0]...\n");
	SBM_dumpTrans();

	printf("\n");

	for (i = 0; i < numTrans; i++)
	{
	    for (j = 0; j < numAllocs; j++)
	    {
		printf("Trans[%d], Mem[%d] = \"%s\"\n",
		       i, j, (char *) SBM_getMemPointer(trans[i], mem[j]));
	    }
	}
    }

    printf("\nVia nextTransaction()...\n");
    for (transNum = SBM_FIRST;
	 SBM_nextTrans(1, transNum, &transNum) == Success;
	 )
    {
	printf("Trans=0x%lx, Mem=FIRST = \"%s\"\n",
	       transNum, (char *) SBM_getMemPointer(transNum, SBM_FIRST));
    }

    return 0;
}
