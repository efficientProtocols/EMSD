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
 * File name: erop_sap.h (Service Access Point)
 *
 * Description: EROP Service Access Point.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: erop_sap.h,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
 */

#ifndef _EROP_SAP_H_	/*{*/
#define _EROP_SAP_H_

#include "esro.h"
#include "erop.h"
#include "invoke.h"
#include "local.h"

/*
 *  Information required for support of communication
 *  with multiple Service Users through the SAP abstraction.
 */
typedef struct SapInfoSeq {
    struct SapInfo *first;
    struct SapInfo *last;
} SapInfoSeq;

typedef struct SapInfo {
    struct SapInfo	*next;		/* next SAP structure */
    struct SapInfo	*prev;		/* previous SAP structure */
    ESRO_SapSel	sapSel;			/* SAP-ID for this user */
    ESRO_FunctionalUnit	functionalUnit;	/* Functional Unit for this SAP */
    InvokeInfoSeq	invokeSeq;	/* associated invokations */

    int (*invokeInd) (
		      ESRO_SapSel 	locESROSap,
		      ESRO_SapSel 	remESROSap,
		      T_SapSel		*remTsap,
		      N_SapAddr		*remNsap,
		      EROP_InvokeDesc invoke,
		      EROP_OperationValue	operationValue,
		      EROP_EncodingType	encodingType,
		      DU_View		parameter);
    int (*resultInd) (EROP_InvokeDesc invoke,
		      EROP_UserInvokeRef userInvokeRef,
		      EROP_EncodingType	encodingType,
		      DU_View		parameter);
    int (*errorInd) (EROP_InvokeDesc invoke,
		     EROP_UserInvokeRef userInvokeRef,
		     EROP_EncodingType	encodingType,
		     EROP_ErrorValue	errorValue,
		     DU_View 		parameter);
    int (*resultCnf) (EROP_InvokeDesc invoke,
		      EROP_UserInvokeRef userInvokeRef);
    int (*errorCnf) (EROP_InvokeDesc invoke,
		     EROP_UserInvokeRef userInvokeRef);
    int (*failureInd) (EROP_InvokeDesc invoke,
		       EROP_UserInvokeRef userInvokeRef,
		       EROP_FailureValue	failureValue);
} SapInfo;

/* This really belongs to erop.h */
/* typedef Void *EROP_SapDesc; */

#ifdef LINT_ARGS /*{*/

extern void erop_sapInit (Int nuOfSaps); 

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
extern SapInfo  *erop_getSapInfo (ESRO_SapSel);
extern SuccFail EROP_sapUnBind  (ESRO_SapSel sapSel); 
extern SapInfo  *erop_getSapInfo (ESRO_SapSel);
extern SapInfo  *erop_getSapInfo (ESRO_SapSel sapSel); 

#else

extern void erop_sapInit (); 
extern Int EROP_sapBind ();
extern SapInfo  *erop_getSapInfo (ESRO_SapSel);
extern SuccFail EROP_sapUnBind  (ESRO_SapSel sapSel); 
extern SapInfo  *erop_getSapInfo (ESRO_SapSel);
extern SapInfo  *erop_getSapInfo (ESRO_SapSel sapSel); 

#endif


#ifdef __STDC__
#else
#endif


#endif
	
