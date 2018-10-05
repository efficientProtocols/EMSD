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

/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:
 *
 */


#include "estd.h"
#include "eh.h"


/*
 * Redefine the fs_ functions in terms of OS_malloc() et. al.
 */

void *
fs_get(size_t size)
{
  void * pMem;

  if ((pMem = OS_alloc(size ? size : (size_t) 1)) == NULL)
  {
      EH_fatal("fs_get() out of memory");
  }

  return (pMem);
}


void
fs_resize (void ** ppSrc,
	   size_t newSize)
{
    void *	    pNew;

    if ((pNew = OS_realloc(*ppSrc, newSize)) == NULL)
    {
	EH_fatal("Can't resize memory\n");
    }

    *ppSrc = pNew;
}


void
fs_give(void ** ppSrc)
{
    OS_free(*ppSrc);
    *ppSrc = NULL;
}
