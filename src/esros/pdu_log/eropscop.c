/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
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
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: eropscop.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "oe.h"
#include "pf.h"
#include "eh.h"
#include "target.h"
#include "tm.h"
#include "sf.h"
#include "eropdu.h"
#include "udp_po.h"
#include "inetaddr.h"

#ifdef MSDOS
#include "g.c" /* stupid linker (*sigh*) */
#endif

#define LINE_LENGTH 80

STATIC Void usage();
extern char *timeStamp();
extern Void toprintstr();

#ifdef TM_ENABLED
TM_ModuleCB *SCOP_modCB;	      /* Trace Module: eropscop Ctrl Blk */
#endif

static char *typeStr[5] = { 	      /* String representation of PDU type */
    	"inv",		/* 0x00 */		/* Invoke */
	"res",		/* 0x01 */		/* Result */
	"err",		/* 0x02 */		/* Error  */
	"ack",		/* 0x03 */		/* Ack    */
	"fai",		/* 0x04 */		/* Failure*/
};

PUBLIC DU_Pool *mainDuPool;		/* DU Main Pool */

static Pdu pdu;
static Bool hexSw = 0;			/* Switch: display complete hex dump */
Bool   followFlag = FALSE;		/* Follow flag similar to tail -f */
int lineLength = LINE_LENGTH;		/* Line length */

extern getopt();
extern readHeader();
extern readData();
extern logDisplay();
extern Void printLine();
extern parsePdu();
extern char *pduType2Str();

char * __applicationName;


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
main (int argc, char *argv[])
{
    extern int optind;
    extern char *optarg;

    Bool errFlag   = FALSE;
    Bool usageFlag = FALSE;
    int c;

    static Bool virgin = TRUE;

    Char *copyrightNotice;
    EXTERN Char *RELID_getRelidNotice(Void);
    EXTERN Char *RELID_getRelidNotice(Void);
    EXTERN Char *RELID_getRelidNotice(Void);

    if ( !virgin ) {
	return (0);
    }
    virgin = FALSE;

    TM_INIT();
    (void) TM_SETUP("SCOP_,01");

    if (TM_OPEN(SCOP_modCB, "SCOP_") == NULL) {
    	EH_fatal("SCOP_init:    Error in TM_open SCOP_");
    }

    __applicationName = argv[0];

    while ((c = getopt(argc, argv, "uUfhVl:")) != EOF) {
        switch (c) {
	case 'u':			/* Usage */
	case 'U':
        case '?':
	    usageFlag = TRUE;
	    break;

	case 'f':			/* Follow flag */
	    followFlag = TRUE;
	    break;

        case 'h':			/* Hex */
	    hexSw = 1;
            break;

        case 'l':			/* Line length */
	    if (PF_getInt(optarg, &lineLength, LINE_LENGTH, 2, 10000)) {
	    	lineLength = LINE_LENGTH;
	    }
            break;

	case 'V':		/* specify configuration directory */
	    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
		EH_fatal("main: Get copyright notice failed\n");
	    }
	    fprintf(stdout, "%s\n", copyrightNotice);
	    exit(0);

        default:
            errFlag = TRUE;
            break;
        } 
    }

    if ( usageFlag ) {
	usage();
	exit(0);
    }
    if ( errFlag ) {
        usage();
        exit(1);
    }

    /* Verify that the CopyRight notice is authentic  and print it */
    if ( ! (copyrightNotice = RELID_getRelidNotice()) ) {
	EH_fatal("main: Get copyright notice failed!\n");
    }
    fprintf(stdout, "%s\n", argv[0]);
    fprintf(stdout, "%s\n\n", copyrightNotice);


    OS_init();

    mainDuPool = DU_buildPool(MAXBFSZ, BUFFERS, VIEWS);	/* build buffer pool */

    do {
	FILE  *fp;
	char  *fname;
	UDP_PO_LogRecord tm;
        DU_View  du;
	N_SapAddr remNsapAddr;
	Int retVal;

	if ( optind >= argc ) {
	    fp = stdin;
	} else {
	    fname = argv[optind];
	    if (!(fp = fopen(fname, "r"))) {
		fprintf(stderr, "%s: Can Not Open %s\n", 
			__applicationName, fname);
		continue;
	    } 
#ifdef MSDOS
	    setmode(fileno(fp), _O_BINARY);
#endif
	}

	while (TRUE) {
	    while ( (retVal = readHeader(fp, &tm)) > 0 ) {

	        if ( (du = DU_alloc(mainDuPool, tm.size)) == (DU_View)NULL ) {
		    EH_fatal("main: DU_alloc failed");
	    	}

	    	if ((retVal = readData(fp, DU_data(du), tm.size)) != tm.size) {
		    if (retVal < 0) {
		        EH_fatal("main: readData failed");
		    }
		    else {
		        TM_TRACE((SCOP_modCB, TM_ENTER, 
		        	 "Warning: data size doesn't match "
				 "the size field\n"));
		        printf("Warning: data size doesn't match "
			       "the size field\n");
		    }
	    	}

	    	if ((retVal = readData(fp, &remNsapAddr, sizeof(remNsapAddr))) 
		    != sizeof(remNsapAddr)) {
		    if (retVal < 0) {
		        EH_fatal("main: readData failed");
		    }
		    else {
		        TM_TRACE((SCOP_modCB, TM_ENTER, 
			 	 "Warning: IP address can not be read\n"));
			printf("Warning: IP address can not be read\n");
		    }
	    	}
	    	logDisplay(&tm, du, &remNsapAddr);	/* Display log info */

	    	DU_free(du);
	    }

   	    if ((optind < argc-1) || !followFlag) {
	 	break;
	    }
#ifndef WINDOWS
	    sleep(1);
#endif
	}    

	fclose(fp);

        printf("\n");

    } while ( ++optind < argc );

    exit(0);

} /* main() */


STATIC Void
usage ()
{
    fprintf(stderr, "Usage: %s [-V] [-f] [-h] [-l linelength] <filename> \n", 
	    __applicationName);
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
Int
readHeader (FILE *fp, UDP_PO_LogRecord *tm)
{
    Int retVal;

    if ((retVal = read(fileno(fp), (char *)tm, sizeof(*tm))) != sizeof(*tm)) {
	if ( ferror(fp) ) {
	    EH_problem("readHeader: read failed\n");
	    perror("readHeader:");
            return (FAIL);
	} 
    } 

    if (retVal == 0) {
	return (FAIL);
    }

    if ( tm->magic != 0x4711 ) {
    	EH_problem("readHeader: magic number doesn't match\n");
    	return (FAIL);
    } 

    return (retVal);

} /* readHeader() */


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

Int
readData (FILE *fp, Byte *data, int  size)
{
    Int retVal;

    if ( (retVal = read(fileno(fp), data, size)) != size ) {
	if ( ferror(fp) ) {
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
SuccFail
logDisplay (UDP_PO_LogRecord *logHeader, DU_View du, N_SapAddr *remNsapAddr)
{
    char *cIut;

    char *obuf;
    char *buf;

    char *ostr;
    char *str;

    static int nsdu = 0;
    Bool locSw = FALSE;
    Int gotVal;
    static int firstHeaderPrinted = 0;
    long int textLen = 0;
    struct in_addr remIpAddr;
    char ipStr[128];

    obuf = (unsigned char *)SF_memGet(lineLength + 1024);
    buf  = (unsigned char *)SF_memGet(lineLength + 1024);

    INET_nsapAddrToin_addr(remNsapAddr, &remIpAddr);
    strcpy (ipStr, inet_ntoa(remIpAddr));

    switch (logHeader->code) {

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
	printf("  Time     TSDU Tsiz  Loc    Rem Ref Dst Src Op Enc  "
	       "Remote IP      Parameter\n");
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
	printLine();
	printf("  Time     TSDU Tsiz  Loc    Rem Ref Dst Src Op Enc  "
	       "Remote IP      Parameter\n");
	printLine();
	firstHeaderPrinted = 1;
    }

    if ( cIut ) {
	int duSize;

	++nsdu;
	duSize = DU_size(du);
	if ( (gotVal = parsePdu(du, &pdu)) != EOTSDU ) {
	    str = buf;
	    ostr = timeStamp(obuf, logHeader->tmx);
	    strcat(ostr, " ");
	    /* Skip the Hours part */
	    /* ostr = obuf + 3; */
	    ostr = obuf;
	    sprintf(str, "%4d %4d  ", nsdu, duSize); duSize = DU_size(du);
	    strcat(ostr, str);
		
	    if (locSw) {
   	    	sprintf(str, "%s %s    ", pduType2Str(pdu.pdutype), cIut);
	    } else {
   	    	sprintf(str, "    %s %s", cIut, pduType2Str(pdu.pdutype));
	    }

	    strcat(ostr,str);
	    switch ( pdu.pdutype ) {

	    case INVOKE_PDU:
		sprintf(str, "%4d%4d%4d %2d %2d  %s  ",
			pdu.invokeRefNu, 
			pdu.fromESROSap, pdu.toESROSap,
			pdu.operationValue, pdu.encodingType,
			ipStr);
		strcat(ostr,str);
		textLen = lineLength - strlen(ostr) - 1;
		textLen = (DU_size(du) > textLen) ? textLen : DU_size(du);
		toprintstr(str, DU_data(du), textLen);
		*(str + textLen) = '\0';
		strcat(ostr, str);
		break;

	    case RESULT_PDU:
		sprintf(str, "%4d          %4d  %s  ",
			pdu.invokeRefNu, 
			pdu.encodingType,
			ipStr);
		strcat(ostr,str);
		textLen = lineLength - strlen(ostr) - 1;
		textLen = (DU_size(du) > textLen) ? textLen : DU_size(du);
		toprintstr(str, DU_data(du), textLen);
		*(str + textLen) = '\0';
		strcat(ostr, str);
		break;

	    case ERROR_PDU:
		sprintf(str, "%4d          %4d  %s  ",
			pdu.invokeRefNu, 
			pdu.encodingType,
			ipStr);
		strcat(ostr,str);
		textLen = lineLength - strlen(ostr) - 1;
		textLen = (DU_size(du) > textLen) ? textLen : DU_size(du);
		toprintstr(str, DU_data(du), textLen);
		*(str + textLen) = '\0';
		strcat(ostr, str);
		break;

	    case ACK_PDU:
		sprintf(str, "%4d                %s",
			pdu.invokeRefNu,
			ipStr);

		strcat(ostr,str);
		break;

	    case FAILURE_PDU:
		sprintf(str, "%4d  ", pdu.invokeRefNu);
		strcat(ostr,str);
		strcat(ostr, "              ");
		sprintf(str, "%s  ", ipStr);
		strcat(ostr,str);
		textLen = lineLength - strlen(ostr) - 1;
		textLen = (DU_size(du) > textLen) ? textLen : DU_size(du);
		toprintstr(str, DU_data(du), textLen);
		*(str + textLen) = '\0';
		strcat(ostr, str);
		break;

	    default:
		EH_problem("Invalid PDU type\n");
		return (FAIL);
	    }

	    printf("%s", ostr);

	    if (hexSw) {
	        TM_HEXDUMP(SCOP_modCB, TM_ENTER, pduType2Str(pdu.pdutype), 
	   	       	   DU_data(du), DU_size(du));
	    }
	    printf("\n");

	    if ( pdu.data ) {
		/* So there is some data that must be freed */
		DU_free(pdu.data);
	    }

	} /* if(parsePdu) */

    } /* if (cIut) */

    return (SUCCESS);

} /* logDisplay () */


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
Void
printLine()
{
    int i;

    int maxLen;

    maxLen = (lineLength > 136) ? 136 : lineLength;

    for (i = 0; i < maxLen - 1; ++i) {
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

char *
timeStamp (char *str, OS_Sint32 milli)
{
    OS_Sint32 hour;
    OS_Sint32 min;
    OS_Sint32 second;
    OS_Sint32 tenth;

    tenth = ( (milli % 1000) + 50) / 100;
    if (tenth > 9) {
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

char * 
pduType2Str (int pduType)
{
    return ( typeStr[pduType & 0x0F] );
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

Void
toprintstr (unsigned char *dst, unsigned char *src, long int length)
{
    unsigned char *srcEnd = src + length;

    if (src == NULL || dst == NULL || length == 0)
	return;

    for ( ; src < srcEnd; ++src) {
        if (isprint (*src)) {
	    *dst++ = *src;
        } else {
	    *dst++ = '.';
        }
    }

} /* toprintstr() */

