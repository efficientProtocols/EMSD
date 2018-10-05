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
/*
 * Program:	Mailbox Access routines
 *
 * 
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	22 November 1989
 * Last Edited:	6 October 1994
 *
 * Copyright 1994 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appears in all copies and that both the
 * above copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  This software is made
 * available "as is", and
 * THE UNIVERSITY OF WASHINGTON DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN
 * NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * Some portions are:
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * CAUTION!!!
 *
 * Although this file is primarily for use by the UA, which has its own
 * independent emulation of the c-client mail_*() and rfc822_*() functions, it
 * is also included by emsd-mtsd.c.  The reason is that a small number of
 * functions in the generic daemon code call rfc822_*() functions.  Therefore,
 * if the definition of any of these functions changes for the UA, some
 * modification will have to be made to the MTS to no longer require including
 * this file.
 *
 */

#ifndef __EMSD822_H__
#define	__EMSD822_H__


void rfc822_address (char *dest,ADDRESS *adr);

void rfc822_write_address (char *dest,ADDRESS *adr);

void rfc822_cat (char *dest,char *src,const char *specials);

void rfc822_address_line (char **header,char *type,ENVELOPE *env,ADDRESS *adr);

void rfc822_header_line (char **header,char *type,ENVELOPE *env,char *text);

char *rfc822_default_subtype(unsigned short type);

void rfc822_parse_msg (ENVELOPE **en,BODY **bdy,char *s,unsigned long i,
		       STRING *bs,char *host,char *tmp);

void rfc822_parse_content (BODY *body,STRING *bs,char *h,char *t);

void rfc822_parse_content_header (BODY *body,char *name,char *s);

void rfc822_parse_adrlist (ADDRESS **lst,char *string,char *host);

ADDRESS *rfc822_parse_address (char **string,char *defaulthost);

ADDRESS *rfc822_parse_routeaddr (char *string,char **ret,char *defaulthost);

ADDRESS *rfc822_parse_addrspec (char *string,char **ret,char *defaulthost);

char *rfc822_parse_phrase (char *s);

char *rfc822_parse_word (char *s,const char *delimiters);

char *rfc822_cpy (char *src);

char *rfc822_quote (char *src);

ADDRESS *rfc822_cpy_adr (ADDRESS *adr);

void rfc822_skipws (char **s);


#endif /* __EMSD822_H__ */
