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

#ifndef __EMSDMAIL_H__
#define	__EMSDMAIL_H__


/* Build parameters */

#define MAILTMPLEN	1024		/* size of a temporary buffer */


/* Constants */

#define NIL		0		/* convenient name */
#define T		1		/* opposite of NIL */
#define LONGT		(long) 1	/* long T */

#undef ERROR
#define WARN		(long) 1	/* mm_log warning type */
#define ERROR		(long) 2	/* mm_log error type */
#define PARSE		(long) 3	/* mm_log parse error type */



/*
 * Message structures
 */

/* Item in an address list */
	
#define ADDRESS struct mail_address
ADDRESS
{
    char * 	    personal;		/* personal name phrase */
    char * 	    adl;		/* at-domain-list source route */
    char * 	    mailbox;		/* mailbox name */
    char * 	    host;		/* domain name of mailbox's host */
    char * 	    error;		/* error in address from SMTP module */
    ADDRESS * 	    next;		/* pointer to next address in list */
};


/* Message envelope */

typedef struct mail_envelope
{
    char * 	    remail;		/* remail header if any */
    ADDRESS * 	    return_path;	/* error return address */
    char * 	    date;		/* message composition date string */
    ADDRESS * 	    from;		/* originator address list */
    ADDRESS * 	    sender;		/* sender address list */
    ADDRESS * 	    reply_to;		/* reply address list */
    char * 	    subject;		/* message subject string */
    ADDRESS * 	    to;			/* primary recipient list */
    ADDRESS * 	    cc;			/* secondary recipient list */
    ADDRESS * 	    bcc;		/* blind secondary recipient list */
    char * 	    in_reply_to;	/* replied message ID */
    char * 	    message_id;		/* message ID */
    char * 	    newsgroups;		/* USENET newsgroups */
} ENVELOPE;

/*
 * Primary body types
 *
 * If you change any of these you must also change body_types in rfc822.c
 */

extern char *body_types[];	/* defined body type strings */

#define TYPETEXT		0	/* unformatted text */
#define TYPEMULTIPART		1	/* multiple part */
#define TYPEMESSAGE		2	/* encapsulated message */
#define TYPEAPPLICATION		3	/* application data */
#define TYPEAUDIO		4	/* audio */
#define TYPEIMAGE		5	/* static image */
#define TYPEVIDEO		6	/* video */
#define TYPEOTHER		7	/* unknown */
#define TYPEMAX			15	/* maximum type code */


/*
 * Body encodings
 *
 * If you change any of these you must also change body_encodings in rfc822.c
 */

extern char *body_encodings[];	/* defined body encoding strings */

#define ENC7BIT			0	/* 7 bit SMTP semantic data */
#define ENC8BIT			1	/* 8 bit SMTP semantic data */
#define ENCBINARY		2	/* 8 bit binary data */
#define ENCBASE64		3	/* base-64 encoded data */
#define ENCQUOTEDPRINTABLE	4	/* human-readable 8-as-7 bit data */
#define ENCOTHER		5	/* unknown */
#define ENCMAX			10	/* maximum encoding code */


/* Body contents */

#define BINARY		void
#define BODY		struct mail_body
#define MESSAGE		struct mail_body_message
#define PARAMETER	struct mail_body_parameter
#define PART		struct mail_body_part


/* Message content (ONLY for parsed messages) */

MESSAGE
{
    ENVELOPE * 	    env;		/* message envelope */
    BODY * 	    body;		/* message body */
    char * 	    text;		/* message in RFC-822 form */
    unsigned long   offset;		/* offset of text from header */
};


/* Parameter list */

PARAMETER
{
  char * 	  attribute;		/* parameter attribute name */
  char * 	  value;		/* parameter value */
  PARAMETER * 	  next;			/* next parameter in list */
};


/* Message body structure */

BODY
{
    unsigned short  type;		/* body primary type */
    unsigned short  encoding;		/* body transfer encoding */
    char * 	    subtype;		/* subtype string */
    PARAMETER *     parameter;		/* parameter list */
    char * 	    id;			/* body identifier */
    char * 	    description;	/* body description */
    union
    {
	/* different ways of accessing contents */
	unsigned char * text;		/* body text (+ enc. msg composing) */
	BINARY * 	binary;		/* body binary */
	PART * 		part;		/* body part list */
	MESSAGE 	msg;		/* body encapsulated message */
					/* (PARSE ONLY) */
    } contents;
    struct
    {
	unsigned long 	lines;		/* size in lines */
	unsigned long 	bytes;		/* size in bytes */
	unsigned long 	ibytes;		/* internal size in bytes */
					/* (drivers ONLY!!) */
    } size;
};


/* Multipart content list */

PART
{
    BODY 	    body;		/* body information for this part */
    unsigned long   offset;		/* offset from body origin */
    PART * 	    next;		/* next body part */
};


/* String structure */

#define STRINGDRIVER struct string_driver

typedef struct mailstring
{
    void * 	    data;		/* driver-dependent data */
    unsigned long   data1;		/* driver-dependent data */
    unsigned long   size;		/* total length of string */
    char * 	    chunk;		/* base address of chunk */
    unsigned long   chunksize;		/* size of chunk */
    unsigned long   offset;		/* offset of this chunk in base */
    char * 	    curpos;		/* current position in chunk */
    unsigned long   cursize;		/* # of bytes remaining in chunk */
    STRINGDRIVER *  dtb;		/* driver to handle this string type */
} STRING;


/* Dispatch table for string driver */

STRINGDRIVER
{
    /* initialize string driver */
    void (*init) (STRING *s,void *data,unsigned long size);

    /* get next character in string */
    char (*next) (STRING *s);

    /* set position in string */
    void (*setpos) (STRING *s,unsigned long i);
};


/* Stringstruct access routines */

#define INIT(s, d, data, size)	((*((s)->dtb = &d)->init) (s,data,size))
#define SIZE(s)			((s)->size - GETPOS (s))
#define CHR(s)			(*(s)->curpos)
#define SNX(s)			(--(s)->cursize ? \
				 *(s)->curpos++ : (*(s)->dtb->next) (s))
#define GETPOS(s)		((s)->offset + ((s)->curpos - (s)->chunk))
#define SETPOS(s, i)		(*(s)->dtb->setpos) (s,i)


/* Mail I/O stream */
	
typedef struct mail_stream
{
    char *	    pHeaderText;	/* pointer to RFC-822 header text */
    char *	    pBodyText;		/* pointer to RFC-822 body text */
    ENVELOPE * 	    pEnv;		/* pointer to message envelope */
    BODY * 	    pBody;		/* pointer to message body */
} MAILSTREAM;


void
mail_link(void * hDriver);


MAILSTREAM *
mail_open (MAILSTREAM * pStream,
	   char * pMailFileName,
	   long options,
	   char * pLocalHostName);

MAILSTREAM *
mail_close(MAILSTREAM * pStream);


ENVELOPE *
mail_newenvelope(void);

void
mail_free_env(ENVELOPE * pEnv);

void
mail_free_envelope(ENVELOPE ** ppEnv);


ADDRESS *
mail_newaddr(void);


void
mail_free_address(ADDRESS ** ppAddress);


BODY *
mail_newbody(void);


PARAMETER *
mail_newbody_parameter(void);


PART *
mail_newbody_part(void);


void
mail_free_body(BODY ** ppBody);


ENVELOPE *
mail_fetchstructure(MAILSTREAM * pStream,
		    long msgno,
		    BODY ** ppBody);


char *
mail_fetchheader(MAILSTREAM * pStream,
		 long msgno);


char *
mail_fetchtext (MAILSTREAM *pStream,
		long msgno);


void
mm_log(char * pString,
       long errFlag);

#endif /* __EMSDMAIL_H__ */
