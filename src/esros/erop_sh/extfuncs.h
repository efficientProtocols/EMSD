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


#ifdef LINT_ARGS	/* Arg. checking enabled */
extern void erop_invokeInit(Int);
extern void erop_sapInit();
extern void erop_refInit();
extern LOCAL Int lower_dataInd();
extern erop_freeLopsInvoke;

extern int getConf();


#else

extern SuccFail erop_relPdu();
extern SuccFail erop_relAllPdu();
extern DU_View erop_pduRetrieve();
extern SuccFail erop_pduKeep();
extern DU_View erop_invokePdu ();
extern DU_View erop_resultPdu ();
extern DU_View erop_ackPdu ();
extern Int erop_resultInd ();
extern Int erop_errorInd ();
extern Int erop_failureInd ();

extern int getConf();

#endif
