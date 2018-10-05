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
 * File name: outstrm.h
 *
 * Description: OUTSTRM_ Module header file.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: outstrm.h,v 1.1.1.1 1998/02/01 00:51:49 mohsen Exp $
 */

#ifndef _OUTSTRM_H_
#define _OUTSTRM_H_

#include "estd.h"
#include "queue.h"

typedef enum {
    OUTSTRM_K_file = 0,
    OUTSTRM_K_network,
    OUTSTRM_K_comPort,
    OUTSTRM_K_console
} OUTSTRM_Type;


typedef Void *OUTSTRM_ModDesc;  /* Internal: (maps to (outstrm_ModInfo *) */


PUBLIC Void
OUTSTRM_init (void);

PUBLIC OUTSTRM_ModDesc
OUTSTRM_open(Char *moduleName, OUTSTRM_Type type);

PUBLIC SuccFail
OUTSTRM_close(OUTSTRM_ModDesc outstrmDesc);

PUBLIC SuccFail
OUTSTRM_puts(OUTSTRM_ModDesc outstrmDesc, String outStr);

#endif /* _OUTSTRM_H_ */
