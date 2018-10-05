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

/* imq.c:67:NC */ extern void imq_exit (Int);
/* imq.c:82:OF */ extern int IMQ_init (void); /* () */
/* imq.c:118:NF */ extern void imq_exit (Int code); /* (code) Int code; */
/* imq.c:137:OF */ extern char *IMQ_nameCreate (void); /* () */
/* imq.c:168:NF */ extern IMQ_Key IMQ_keyCreate (String name, Int subID); /* (name, subID) String name; Int subID; */
/* imq.c:194:NF */ extern IMQ_PrimDesc IMQ_primGet (IMQ_Key key); /* (key) IMQ_Key key; */
/* imq.c:291:NF */ extern int IMQ_primSnd (IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size); /* (primDesc, data, size) IMQ_PrimDesc primDesc; IMQ_PrimMsg *data; Int size; */
/* imq.c:321:NF */ extern int IMQ_primRcv (IMQ_PrimDesc primDesc, IMQ_PrimMsg *data, Int size); /* (primDesc, data, size) IMQ_PrimDesc primDesc; IMQ_PrimMsg *data; Int size; */
/* imq.c:362:NF */ extern void IMQ_primDelete (IMQ_PrimDesc primDesc); /* (primDesc) IMQ_PrimDesc primDesc; */
/* imq.c:382:NF */ extern IMQ_DataDesc IMQ_dataGet (IMQ_Key key, Int size); /* (key, size) IMQ_Key key; Int size; */
/* imq.c:478:NF */ extern int IMQ_dataSnd (IMQ_DataDesc dataDesc, Ptr data, Int size); /* (dataDesc, data, size) IMQ_DataDesc dataDesc; Ptr data; Int size; */
/* imq.c:510:NF */ extern Int IMQ_duSnd (IMQ_DataDesc dataDesc, QU_Head *duHead, Int size); /* (dataDesc, duHead, size) IMQ_DataDesc dataDesc; QU_Head *duHead; Int size; */
/* imq.c:556:NF */ extern char *IMQ_dataRcv (IMQ_DataDesc dataDesc, Int *size); /* (dataDesc, size) IMQ_DataDesc dataDesc; Int *size; */
/* imq.c:590:NF */ extern int IMQ_dataRcvComplete (IMQ_DataDesc dataDesc, char *data); /* (dataDesc, data) IMQ_DataDesc dataDesc; char *data; */
/* imq.c:606:NF */ extern void IMQ_dataDelete (IMQ_DataDesc dataDesc); /* (dataDesc) IMQ_DataDesc dataDesc; */

/* psq.c:65:OF */ extern void PSQ_init (void); /* () */
/* psq.c:134:OF */ extern void PSQ_poll (void); /* () */
/* psq.c:207:NF */ extern int PSQ_putEvent (PSQ_PrimQuInfo *quInfo, Byte *data, Int size); /* (quInfo, data, size) PSQ_PrimQuInfo *quInfo; Byte *data; Int size; */
/* psq.c:223:NF */ extern struct PSQ_PrimQuInfo *PSQ_getQuInfo (String chnlName); /* (chnlName) String chnlName; */
/* psq.c:255:NF */ extern Int PSQ_procPubQu (Ptr data, Int size); /* (data, size) Ptr data; Int size; */
/* psq.c:310:NF */ extern Int PSQ_primQuInit (Int nuOfQus); /* (nuOfQus) Int nuOfQus; */
/* psq.c:338:NF */ extern int PSQ_primQuAdd (SP_Action *spsPrim); /* (spsPrim) SP_Action *spsPrim; */
/* psq.c:381:NF */ extern int PSQ_primQuRemove (PSQ_PrimQuInfo *quInfo); /* (quInfo) PSQ_PrimQuInfo *quInfo; */

/* usq.c:46:OF */ extern void USQ_init (void); /* () */
/* usq.c:63:OF */ extern struct USQ_PrimQuInfo *USQ_primQuCreate (void); /* () */
/* usq.c:141:OF */ extern int ps_primQuDelete (USQ_PrimQuInfo *quInfo); /* (quInfo) USQ_PrimQuInfo *quInfo; */
/* usq.c:160:OF */ extern int USQ_putAction (USQ_PrimQuInfo *quInfo, Ptr data, Int size); /* (quInfo, data, size) USQ_PrimQuInfo *quInfo; Ptr data; Int size; */
/* usq.c:180:OF */ extern int USQ_getEvent (USQ_PrimQuInfo *quInfo, Ptr data, Int size); /* (quInfo, data, size) USQ_PrimQuInfo *quInfo; Ptr data; Int size; */

