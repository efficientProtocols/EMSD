#include "estd.h"
#include "tm.h"
#include "sbm.h"


typedef struct QElement
{
    SBM_QUELEMENT;

    SBM_Memory		hString1;
    SBM_Memory		hString2;
} QElement;

int
main(int argc, char * argv[])
{
    int 		c;
    int			numBytes = 1024;
    char *		__applicationName;
    void *		pMem;
    ReturnCode 		rc;
    SBM_TransInProcess	tip;
    SBM_Transaction	hHead1Trans;
    SBM_Memory		hHead1Mem;
    SBM_Transaction	hHead2Trans;
    SBM_Memory		hHead2Mem;
    SBM_Transaction	hTrans;
    SBM_Memory		hElem;
    QElement *		pElem;

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
    while ((c = getopt(argc, argv, "T:n:")) != EOF)
    {
        switch (c)
	{
	case 'T':		/* enable trace module tracing */
	    TM_SETUP(optarg);
	    break;

	case 'n':
	    numBytes = atoi(optarg);
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


    /* Begin a transaction */
    if ((rc = SBM_beginTrans(1, &tip, &hTrans)) != Success)
    {
	printf("SBM_beginTrans() failed, reason 0x%x\n", rc);
	return 1;
    }

    hHead1Trans = hHead2Trans = hTrans;

    /* Allocate a queue head */
    if ((rc = SBM_alloc(tip, sizeof(SBM_QuElement), &hHead1Mem)) != Success)
    {
	printf("SBM_alloc(sizeof(SBM_QuElement)) failed, reason 0x%x\n", rc);
	return 1;
    }

    /* Initialize the queue head */
    SBM_quInit(hTrans, hHead1Mem);

    /* Allocate another queue head */
    if ((rc = SBM_alloc(tip, sizeof(SBM_QuElement), &hHead2Mem)) != Success)
    {
	printf("SBM_alloc(sizeof(SBM_QuElement)) failed, reason 0x%x\n", rc);
	return 1;
    }

    /* Initialize the queue head */
    SBM_quInit(hTrans, hHead2Mem);

    /* Allocate a queue element */
    if ((rc = SBM_alloc(tip, sizeof(QElement), &hElem)) != Success)
    {
	printf("SBM_alloc(sizeof(QElement)) failed, reason 0x%x\n", rc);
	return 1;
    }

    /* Initialize the queue element */
    SBM_quInit(hTrans, hElem);

    /* Point to the queue element */
    pElem = SBM_getMemPointer(hTrans, hElem);

    /* Allocate the first string */
    if ((rc = SBM_allocStringCopy(tip, "Alloc 1a",
				  &pElem->hString1)) != Success)
    {
	printf("Alloc 1a failed, reason 0x%x\n", rc);
	return 1;
    }

    /* Allocate the second string */
    if ((rc = SBM_allocStringCopy(tip, "Alloc 2a",
				  &pElem->hString2)) != Success)
    {
	printf("Alloc 2a failed, reason 0x%x\n", rc);
	return 1;
    }

    /* Insert this queue element onto the queue head */
    SBM_quInsert(hTrans, hElem, hTrans, hHead1Mem);

    /* Allocate another queue element */
    if ((rc = SBM_alloc(tip, sizeof(QElement), &hElem)) != Success)
    {
	printf("SBM_alloc(sizeof(QElement)) failed, reason 0x%x\n", rc);
	return 1;
    }

    /* Initialize the queue element */
    SBM_quInit(hTrans, hElem);

    /* Point to the queue element */
    pElem = SBM_getMemPointer(hTrans, hElem);

    /* Allocate the first string */
    if ((rc = SBM_allocStringCopy(tip, "Alloc 1b",
				  &pElem->hString1)) != Success)
    {
	printf("Alloc 1b failed, reason 0x%x\n", rc);
	return 1;
    }

    /* Allocate the second string */
    if ((rc = SBM_allocStringCopy(tip, "Alloc 2b",
				  &pElem->hString2)) != Success)
    {
	printf("Alloc 2b failed, reason 0x%x\n", rc);
	return 1;
    }

    /* Insert this queue element onto the queue head */
    SBM_quInsert(hTrans, hElem, hTrans, hHead1Mem);

    if ((rc = SBM_endTrans(tip)) != Success)
    {
	printf("SBM_endTrans() failed, reason 0x%x\n", rc);
	return 1;
    }

    printf("Elements on queue 1...\n");
    
    for (SBM_quNext(hHead1Trans, hHead1Mem, &hTrans, &hElem);
	 hHead1Trans != hTrans || hHead1Mem != hElem;
	 SBM_quNext(hTrans, hElem, &hTrans, &hElem))
    {
	pElem = SBM_getMemPointer(hTrans, hElem);
	printf("Got (%s) and (%s)\n",
	       (char *) SBM_getMemPointer(hTrans, pElem->hString1),
	       (char *) SBM_getMemPointer(hTrans, pElem->hString2));
    }

    printf("Elements on queue 2...\n");
    
    for (SBM_quNext(hHead2Trans, hHead2Mem, &hTrans, &hElem);
	 hHead2Trans != hTrans || hHead2Mem != hElem;
	 SBM_quNext(hTrans, hElem, &hTrans, &hElem))
    {
	pElem = SBM_getMemPointer(hTrans, hElem);
	printf("Got (%s) and (%s)\n",
	       (char *) SBM_getMemPointer(hTrans, pElem->hString1),
	       (char *) SBM_getMemPointer(hTrans, pElem->hString2));
    }

    printf("Moving elements from queue 1 to queue 2...\n");
    
    for (SBM_quNext(hHead1Trans, hHead1Mem, &hTrans, &hElem);
	 hHead1Trans != hTrans || hHead1Mem != hElem;
	 SBM_quNext(hHead1Trans, hHead1Mem, &hTrans, &hElem))
    {
	SBM_quRemove(hTrans, hElem);
	SBM_quInsert(hTrans, hElem, hHead2Trans, hHead2Mem);
    }

    printf("Elements on queue 1...\n");
    
    for (SBM_quNext(hHead1Trans, hHead1Mem, &hTrans, &hElem);
	 hHead1Trans != hTrans || hHead1Mem != hElem;
	 SBM_quNext(hTrans, hElem, &hTrans, &hElem))
    {
	pElem = SBM_getMemPointer(hTrans, hElem);
	printf("Got (%s) and (%s)\n",
	       (char *) SBM_getMemPointer(hTrans, pElem->hString1),
	       (char *) SBM_getMemPointer(hTrans, pElem->hString2));
    }

    printf("Elements on queue 2...\n");
    
    for (SBM_quNext(hHead2Trans, hHead2Mem, &hTrans, &hElem);
	 hHead2Trans != hTrans || hHead2Mem != hElem;
	 SBM_quNext(hTrans, hElem, &hTrans, &hElem))
    {
	pElem = SBM_getMemPointer(hTrans, hElem);
	printf("Got (%s) and (%s)\n",
	       (char *) SBM_getMemPointer(hTrans, pElem->hString1),
	       (char *) SBM_getMemPointer(hTrans, pElem->hString2));
    }

    return 0;
}
