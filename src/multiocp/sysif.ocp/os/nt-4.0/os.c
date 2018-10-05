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

/*
 * Author: Derrell Lipman
 * History:
 *
 */

#include "estd.h"

#if defined(OS_TYPE_UNIX)

# if defined(OS_VARIANT_SYSV)
#  include "os_s5.c"
# elif defined(OS_VARIANT_BSD)
#  include "os_bsd.c"
# else
   Unknown_UNIX_variant;
# endif

#elif defined(OS_TYPE_MSDOS) && !defined(OS_VARIANT_WinCE) && !defined(OS_VARIANT_WinNT) 

# include "os_dos.c"

#else

   Unknown_OS_type;

#endif
