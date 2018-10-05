/*
 * Copyright (C) 1995,1999  Neda Communications, Inc. All rights reserved.
 *
 * This software is furnished under a license and use, duplication,
 * disclosure and all other uses are restricted to the rights specified
 * in the written license between the licensee and copyright holders.
 *
*/
/*+
 * File name: local.h (UDP interface)
 *
 * Description: UDP interface.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: local.h,v 1.10 1999/11/14 04:56:18 mohsen Exp $
 */

#ifndef _LOCAL_H_
#define _LOCAL_H_

LCL_XTRN TM_ModuleCB *DU_modCB;
#define TM_PDUIN    TM_BIT5
#define TM_PDUOUT   TM_BIT6

#define MAX_LOG     80

#define DU_MALLOC TM_BIT10

#endif	/* _LOCAL_H_ */
