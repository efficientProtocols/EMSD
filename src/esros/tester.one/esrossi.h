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
/*+
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */


#ifndef _ESROSSI_H_
#define _ESROSSI_H_

#include "estd.h"
#include "addr.h"

typedef struct esrossi_Info {
    T_SapSel  locTsap;
    Int       locTsuDesc;
    T_SapSel  remTsap;
    Int       remTsuDesc;
    N_SapAddr remNsap;
    Int       tcepDesc;
} esrossi_Info;

EXTERN esrossi_Info locInfo;
EXTERN esrossi_Info remInfo;


#endif	/* _ESROSSI_H_ */
