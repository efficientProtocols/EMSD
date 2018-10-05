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
#include "buf.h"
#include "buflocal.h"
#include "eh.h"

/**
 ** Forward declarations
 **/
static Buf_Segment * allocSegment(OS_Uint16 minSize);


/**
 ** Public Functions
 **/

/*
 * Function:	BUF_alloc()
 *
 * Description: Allocate a new buffer.  The buffer will contain, initially, 
 *		one segment which is large enough to hold (at least) the 
 *		specified number of octets.
 *
 * 		If the minimum size is not known, zero may be passed, and 
 *		a default zero-size buffer segment will be allocated, initially.
 *
 *
 * Parameters:
 *         minSize --
 *                 Minimum size of for the data area within the
 *                 buffer segment.  A buffer segment will be
 *                 provided (if possible) which has at least this
 *                 much space available.
 *
 *         phBuf --
 *                 Pointer to memory where a buffer handle is to be
 *                 placed.
 *
 * Returns:
 *
 *         Success or ResourceError or one of the BUF_RC_* return
 *         codes.
 */
ReturnCode
BUF_alloc(OS_Uint16 minSize,
	  void ** phBuf)
{
    Buf_Head * 	    pBuf;
    Buf_Segment *   pSeg;

    /* allocate a buffer header */
    if ((pBuf = OS_alloc(sizeof(Buf_Head))) == NULL)
    {
	/* couldn't allocate the buffer header */
	EH_problem("BUF_alloc: OS_alloc failed\n");
	return FAIL_RC(ResourceError, ("BUF_alloc: OS_alloc"));
    }

    /* Initialize the buffer queue pointers */
    QU_INIT(pBuf);

    /* allocate a segment */
    if ((pSeg = allocSegment(minSize)) == NULL)
    {
	/* couldn't allocate it. */
	OS_free(pBuf);
	EH_problem("BUF_alloc: allocSegment failed\n");
	return FAIL_RC(ResourceError, ("BUF_alloc: allocSegment"));
    }

    /* Insert the segment onto the buffer head */
    QU_INSERT(pSeg, pBuf);

    /* Point the buffer pointers at the end of this segment */
    pBuf->pCurrentSeg = pSeg;
    pBuf->pX = pSeg->pEnd;

    /* Give 'em what they came for */
    *phBuf = (void *) pBuf;

    return Success;
}


/*
 * Function:	BUF_free()
 *
 * Description:	Free the specified buffer and all of its associated segments.
 *
 * Parameters:
 *
 *         hBuf --
 *                 Handle to a buffer, previously returned by
 *                 BUF_alloc(), which is to be freed.
 *
 * Returns:
 *         Nothing.
 */
void
BUF_free(void * hBuf)
{
    Buf_Segment *   pSeg;
    Buf_Head *      pBuf = (Buf_Head *) hBuf;

    /* Don't free null buffer pointers */
    if (pBuf == NULL)
    {
	EH_problem("BUF_free: Null pointer passed to BUF_free!\n");
	return;
    }

    /* Free each segment */
    for (pSeg = QU_FIRST(pBuf);
	 ! QU_EQUAL(pSeg, pBuf);
	 pSeg = QU_FIRST(pBuf))
    {
	/* remove this segment from the queue */
	QU_REMOVE(pSeg);

	/* free our reference to the string memory */
	STR_free(pSeg->string);

	/* free the memory associated with this segment */
	OS_free(pSeg);
    }

    /* free the memory associated with this buffer */
    OS_free(pBuf);
}


/*
 * Function:	BUF_addOctet()
 *
 * Description:	Prepend a single octet to a buffer.  If there is insufficient
 * 		space in the current segment, a new segment is allocated, of 
 *		the default size.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to the buffer, previously returned by
 *                 BUF_alloc(), in which the octet is to be
 *                 prepended.
 *
 *         octet --
 *                 The octet value to be prepended to the buffer.
 *
 * Returns:
 *         Success upon success;
 *         Fail if allocating a new segment failed.
 */
ReturnCode
BUF_addOctet(void * hBuf,
	     OS_Uint8 octet)
{
    Buf_Segment *   pSeg;
    Buf_Head *      pBuf = (Buf_Head *) hBuf;

    /* Point to the current (first?) segment in the buffer */
    pSeg = pBuf->pCurrentSeg;

    /* Is there enough space in this segment? */
    if (pSeg->pStart == STR_stringStart(pSeg->string))
    {
	/* No.  Allocate a new segment */
	if ((pSeg = allocSegment(BUF_MIN_DATA_AREA)) == NULL)
	{
	    /* couldn't allocate it */
	    EH_problem("BUF_addOctet: allocSegment failed!\n");
	    return FAIL_RC(ResourceError,
			   ("BUF_addOctet: allocSegment"));
	}

	/* Prepend this segment to the buffer's segment list */
	QU_PREPEND(pSeg, pBuf);

	/* The position to be updated is in this new buffer now. */
	pBuf->pCurrentSeg = pSeg;
	pBuf->pX = pSeg->pEnd;
    }

    /* Adjust the start pointer */
    --pSeg->pStart;

    /* Add the octet to the buffer */
    *--pBuf->pX = octet;

    return Success;
}


/*
 * Function:	BUF_getOctet()
 *
 * Description:	Get the next octet in the buffer.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to the buffer, previously returned by
 *                 Buf_alloc(), from which the octet is to be
 *                 obtained.
 *
 *         pOctet --
 *                 Pointer to a memory location in which the
 *                 retrieved octet is to be placed.
 *
 * Returns:
 *         Success if an octet was available; Fail otherwise.
 */
ReturnCode
BUF_getOctet(void * hBuf,
	     OS_Uint8 * pOctet)
{
    Buf_Segment *   pSeg;
    Buf_Head *	    pBuf = (Buf_Head *) hBuf;

    /* Point to the current segment */
    pSeg = pBuf->pCurrentSeg;

    /* Are there any more octets available? */
    if (pBuf->pX == pSeg->pEnd)
    {
	/* No.  Move to the next segment. */
	pSeg = QU_NEXT(pSeg);

	/* Is there more data here? */
	if (QU_EQUAL(pSeg, pBuf))
	{
	    /* No, no more data. */
	    return FAIL_RC(BUF_RC_BufferExhausted,
			   ("BUF_getOctet: no more data"));
	}

	pBuf->pCurrentSeg = pSeg;
	pBuf->pX = pSeg->pStart;
    }

    /* Give 'em the next octet. */
    *pOctet = *pBuf->pX++;

    return Success;
}

/*
 * Function:	BUF_ungetOctet()
 *
 * Description:	Return the most recently retrieved octet to the input buffer
 * 		stream.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to the buffer, previously returned by
 *                 Buf_alloc(), from which an octet is to be
 *                 returned.
 *
 * Returns:
 *         Nothing.
 */
ReturnCode
BUF_ungetOctet(void * hBuf)
{
    Buf_Segment *   pSeg;
    Buf_Head *	    pBuf = (Buf_Head *) hBuf;

    /* Point to the current segment */
    pSeg = pBuf->pCurrentSeg;

    /* Is there room to put back an octet? */
    if (pBuf->pX == pSeg->pStart)
    {
	pSeg = QU_PREV(pSeg);
	pBuf->pCurrentSeg = pSeg;
	pBuf->pX = pSeg->pEnd;
    }

    /* Back up one octet */
    --pBuf->pX;

    return Success;
}

/*
 * Function:	BUF_prependChunk()
 *
 * Description:	This function allocates a new buffer segment, and assigns the
 * 		specified string to that segment.  The internal buffer pointers
 * 		are left in such a state as to allow prepending additional
 * 		octets.  Any additional octets prepended will cause a new buffer
 * 		segment to be created, as the segment for this prepended chunk
 * 		takes up its own whole segment.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to the buffer, previously returned by
 *                 BUF_alloc(), in which space is being requested.
 *
 *         string --
 *                 A string handle, which is to be prepended to the
 *                 buffer.
 *
 * Returns:
 *
 *         On error, ResourceError is returned.
 */
ReturnCode
BUF_prependChunk(void * hBuf,
		 STR_String string)
{
    Buf_Segment *   pSeg;
    Buf_Head *      pBuf = (Buf_Head *) hBuf;

    /* Allocate a new segment with no data area */
    if ((pSeg = allocSegment(0)) == NULL)
    {
	/* couldn't allocate it */
	EH_problem("BUF_prependChunk: allocSegment failed!\n");
	return FAIL_RC(ResourceError,
		       ("prependChunk: allocSegment"));
    }
    
    /* Replace the segment's string with the specified one */
    STR_replaceString(&pSeg->string, string);

    /* This segment contains only the string data */
    pSeg->pStart = STR_stringStart(string);
    pSeg->pEnd = STR_stringEnd(string);
    
    /* Make this the current segment */
    pBuf->pCurrentSeg = pSeg;
    pBuf->pX = pSeg->pStart;
    
    /* Prepend this segment to the buffer's segment list */
    QU_PREPEND(pSeg, pBuf);
    
    return Success;
}


/*
 * Function:	BUF_prependBuffer()
 *
 * Description:	This function prepends one buffer to another.  The internal
 * 		buffer pointers are left in such a state as to allow prepending
 * 		additional octets.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to the buffer, previously returned by
 *                 BUF_alloc(), in which space is being requested.
 *
 *         hPrependThisBuf --
 *                 Handle to the buffer, previous returned by
 *                 BUF_alloc(), which is to be prepended to hBuf.
 *
 * Returns:
 *                 
 *         Currently, this function always returns Success.
 */
ReturnCode
BUF_prependBuffer(void * hBuf,
		  void * hPrependThisBuf)
{
    Buf_Head *	    pBuf = (Buf_Head *) hBuf;
    Buf_Head *	    pPrependThisBuf = (Buf_Head *) hPrependThisBuf;
    Buf_Segment *   pSeg;

    /* For each segment in the buffer being prepended... */
    for (pSeg = QU_LAST(pPrependThisBuf);
	 ! QU_EQUAL(pSeg, pPrependThisBuf);
	 pSeg = QU_LAST(pPrependThisBuf))
    {
	/* ... remove it from its current buffer, ... */
	QU_REMOVE(pSeg);

	/* ... and add it to the new one */
	QU_PREPEND(pSeg, pBuf);
    }

    /* Point to the first segment in the buffer */
    pSeg = QU_FIRST(pBuf);

    /* This becomes the current segment */
    pBuf->pCurrentSeg = pSeg;
    pBuf->pX = pSeg->pStart;

    return Success;
}


/*
 * Function:	BUF_getChunk()
 *
 * Description:	When parsing, this function returns a pointer to the next 
 *		chunk of the PDU.  The size of the chunk is determined by 
 *		the value of *pChunkLength when this function is called, 
 *		and by the amount of data remaining in the current (or 
 *		first non-zero-length) segment.
 * 		A chunk of no more then the requested chunk length will be
 * 		provided.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to a buffer, previously returned by
 *                 BUF_alloc(), from which a chunk of data is being
 *                 requested.
 *
 *         pChunkLength --
 *                 Pointer to memory containing the number of octets
 *                 being requested in the chunk.  This value may be
 *                 zero, to indicate that a pointer to as many
 *                 octets as possible should be returned.
 *
 *         ppData --
 *                 A pointer to a location in which a pointer to the
 *                 data in the chunk is placed.  Also, the value
 *                 pointed to by pChunkLength is updated to contain
 *                 the length of data being provided.  This value
 *                 may equal the requested chunk length, or may be
 *                 less then that length.
 *
 * Returns:
 *         Success or ResourceError or one of the BUF_RC_* return
 *         codes.
 */
ReturnCode
BUF_getChunk(void * hBuf,
	     OS_Uint16 * pChunkLength,
	     unsigned char ** ppData)
{
    OS_Uint16	    octetsRemaining;
    Buf_Head *      pBuf = (Buf_Head *) hBuf;

    /* find a segment containing more than zero octets */
    do
    {
	octetsRemaining = pBuf->pCurrentSeg->pEnd - pBuf->pX;

	/* if there are no octets remaining, skip to next segment */
	if (octetsRemaining == 0)
	{
	    pBuf->pCurrentSeg = QU_NEXT(pBuf->pCurrentSeg);
	    pBuf->pX = pBuf->pCurrentSeg->pStart;
	}
    } while (octetsRemaining == 0 &&
	     ! QU_EQUAL(pBuf->pCurrentSeg, pBuf));

    if (octetsRemaining == 0)
    {
	/* there were no octets remaining in the whole buffer */
	*pChunkLength = 0;
	return (BUF_RC_BufferExhausted);
    }

    /*
     * Give 'em no more then they requested.  If they requested zero
     * octets, give 'em how ever many we found here.
     */
    if (*pChunkLength != 0 && octetsRemaining > *pChunkLength)
    {
	octetsRemaining = *pChunkLength;
    }

    /* Return the buffer pointer and length that we're giving 'em */
    *pChunkLength = octetsRemaining;

    *ppData = pBuf->pX;

    /* Update current location pointer */
    pBuf->pX += octetsRemaining;

    return Success;
}


/*
 * Function:	BUF_appendChunk()
 *
 * Description:	Append a string to the end of a buffer.  This function is
 * 		primarily for use when receiving data from the network, which
 * 		is to later be parsed.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to a buffer, previously returned by
 *                 BUF_alloc(), to which a chunk of data is to be
 *                 added.
 *
 *         string --
 *                 String to be appended to the buffer.
 *                 
 * Returns:
 *         Success or ResourceError.
 */
ReturnCode
BUF_appendChunk(void * hBuf,
		STR_String string)
{
    Buf_Segment *   pSeg;
    Buf_Head *      pBuf = (Buf_Head *) hBuf;

    /* Allocate a new segment with no data area */
    if ((pSeg = allocSegment(0)) == NULL)
    {
	/* couldn't allocate it */
	EH_problem("BUF_appendChunk: allocSegment failed!\n");
	return FAIL_RC(ResourceError,
		       ("prependChunk: allocSegment"));
    }
    
    /* Replace the segment's string with the specified one */
    STR_replaceString(&pSeg->string, string);

    /* This segment contains only the string data */
    pSeg->pStart = STR_stringStart(string);
    pSeg->pEnd = STR_stringEnd(string);
    
    /* Make this the current segment */
    pBuf->pCurrentSeg = pSeg;
    pBuf->pX = pSeg->pStart;
    
    /* Append this segment to the buffer's segment list */
    QU_INSERT(pSeg, pBuf);
    
    return Success;
}


/*
 * Function:	BUF_appendBuffer()
 *
 * Description:	This function appends one buffer to another.  The internal
 * 		buffer pointers are left in such a state as to allow appending
 * 		additional octets.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to the buffer, previously returned by
 *                 BUF_alloc(), in which space is being requested.
 *
 *         hAppendThisBuf --
 *                 Handle to the buffer, previous returned by
 *                 BUF_alloc(), which is to be appended to hBuf.
 *
 * Returns:
 *                 
 *         Currently, this function always returns Success.
 */
ReturnCode
BUF_appendBuffer(void * hBuf,
		 void * hAppendThisBuf)
{
    Buf_Head *	    pBuf = (Buf_Head *) hBuf;
    Buf_Head *	    pAppendThisBuf = (Buf_Head *) hAppendThisBuf;
    Buf_Segment *   pSeg;

    /* For each segment in the buffer being appended... */
    for (pSeg = QU_FIRST(pAppendThisBuf);
	 ! QU_EQUAL(pSeg, pAppendThisBuf);
	 pSeg = QU_FIRST(pAppendThisBuf))
    {
	/* ... remove it from its current buffer, ... */
	QU_REMOVE(pSeg);

	/* ... and add it to the new one */
	QU_APPEND(pSeg, pBuf);
    }

    /* Point to the last segment in the buffer */
    pSeg = QU_LAST(pBuf);

    /* This becomes the current segment */
    pBuf->pCurrentSeg = pSeg;
    pBuf->pX = pSeg->pEnd;

    return Success;
}


/*
 * Function:	BUF_resetParse()
 *
 * Description:	Reset the internal buffer pointers for another parse of 
 * 		the buffer.
 *
 * Parameters:
 *
 *         hBuf --
 *                 Handle to a buffer, previously returned by
 *                 BUF_alloc(), which is to be freed.
 *
 * Returns:
 *         Nothing.
 */
void
BUF_resetParse(void * hBuf)
{
    Buf_Head *      pBuf = (Buf_Head *) hBuf;

    /* Reset the current segment pointer */
    pBuf->pCurrentSeg = QU_FIRST(pBuf);

    /* Reset the current location in the current segment */
    pBuf->pX = pBuf->pCurrentSeg->pStart;
}


/*
 * Function:	BUF_copy()
 *
 * Description:	Copy an entire buffer.  All data is copied, so string data is
 * 		independent of the source buffer (as opposed to the way
 * 		BUF_cloneBufferPortion() works).
 *
 * Parameters:
 *
 *
 *	hBufSrc --
 *		Handle to a buffer, previously returned by
 *		BUF_alloc(), which contains the data to be copied
 *
 *	phBufDest --
 *		Pointer to a buffer handle.  A new buffer is allocated
 *		by this function, the data from hBufSrc is copied to
 *		it, and the location pointed to by this parameter is
 *		set to be the new buffer handle.
 *
 * Returns:
 *	Success or ResourceError
 */
ReturnCode
BUF_copy(void * hBufSrc,
	       void ** phBufDest)
{
    ReturnCode	    rc;
    Buf_Segment *   pSegSrc;
    void *	    hBuf;
    STR_String	    hString;

    /* Allocate a new buffer */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	EH_problem("BUF_copy: BUF_alloc failed!\n");
	return rc;
    }

    /* For each segment in the source buffer... */
    for (pSegSrc = QU_FIRST(hBufSrc);
	 ! QU_EQUAL(pSegSrc, hBufSrc);
	 pSegSrc = QU_NEXT(pSegSrc))
    {
	/* Allocate a string for the segment's data */
	if ((rc = STR_alloc(pSegSrc->pEnd - pSegSrc->pStart,
			    &hString)) != Success)
	{
	    EH_problem("BUF_copy: STR_alloc failed!\n");
	    BUF_free(hBuf);
	    return rc;
	}

	/* Copy the segment's data to the new string */
	if ((rc = STR_assignString(hString,
				   pSegSrc->pEnd - pSegSrc->pStart,
				   pSegSrc->pStart)) != Success)
	{
	    EH_problem("BUF_copy: STR_assignString failed!\n");
	    STR_free(hString);
	    BUF_free(hBuf);
	    return rc;
	}

	/* Append this string to the new buffer */
	if ((rc = BUF_appendChunk(hBuf, hString)) != Success)
	{
	    EH_problem("BUF_copy: BUF_appendChunk failed!\n");
	    STR_free(hString);
	    BUF_free(hBuf);
	    return rc;
	}
    }

    /* Reset the new buffer pointers back to the beginning of the buffer */
    BUF_resetParse(hBuf);

    /* Give 'em what they came for */
    *phBufDest = hBuf;

    return Success;
}



/*
 * Function:	BUF_cloneBufferPortion()
 *
 * Description:	Clone a portion of a buffer.  A new buffer handle is provided,
 * 		that contains a (possibly) partial list of the segments from the
 * 		cloned buffer.  The new buffer and original buffer may each be
 * 		freed or manipulated independently, with the caveat that the
 * 		String Data pointed to by the segments is the same in both
 * 		buffers.  Any modifications to the data within the cloned buffer
 * 		that is in the common portion to the original buffer will be
 * 		reflected in both buffers.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to a buffer, previously returned by
 *                 BUF_alloc(), a portion of which is to be cloned.
 *
 *         len --
 *                 Length of data, beginning at the current location
 *                 within the buffer indicated by hBuf, which is to
 *                 be cloned.  If len is BUF_REMAINDER, the buffer
 *                 portion beginning at the current buffer pointer
 *                 and ending at the end of the buffer is cloned.
 *
 *	   bStripClonedPortion --
 *		   If TRUE, update the current position pointers in
 *		   the source buffer to be just beyond the cloned
 *		   portion.
 *
 *         phNewBuf --
 *                 Pointer to location to put the handle of the new
 *                 cloned buffer.
 *                 
 * Returns:
 *         Success or ResourceError.
 */
ReturnCode
BUF_cloneBufferPortion(void * hBuf,
		       OS_Uint32 len,
		       OS_Boolean bStripClonedPortion,
		       void ** phNewBuf)
{
    unsigned char * pStart;
    Buf_Head *      pBuf = (Buf_Head *) hBuf;
    Buf_Head *      pNewBuf;
    Buf_Segment *   pOrigSeg;
    Buf_Segment *   pClonedSeg;

    /* Allocate a new buffer header */
    if ((pNewBuf = OS_alloc(sizeof(Buf_Head))) == NULL)
    {
	/* couldn't allocate the buffer header */
	EH_problem("BUF_cloneBufferPortion: OS_alloc failed!\n");
	return FAIL_RC(ResourceError,
		       ("BUF_cloneBufferPortion: alloc buf head"));
    }

    /* Initialize the buffer queue pointers */
    QU_INIT(pNewBuf);

    /*
     * For each segment from the current position in the original
     * buffer until we've exhausted the specified length, create a
     * new segment for the cloned buffer, point to the same string,
     * but add a reference to the string.  The last segment may have
     * a different pEnd pointer, to make sure that the total length
     * of the cloned buffer is as specified.
     */
    for (pOrigSeg = (len == BUF_ENTIRE ? QU_FIRST(pBuf) : pBuf->pCurrentSeg);
	 len != 0 && ! QU_EQUAL(pOrigSeg, pBuf);
	 )
    {
	/* Allocate a new segment */
	if ((pClonedSeg = OS_alloc(sizeof(Buf_Segment))) == NULL)
	{
	    /* couldn't allocate the segment */
	    EH_problem("BUF_cloneBufferPortion: OS_alloc failed!\n");
	    BUF_free(pNewBuf);
	    return FAIL_RC(ResourceError,
			   ("BUF_cloneBufferPortion: alloc segment"));
	}

	/* Initialize the queue pointers */
	QU_INIT(pClonedSeg);

	/* Point to the buffer head */
	pClonedSeg->pHeader = pNewBuf;

	/* Point to the same string as in the original */
	pClonedSeg->string = pOrigSeg->string;

	/* Update the reference count on the string */
	STR_addReference(pClonedSeg->string);

	/* If this is the first segment, start at current location */
	if (len != BUF_ENTIRE && pOrigSeg == pBuf->pCurrentSeg)
	{
	    pStart = pClonedSeg->pStart = pBuf->pX;
	}
	else
	{
	    pStart = pClonedSeg->pStart = pOrigSeg->pStart;
	}

	/* If this segment exhausts length, ... */
	if (pOrigSeg->pEnd - pStart >= len)
	{
	    /* ...set pEnd accordingly */
	    pClonedSeg->pEnd = pClonedSeg->pStart + len;

	    /* If we're updating the source buffer... */
	    if (bStripClonedPortion)
	    {
		/* ... make sure we're pointing to the current segment, ... */
		pBuf->pCurrentSeg = pOrigSeg;

		/* ... and update the current location by the amount cloned */
		pBuf->pX = (pStart + (pClonedSeg->pEnd - pStart));
	    }
	}
	else
	{
	    /* Otherwise, the cloned segment end is the same as the original */
	    pClonedSeg->pEnd = pOrigSeg->pEnd;

	    /* Point to the next segment */
	    pOrigSeg = QU_NEXT(pOrigSeg);

	    /* If we're updating the source buffer... */
	    if (bStripClonedPortion)
	    {
		/* ... point to the new buffer, ... */
		pBuf->pCurrentSeg = pOrigSeg;

		/* ... and point to its beginning */
		pBuf->pX = pOrigSeg->pStart;
	    }
	}
	
	/* Append the new segment to the buffer. */
	QU_INSERT(pClonedSeg, pNewBuf);

	/* Update the length; decrement what we just used. */
	len -= pClonedSeg->pEnd - pClonedSeg->pStart;
    }

    /* Set the current buffer pointers to the beginning */
    pClonedSeg = QU_FIRST(pNewBuf);
    pNewBuf->pCurrentSeg = pClonedSeg;
    pNewBuf->pX = pClonedSeg->pStart;

    /* Give 'em what they came for */
    *phNewBuf = pNewBuf;

    return Success;
}



/*
 * Function:	BUF_getBufferLength()
 *
 * Description:	Determine the length of the buffer, by adding the lengths of the
 * 		data strings of each of the buffer segments.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to a buffer, previously returned by
 *                 BUF_alloc(), a portion of which is to be cloned.
 *                 
 * Returns:
 *         The length of the buffer.
 */
OS_Uint32
BUF_getBufferLength(void * hBuf)
{
    Buf_Head *      pBuf = (Buf_Head *) hBuf;
    Buf_Segment *   pSeg;
    OS_Uint32	    length;

    if (hBuf == NULL)
    {
	EH_problem("BUF_getBufferLength: Null pointer passed to BUF_getBuffer!\n");
	return 0;
    }

    for (pSeg = QU_FIRST(pBuf), length = 0;
	 ! QU_EQUAL(pSeg, pBuf);
	 pSeg = QU_NEXT(pSeg))
    {
	length += pSeg->pEnd - pSeg->pStart;
    }

    return length;
}


/*
 * BUF_getRemainingBufferLength()
 *
 * Determine the length of the buffer, by adding the lengths of the
 * data strings of each of the buffer segments.  The buffer beginning is
 * assumed to be the current parse location.
 *
 * Parameters:
 *         hBuf --
 *                 Handle to a buffer, previously returned by
 *                 BUF_alloc(), a portion of which is to be cloned.
 *                 
 * Returns:
 *         The length of the buffer.
 */
OS_Uint32
BUF_getRemainingBufferLength(void * hBuf)
{
    Buf_Head *      pBuf = (Buf_Head *) hBuf;
    Buf_Segment *   pSeg;
    OS_Uint32	    length;

    if (hBuf == NULL)
    {
	EH_problem("BUF_getRemainingBufferLength: Null pointer passed to me!\n");
	return 0;
    }

    /* Get length of current segment */
    length = pBuf->pCurrentSeg->pEnd - pBuf->pX;
    
    /* Get length of each successive segment */
    for (pSeg = QU_NEXT(pBuf->pCurrentSeg);
	 ! QU_EQUAL(pSeg, pBuf);
	 pSeg = QU_NEXT(pSeg))
    {
	length += pSeg->pEnd - pSeg->pStart;
    }

    return length;
}

/**
 ** Local (static) Functions
 **/

/*
 * Function:	allocSegment()
 *
 * Description:	Local function to allocate segment memory and initialize a
 * 		segment's pointers.
 *
 * Parameters:  Minimum size.
 *
 * Returns:	Pointer to buf segment.
 *
 */
static Buf_Segment *
allocSegment(OS_Uint16 minSize)	     
{
    Buf_Segment *   pSeg;

    /* allocate the segment */
    if ((pSeg = OS_alloc(sizeof(Buf_Segment) + minSize)) == NULL)
    {
	/* couldn't allocate the segment */
	EH_problem("allocSegment: OS_alloc failed!\n");
	return NULL;
    }

    /* Initialize the segment pointers */
    QU_INIT(pSeg);
    pSeg->pHeader = NULL;
    pSeg->pStart = pSeg->pEnd =
	((unsigned char *) (pSeg + 1)) + minSize;
    
    /* Allocate a string handle and attach our memory to it */
    if (STR_attachString(minSize, minSize,
			 (unsigned char *) (pSeg + 1),
			 FALSE,
			 &pSeg->string) != Success)
    {
	EH_problem("allocSegment: STR_attachString failed!\n");
	OS_free(pSeg);
	return NULL;
    }

    return pSeg;
}
