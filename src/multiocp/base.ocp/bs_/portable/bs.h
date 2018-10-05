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
 * File name: bs.h
 *
 * Description:
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: bs.h,v 1.4 1995/08/11 04:36:54 mohsen Exp $
 */

#ifndef _BS_H_	/*{*/
#define _BS_H_


#ifdef LINT_ARGS /*{*/

unsigned char  *BS_memCopy(unsigned char  *,unsigned char  *,int );
int BS_memCmp(unsigned char  *,unsigned char  *,int );

#else /*}{*/

unsigned char  *BS_memCopy();
int BS_memCmp();

#endif /*}*/

#endif	/*}*/
