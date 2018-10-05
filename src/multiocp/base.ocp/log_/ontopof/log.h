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
 * File name: log.h
 *
 * Description: LOG_ Module header file.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: log.h,v 1.1.1.1 1998/02/01 00:49:26 mohsen Exp $
 */

#ifndef _LOG_H_
#define _LOG_H_

#include "estd.h"
#include "queue.h"

/*
 * Define the functions that they can use even if LOG_ENABLED is false.
 */

#ifndef LOG_here
# define LOG_here()  log_here(__FILE__, __LINE__)
#endif

PUBLIC Char *
log_here(Char *fileName, Int lineNu);

typedef struct log_ModInfo {
    struct log_ModInfo *next;
    struct log_ModInfo *prev;
    Char moduleName[16];
} log_ModInfo;

typedef log_ModInfo *LOG_ModDesc;


LCL_XTRN String log_loc;
#define LOG_modEvent    log_loc = LOG_here(); log_modEvent
#define LOG_event    log_loc = LOG_here(); log_event

PUBLIC Void
LOG_init (void);


PUBLIC SuccFail
LOG_config(Char *tmFileName);

PUBLIC log_ModInfo *
LOG_open(Char *moduleName);

PUBLIC SuccFail
LOG_close();

PUBLIC Bool
log_modEvent(log_ModInfo *modInfo, Char *format, ...);

PUBLIC Bool
log_event(Char *format, ...);

#endif /* _LOG_H_ */
