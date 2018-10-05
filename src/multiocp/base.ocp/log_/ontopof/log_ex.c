/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: log_ex.c,v 1.2 2002/02/12 23:37:39 pinneke Exp $";
#endif /*}*/


#include "estd.h"
#include "log.h"

Void G_init();


LOG_ModDesc ACC_logDesc;


Int
main(Int argc, String argv[])
{
    LOG_init();

    LOG_config("thisLogFile");

    ACC_logDesc = LOG_open("ACC_");

    LOG_event("Loging Some Event\n");

    LOG_modEvent(ACC_logDesc,
	     "ACC_ can be used for accounting purposes\n");

    exit(0);
}

