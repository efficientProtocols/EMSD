/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of ESRO. An implementation of RFC-2188.
 *
 *ESRO is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *ESRO is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with ESRO; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 *
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 *
*/
/*+
 * File name: erop.h 
 *
 * Description:
 *	This header file defines the Data Structures needed for 
 *      ESRO protocol engine.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: erop.h,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
 */

#ifndef _EROP_H_
#define _EROP_H_

#include "addr.h"
#include "du.h"
#include "esro.h"

typedef Void *EROP_SapDesc;
typedef Void *EROP_InvokeDesc;

typedef Int EROP_OperationValue;        /* 0-63 */

typedef Byte EROP_SapSel;

typedef Int EROP_FunctionalUnit; 

typedef ESRO_ErrorValue EROP_ErrorValue; 

typedef ESRO_UserInvokeRef  EROP_UserInvokeRef; 

/* This should match ESRO_EncodingType */
typedef enum EROP_EncodingType {
    EROP_EncodingBER	    = 0,	/* Basic Encoding Rules */
    EROP_EncodingPER        = 1,	/* Packed Encoding Rules */
    EROP_EncodingReserved1  = 2,	/* Reserved */
    EROP_EncodingReserved2  = 3 	/* Reserved */
} EROP_EncodingType;

/* This should match ESRO_FailureValue */
typedef enum EROP_FailureValue {
    EROP_FailureTransmission	  = 0,	
    EROP_FailureLocResource	  = 1,	
    EROP_FailureUserNotResponding = 2,	
    EROP_FailureRemResource	  = 3
} EROP_FailureValue;

#ifdef LINT_ARGS	/* Arg. checking enabled */

extern SuccFail EROP_init(Int udpSapSel, Int nuOfSaps, Int nuOfInvokes, Int invokePduSize);

extern SuccFail EROP_sapUnbind (ESRO_SapSel sapSel); 
extern Int 
EROP_invokeReq (EROP_InvokeDesc *invokeDesc, 
		EROP_UserInvokeRef userInvokeRef, 
	        ESRO_SapSel locESROSap, ESRO_SapSel remESROSap, 
		T_SapSel *remTsap, N_SapAddr *remNsap, 
		EROP_OperationValue opValue, EROP_EncodingType encodingType, 
		DU_View parameter); 
extern Int 
EROP_resultReq (EROP_InvokeDesc invokeDesc, 
		EROP_UserInvokeRef userInvokeRef,
		EROP_EncodingType encodingType, DU_View parameter); 
extern Int 
EROP_errorReq  (EROP_InvokeDesc invokeDesc, 
		EROP_UserInvokeRef userInvokeRef,
		EROP_EncodingType encodingType, EROP_ErrorValue errorValue, 
		DU_View parameter); 

extern Int
EROP_sapBind(EROP_SapDesc *sapDesc, ESRO_SapSel sapSel, 
	     ESRO_FunctionalUnit functionalUnit,		
	     int (*invokeInd) (ESRO_SapSel 	locESROSap,
			       ESRO_SapSel 	remESROSap,
			       T_SapSel		*remTsap,
			       N_SapAddr		*remNsap,
			       EROP_InvokeDesc invoke,
			       EROP_OperationValue	operationValue,
			       EROP_EncodingType	encodingType,
			       DU_View		parameter),
	     int (*resultInd) (EROP_InvokeDesc invoke,
			       EROP_UserInvokeRef userInvokeRef,
			       EROP_EncodingType	encodingType,
			       DU_View		parameter),
	     int (*errorInd) (EROP_InvokeDesc invoke,
			      EROP_UserInvokeRef userInvokeRef,
			      EROP_EncodingType	encodingType,
			      EROP_ErrorValue   errorValue,
			      DU_View 		parameter),
	     int (*resultCnf) (EROP_InvokeDesc invoke,
			      EROP_UserInvokeRef userInvokeRef),
	     int (*errorCnf) (EROP_InvokeDesc invoke,
			      EROP_UserInvokeRef userInvokeRef),
	     int (*failureInd) (EROP_InvokeDesc invoke,
			        EROP_UserInvokeRef userInvokeRef,
				EROP_FailureValue	failureValue));

#else

extern SuccFail EROP_init ();

extern SuccFail EROP_sapUnbind (); 
extern Int EROP_invokeReq (); 
extern Int EROP_resultReq ();
extern Int EROP_errorReq (); 

extern EROP_SapDesc EROP_sapBind();


#endif /* LINT_ARGS */

#endif

