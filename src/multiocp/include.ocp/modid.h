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


/*
 * Module Identifiers
 *
 * These are used to identify error return codes by module.  Global
 * module identifiers are used for modules that have public access
 * (i.e. those with include files in the top-level include
 * directory).  Local module identifiers are used within
 * applications which themselves contain multiple modules.
 */

#ifndef __MODID_H__
#define	__MODID_H__

enum
{
    ModId_Type_Global	= (1 << 15),
    ModId_Type_Local	= (0 << 15)
};

#define	MODID_GETID(n)	((n << 10) | ModId_Type_Global)

enum
{
    ModId_OpSys				= MODID_GETID(1),
    ModId_Queue				= MODID_GETID(2),
    ModId_Buf				= MODID_GETID(3),
    ModId_Asn				= MODID_GETID(4),
    ModId_Str				= MODID_GETID(5),
    ModId_Mm				= MODID_GETID(6),
    ModId_Nvq				= MODID_GETID(7),
    ModId_Sbm				= MODID_GETID(8),
    ModId_Nvm				= MODID_GETID(9),
    ModId_Config			= MODID_GETID(10)
};



/*
 * MODID_RCPARAMS
 *
 * This macro may be used by TM_TRACE statements, to display the module name
 * and return code number for a specific return code.
 *
 * NOTE: This macro provides TWO parameters: the module name and the return
 *       code value specific to that module.
 *
 * Example usage:
 *
 *       TM_TRACE(("Function failed in module %s, reason %d\n",
 *		   MODID_RCPARAMS(rc), rc & 0x3ff));
 */
#define	MODID_RCPARAMS(rc)				\
((((rc) & 0xfc00) == ModId_OpSys ? "OS" :		\
  (((rc) & 0xfc00) == ModId_Queue ? "QUEUE" :		\
   (((rc) & 0xfc00) == ModId_Buf ? "BUF" :		\
    (((rc) & 0xfc00) == ModId_Asn ? "ASN" :		\
     (((rc) & 0xfc00) == ModId_Str ? "STR" :		\
      (((rc) & 0xfc00) == ModId_Mm ? "MM" :		\
       (((rc) & 0xfc00) == ModId_Nvq ? "NVQ" :		\
	(((rc) & 0xfc00) == ModId_Sbm ? "SBM" :		\
	 (((rc) & 0xfc00) == ModId_Nvm ? "NVM" :	\
	  (((rc) & 0xfc00) == ModId_Config ? "CONFIG" :	\
	   "Unknown module")))))))))),			\
 ((rc) & 0x3ff))



/* Redefine MODID_GETID for user's use */
#undef MODID_GETID
#define	MODID_GETID(n)	((n << 10) | ModId_Type_Local)

#endif /* __MODID_H__ */

