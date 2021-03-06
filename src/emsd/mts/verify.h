/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of Neda-EMSD. An implementation of RFC-2524.
 *
 *Neda-EMSD is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with Neda-EMSD; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/

#define MAX_VERIFY_MESSAGES    100
#define MAX_VERIFY_MSGID_LEN    63

typedef int VState;
typedef struct VerifyList
{
    char    NsapAddr[16];			/* not used */
    char    messageId[MAX_VERIFY_MSGID_LEN + 1];
    VState  status;				/* Unknown=0, None=1, Deliver=2, Non-Deliver=3 */
} DeliverVerifyList;

extern int                messageCount;
extern DeliverVerifyList  deliverVerifyList[MAX_VERIFY_MESSAGES];

void      verify_setStatus (char *messageId, VState status);
VState    verify_getStatus (char *messageId);
