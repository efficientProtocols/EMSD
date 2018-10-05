/*
 * 
 * Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 *
 * This software is furnished under a license and use, duplication,
 * disclosure and all other uses are restricted to the rights specified
 * in the written license between the licensee and copyright holders.
 *
*/
/*+
 * File name: imq.h
 *
 * Description: Inter Module Queue
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: imq.h,v 1.4 1999/09/30 00:04:19 mohsen Exp $
 */

#include "target.h"

#ifdef IMQ_SIMU
#include "imq_simu.h"
#endif

#ifdef IMQ_SYSV
#include "imq_sysv.h"
#endif

#ifdef IMQ_BSD
#include "imq_bsd.h"
#endif
