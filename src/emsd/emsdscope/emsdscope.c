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
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File name: eropscop.c
 *
 * Module: PDU Logger (ESROS)
 *
 * Description: Display the log file (formatted).
 *
 * Functions:
 *   main       (int argc, char *argv[])
 *   usage      ()
 *   readHeader (FILE *fp, UDP_PO_LogRecord *tm)
 *   readData   (FILE *fp, Byte *data, int  size)
 *   logDisplay  (UDP_PO_LogRecord *logHeader, DU_View du)
 *   printLine  ()
 *   timeStamp  (char *str, long milli)
 *   pduType2Str(int pduType)
 *   Void toprintstr (unsigned char *dst, unsigned char *src, long int length)
 *
-*/

/*
 * 
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: emsdscope.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "oe.h"
#include "pf.h"
#include "eh.h"
#include "target.h"
#include "tm.h"
#include "sf.h"
#include "byteordr.h"
#include "queue.h"
#include "du.h"
#include "buf.h"
#include "eropdu.h"
#include "udp_po.h"
#include "mtsua.h"
#include "ua.h"
#include "inetaddr.h"


#define LINE_LENGTH 80

extern getopt();

static char *typeStr[5] = { 	      /* String representation of PDU type */
    	"inv",		/* 0x00 */		/* Invoke */
	"res",		/* 0x01 */		/* Result */
	"err",		/* 0x02 */		/* Error  */
	"ack",		/* 0x03 */		/* Ack    */
	"fai",		/* 0x04 */		/* Failure*/
};

PUBLIC DU_Pool *mainDuPool;		/* DU Main Pool */

static Pdu pdu;
int lineLength = LINE_LENGTH;		/* Line length */
int firstHeaderPrinted = 0;
Bool   followFlag = FALSE;		/* Follow flag similar to tail -f */

static Void
usage();

static Int
readHeader (FILE *fp, UDP_PO_LogRecord *tm);

static Int
readData (FILE *fp, Byte *data, int  size);

static SuccFail
logDisplay (UDP_PO_LogRecord *logHeader, DU_View du);

static Void
printLine(void);

static char *
timeStamp(char *str, OS_Sint32 milli);

static char * 
pduType2Str(int pduType);

static Void
toprintstr(unsigned char *dst,
	   unsigned char *src,
	   long int length);

static Int
parseEsrosPdu(DU_View du, Pdu *pPdu) ;

static ReturnCode
parseEmsdPdu(void * hBuf,
	    int pduType,
	    OS_Uint32 operationValue,
	    ESRO_EncodingType encodingType,
	    OS_Uint32 protocolVersionMajor,
	    OS_Uint32 protocolVersionMinor);

static ReturnCode
viewToBuf(DU_View hView,
	  void ** phBuf);


char * __applicationName;

Char *copyrightNotice;
EXTERN Char *RELID_getRelidNotice(Void);
EXTERN Char *RELID_getRelidNotice(Void);
EXTERN Char *RELID_getRelidNotice(Void);


/*<
 * Function:    main()
 *
 * Description: eropscop's main
 *
 * Arguments: argv, argc.
 *
 * Returns: 0 on successful completion, a negative value otherwise.
 * 
>*/

Int
main(int argc,
     char *argv[])
{
    int 		c;
    Int 		retVal;
    char  * 		fname;
    char		buf[256];
    FILE  * 		fp;
    Bool		errFlag   = FALSE;
    Bool		usageFlag = FALSE;
    ReturnCode 		rc;
    DU_View 		du;
    UDP_PO_LogRecord	tm;
    N_SapAddr remNsapAddr;
    
    __applicationName = argv[0];

    /* Initialize the OS module */
    if (OS_init() != Success)
    {
	printf("%s: Could not initialize OS module", __applicationName);
	exit(1);
    }

    while ((c = getopt(argc, argv, "uUfl:V")) != EOF)
    {
        switch (c)
	{
	case 'f':			/* Follow flag */
	    followFlag = TRUE;
	    break;

	case 'u':			/* Usage */
	case 'U':
	    usageFlag = TRUE;
	    break;

        case 'l':			/* Line length */
	    if (PF_getInt(optarg, &lineLength, LINE_LENGTH, 2, 10000))
	    {
	    	lineLength = LINE_LENGTH;
	    }
            break;

	case 'V':		/* specify configuration directory */
	case 'v':		/* specify configuration directory */
	    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
		EH_fatal("main: Get copyright notice failed\n");
	    }
	    fprintf(stdout, "%s\n", copyrightNotice);
	    exit(0);

        case '?':
        default:
            errFlag = TRUE;
            break;
        } 
    }

    if ( usageFlag || errFlag)
    {
	usage();
	exit(0);
    }

    /* Verify that the CopyRight notice is authentic  and print it */
    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);

    TM_INIT();

    /* Initialize the ASN Module */
    if ((rc = ASN_init()) != Success)
    {
	printf("%s: Could not initialize ASN module\n", __applicationName);
	exit(1);
    }

    /* build buffer pool */
    mainDuPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS);

    /* Compile the ASN trees */
    if ((rc = UA_initCompileTrees(buf)) != Success)
    {
	printf("%s: Error compiling ASN trees\n", __applicationName);
	exit(1);
    }

    if ( optind >= argc )
    {
	fp = stdin;
    }
    else
    {
	fname = argv[optind];
	if ((fp = fopen(fname, "r")) == NULL)
	{
	    printf("%s: Can Not Open %s\n", __applicationName, fname);
	    exit(1);
	} 
#ifdef MSDOS
	setmode(fileno(fp), _O_BINARY);
#endif
    }

    while (TRUE) {
    	while ( (retVal = readHeader(fp, &tm)) > 0 )
    	{

	    if ((du = DU_alloc(mainDuPool, tm.size)) == NULL)
	    {
	    	EH_fatal("main: DU_alloc failed");
	    }

	    if ((retVal = readData(fp, DU_data(du), tm.size)) != tm.size)
	    {
	    	if (retVal < 0)
	    	{
		    EH_fatal("main: readData failed");
	    	}
	        else
	    	{
		    EH_problem("Warning: data size doesn't match "
			       "the size field");
	        }
	    }

	    if ((retVal = readData(fp, (char *)&remNsapAddr, 
				   sizeof(remNsapAddr))) 
	        != sizeof(remNsapAddr)) 
	    {
	    	if (retVal < 0) 
	    	{
	            EH_fatal("main: readData failed");
	    	}
	    	else 
	    	{
		    printf("Warning: IP address can not be read\n");
	    	}
	    }

	    logDisplay(&tm, du);		/* Display log info */

	    DU_free(du);
        }

   	if (!followFlag) 
	{
	    break;
	}
#ifndef WINDOWS
   	sleep(1);
#endif
    }    

    if (fp != stdin)
    {
	fclose(fp);
    }

    printf("\n");

    exit(0);

}


static Void
usage()
{
    fprintf(stderr,
	    "Usage: %s [-l linelength] <filename> \n", __applicationName);
}


/*<
 * Function:    readHeader
 *
 * Description: Read header of the log record.
 *
 * Arguments:   Log file pointer, address of the buffer to store header
 *
 * Returns:     0 if successfule
 *
 * 
>*/
static Int
readHeader(FILE *fp,
	   UDP_PO_LogRecord *tm)
{
    Int retVal;

    if ((retVal = read(fileno(fp), (char *)tm, sizeof(*tm))) != sizeof(*tm))
    {
	if (ferror(fp))
	{
	    EH_problem("readHeader: read failed\n");
	    perror("readHeader:");
            return (FAIL);
	} 
    } 

    if (retVal == 0)
    {
	return (FAIL);
    }

    if ( tm->magic != 0x4711 )
    {
    	EH_problem("readHeader: magic number doesn't match\n");
    	return (FAIL);
    } 

    return (retVal);

}


/*<
 * Function:    readData
 *
 * Description: read data part of the log record.
 *
 * Arguments:   Log file pointer, data buffer, size.
 *
 * Returns:     0 if successful
 * 
>*/

static Int
readData(FILE *fp,
	 Byte *data,
	 int  size)
{
    Int retVal;

    if ((retVal = read(fileno(fp), data, size)) != size)
    {
	if (ferror(fp))
	{
	    EH_problem("readData: read failed\n");
	    perror("readData:");
	    return (FAIL);
	} 
    } 

    return (retVal);
}


/*<
 * Function:    logDisplay
 *
 * Description: Display the log record.
 *
 * Arguments:   Record header, PDU
 *
 * Returns:     0 if successful.
 *
 * 
>*/
static SuccFail
logDisplay(UDP_PO_LogRecord *logHeader,
	   DU_View du)
{
    Int 		gotVal;
    int 		duSize;
    long		textLen = 0;
    char * 		cIut;
    char * 		obuf;
    char * 		buf;
    char * 		ostr;
    char * 		str;
    void *		hBuf;
    Bool		locSw = FALSE;
    ReturnCode		rc;
    static int		nsdu = 0;

    obuf = (unsigned char *)SF_memGet(lineLength + 1024);
    buf  = (unsigned char *)SF_memGet(lineLength + 1024);

    switch (logHeader->code)
    {
    case IN_PDU:
	cIut = "<-";
	locSw = FALSE;
	break;

    case OUT_PDU:
	cIut = "->";
	locSw = TRUE;
	break;

    case STAMP:
	str = buf;
	ostr = timeStamp(obuf, logHeader->tmx);
	strcat(ostr, " ");  /* strcat(obuf, " "); */

	/* Skip the Hours part */
	ostr = obuf + 3;
	strncat(ostr, DU_data(du), logHeader->size);
	printLine();
	printf("%s\n", ostr);
	printLine();
	printf("  Time  TSDU Tsiz  Loc     Rem Ref  Dst  Src  "
	       "OpVal Encod  Parameter\n");
	printLine();
	cIut = (char *)NULL;
	firstHeaderPrinted = 1;
	DU_free(du);
	break;

    default:
	EH_problem("Invalid header code\n");
	return (FAIL);
    }

    if (!firstHeaderPrinted) {
	putchar('\n');
	putchar('\n');
	printLine();
	printf("  Time  TSDU Tsiz  Loc    Rem Ref  Dst  Src  "
	       "OpVal Encod  Parameter\n");
	printLine();
	firstHeaderPrinted = 1;
    }

    if (cIut)
    {
	++nsdu;
	duSize = DU_size(du);
	if ( (gotVal = parseEsrosPdu(du, &pdu)) != EOTSDU )
	{
	    str = buf;
	    ostr = timeStamp(obuf, logHeader->tmx);
	    strcat(ostr, " ");

	    /* Skip the Hours part */
	    ostr = obuf + 3;
	    sprintf(str, "%4d %4d  ", nsdu, duSize); duSize = DU_size(du);
	    strcat(ostr, str);
		
	    if (locSw)
	    {
   	    	sprintf(str, "%s %s    ", pduType2Str(pdu.pdutype), cIut);
	    }
	    else
	    {
   	    	sprintf(str, "    %s %s", cIut, pduType2Str(pdu.pdutype));
	    }

	    strcat(ostr,str);
	    switch (pdu.pdutype)
	    {
	    case INVOKE_PDU:
		sprintf(str, "%4d %4d %4d   %2d    %2d ",
			pdu.invokeRefNu, 
			pdu.fromESROSap, pdu.toESROSap,
			pdu.operationValue, pdu.encodingType);
		strcat(ostr,str);
		strcat(ostr, "   ");
		textLen = lineLength - strlen(ostr) - 1;
		textLen = (DU_size(du) > textLen) ? textLen : DU_size(du);
		toprintstr(str, DU_data(du), textLen);
		*(str + textLen) = '\0';
		strcat(ostr, str);
		printf("%s\n\n", ostr);

		/* Strip off the ESROS PCI */
		DU_strip(du, 3);

		if ((rc = viewToBuf(du, &hBuf)) != Success)
		{
		    printf("Could not convert DU to BUF\n");
		    break;
		}

		putchar('\n');
		BUF_dump(hBuf, "INVOKE PDU");

		(void) parseEmsdPdu(hBuf,
				   pdu.pdutype,
				   pdu.operationValue,
				   pdu.encodingType,
				   1, 1);

		BUF_free(hBuf);

		break;

	    case RESULT_PDU:
		sprintf(str, "%4d                 %4d ",
			pdu.invokeRefNu, 
			pdu.encodingType);
		strcat(ostr,str);
		strcat(ostr, "   ");
		textLen = lineLength - strlen(ostr) - 1;
		textLen = (DU_size(du) > textLen) ? textLen : DU_size(du);
		toprintstr(str, DU_data(du), textLen);
		*(str + textLen) = '\0';
		strcat(ostr, str);
		printf("%s\n", ostr);

		/* Strip off the ESROS PCI */
		DU_strip(du, 2);

		if ((rc = viewToBuf(du, &hBuf)) != Success)
		{
		    printf("Could not convert DU to BUF\n");
		    break;
		}

		putchar('\n');
		BUF_dump(hBuf, "RESULT PDU");

		(void) parseEmsdPdu(hBuf,
				   pdu.pdutype,
				   pdu.operationValue,
				   pdu.encodingType,
				   1, 1);

		BUF_free(hBuf);
		
		break;

	    case ERROR_PDU:
		sprintf(str, "%4d                 %4d ",
			pdu.invokeRefNu, 
			pdu.encodingType);
		strcat(ostr,str);
		strcat(ostr, "   ");
		textLen = lineLength - strlen(ostr) - 1;
		textLen = (DU_size(du) > textLen) ? textLen : DU_size(du);
		toprintstr(str, DU_data(du), textLen);
		*(str + textLen) = '\0';
		strcat(ostr, str);
		printf("%s\n", ostr);

		/* Strip off the ESROS PCI */
		DU_strip(du, 2);

		if ((rc = viewToBuf(du, &hBuf)) != Success)
		{
		    printf("Could not convert DU to BUF\n");
		    break;
		}

		putchar('\n');
		BUF_dump(hBuf, "ERROR PDU");

		(void) parseEmsdPdu(hBuf,
				   pdu.pdutype,
				   pdu.operationValue,
				   pdu.encodingType,
				   1, 1);

		BUF_free(hBuf);
		break;

	    case ACK_PDU:
		sprintf(str, "%4d  ",
			pdu.invokeRefNu);

		strcat(ostr,str);
		printf("%s\n", ostr);
		break;

	    case FAILURE_PDU:
		sprintf(str, "%4d ", pdu.invokeRefNu);
		strcat(ostr,str);
		strcat(ostr, "                        ");
		textLen = lineLength - strlen(ostr) - 1;
		textLen = (DU_size(du) > textLen) ? textLen : DU_size(du);
		toprintstr(str, DU_data(du), textLen);
		*(str + textLen) = '\0';
		strcat(ostr, str);
		printf("%s\n", ostr);
		break;

	    default:
		EH_problem("Invalid PDU type\n");
		return (FAIL);
	    }

	    if (pdu.data)
	    {
		/* So there is some data that must be freed */
		DU_free(pdu.data);
	    }
	}
    }

    return (SUCCESS);
}


/*<
 * Function:    printLine
 *
 * Description: Print a separator line.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
 * 
>*/
static Void
printLine(void)
{
    int i;
    int maxLen;

    maxLen = (lineLength > 136) ? 136 : lineLength;

    for (i = 0; i < maxLen - 1; ++i)
    {
	putchar('-');
    }

    putchar('\n');
}


/*<
 * Function:    timeStamp
 *
 * Description: Translate a time from milli seconds to string format.
 *
 * Arguments:   String to store time, time in milli seconds.
 *
 * Returns:     Pointer to the end of the time string.
 * 
>*/

static char *
timeStamp(char *str,
	  OS_Sint32 milli)
{
    OS_Sint32 hour;
    OS_Sint32 min;
    OS_Sint32 second;
    OS_Sint32 tenth;

    tenth = ( (milli % 1000) + 50) / 100;
    if (tenth > 9)
    {
	tenth = 0;
	second++;
    }
    second = milli / 1000;
    second = second % (24L*60*60);	
    hour   = second / (60*60);
    second = second % (60*60);		
    min    = second / 60;		/* minutes of the hour */
    second = second % 60;		/* seconds of the minute */

    sprintf(str, "%02ld:%02ld:%02ld.%1ld", hour, min, second, tenth);
    return (str + 10);
}


/*<
 * Function:    pduType2Str
 *
 * Description: Translate PDU type from code to description.
 *
 * Arguments:   PDU type code
 *
 * Returns:     Pointer to type description string.
 *
 * 
>*/

static char * 
pduType2Str(int pduType)
{
    return typeStr[pduType & 0x0F];
}


/*<
 * Function:    toprintstr
 *
 * Description: Covert an input string to printable (replace cntrl with dot)
 *
 * Arguments:   Destination string, source string, length 
 *
 * Returns:
 *
>*/

static Void
toprintstr(unsigned char *dst,
	   unsigned char *src,
	   long int length)
{
    unsigned char *srcEnd = src + length;

    if (src == NULL || dst == NULL || length == 0)
    {
	return;
    }

    for ( ; src < srcEnd; ++src)
    {
        if (isprint (*src))
	{
	    *dst++ = *src;
        }
	else
	{
	    *dst++ = '.';
        }
    }

}


static Int
parseEsrosPdu(DU_View du,
	      Pdu *pPdu) 
{
    unsigned char c;
    char *p;

    p = DU_data(du); 

    /* PCI, Byte1 , PDU-TYPE + remEsroSapSel or encodingType ... */

    BO_get1(c, p);

    pPdu->pdutype = c & 0x07;
    switch (c & 0x07)
    {
    case INVOKE_PDU:
	/* PCI, BYTE-1 */
	pPdu->toESROSap   =   (c & 0xF0) >> 4;	
	pPdu->fromESROSap =  ((c & 0xF0) >> 4) - 1;

	/* PCI Byte-2 */
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

	/* PCI, Byte-3 , operationValue + EncodingType */
	BO_get1(c, p);  

	pPdu->operationValue = c & 0x3F;
	pPdu->encodingType = (c & 0xC0) >> 6;

	break;

    case RESULT_PDU:
	/* PCI, BYTE-1 */
	pPdu->encodingType = (c & 0xC0) >> 6;

	/* PCI Byte-2 */
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

	break;

    case ERROR_PDU:
	/* PCI, BYTE-1 */
	pPdu->encodingType = (c & 0xC0) >> 6;

	/* PCI Byte-2 */
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

	break;

    case ACK_PDU:
	/* PCI Byte-1 */
	pPdu->encodingType = (c & 0x10) >> 4;

	/* PCI Byte-2 */
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

        break; 

    case FAILURE_PDU:
	BO_get1(c, p);
	pPdu->invokeRefNu = c;

	break;

    default:
	printf("\nUnknown PDU type %d", c & 0x07);
	break;
    }

    return (SUCCESS);

}



static ReturnCode
parseEmsdPdu(void * hBuf,
	    int pduType,
	    OS_Uint32 operationValue,
	    ESRO_EncodingType encodingType,
	    OS_Uint32 protocolVersionMajor,
	    OS_Uint32 protocolVersionMinor)
{
    OS_Uint8		    c;
    ReturnCode		    rc = Success;
    OS_Uint32		    pduLength;
    OS_Uint32		    contentType;
    EMSD_SubmitArgument *    pSubmitArg;
    EMSD_DeliverArgument *   pDeliverArg;
    EMSD_ReportDelivery *    pReport;
    EMSD_Message *	    pIpm;
    QU_Head *		    pCompTree;
    void *		    hStruct = NULL;
    void *		    hContents;
    void *		    hExtraBuf = NULL;
    void *		    hOperationData;
    void		 (* pfFree)(void * p) = NULL;
    
    /* Right now, we only support Version 1.1 */
    if (protocolVersionMajor != 1 || protocolVersionMinor != 1)
    {
	printf("Protocol version %lu.%lu not supported.\n",
		(unsigned long) protocolVersionMajor,
		(unsigned long) protocolVersionMinor);
	return UnsupportedOption;
    }

    /* Determine if there's an Operation Instance Id to extract */
    if (pduType == INVOKE_PDU && operationValue >= 32)
    {
	/* There is.  Extract and print it. */
	if ((rc = BUF_getOctet(hBuf, &c)) != Success)
	{
	    printf("Could not extract instance id from PDU\n");
	    firstHeaderPrinted = 0;
	    return rc;
	}

	putchar('\n');
	printf("=========================\n"
	       "Operation Instance Id: %u\n"
	       "=========================\n",
	       (unsigned int) c);
    }
    else
    {
	putchar('\n');
	printf("=========================\n");
    }

    /*
     * Create our internal rendition of the operation value, which
     * includes the protocol version number in the high-order 16 bits.
     */
    operationValue =
	EMSD_OP_PROTO(operationValue,
		     protocolVersionMajor,
		     protocolVersionMinor);
    
    /* see what PDU type occured */
    switch(pduType)
    {
    case INVOKE_PDU:
	/* See what operation is being requested */
	switch(operationValue)
	{
	case EMSD_Operation_Submission_1_1:
	    /* Allocate an EMSD Submit Argument structure */
	    if ((rc = mtsua_allocSubmitArg(&pSubmitArg)) != Success)
	    {
		printf("Could not allocate SubmitArg\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeSubmitArg;
	    hStruct = pSubmitArg;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_SubmitArg);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, pSubmitArg,
				&pduLength)) != Success)
	    {
		printf("Could not parse SubmitArg\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, pSubmitArg, TRUE, NULL);
	    putchar('\n');

	    /* There's now an extra buffer to be freed */
	    hExtraBuf = pSubmitArg->hContents;

	    /* Is this a part of a segmented message? */
	    if (pSubmitArg->segmentInfo.exists)
	    {
		/* Yup.  We can't parse the content data. */
		printf("Contents are segmented, thus not displayed.\n");
		goto cleanUp;
	    }
	
	    contentType = pSubmitArg->contentType;
	    hContents = pSubmitArg->hContents;
	    pduLength = BUF_getBufferLength(pSubmitArg->hContents);

	    goto printContents;

	    break;

	case EMSD_Operation_Delivery_1_1:
	    /* Allocate an EMSD Deliver Argument structure */
	    if ((rc = mtsua_allocDeliverArg(&pDeliverArg)) != Success)
	    {
		printf("Could not allocate DeliverArg\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeDeliverArg;
	    hStruct = pDeliverArg;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliverArg);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, pDeliverArg,
				&pduLength)) != Success)
	    {
		printf("Could not parse DeliverArg\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, pDeliverArg, TRUE, NULL);
	    putchar('\n');

	    /* There's now an extra buffer to be freed */
	    hExtraBuf = pDeliverArg->hContents;

	    /* Is this a part of a segmented message? */
	    if (pDeliverArg->segmentInfo.exists)
	    {
		/* Yup.  We can't parse the content data. */
		printf("Contents are segmented, thus not displayed.\n");
		goto cleanUp;
	    }
	
	    contentType = pDeliverArg->contentType;
	    hContents = pDeliverArg->hContents;
	    pduLength = BUF_getBufferLength(pDeliverArg->hContents);
	    

	  printContents:

	    /* Parse the contents. See what type of contents we've received */
	    switch(contentType)
	    {
	    case EMSD_ContentType_Ipm95:
		/* Allocate an EMSD Message structure */
		if ((rc = mtsua_allocMessage(&pIpm)) != Success)
		{
		    printf("Out of memory allocating IPM\n");
		    goto cleanUp;
		}
	    
		/* Determine which protocol tree to use */
		pCompTree = UA_getProtocolTree(MTSUA_Protocol_Ipm);
	    
		/* Parse the IPM PDU */
		if ((rc = ASN_parse(ASN_EncodingRules_Basic,
				    pCompTree,
				    hContents,
				    pIpm, &pduLength)) != Success)
		{
		    printf("Could not parse IPM\n");
		    goto cleanUp;
		}
	    
		/* Print the data */
		ASN_printTree(pCompTree, pIpm, TRUE, NULL);
		putchar('\n');

		/* Free the allocated structure */
		mtsua_freeMessage(pIpm);

		break;
	    
	    case EMSD_ContentType_Probe:
		printf("PROBE not supported\n");
		break;
	    
	    case EMSD_ContentType_VoiceMessage:
		printf("VOICE MESSAGE not supported\n");
		break;
	    
	    case EMSD_ContentType_DeliveryReport:
		/* Allocate an EMSD Report Delivery structure */
		if ((rc = mtsua_allocReportDelivery(&pReport)) != Success)
		{
		    printf("Out of memory allocating IPM\n");
		    goto cleanUp;
		}
	    
		/* Determine which protocol tree to use */
		pCompTree = UA_getProtocolTree(MTSUA_Protocol_ReportDelivery);
	    
		/* Parse the IPM PDU */
		if ((rc = ASN_parse(ASN_EncodingRules_Basic,
				    pCompTree,
				    hContents,
				    pReport, &pduLength)) != Success)
		{
		    printf("Could not parse Report\n");
		    goto cleanUp;
		}
	    
		/* Print the data */
		ASN_printTree(pCompTree, pReport, TRUE, NULL);
		putchar('\n');

		/* Free the allocated structure */
		mtsua_freeReportDelivery(pReport);
		break;

	    default:
		printf("Content type %lu (unknown) not supported\n",
			(unsigned long) pDeliverArg->contentType);
		break;
	    }

	    break;
	
	case EMSD_Operation_DeliveryControl_1_1:
	    /* Allocate an EMSD Delivery Control Argument structure */
	    if ((rc =
		 mtsua_allocDeliveryControlArg((EMSD_DeliveryControlArgument **)
					      &hOperationData)) != Success)
	    {
		printf("Could not allocate Delivery Control Arg\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeDeliveryControlArg;
	    hStruct = hOperationData;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliveryControlArg);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, hOperationData,
				&pduLength)) != Success)
	    {
		printf("Could not parse DeliveryControlArg\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, hOperationData, TRUE, NULL);
	    putchar('\n');

	    break;
	
	case EMSD_Operation_SubmissionControl_1_1:
	    printf("Submission Control not supported\n");
	    break;
	
	case EMSD_Operation_DeliveryVerify_1_1:
	    /* Allocate an EMSD Delivery Verify Argument structure */
	    if ((rc =
		 mtsua_allocDeliveryVerifyArg((EMSD_DeliveryVerifyArgument **)
					      &hOperationData)) != Success)
	    {
		printf("Could not allocate Delivery Verify Arg\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeDeliveryVerifyArg;
	    hStruct = hOperationData;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliveryVerifyArg);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, hOperationData,
				&pduLength)) != Success)
	    {
		printf("Could not parse DeliveryVerifyArg\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, hOperationData, TRUE, NULL);
	    putchar('\n');

	    break;

	case EMSD_Operation_SubmissionVerify_1_1:
	    /* Allocate an EMSD Submission Verify Argument structure */
	    if ((rc =
		 mtsua_allocSubmissionVerifyArg((EMSD_SubmissionVerifyArgument **)
						&hOperationData)) != Success)
	    {
		printf("Could not allocate Submission Verify Arg\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeSubmissionVerifyArg;
	    hStruct = hOperationData;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_SubmissionVerifyArg);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, hOperationData,
				&pduLength)) != Success)
	    {
		printf("Could not parse SubmissionVerifyArg\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, hOperationData, TRUE, NULL);
	    putchar('\n');

	    break;
	
	case EMSD_Operation_GetConfiguration_1_1:
	    printf("Get Configuration not supported\n");
	    rc = UnsupportedOption;
	    break;

	case EMSD_Operation_SetConfiguration_1_1:
	    printf("Set Configuration not supported\n");
	    rc = UnsupportedOption;
	    break;

	default:
	    printf("Operation %lu (unknown) not supported.\n",
		   (unsigned long) operationValue);
	    rc = UnsupportedOption;
	    break;
	}
	break;

    case RESULT_PDU:
	/* See what operation is being requested */
	switch(operationValue)
	{
	case EMSD_Operation_Submission_1_1:
	    /* Allocate an EMSD Submit Result structure */
	    if ((rc =
		 mtsua_allocSubmitRes((EMSD_SubmitResult **)
				      &hOperationData)) != Success)
	    {
		printf("Could not allocate SubmitRes\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeSubmitRes;
	    hStruct = hOperationData;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_SubmitResult);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, hOperationData,
				&pduLength)) != Success)
	    {
		printf("Could not parse SubmitRes\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, hOperationData, TRUE, NULL);
	    putchar('\n');

	    break;

	case EMSD_Operation_Delivery_1_1:
	    /* Deliver Result is NULL */
	    printf("Deliver Result\n");
	    printf("=========================\n");
	    break;
	
	case EMSD_Operation_DeliveryControl_1_1:
	    /* Allocate an EMSD Delivery Control Result structure */
	    if ((rc =
		 mtsua_allocDeliveryControlRes((EMSD_DeliveryControlResult **)
					       &hOperationData)) != Success)
	    {
		printf("Could not allocate Delivery Control Result\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeDeliveryControlRes;
	    hStruct = hOperationData;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliveryControlRes);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, hOperationData,
				&pduLength)) != Success)
	    {
		printf("Could not parse DeliveryControlArg\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, hOperationData, TRUE, NULL);
	    putchar('\n');

	    break;
	
	case EMSD_Operation_SubmissionControl_1_1:
	    printf("Submission Control not supported\n\n");
	    break;
	
	case EMSD_Operation_DeliveryVerify_1_1:
	    /* Allocate an EMSD Delivery Verify Argument structure */
	    if ((rc =
		 mtsua_allocDeliveryVerifyRes((EMSD_DeliveryVerifyResult **)
					      &hOperationData)) != Success)
	    {
		printf("Could not allocate Delivery Verify Result\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeDeliveryVerifyRes;
	    hStruct = hOperationData;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_DeliveryVerifyRes);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, hOperationData,
				&pduLength)) != Success)
	    {
		printf("Could not parse DeliveryVerifyArg\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, hOperationData, TRUE, NULL);
	    putchar('\n');

	    break;

	case EMSD_Operation_SubmissionVerify_1_1:
	    /* Allocate an EMSD Submission Verify Argument structure */
	    if ((rc =
		 mtsua_allocSubmissionVerifyRes((EMSD_SubmissionVerifyResult **)
						&hOperationData)) != Success)
	    {
		printf("Could not allocate Submission Verify Result\n");
		goto cleanUp;
	    }
	
	    /* Point to the function to free this structure */
	    pfFree = (void (*)(void *)) mtsua_freeSubmissionVerifyRes;
	    hStruct = hOperationData;
	
	    /* Specify the maximum parse length */
	    pduLength = BUF_getBufferLength(hBuf);
	
	    /* Determine which protocol tree to use */
	    pCompTree = UA_getProtocolTree(MTSUA_Protocol_SubmissionVerifyRes);
	
	    /* Parse the EMSD PDU */
	    if ((rc = ASN_parse(encodingType,
				pCompTree,
				hBuf, hOperationData,
				&pduLength)) != Success)
	    {
		printf("Could not parse SubmissionVerifyRes\n");
		goto cleanUp;
	    }
	
	    /* Print the data */
	    ASN_printTree(pCompTree, hOperationData, TRUE, NULL);
	    putchar('\n');

	    break;
	
	case EMSD_Operation_GetConfiguration_1_1:
	    printf("Get Configuration not supported\n");
	    rc = UnsupportedOption;
	    break;

	case EMSD_Operation_SetConfiguration_1_1:
	    printf("Set Configuration not supported\n");
	    rc = UnsupportedOption;
	    break;

	default:
	    printf("Operation %lu (unknown) not supported.\n",
		   (unsigned long) operationValue);
	    rc = UnsupportedOption;
	    break;
	}
	break;

    case ERROR_PDU:
	/* Allocate an EMSD Submit Argument structure */
	if ((rc =
	     mtsua_allocErrorRequest((EMSD_ErrorRequest **)
				     &hOperationData)) != Success)
	{
	    printf("Could not allocate ErrorReq\n");
	    goto cleanUp;
	}
	
	/* Point to the function to free this structure */
	pfFree = (void (*)(void *)) mtsua_freeErrorRequest;
	hStruct = hOperationData;
	
	    /* Specify the maximum parse length */
	pduLength = BUF_getBufferLength(hBuf);
	
	/* Determine which protocol tree to use */
	pCompTree = UA_getProtocolTree(MTSUA_Protocol_ErrorRequest);
	
	/* Parse the EMSD PDU */
	if ((rc = ASN_parse(encodingType,
			    pCompTree,
			    hBuf, hOperationData,
			    &pduLength)) != Success)
	{
	    printf("Could not parse ErrorReq\n");
	    goto cleanUp;
	}
	
	/* Print the data */
	ASN_printTree(pCompTree, hOperationData, TRUE, NULL);
	putchar('\n');
	break;

    default:
	/* We won't get here. */
	break;
    }
    
  cleanUp:

    /* Free the allocated structure */
    if (pfFree != NULL)
    {
	(* pfFree)(hStruct);
    }

    /* If there's an extra buffer that needs freeing... */
    if (hExtraBuf != NULL)
    {
	/* ... then free it. */
	BUF_free(hExtraBuf);
    }

    firstHeaderPrinted = 0;
    return rc;
}


static ReturnCode
viewToBuf(DU_View hView,
	  void ** phBuf)
{
    unsigned char * p;
    STR_String 	    hStr;
    ReturnCode	    rc;

    /* Get a pointer to the data in the view */
    p = DU_data(hView);

    /* Allocate an STR_String for this data */
    if ((rc =
	 STR_attachString(DU_size(hView), DU_size(hView), p, FALSE, &hStr)) !=
	Success)
    {
	return rc;
    }

    /* Allocate a buffer */
    if ((rc = BUF_alloc(0, phBuf)) != Success)
    {
	STR_free(hStr);
	return rc;
    }

    /* Append the view data to the new buffer */
    if ((rc = BUF_appendChunk(*phBuf, hStr)) != Success)
    {
	STR_free(hStr);
	BUF_free(phBuf);
	return rc;
    }

    /*
     * The reference count on the STR_string has been incremented,
     * since the buffer now has its copy of it.  We can free our copy
     * of it.
     */
    STR_free(hStr);

    return Success;
}
