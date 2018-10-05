#include "estd.h"
#include "tm.h"
#include "nvm.h"

int
main(int argc, char * argv[])
{
    int 		c;
    int 		i;
    int 		j;
    int			numBytes = 1024;
    int			numTrans = 5;
    int			numAllocs = 1;
    char		buf[256];
    char *		__applicationName;
    ReturnCode 		rc;
    NVM_TransInProcess	tip[5];
    NVM_Memory		mem[5];
    NVM_Transaction	trans[5];
    NVM_Transaction	transNum;

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
	printf("SBM_init() returned 0x%x\n", rc);
	return 1;
    }

    if ((rc = NVM_init()) != Success)
    {
	printf("NVM_init() returned 0x%x\n", rc);
	return 1;
    }

    if ((rc = NVM_open("/tmp/nvmtest.dat", numBytes)) != Success)
    {
	printf("NVM_open() failed, reason 0x%x\n", rc);
	return 1;
    }

    for (i = 0; i < numTrans; i++)
    {
	if ((rc = NVM_beginTrans(1, &tip[i], &trans[i])) != Success)
	{
	    printf("NVM_beginTrans() #%d failed, reason 0x%x\n", i+1, rc);
	    return 1;
	}

	for (j = 0; j < numAllocs; j++)
	{
	    sprintf(buf, "Trans %d, Alloc %d", i, j);
	    
	    if ((rc = NVM_allocStringCopy(tip[i], buf, &mem[j])) != Success)
	    {
		printf("NVM_allocStringCopy() #%d failed, reason 0x%x\n",
		       i+1, rc);
		return 1;
	    }
	}

	if ((rc = NVM_endTrans(tip[i])) != Success)
	{
	    printf("NVM_endTrans() #%d failed, reason 0x%x\n", i+1, rc);
	    return 1;
	}
    }

    for (i = 0; i < numTrans; i++)
    {
	for (j = 0; j < numAllocs; j++)
	{
	    printf("Trans[%d], Mem[%d] = \"%s\"\n",
		   i, j, (char *) NVM_getMemPointer(trans[i], mem[j]));
	}
    }

    printf("\nAfter initial allocation...\n");
    NVM_dumpTrans();
    
    if (numTrans > 2)
    {
	if ((rc = NVM_freeTrans(trans[1])) != Success)
	{
	    printf("NVM_freeTrans() failed, reason 0x%x\n", rc);
	    return 1;
	}

	printf("\nAfter free [1]...\n");
	NVM_dumpTrans();

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
		       i, j, (char *) NVM_getMemPointer(trans[i], mem[j]));
	    }
	}

	if ((rc = NVM_beginTrans(1, &tip[1], &trans[1])) != Success)
	{
	    printf("NVM_beginTrans() #%d failed, reason 0x%x\n", 1+1, rc);
	    return 1;
	}

	for (j = 0; j < numAllocs; j++)
	{
	    sprintf(buf, "Trans %d, Alloc %d", 1, j);
	    
	    if ((rc = NVM_allocStringCopy(tip[1], buf, &mem[j])) != Success)
	    {
		printf("NVM_allocStringCopy() #%d failed, reason 0x%x\n",
		       1+1, rc);
		return 1;
	    }
	}

	if ((rc = NVM_endTrans(tip[1])) != Success)
	{
	    printf("NVM_endTrans() #%d failed, reason 0x%x\n", 1+1, rc);
	    return 1;
	}

	printf("\nAfter realloc [1]...\n");
	NVM_dumpTrans();

	printf("\n");

	for (i = 0; i < numTrans; i++)
	{
	    for (j = 0; j < numAllocs; j++)
	    {
		printf("Trans[%d], Mem[%d] = \"%s\"\n",
		       i, j, (char *) NVM_getMemPointer(trans[i], mem[j]));
	    }
	}

	if ((rc = NVM_freeTrans(trans[0])) != Success)
	{
	    printf("NVM_freeTrans() failed, reason 0x%x\n", rc);
	    return 1;
	}

	printf("\nAfter free [0]...\n");
	NVM_dumpTrans();

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
		       i, j, (char *) NVM_getMemPointer(trans[i], mem[j]));
	    }
	}

	if ((rc = NVM_beginTrans(1, &tip[0], &trans[0])) != Success)
	{
	    printf("NVM_beginTrans() #%d failed, reason 0x%x\n", 1+0, rc);
	    return 1;
	}

	for (j = 0; j < numAllocs; j++)
	{
	    sprintf(buf, "Trans %d, Alloc %d", 0, j);
	    
	    if ((rc = NVM_allocStringCopy(tip[0], buf, &mem[j])) != Success)
	    {
		printf("NVM_allocStringCopy() #%d failed, reason 0x%x\n",
		       1+0, rc);
		return 1;
	    }
	}

	if ((rc = NVM_endTrans(tip[0])) != Success)
	{
	    printf("NVM_endTrans() #%d failed, reason 0x%x\n", 1+0, rc);
	    return 1;
	}

	printf("\nAfter realloc [0]...\n");
	NVM_dumpTrans();

	printf("\n");

	for (i = 0; i < numTrans; i++)
	{
	    for (j = 0; j < numAllocs; j++)
	    {
		printf("Trans[%d], Mem[%d] = \"%s\"\n",
		       i, j, (char *) NVM_getMemPointer(trans[i], mem[j]));
	    }
	}
    }

    printf("\nVia nextTransaction()...\n");
    for (transNum = NVM_FIRST;
	 NVM_nextTrans(1, transNum, &transNum) == Success;
	 )
    {
	printf("Trans=0x%lx, Mem=FIRST = \"%s\"\n",
	       transNum, (char *) NVM_getMemPointer(transNum, NVM_FIRST));
    }

    if ((rc = NVM_close()) != Success)
    {
	printf("NVM_close() failed, reason 0x%x\n", rc);
	return 1;
    }

    if ((rc = NVM_open("/tmp/nvmtest.dat", 0)) != Success)
    {
	printf("NVM_open() failed, reason 0x%x\n", rc);
	return 1;
    }

    printf("\nVia nextTransaction()...\n");
    for (transNum = NVM_FIRST;
	 NVM_nextTrans(1, transNum, &transNum) == Success;
	 )
    {
	printf("Trans=0x%lx, Mem=FIRST = \"%s\"\n",
	       transNum, (char *) NVM_getMemPointer(transNum, NVM_FIRST));
    }

    return 0;
}
