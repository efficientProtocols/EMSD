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

    /* Point to the segment info and exists flag */
    pOptionalSegInfo = (OptionalSegmentInfo *) &pSubmitArg->segmentInfo;
    
	/* Is this the first segment? */
	if (pOptionalSegInfo->data.choice == EMSD_SEGMENTINFO_CHOICE_FIRST)
	{
	    /* Handle the special case of one segment, sent segmented */
	    if (pOptionalSegInfo->data.segmentData.numSegments == 1)
	    {
	        TM_TRACE((globals.hTM, MTS_TRACE_WARNING, 
			 "Single segment sent segmented"));
		pOperation->bComplete = TRUE;
	    }
	    else
	    {
	        TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, "First of %ld segments",
			  pOptionalSegInfo->data.segmentData.numSegments));
		/* Save number of the last segment in this operation */
		pOperation->lastSegmentNumber =
		    pOptionalSegInfo->data.segmentData.numSegments - 1;
		
		/* This implementation won't support > MTSUA_MAX_SEGMENTS segments */
		if (pOperation->lastSegmentNumber >= MTSUA_MAX_SEGMENTS)
		{
		    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
			      "Submit operation has too many segments (%ld)",
			      pOperation->lastSegmentNumber));
		    EH_violation("More than MTSUA_MAX_SEGMENTS segments not supported\n");
		    return FALSE;
		}

		/* The next expected segment number is #1 */
		pOperation->nextSegmentNumber = 1;
		
		/* Save the sequence id for this segment sequence */
		pOperation->sequenceId =
		    pOptionalSegInfo->data.segmentData.sequenceId;
		
		/* Save the first-segment operation structure */
		pOperation->hFirstOperationData = pSubmitArg;
		
		/* Copy buffer data (since it'll get freed when we return) */
		if ((pOperation->rc = BUF_copy(pSubmitArg->hContents,
					       &pOperation->hSegmentBuf[0])) != Success)
		{
		    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
		 	     "BUF_copy failed"));
		    return FALSE;
		}
		
		/* Arrange for this segment to be freed with the operation */
		if ((pOperation->rc =
		     mts_operationAddUserFree(pOperation,
					      BUF_free,
					      pOperation->hSegmentBuf[0])) != Success)
		{
		    BUF_free(pOperation->hSegmentBuf[0]);
                    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			     "MTS Op AddUserFree failed"));
		    return FALSE;
		}

		/*
		 * Initialize the segment mask indicating what segment
		 * numbers we expect.
		 *
		 * ASSUMPTION: host uses 2's complement arithmetic
		 */
		pOperation->expectedSegmentMask =
		    ((1 << (pOperation->lastSegmentNumber + 1)) - 1);

		/*
		 * Initialize the segment mask indicating that we
		 * received segment zero.
		 */
		pOperation->receivedSegmentMask = (1 << 0);

		/* Start the segmentation timer */
		if ((pOperation->rc =
		     TMR_start(TMR_SECONDS(MTS_SEGMENTATION_TIMER),
			       pOperation,
			       NULL,
			       segmentationTimerExpired,
			       &pOperation->hSegmentationTimer)) != Success)
		{
		    BUF_free(pOperation->hSegmentBuf[0]);
		    TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE,
			      "Timer start failed, rc=%d", pOperation->rc));
		    return FALSE;
		}
	    }
	}
	else
	{
	    /*
	     * It's not a First Segment
	     */
	    
	    /* Make sure it's a legal segment number */
	    thisSeg = pOptionalSegInfo->data.segmentData.thisSegmentNumber;

	    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
		     "Segment %ld: %ld of %ld segments",
		      thisSeg, thisSeg + 1, 
		     pOptionalSegInfo->data.segmentData.numSegments));

	    if (thisSeg > 31 || (1 << thisSeg) > pOperation->expectedSegmentMask)
	    {
		EH_violation("received segment number too large\n");
                TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "Segment greater than 31 rejected"));
		return FALSE;
	    }

	    /* Have we received this segment number already? */
	    if ((pOperation->receivedSegmentMask & (1 << thisSeg)) != 0)
	    {
		EH_violation("duplicate segment received\n");
                TM_TRACE((globals.hTM, MTS_TRACE_WARNING, 
			 "Duplicate segment rejected"));
		return FALSE;
	    }

	    /* Copy the buffer data (since it'll get freed when we return) */
	    pOperation->rc = BUF_copy(pSubmitArg->hContents, 
				      &pOperation->hSegmentBuf[thisSeg]);
	    if (pOperation->rc != Success)
	    {
                TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, "BUF_copy failed"));
		return FALSE;
	    }
	    
	    /* Arrange for this segment to be freed with the operation */
	    if ((pOperation->rc =
		 mts_operationAddUserFree(pOperation,
					  BUF_free,
					  pOperation->hSegmentBuf[thisSeg])) !=
		Success)
	    {
		BUF_free(pOperation->hSegmentBuf[thisSeg]);
                TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
			 "MTS Op AddFreeUser failed"));
		return FALSE;
	    }

	    /* Set the mask indicating that we received this segment */
	    pOperation->receivedSegmentMask |= (1 << thisSeg);

	    /* Have we received all of the segments? */
	    if (pOperation->expectedSegmentMask ==
		pOperation->receivedSegmentMask)
	    {
                TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
			 "All segments received"));

		/* Yup.  Append the segment buffers to the first one. */
		pOperation->hFullPduBuf = pOperation->hSegmentBuf[0];
		for (i=1; i<=pOperation->lastSegmentNumber; i++)
		{
		    TM_TRACE((globals.hTM, MTS_TRACE_DETAIL, 
			     "=== Appending segment %d", i));
		    if ((pOperation->rc =
			 BUF_appendBuffer(pOperation->hFullPduBuf,
					  pOperation->hSegmentBuf[i])) != Success)
		    {
		        TM_TRACE((globals.hTM, MTS_TRACE_PREDICATE, 
				 "BUF_appendBuf failed"));
			return FALSE;
		    }
		}
	    
		/*
		 * Yup.  Copy our saved Submit argument into this last one.
		 */
		
		/* Give 'em the original Submit Argument */
		pSubmitArg = pOperation->hFirstOperationData;
		
		/* Give 'em the content buffer */
		pSubmitArg->hContents = pOperation->hFullPduBuf;
		
		/* Make sure we don't try to free this later */
		pOperation->hFullPduBuf = NULL;
		
		/* Reset the buffer pointers to the beginning */
		BUF_resetParse(pSubmitArg->hContents);
		
		/* Cancel the segmentation timer */
		TMR_stop(pOperation->hSegmentationTimer);
		
		/* Indicate that we're ready to parse the contents */
		pOperation->bComplete = TRUE;

		TM_CALL(globals.hTM, MTS_TRACE_PDU, BUF_dump,
			pSubmitArg->hContents, "Dump of concatenated PDU");
	    }
	    else
	    {
		/* Increment the next-expected segment number */
		++pOperation->nextSegmentNumber;
	    }
	}
