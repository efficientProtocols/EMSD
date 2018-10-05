/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of Neda-EMSD. An implementation of RFC-2524.
 * 
 * Neda-EMSD is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Neda-EMSD; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 * Program:	RFC-822 routines (originally from SMTP)
 *
 * 
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	27 July 1988
 * Last Edited:	4 October 1994
 *
 * Sponsorship:	The original version of this work was developed in the
 *		Symbolic Systems Resources Group of the Knowledge Systems
 *		Laboratory at Stanford University in 1987-88, and was funded
 *		by the Biomedical Research Technology Program of the National
 *		Institutes of Health under grant number RR-00785.
 *
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1994 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  This software is made available "as is", and
 * THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
 * DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
 * INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
 * WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */


#include "estd.h"
#include "eh.h"
#include "emsdd.h"
#include "emsdmail.h"
#include "emsd822.h"

#define	mm_log(s, x)	EH_problem(s)

void *
cpystr(char * pSource)
{
    char *	    pDest;

    if (pSource == NULL)
    {
	return NULL;
    }

    if ((pDest = OS_allocStringCopy(pSource)) == NULL)
    {
	EH_fatal("cpystr");
    }

    return pDest;
}

#define	fs_get(len)	OS_alloc(len)

void
fs_give(void ** ppSrc)
{
    OS_free(*ppSrc);
    *ppSrc = NULL;
}

/* Token delimiting special characters */

				/* full RFC-822 specials */
const char *rspecials =  "()<>@,;:\\\"[].";
				/* body token specials */
const char *tspecials = " ()<>@,;:\\\"[]./?=";


/* Once upon a time, CSnet had a mailer which assigned special semantics to
 * dot in e-mail addresses.  For the sake of that mailer, dot was added to
 * the RFC-822 definition of `specials', even though it had numerous bad side
 * effects:
 *   1)	It broke mailbox names on systems which had dots in user names, such as
 *	Multics and TOPS-20.  RFC-822's syntax rules require that `Admin . MRC'
 *	be considered equivalent to `Admin.MRC'.  Fortunately, few people ever
 *	tried this in practice.
 *   2) It required that all personal names with an initial be quoted, a widely
 *	detested user interface misfeature.
 *   3)	It made the parsing of host names be non-atomic for no good reason.
 * To work around these problems, the following alternate specials lists are
 * defined.  hspecials and wspecials are used in lieu of rspecials, and
 * ptspecials are used in lieu of tspecials.  These alternate specials lists
 * make the parser work a lot better in the real world.  It ain't politically
 * correct, but it lets the users get their job done!
 */

				/* parse-host specials */
const char *hspecials = " ()<>@,;:\\\"";
				/* parse-word specials */
const char *wspecials = " ()<>@,;:\\\"[]";
				/* parse-token specials for parsing */
const char *ptspecials = " ()<>@,;:\\\"[]/?=";



/* Convert string to all uppercase
 * Accepts: string pointer
 * Returns: string pointer
 */

static char *
ucase(char * p)
{
  char *	  pRetval = p;

  /* For each character in the string... */
  for (; *p != '\0' ; p++)
  {
      /* ... if it's a lower-case character... */
      if (islower(*p))
      {
	  /* ... convert it to upper case. */
	  *p = toupper(*p);
      }
  }

  return (pRetval);			/* return string */
}


/* Case independent slow search within a word
 * Accepts: base string
 *	    number of tries left
 *	    pattern string
 *	    multi-word hunt flag
 * Returns: T if pattern exists inside base, else NIL
 */

static long
ssrc(char **base, long *tries, char *pat, long multiword)
{
  register char *s = *base;
  register long c = (multiword ? *tries :
		     (*tries < (long) 4 ? *tries : (long) 4));
  register unsigned char *p = (unsigned char *) pat;
				/* search character at a time */
  if (c > 0) do if (!((*p ^ *s++) & (unsigned char) 0xDF)) {
    char *ss = s;		/* remember were we began */
    do if (!*++p) return T;	/* match case-independent until end */
    while (!((*p ^ *s++) & (unsigned char) 0xDF));
    s = ss;			/* try next character */
    p = (unsigned char *) pat;	/* start at beginning of pattern */
  } while (--c);		/* continue if multiword or not at boundary */
  *tries -= s - *base;		/* update try count */
  *base = s;			/* update base */
  return NIL;			/* string not found */
}



/* Case independent search (fast on 32-bit machines)
 * Accepts: base string
 *	    length of base string
 *	    pattern string
 *	    length of pattern string
 * Returns: T if pattern exists inside base, else NIL
 */

#define Word unsigned long

static long
search(char *s, long c, char *pat, long patc)
{
  register Word m;
  long cc;
  union {
    unsigned long wd;
    char ch[9];
  } wdtest;
  strcpy (wdtest.ch,"AAAA1234");/* constant for word testing */
				/* validate arguments, c becomes # of tries */
  if (!(s && c > 0 && pat && patc > 0 && (c -= (patc - 1)) > 0)) return NIL;
				/* do slow search if long is not 4 chars */
#ifdef MSDOS
	return ssrc(&s,&c,pat,(long) T);
#else
  if (wdtest.wd != 0x41414141) return ssrc (&s,&c,pat,(long) T);
  /*
   * Fast search algorithm XORs the mask with each word from the base string
   * and complements the result. This will give bytes of all ones where there
   * are matches.  We then mask out the high order and case bits in each byte
   * and add 21 (case + overflow) to all the bytes.  If we have a resulting
   * carry, then we have a match.
   */
  if ((cc = ((int) s & 3))) {	/* any chars before word boundary? */
    c -= (cc = 4 - cc);		/* yes, calculate how many, account for them */
				/* search through those */
    if (ssrc (&s,&cc,pat,(long) NIL)) return T;
  }
  m = *pat * 0x01010101;	/* search mask */
  do {				/* interesting word? */
    if (0x80808080&(0x21212121+(0x5F5F5F5F&~(m^*(Word *) s)))) {
				/* yes, commence a slow search through it */
      if (ssrc (&s,&c,pat,(long) NIL)) return T;
    }
    else s += 4,c -= 4;		/* try next word */
  } while (c > 0);		/* continue until end of string */
  return NIL;			/* string not found */
#endif
}


/* Length of string after strcrlfcpy applied
 * Accepts: source string
 * Returns: length of string
 */

unsigned long
strcrlflen (STRING *s)
{
  unsigned long pos = GETPOS (s);
  unsigned long i = SIZE (s);
  unsigned long j = i;
  while (j--) switch (SNX (s)) {/* search for newlines */
  case '\015':			/* unlikely carriage return */
    if (j && (CHR (s) == '\012')) {
      SNX (s);			/* eat the line feed */
      j--;
    }
    break;
  case '\012':			/* line feed? */
    i++;
  default:			/* ordinary chararacter */
    break;
  }
  SETPOS (s,pos);		/* restore old position */
  return i;
}


/* Body formats constant strings, must match definitions in mail.h */

char *body_types[TYPEMAX+1] = {
  "TEXT", "MULTIPART", "MESSAGE", "APPLICATION", "AUDIO", "IMAGE", "VIDEO",
  "X-UNKNOWN"
};


char *body_encodings[ENCMAX+1] = {
  "7BIT", "8BIT", "BINARY", "BASE64", "QUOTED-PRINTABLE", "X-UNKNOWN"
};



/* Write RFC822 route-address to string
 * Accepts: pointer to destination string
 *	    address to interpret
 */

void rfc822_address (char *dest,ADDRESS *adr)
{
  if (adr && adr->host) {	/* no-op if no address */
    if (adr->adl) {		/* have an A-D-L? */
      strcat (dest,adr->adl);
      strcat (dest,":");
    }
				/* write mailbox name */
    rfc822_cat (dest,adr->mailbox,wspecials);
    if (*adr->host != '@') {	/* unless null host (HIGHLY discouraged!) */
      strcat (dest,"@");	/* host delimiter */
      strcat (dest,adr->host);	/* write host name */
    }
  }
}


/* Concatenate RFC822 string
 * Accepts: pointer to destination string
 *	    pointer to string to concatenate
 *	    list of special characters
 */

void rfc822_cat (char *dest,char *src,const char *specials)
{
  char *s;
  if (strpbrk (src,specials)) {	/* any specials present? */
    strcat (dest,"\"");		/* opening quote */
				/* truly bizarre characters in there? */
    while ((s = strpbrk (src,"\\\""))) {
      strncat (dest,src,(unsigned int)(s-src));	/* yes, output leader */
      strcat (dest,"\\");	/* quoting */
      strncat (dest,s,1);	/* output the bizarre character */
      src = ++s;		/* continue after the bizarre character */
    }
    if (*src) strcat (dest,src);/* output non-bizarre string */
    strcat (dest,"\"");		/* closing quote */
  }
  else strcat (dest,src);	/* otherwise it's the easy case */
}


/* Write RFC822 address from header line
 * Accepts: pointer to destination string pointer
 *	    pointer to header type
 *	    message to interpret
 *	    address to interpret
 */

void rfc822_address_line (char **header,char *type,ENVELOPE *env,ADDRESS *adr)
{
  char *t,tmp[MAILTMPLEN];
  long i,len,n = 0;
  char *s = (*header += strlen (*header));
  if (adr) {			/* do nothing if no addresses */
    if (env && env->remail) strcat (s,"ReSent-");
    strcat (s,type);		/* write header name */
    strcat (s,": ");
    s += (len = strlen (s));	/* initial string length */
    do {			/* run down address list */
      *(t = tmp) = '\0';	/* initially empty string */
				/* start of group? */
      if (adr->mailbox && !adr->host) {
				/* yes, write group name */
	rfc822_cat (t,adr->mailbox,rspecials);
	strcat (t,": ");	/* write group identifier */
	n++;			/* in a group, suppress expansion */
      }
      else {			/* not start of group */
	if (!adr->host && n) {	/* end of group? */
	  strcat (t,";");	/* write close delimiter */
	  n--;			/* no longer in a group */
	}
	else if (!n) {		/* only print if not inside a group */
				/* simple case? */
	  if (!(adr->personal || adr->adl)) rfc822_address (t,adr);
	  else {		/* no, must use phrase <route-addr> form */
	    if (adr->personal) rfc822_cat (t,adr->personal,rspecials);
	    strcat (t," <");	/* write address delimiter */
				/* write address */
	    rfc822_address (t,adr);
	    strcat (t,">");	/* closing delimiter */
	  }
	}
				/* write delimiter for next recipient */
	if (!n && adr->next && adr->next->mailbox) strcat (t,", ");
      }
				/* if string would overflow */
      if ((len += (i = strlen (t))) > 78) {
	len = 4 + i;		/* continue it on a new line */
	*s++ = '\015'; *s++ = '\012';
	*s++ = ' '; *s++ = ' '; *s++ = ' '; *s++ = ' ';
      }
      while (*t) *s++ = *t++;	/* write this address */
    } while ((adr = adr->next));
				/* tie off header line */
    *s++ = '\015'; *s++ = '\012'; *s = '\0';
    *header = s;		/* set return value */
  }
}


/* Write RFC822 text from header line
 * Accepts: pointer to destination string pointer
 *	    pointer to header type
 *	    message to interpret
 *	    pointer to text
 */

void rfc822_header_line (char **header,char *type,ENVELOPE *env,char *text)
{
  if (text) sprintf ((*header += strlen (*header)),"%s%s: %s\015\012",
		     env->remail ? "ReSent-" : "",type,text);
}



/* Subtype defaulting (a no-no, but regretably necessary...)
 * Accepts: type code
 * Returns: default subtype name
 */

char *rfc822_default_subtype (unsigned short type)
{
  switch (type) {
  case TYPETEXT:		/* default is TEXT/PLAIN */
    return "PLAIN";
  case TYPEMULTIPART:		/* default is MULTIPART/MIXED */
    return "MIXED";
  case TYPEMESSAGE:		/* default is MESSAGE/RFC822 */
    return "RFC822";
  case TYPEAPPLICATION:		/* default is APPLICATION/OCTET-STREAM */
    return "OCTET-STREAM";
  case TYPEAUDIO:		/* default is AUDIO/BASIC */
    return "BASIC";
  default:			/* others have no default subtype */
    return "UNKNOWN";
  }
}


/* RFC822 parsing routines */


/* Parse an RFC822 message
 * Accepts: pointer to return envelope
 *	    pointer to return body
 *	    pointer to header
 *	    header byte count
 *	    pointer to body stringstruct
 *	    pointer to local host name
 *	    pointer to scratch buffer
 */

void rfc822_parse_msg (ENVELOPE **en,BODY **bdy,char *s,unsigned long i,
		       STRING *bs,char *host,char *tmp)
{
  char c,*t,*d;
  ENVELOPE *env = (*en = mail_newenvelope ());
  BODY *body = bdy ? (*bdy = mail_newbody ()) : NIL;
  long MIMEp = NIL;		/* flag that MIME semantics are in effect */
  while (i && *s != '\n') {	/* until end of header */
    t = tmp;			/* initialize buffer pointer */
    c = ' ';			/* and previous character */
    while (c) {			/* collect text until logical end of line */
      switch (*s) {
      case '\n':		/* newline, possible end of logical line */
				/* tie off unless next line starts with WS */
	if (s[1] != ' ' && s[1] != '\t') *t = c = '\0';
	break;
      case '\015':		/* return, unlikely but just in case */
      case '\t':		/* tab */
      case ' ':			/* here for all forms of whitespace */
				/* insert whitespace if not already there */
	if (c != ' ') *t++ = c = ' ';
	break;
      default:			/* all other characters */
	*t++ = c = *s;		/* insert the character into the line */
	break;
      }
      if (--i) s++;		/* get next character */
      else *t++ = c = '\0';	/* end of header */
    }

				/* find header item type */
    if ((t = d = strchr (tmp,':'))) {
      *d++ = '\0';		/* tie off header item, point at its data */
      while (*d == ' ') d++;	/* flush whitespace */
      while ((tmp < t--) && (*t == ' ')) *t = '\0';
      switch (*ucase (tmp)) {	/* dispatch based on first character */
      case '>':			/* possible >From: */
	if (!strcmp (tmp+1,"FROM")) rfc822_parse_adrlist (&env->from,d,host);
	break;
      case 'B':			/* possible bcc: */
	if (!strcmp (tmp+1,"CC")) rfc822_parse_adrlist (&env->bcc,d,host);
	break;
      case 'C':			/* possible cc: or Content-<mumble>*/
	if (!strcmp (tmp+1,"C")) rfc822_parse_adrlist (&env->cc,d,host);
	else if ((tmp[1] == 'O') && (tmp[2] == 'N') && (tmp[3] == 'T') &&
		 (tmp[4] == 'E') && (tmp[5] == 'N') && (tmp[6] == 'T') &&
		 (tmp[7] == '-') && body &&
		 (MIMEp || (search (s-1,i,"\012MIME-Version",(long) 13))))
	  rfc822_parse_content_header (body,tmp+8,d);
	break;
      case 'D':			/* possible Date: */
	if (!env->date && !strcmp (tmp+1,"ATE")) env->date = cpystr (d);
	break;
      case 'F':			/* possible From: */
	if (!strcmp (tmp+1,"ROM")) rfc822_parse_adrlist (&env->from,d,host);
	break;
      case 'I':			/* possible In-Reply-To: */
	if (!env->in_reply_to && !strcmp (tmp+1,"N-REPLY-TO"))
	  env->in_reply_to = cpystr (d);
	break;

      case 'M':			/* possible Message-ID: or MIME-Version: */
	if (!env->message_id && !strcmp (tmp+1,"ESSAGE-ID"))
	  env->message_id = cpystr (d);
	else if (!strcmp (tmp+1,"IME-VERSION")) {
				/* tie off at end of phrase */
	  if ((t = rfc822_parse_phrase (d))) *t = '\0';
	  rfc822_skipws (&d);	/* skip whitespace */
				/* known version? */
	  if (strcmp (d,"1.0") && strcmp (d,"RFC-XXXX"))
	    mm_log ("Warning: message has unknown MIME version",PARSE);
	  MIMEp = T;		/* note that we are MIME */
	}
	break;
      case 'N':			/* possible Newsgroups: */
	if (!env->newsgroups && !strcmp (tmp+1,"EWSGROUPS")) {
	  t = env->newsgroups = (char *) fs_get (1 + strlen (d));
	  while ((c = *d++)) if (c != ' ' && c != '\t') *t++ = c;
	  *t++ = '\0';
	}
	break;
      case 'R':			/* possible Reply-To: */
	if (!strcmp (tmp+1,"EPLY-TO"))
	  rfc822_parse_adrlist (&env->reply_to,d,host);
	break;
      case 'S':			/* possible Subject: or Sender: */
	if (!env->subject && !strcmp (tmp+1,"UBJECT"))
	  env->subject = cpystr (d);
	else if (!strcmp (tmp+1,"ENDER"))
	  rfc822_parse_adrlist (&env->sender,d,host);
	break;
      case 'T':			/* possible To: */
	if (!strcmp (tmp+1,"O")) rfc822_parse_adrlist (&env->to,d,host);
	break;
      default:
	break;
      }
    }
  }
				/* default Sender: and Reply-To: to From: */
  if (!env->sender) env->sender = rfc822_cpy_adr (env->from);
  if (!env->reply_to) env->reply_to = rfc822_cpy_adr (env->from);
				/* now parse the body */
  if (body) rfc822_parse_content (body,bs,host,tmp);
}



/* Parse a message body content
 * Accepts: pointer to body structure
 *	    body string
 *	    pointer to local host name
 *	    pointer to scratch buffer
 */

void rfc822_parse_content (BODY *body,STRING *bs,char *h,char *t)
{
  char c,c1,*s,*s1;
  unsigned long pos = GETPOS (bs);
  unsigned long i = SIZE (bs);
  unsigned long j,k,m = 0;
  PARAMETER *param;
  PART *part = NIL;
  body->size.ibytes = i;	/* note body size in all cases */
  body->size.bytes = ((body->encoding == ENCBINARY) ||
		      (body->type == TYPEMULTIPART)) ? i : strcrlflen (bs);
  switch (body->type) {		/* see if anything else special to do */
  case TYPETEXT:		/* text content */
    if (!body->subtype)		/* default subtype */
      body->subtype = cpystr (rfc822_default_subtype (body->type));
    if (!body->parameter) {	/* default parameters */
      body->parameter = mail_newbody_parameter ();
      body->parameter->attribute = cpystr ("CHARSET");
      body->parameter->value = cpystr ("US-ASCII");
    }
				/* count number of lines */
    while (i--) if ((SNX (bs)) == '\n') body->size.lines++;
    break;

  case TYPEMESSAGE:		/* encapsulated message */
    body->contents.msg.env = NIL;
    body->contents.msg.body = NIL;
    body->contents.msg.text = NIL;
    body->contents.msg.offset = pos;
				/* encapsulated RFC-822 message? */
    if (!strcmp (body->subtype,"RFC822")) {
      if ((body->encoding == ENCBASE64) ||
	  (body->encoding == ENCQUOTEDPRINTABLE)
	  || (body->encoding == ENCOTHER)) {
	mm_log ("Nested encoding of message contents",PARSE);
	return;
      }
				/* hunt for blank line */
      for (c = '\012',j = 0; (i > j) && ((c != '\012') || (CHR(bs) != '\012'));
	   SNX (bs),j++) if (CHR (bs) != '\015') c = CHR (bs);
      if (i > j) SNX (bs);	/* unless no more text, body starts here */
				/* note body text offset and header size */
      j = (body->contents.msg.offset = GETPOS (bs)) - pos;
      SETPOS (bs,pos);		/* copy header string */
      s = (char *) fs_get (j + 1);
      for (s1 = s,k = j; k--; *s1++ = SNX (bs));
      s[j] = '\0';		/* tie off string (not really necessary) */
				/* now parse the body */
      rfc822_parse_msg (&body->contents.msg.env,&body->contents.msg.body,s,j,
			bs,h,t);
      fs_give ((void **) &s);	/* free header string */
      SETPOS (bs,pos);		/* restore position */
    }
				/* count number of lines */
    while (i--) if (SNX (bs) == '\n') body->size.lines++;
    break;
  case TYPEMULTIPART:		/* multiple parts */
    if ((body->encoding == ENCBASE64) || (body->encoding == ENCQUOTEDPRINTABLE)
	|| (body->encoding == ENCOTHER)) {
      mm_log ("Nested encoding of multipart contents",PARSE);
      return;
    }				/* find cookie */
    for (*t = '\0',param = body->parameter; param && !*t; param = param->next)
      if (!strcmp (param->attribute,"BOUNDARY")) strcpy (t,param->value);
    if (!*t) strcpy (t,"-");	/* yucky default */
    j = strlen (t);		/* length of cookie and header */
    c = '\012';			/* initially at beginning of line */

    while (i > j) {		/* examine data */
      m = GETPOS (bs);		/* note position */
      if (m) m--;		/* get position in front of character */
      switch (c) {		/* examine each line */
      case '\015':		/* handle CRLF form */
	if (CHR (bs) == '\012'){/* following LF? */
	  c = SNX (bs); i--;	/* yes, slurp it */
	}
      case '\012':		/* at start of a line, start with -- ? */
	if (i-- && ((c = SNX (bs)) == '-') && i-- && ((c = SNX (bs)) == '-')) {
				/* see if cookie matches */
	  for (k = j,s = t; i-- && *s++ == (c = SNX (bs)) && --k;);
	  if (k) break;		/* strings didn't match if non-zero */
				/* look at what follows cookie */
	  if (i && i--) switch (c = SNX (bs)) {
	  case '-':		/* at end if two dashes */
	    if ((i && i--) && ((c = SNX (bs)) == '-') &&
		((i && i--) ? (((c = SNX (bs)) == '\015') || (c=='\012')):T)) {
				/* if have a final part calculate its size */
	      if (part) part->body.size.bytes = (m > part->offset) ?
		(m - part->offset) : 0;
	      part = NIL; i = 1; /* terminate scan */
	    }
	    break;
	  case '\015':		/* handle CRLF form */
	    if (i && CHR (bs) == '\012') {
	      c = SNX (bs); i--;/* yes, slurp it */
	    }
	  case '\012':		/* new line */
	    if (part) {		/* calculate size of previous */
	      part->body.size.bytes = (m>part->offset) ? (m-part->offset) : 0;
				/* instantiate next */
	      part = part->next = mail_newbody_part ();
	    }			/* otherwise start new list */
	    else part = body->contents.part = mail_newbody_part ();
				/* note offset from main body */
	    part->offset = GETPOS (bs);
	    break;
	  default:		/* whatever it was it wasn't valid */
	    break;
	  }
	}
	break;
      default:			/* not at a line */
	c = SNX (bs); i--;	/* get next character */
	break;
      }				/* calculate size of any final part */
    }
    if (part) part->body.size.bytes = i + ((GETPOS (bs) > part->offset) ?
					   (GETPOS (bs) - part->offset) : 0);

				/* parse body parts */
    for (part = body->contents.part; part; part = part->next) {
      SETPOS (bs,part->offset);	/* move to that part of the body */
				/* get size of this part, ignore if empty */
      if ((i = part->body.size.bytes)) {
				/* until end of header */
	while (i && ((c = CHR (bs)) != '\015') && (c != '\012')) {
	  s1 = t;		/* initialize buffer pointer */
	  c = ' ';		/* and previous character */
	  while (c) {		/* collect text until logical end of line */
	    switch (c1 = SNX (bs)) {
	    case '\015':	/* return */
	      if (i && (CHR (bs) == '\012')) {
		SNX (bs); i--;	/* eat any LF following */
	      }
	    case '\012':	/* newline, possible end of logical line */
	      if (!i || ((CHR (bs) != ' ') && (CHR (bs) != '\t')))
		*s1 = c = '\0';	/* tie off unless continuation */
	      break;
	    case '\t':		/* tab */
	    case ' ':		/* insert whitespace if not already there */
	      if (c != ' ') *s1++ = c = ' ';
	      break;
	    default:		/* all other characters */
	      *s1++ = c = c1;	/* insert the character into the line */
	      break;
	    }
				/* end of data ties off the header */
	    if (!--i) *s1++ = c = '\0';
	  }
				/* find header item type */
	  if ((s = strchr (t,':'))) {
	    *s++ = '\0';	/* tie off header item, point at its data */
				/* flush whitespace */
	    while (*s == ' ') s++;
	    if ((s1 = strchr (ucase (t),' '))) *s1 = '\0';
	    if ((t[0] == 'C') && (t[1] == 'O') && (t[2] == 'N') &&
		(t[3] == 'T') && (t[4] == 'E') && (t[5] == 'N') &&
		(t[6] == 'T') && (t[7] == '-'))
	      rfc822_parse_content_header (&part->body,t+8,s);
	  }
	}			/* skip header trailing (CR)LF */
	if (i && (CHR (bs) =='\015')) {i--; SNX (bs);}
	if (i && (CHR (bs) =='\012')) {i--; SNX (bs);}
	j = bs->size;		/* save upper level size */
      }
				/* set offset for next level, fake size to i */
      bs->size = (part->offset = GETPOS (bs)) + i;
				/* now parse it */
      rfc822_parse_content (&part->body,bs,h,t);
      bs->size = j;		/* restore current level size */
    }
    break;
  default:			/* nothing special to do in any other case */
    break;
  }
}



/* Parse RFC822 body content header
 * Accepts: body to write to
 *	    possible content name
 *	    remainder of header
 */

void rfc822_parse_content_header (BODY *body,char *name,char *s)
{
  PARAMETER *param = NIL;
  char tmp[MAILTMPLEN];
  char c,*t;
  long i;
  switch (*name) {
  case 'I':			/* possible Content-ID */
    if (!(strcmp (name+1,"D") || body->id)) body->id = cpystr (s);
    break;
  case 'D':			/* possible Content-Description */
    if (!(strcmp (name+1,"ESCRIPTION")) || body->description)
      body->description = cpystr (s);
    break;
  case 'T':			/* possible Content-Type/Transfer-Encoding */
    if (!(strcmp (name+1,"YPE") || body->type || body->subtype ||
	  body->parameter)) {
				/* get type word */
      if (!(name = rfc822_parse_word (s,ptspecials))) break;
      c = *name;		/* remember delimiter */
      *name = '\0';		/* tie off type */
      ucase (s);		/* search for body type */
      for (i=0; (i<=TYPEMAX) && body_types[i] && strcmp(s,body_types[i]); i++);
      if (i > TYPEMAX) body->type = TYPEOTHER;
      else {			/* if empty slot, assign it to this type */
	if (!body_types[i]) body_types[i] = cpystr (s);
	body->type = i;		/* set body type */
      }
      *name = c;		/* restore delimiter */
      rfc822_skipws (&name);	/* skip whitespace */
      if ((*name == '/') &&	/* subtype? */
	  (name = rfc822_parse_word ((s = ++name),ptspecials))) {
	c = *name;		/* save delimiter */
	*name = '\0';		/* tie off subtype */
	rfc822_skipws (&s);	/* copy subtype */
	body->subtype = ucase (cpystr (s ? s :
				       rfc822_default_subtype (body->type)));
	*name = c;		/* restore delimiter */
	rfc822_skipws (&name);	/* skip whitespace */
      }
				/* subtype defaulting is a no-no, but... */
      else {
	body->subtype = cpystr (rfc822_default_subtype (body->type));
	if (!name) {		/* did the fool have a subtype delimiter? */
	  name = s;		/* barf, restore pointer */
	  rfc822_skipws (&name);/* skip leading whitespace */
	}
      }

				/* parameter list? */
      while (name && (*name == ';') &&
	     (name = rfc822_parse_word ((s = ++name),ptspecials))) {
	c = *name;		/* remember delimiter */
	*name = '\0';		/* tie off attribute name */
	rfc822_skipws (&s);	/* skip leading attribute whitespace */
	if (!*s) *name = c;	/* must have an attribute name */
	else {			/* instantiate a new parameter */
	  if (body->parameter) param = param->next = mail_newbody_parameter ();
	  else param = body->parameter = mail_newbody_parameter ();
	  param->attribute = ucase (cpystr (s));
	  *name = c;		/* restore delimiter */
	  rfc822_skipws (&name);/* skip whitespace before equal sign */
	  if ((*name != '=') ||	/* missing value is a no-no too */
	      !(name = rfc822_parse_word ((s = ++name),ptspecials)))
	    param->value = cpystr ("UNKNOWN");
	  else {		/* good, have equals sign */
	    c = *name;		/* remember delimiter */
	    *name = '\0';	/* tie off value */
	    rfc822_skipws (&s);	/* skip leading value whitespace */
	    if (*s) param->value = rfc822_cpy (s);
	    *name = c;		/* restore delimiter */
	    rfc822_skipws (&name);
	  }
	}
      }
      if (!name) {		/* must be end of poop */
	if (param && param->attribute)
	  sprintf (tmp,"Missing parameter value: %.80s",param->attribute);
	else strcpy (tmp,"Missing parameter");
	mm_log (tmp,PARSE);
      }
      else if (*name) {		/* must be end of poop */
	sprintf (tmp,"Junk at end of parameters: %.80s",name);
	mm_log (tmp,PARSE);
      }
    }
    else if (!strcmp (name+1,"RANSFER-ENCODING")) {
				/* flush out any confusing whitespace */
      if ((t = strchr (ucase (s),' '))) *t = '\0';
				/* search for body encoding */
      for (i = 0; (i <= ENCMAX) && body_encodings[i] &&
	   strcmp (s,body_encodings[i]); i++);
      if (i > ENCMAX) body->type = ENCOTHER;
      else {			/* if empty slot, assign it to this type */
	if (!body_encodings[i]) body_encodings[i] = cpystr (s);
	body->encoding = i;	/* set body encoding */
      }
    }
    break;
  default:			/* otherwise unknown */
    break;
  }
}




/* Parse RFC822 address list
 * Accepts: address list to write to
 *	    input string
 *	    default host name
 */

void rfc822_parse_adrlist (ADDRESS **lst,char *string,char *host)
{
  char tmp[MAILTMPLEN];
  char *p,*s;
  long n = 0;
  ADDRESS *last = *lst;
  ADDRESS *adr;
				/* run to tail of list */
  if (last) while (last->next) last = last->next;
  rfc822_skipws (&string);	/* skip leading WS */
				/* loop until string exhausted */
  if (*string != '\0') while ((p = string)) {
				/* see if start of group */
    while ((*p == ':') || (p = rfc822_parse_phrase (string))) {
      s = p;			/* end of phrase */
      rfc822_skipws (&s);	/* find delimiter */
      if (*s == ':') {		/* really a group? */
	n++;			/* another level */
	*p++ = '\0';		/* tie off group name */
	rfc822_skipws (&p);	/* skip subsequent whitespace */
				/* write as address */
	(adr = mail_newaddr ())->mailbox = rfc822_cpy (string);
	if (!*lst) *lst = adr;	/* first time through? */
	else last->next = adr;	/* no, append to the list */
	last = adr;		/* set for subsequent linking */
	string = p;		/* continue after this point */
      }
      else break;		/* bust out of this */
    }
    rfc822_skipws (&string);	/* skip any following whitespace */
    if (!string) break;		/* punt if unterminated group */
				/* if not empty group */
    if (*string != ';' || n <= 0) {
				/* got an address? */
      if ((adr = rfc822_parse_address (&string,host))) {
	if (!*lst) *lst = adr;	/* yes, first time through? */
	else last->next = adr;	/* no, append to the list */
	last = adr;		/* set for subsequent linking */
      }
      else if (string) {	/* bad mailbox */
	sprintf (tmp,"Bad mailbox: %.80s",string);
	mm_log (tmp,PARSE);
	break;
      }
    }

				/* handle end of group */
    if (string && *string == ';' && n >= 0) {
      n--;			/* out of this group */
      string++;			/* skip past the semicolon */
				/* append end of address mark to the list */
      last->next = (adr = mail_newaddr ());
      last = adr;		/* set for subsequent linking */
      rfc822_skipws (&string);	/* skip any following whitespace */
      switch (*string) {	/* see what follows */
      case ',':			/* another address? */
	++string;		/* yes, skip past the comma */
      case ';':			/* another end of group? */
      case '\0':		/* end of string */
	break;

      default:
	sprintf (tmp,"Junk at end of group: %.80s",string);
	mm_log (tmp,PARSE);
	break;
      }
    }
  }
  while (n-- > 0) {		/* if unterminated groups */
    last->next = (adr = mail_newaddr ());
    last = adr;			/* set for subsequent linking */
  }
}


/* Parse RFC822 address
 * Accepts: pointer to string pointer
 *	    default host
 * Returns: address
 *
 * Updates string pointer
 */

ADDRESS *rfc822_parse_address (char **string,char *defaulthost)
{
  char tmp[MAILTMPLEN];
  ADDRESS *adr;
  char c,*s;
  char *phrase;
  if (!string) return NIL;
  rfc822_skipws (string);	/* flush leading whitespace */

  /* This is much more complicated than it should be because users like
   * to write local addrspecs without "@localhost".  This makes it very
   * difficult to tell a phrase from an addrspec!
   * The other problem we must cope with is a route-addr without a leading
   * phrase.  Yuck!
   */

  if (*(s = *string) == '<') 	/* note start, handle case of phraseless RA */
    adr = rfc822_parse_routeaddr (s,string,defaulthost);
  else {			/* get phrase if any */
    if ((phrase = rfc822_parse_phrase (s)) &&
	(adr = rfc822_parse_routeaddr (phrase,string,defaulthost))) {
      *phrase = '\0';		/* tie off phrase */
				/* phrase is a personal name */
      adr->personal = rfc822_cpy (s);
    }
    else adr = rfc822_parse_addrspec (s,string,defaulthost);
  }
				/* analyze what follows */
  if (*string) switch (c = **string) {
  case ',':			/* comma? */
    ++*string;			/* then another address follows */
    break;
  case ';':			/* possible end of group? */
    break;			/* let upper level deal with it */

  default:
    s = isalnum (c) ? "Must use comma to separate addresses: %.80s" :
      "Junk at end of address: %.80s";
    sprintf (tmp,s,*string);
    mm_log (tmp,PARSE);
				/* falls through */
  case '\0':			/* null-specified address? */
    *string = NIL;		/* punt remainder of parse */
    break;
  }
  return adr;			/* return the address */
}


/* Parse RFC822 route-address
 * Accepts: string pointer
 *	    pointer to string pointer to update
 * Returns: address
 *
 * Updates string pointer
 */

ADDRESS *rfc822_parse_routeaddr (char *string,char **ret,char *defaulthost)
{
  char tmp[MAILTMPLEN];
  ADDRESS *adr;
  char *adl = NIL;
  char *routeend = NIL;
  if (!string) return NIL;
  rfc822_skipws (&string);	/* flush leading whitespace */
				/* must start with open broket */
  if (*string != '<') return NIL;
  if (string[1] == '@') {	/* have an A-D-L? */
    adl = ++string;		/* yes, remember that fact */
    while (*string != ':') {	/* search for end of A-D-L */
				/* punt if never found */
      if (*string == '\0') return NIL;
      ++string;			/* try next character */
    }
    *string = '\0';		/* tie off A-D-L */
    routeend = string;		/* remember in case need to put back */
  }
				/* parse address spec */
  if (!(adr = rfc822_parse_addrspec (++string,ret,defaulthost))) {
    if (adl) *routeend = ':';	/* put colon back since parse barfed */
    return NIL;
  }
				/* have an A-D-L? */
  if (adl) adr->adl = cpystr (adl);
				/* make sure terminated OK */
    if (*ret) if (**ret == '>') {
    ++*ret;			/* skip past the broket */
    rfc822_skipws (ret);	/* flush trailing WS */
				/* wipe pointer if at end of string */
    if (**ret == '\0') *ret = NIL;
    return adr;			/* return the address */
  }
  sprintf (tmp,"Unterminated mailbox: %.80s@%.80s",adr->mailbox,
	   *adr->host == '@' ? "<null>" : adr->host);
  mm_log (tmp,PARSE);
  return adr;			/* return the address */
}


/* Parse RFC822 address-spec
 * Accepts: string pointer
 *	    pointer to string pointer to update
 *	    default host
 * Returns: address
 *
 * Updates string pointer
 */

ADDRESS *rfc822_parse_addrspec (char *string,char **ret,char *defaulthost)
{
  ADDRESS *adr;
  char *end;
  char c,*s,*t;
  if (!string) return NIL;
  rfc822_skipws (&string);	/* flush leading whitespace */
				/* find end of mailbox */
  if (!(end = rfc822_parse_word (string,NIL))) return NIL;
  adr = mail_newaddr ();	/* create address block */
  c = *end;			/* remember delimiter */
  *end = '\0';			/* tie off mailbox */
				/* copy mailbox */
  adr->mailbox = rfc822_cpy (string);
  *end = c;			/* restore delimiter */
  t = end;			/* remember end of mailbox for no host case */
  rfc822_skipws (&end);		/* skip whitespace */
  if (*end == '@') {		/* have host name? */
    ++end;			/* skip delimiter */
    rfc822_skipws (&end);	/* skip whitespace */
    *ret = end;			/* update return pointer */
    				/* search for end of host */
    if ((end = rfc822_parse_word ((string = end),hspecials))) {
      c = *end;			/* remember delimiter */
      *end = '\0';		/* tie off host */
				/* copy host */
      adr->host = rfc822_cpy (string);
      *end = c;			/* restore delimiter */
    }
    else mm_log ("Missing or invalid host name after @",PARSE);
  }
  else end = t;			/* make person name default start after mbx */

				/* default host if missing */
  if (!adr->host) adr->host = cpystr (defaulthost);
  if (end && !adr->personal) {	/* try person name in comments if missing */
    while (*end == ' ') ++end;	/* see if we can find a person name here */
				/* found something that may be a name? */
    if ((*end == '(') && (t = s = rfc822_parse_phrase (end + 1))) {
      rfc822_skipws (&s);	/* heinous kludge for trailing WS in comment */
      if (*s == ')') {		/* look like good end of comment? */
	*t = '\0';		/* yes, tie off the likely name and copy */
	++end;			/* skip over open paren now */
	rfc822_skipws (&end);
	adr->personal = rfc822_cpy (end);
	end = ++s;		/* skip after it */
      }
    }
    rfc822_skipws (&end);	/* skip any other WS in the normal way */
  }
				/* set return to end pointer */
  *ret = (end && *end) ? end : NIL;
  return adr;			/* return the address we got */
}


/* Parse RFC822 phrase
 * Accepts: string pointer
 * Returns: pointer to end of phrase
 */

char *rfc822_parse_phrase (char *s)
{
  char *curpos;
  if (!s) return NIL;		/* no-op if no string */
				/* find first word of phrase */
  curpos = rfc822_parse_word (s,NIL);
  if (!curpos) return NIL;	/* no words means no phrase */
				/* check if string ends with word */
  if (*curpos == '\0') return curpos;
  s = curpos;			/* sniff past the end of this word and WS */
  rfc822_skipws (&s);		/* skip whitespace */
				/* recurse to see if any more */
  return (s = rfc822_parse_phrase (s)) ? s : curpos;
}


/* Parse RFC822 word
 * Accepts: string pointer
 * Returns: pointer to end of word
 */

char *rfc822_parse_word (char *s,const char *delimiters)
{
  char *st,*str;
  if (!s) return NIL;		/* no-op if no string */
  rfc822_skipws (&s);		/* flush leading whitespace */
  if (*s == '\0') return NIL;	/* end of string */
				/* default delimiters to standard */
  if (!delimiters) delimiters = wspecials;
  str = s;			/* hunt pointer for strpbrk */
  while (T) {			/* look for delimiter */
    if (!(st = strpbrk (str,delimiters))) {
      while (*s != '\0') ++s;	/* no delimiter, hunt for end */
      return s;			/* return it */
    }
    switch (*st) {		/* dispatch based on delimiter */
    case '"':			/* quoted string */
				/* look for close quote */
      while (*++st != '"') switch (*st) {
      case '\0':		/* unbalanced quoted string */
	return NIL;		/* sick sick sick */
      case '\\':		/* quoted character */
	if (!*++st) return NIL;	/* skip the next character */
      default:			/* ordinary character */
	break;			/* no special action */
      }
      str = ++st;		/* continue parse */
      break;
    case '\\':			/* quoted character */
      /* This is wrong; a quoted-pair can not be part of a word.  However,
       * domain-literal is parsed as a word and quoted-pairs can be used
       * *there*.  Either way, it's pretty pathological.
       */
      if (st[1]) {		/* not on NUL though... */
	str = st + 2;		/* skip quoted character and go on */
	break;
      }
    default:			/* found a word delimiter */
      return (st == s) ? NIL : st;
    }
  }
}


/* Copy an RFC822 format string
 * Accepts: string
 * Returns: copy of string
 */

char *rfc822_cpy (char *src)
{
				/* copy and unquote */
  return rfc822_quote (cpystr (src));
}


/* Unquote an RFC822 format string
 * Accepts: string
 * Returns: string
 */

char *rfc822_quote (char *src)
{
  char *ret = src;
  if (strpbrk (src,"\\\"")) {	/* any quoting in string? */
    char *dst = ret;
    while (*src) {		/* copy string */
      if (*src == '\"') src++;	/* skip double quote entirely */
      else {
	if (*src == '\\') src++;/* skip over single quote, copy next always */
	*dst++ = *src++;	/* copy character */
      }
    }
    *dst = '\0';		/* tie off string */
  }
  return ret;			/* return our string */
}


/* Copy address list
 * Accepts: address list
 * Returns: address list
 */

ADDRESS *rfc822_cpy_adr (ADDRESS *adr)
{
  ADDRESS *dadr;
  ADDRESS *ret = NIL;
  ADDRESS *prev = NIL;
  while (adr) {			/* loop while there's still an MAP adr */
    dadr = mail_newaddr ();	/* instantiate a new address */
    if (!ret) ret = dadr;	/* note return */
    if (prev) prev->next = dadr;/* tie on to the end of any previous */
    dadr->personal = cpystr (adr->personal);
    dadr->adl = cpystr (adr->adl);
    dadr->mailbox = cpystr (adr->mailbox);
    dadr->host = cpystr (adr->host);
    prev = dadr;		/* this is now the previous */
    adr = adr->next;		/* go to next address in list */
  }
  return (ret);			/* return the MTP address list */
}


/* Skips RFC822 whitespace
 * Accepts: pointer to string pointer
 */

void rfc822_skipws (char **s)
{
  char tmp[MAILTMPLEN];
  char *t;
  long n = 0;
				/* while whitespace or start of comment */
  while ((**s == ' ') || (n = (**s == '('))) {
    t = *s;			/* note comment pointer */
    if (**s == '(') while (n) {	/* while comment in effect */
      switch (*++*s) {		/* get character of comment */
      case '(':			/* nested comment? */
	n++;			/* increment nest count */
	break;
      case ')':			/* end of comment? */
	n--;			/* decrement nest count */
	break;
      case '"':			/* quoted string? */
	while (*++*s != '\"') if (!**s || (**s == '\\' && !*++*s)) {
	  sprintf (tmp,"Unterminated quoted string within comment: %.80s",t);
	  mm_log (tmp,PARSE);
	  *t = '\0';		/* nuke duplicate messages in case reparse */
	  return;
	}
	break;
      case '\\':		/* quote next character? */
	if (*++*s) break;
      case '\0':		/* end of string */
	sprintf (tmp,"Unterminated comment: %.80s",t);
	mm_log (tmp,PARSE);
	*t = '\0';		/* nuke duplicate messages in case reparse */
	return;			/* this is wierd if it happens */
      default:			/* random character */
	break;
      }
    }
    ++*s;			/* skip past whitespace character */
  }
}
