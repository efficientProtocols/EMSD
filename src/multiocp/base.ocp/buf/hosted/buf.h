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

#ifndef __BUF_H__
#define	__BUF_H__

#include "estd.h"
#include "strfunc.h"

/* Succ/Fail Return Codes for the Buffer module */
typedef enum
{
    BUF_RC_BufferExhausted			= (1 | ModId_Buf)
} BUF_ReturnCode;


/*
 * Flag for BUF_cloneBufferPortion() length parameter, to indicate
 * that the entire buffer should be cloned.
 */
#define	BUF_ENTIRE	(0xffffffff)

/*
 * Flag for BUF_cloneBufferPortion() length parameter, to indicate
 * that the remainder of the buffer, from current buffer position to
 * end of buffer, should be cloned.
 */
#define	BUF_REMAINDER	(0xfffffffe)

/*
 * BUF_alloc()
 *
 * Allocate a new buffer.  The buffer will contain, initially, one
 * segment which is large enough to hold (at least) the specified
 * number of octets.
 *
 * If the minimum size is not known, zero may be passed, and a default
 * zero-size buffer segment will be allocated, initially.
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
BUF_alloc(OS_Uint16 minSize, void ** phBuf);


/*
 * BUF_free()
 *
 * Free the specified buffer and all of its associated segments.
 *
 * Parameters:
 *
 *         hBuf -- Handle to a buffer, previously returned by
 *                 BUF_alloc(), which is to be freed.
 *
 * Returns:
 *         Nothing.
 */
void
BUF_free(void * hBuf);


/*
 * BUF_addOctet()
 *
 * Prepend a single octet to a buffer.  If there is insufficient
 * space in the current segment, a new segment is allocated, of the
 * default size.
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
BUF_addOctet(void * hBuf, OS_Uint8 octet);


/*
 * BUF_getOctet()
 *
 * Get the next octet in the buffer.
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
BUF_getOctet(void * hBuf, OS_Uint8 * pOctet);


/*
 * BUF_ungetOctet()
 *
 * Return the most recently retrieved octet to the input buffer
 * stream.
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
BUF_ungetOctet(void * hBuf);


/*
 * BUF_prependChunk()
 *
 * This function allocates a new buffer segment, and assigns the
 * specified string to that segment.  The internal buffer pointers
 * are left in such a state as to allow prepending additional
 * octets.  Any additional octets prepended will cause a new buffer
 * segment to be created, as the segment for this prepended chunk
 * takes up its own whole segment.
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
		 STR_String string);


/*
 * BUF_prependBuffer()
 *
 * This function prepends one buffer to another.  The internal
 * buffer pointers are left in such a state as to allow prepending
 * additional octets.
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
		  void * hPrependThisBuf);


/*
 * BUF_getChunk()
 *
 * When parsing, this function returns a pointer to the next chunk
 * of the PDU.  The size of the chunk is determined by the value of
 * *pChunkLength when this function is called, and by the amount of
 * data remaining in the current (or first non-zero-length) segment.
 * A chunk of no more then the requested chunk length will be
 * provided.
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
 *
 *         Success or ResourceError or one of the BUF_RC_* return
 *         codes.
 */
ReturnCode
BUF_getChunk(void * hBuf,
	     OS_Uint16 * pChunkLength,
	     unsigned char ** ppData);


/*
 * BUF_appendChunk()
 *
 * Append a string to the end of a buffer.  This function is
 * primarily for use when receiving data from the network, which is
 * to later be parsed.
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
		STR_String string);



/*
 * BUF_appendBuffer()
 *
 * This function appends one buffer to another.  The internal
 * buffer pointers are left in such a state as to allow appending
 * additional octets.
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
		 void * hAppendThisBuf);


/*
 * BUF_resetParse()
 *
 * Reset the internal buffer pointers for another parse of 
 * the buffer.
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
BUF_resetParse(void * hBuf);



/*
 * BUF_copy()
 *
 * Copy an entire buffer.  All data is copied, so string data is
 * independent of the source buffer (as opposed to the way
 * BUF_cloneBufferPortion() works).
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
	       void ** phBufDest);



/*
 * BUF_cloneBufferPortion()
 *
 * Clone a portion of a buffer.  A new buffer handle is provided,
 * that contains a (possibly) partial list of the segments from the
 * cloned buffer.  The new buffer and original buffer may each be
 * freed or manipulated independently, with the caveat that the
 * String Data pointed to by the segments is the same in both
 * buffers.  Any modifications to the data within the cloned buffer
 * that is in the common portion to the original buffer will be
 * reflected in both buffers.
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
 *		   If TRUE, update the start of the data in the buffer
 *		   to be just beyond the cloned portion.
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
		       void ** phNewBuf);


/*
 * BUF_getBufferLength()
 *
 * Determine the length of the buffer, by adding the lengths of the
 * data strings of each of the buffer segments.
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
BUF_getBufferLength(void * hBuf);


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
BUF_getRemainingBufferLength(void * hBuf);


/*
 * BUF_dump()
 *
 * Display, on STDOUT, the entire contents of the buffer.
 */
void
BUF_dump(void * hBuf, char * pMsg);

#endif /* __BUF_H__ */
