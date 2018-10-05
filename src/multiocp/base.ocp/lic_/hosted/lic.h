/*
 * This file is part of the Open C Platform (OCP) Library. OCP is a
 * portable library for development of Data Communication Applications.
 *
 * Copyright (C) 1995 Neda Communications, Inc.
 *	Prepared by Mohsen Banan (mohsen@neda.com)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.  This library is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.  You should have received a copy of the GNU
 * Library General Public License along with this library; if not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
*/
/*+
 * File name: lic.h
 *
 * Description: License Checking Module
 *
-*/

/*
 * Author: Pean Lim
 * History:
 * 
 */

/*
 * RCS Revision: $Id: lic.h,v 1.3 1997/05/22 22:38:36 pean Exp $
 */

#ifndef _LIC_H_
#define _LIC_H_

#include <estd.h>

PUBLIC SuccFail
LIC_check(char *pLicenseFile);

#if 0				
Format of a License File:
-------------------------------------------------------------------
[License]
   Licensee	= J Random Company, Inc.
   Start Date	= 19970219
   End Date	= 19970401
   Host ID	= 272629761
   Check Digits	= 7013006635469
-------------------------------------------------------------------
#endif /* 0 */


SuccFail
LIC_computeHash(char *licensee,  /* IN */
		char *startDate, /* IN */
		char *endDate,	/* IN */
		char *hostID,	/* IN */
		char **hash);	/* OUT */

SuccFail
LIC_checkHash(char *licensee,	/* IN */
	      char *startDate,	/* IN */
	      char *endDate,	/* IN */
	      char *hostID,	/* IN */
	      char *hash);	/* IN */

#endif /* _LIC_H_ */
