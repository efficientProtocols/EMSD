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
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:
 *
 */


/*
 * Backwards compatibility library to allow non-use of PINE's c-client module.
 * This library contains only those functions required for the EMSD UA (and,
 * possibly also the EMSD MTA).
 */


#include "estd.h"
#include "emsdmail.h"
#include "emsd822.h"
#include "strfunc.h"
#include "buf.h"
#include "tm.h"
#include "eh.h"
#include "mtsua.h"

void
mail_free_address(ADDRESS ** ppAddress);

static void
mail_free_body_data(BODY * pBody);

static void
mail_free_body_parameter(PARAMETER ** ppParameter);

static void
mail_free_body_part(PART ** ppPart);


/* Dummy string driver for complete in-memory strings */

static void
mail_string_init(STRING * s,
		 void * data,
		 unsigned long size);

static char
mail_string_next(STRING * s);

static void
mail_string_setpos(STRING * s,
		   unsigned long i);


STRINGDRIVER mail_string =
{
    mail_string_init,		/* initialize string structure */
    mail_string_next,		/* get next byte in string structure */
    mail_string_setpos		/* set position in string structure */
};


/* Initialize mail string structure for in-memory string
 * Accepts: string structure
 *	    pointer to string
 *	    size of string
 */

static void
mail_string_init(STRING * s,
		 void * data,
		 unsigned long size)
{
    /* set initial string pointers */
    s->chunk = s->curpos = (char *) (s->data = data);
  
    /* and sizes */
    s->size = s->chunksize = s->cursize = size;

    /* never any offset */
    s->data1 = s->offset = 0;
}

/* Get next character from string
 * Accepts: string structure
 * Returns: character, string structure chunk refreshed
 */

static char
mail_string_next(STRING * s)
{
    return *s->curpos++;		/* return the last byte */
}


/* Set string pointer position
 * Accepts: string structure
 *	    new position
 */

static void
mail_string_setpos(STRING * s,
		   unsigned long i)
{
    s->curpos = s->chunk + i;	/* set new position */
    s->cursize = s->chunksize - i;/* and new size */
}


/* Mail open
 * Accepts: candidate stream for recycling
 *	    mailbox name
 *	    open options
 * Returns: stream to use on success, NULL on failure
 *
 * "pStream" input parameter is ignored
 * "options" input parameter is ignored
 */

MAILSTREAM *
mail_open (MAILSTREAM * pStream,
	   char * pMailFileName,
	   long options,
	   char * pLocalHostName)
{
    ReturnCode	    rc;
    size_t	    maxMsgSize;
    OS_Uint32	    maxAllocSize;
    OS_Uint32	    messageSize;
    void *	    hMailFile; /* Handle to mail file */
    char	    tmp[MAILTMPLEN];
    STRING	    bodyString;
    int		    eolLen;
    int		    count;

    /* Determine the size of the message */
    if ((rc = OS_fileSize(pMailFileName, &messageSize)) != Success)
    {
	return NULL;
    }

    /* We're limited to a message size of what will fill in size_t */
    maxMsgSize = (size_t) messageSize;
    maxAllocSize = maxMsgSize + sizeof(MAILSTREAM) + 1; 

    /* 
     * NOTE! This will cause a failure if the user doesn't delete
     * messages, and the message file grows larger than a size that
     * will fit in a size_t.
     */
    if ((OS_Uint32) maxMsgSize != messageSize ||
        maxAllocSize != (size_t) maxAllocSize)
    {
        return NULL;
    }

    /* Allocate a MAILSTREAM */
    if ((pStream = OS_alloc(sizeof(MAILSTREAM) + maxMsgSize + 1)) == NULL)
    {
	return NULL;
    }

    /* Initialize the mail stream fields */
    pStream->pEnv = NULL;
    pStream->pBody = NULL;

    /* Open the specified file */
#ifdef MSDOS
    if ((hMailFile = OS_fileOpen(pMailFileName, "rt")) == NULL)
#else
    if ((hMailFile = OS_fileOpen(pMailFileName, "r")) == NULL)
#endif
    {
	OS_free(pStream);
	return NULL;
    }

    /* We've allocated room for the message after the stream structure */
    pStream->pHeaderText = (char *) (pStream + 1);

    /* Read the message into our allocated memory */
    if ((maxMsgSize = OS_fileRead(pStream->pHeaderText,
		     		  1, maxMsgSize, hMailFile)) <= 0)
    {
	/* read error */
	OS_fileClose(hMailFile);
	OS_free(pStream);
	return NULL;
    }

    /* We successfully read the file.  We don't need it open any more */
    OS_fileClose(hMailFile);

    /* Null terminate the message */
    pStream->pHeaderText[maxMsgSize] = '\0';

    /* examine message for EOL style (IMPORTANT: look for MOST
     * SPECIFIC eol style first) & find the beginning of the body
     * text, identified by two consecutive newlines. */

#define DOS_EOL_CHARS "\r\n"
#define UNIX_EOL_CHARS "\n"

    if (OS_findSubstring(pStream->pHeaderText, DOS_EOL_CHARS)) { 
	eolLen = strlen(DOS_EOL_CHARS);
	pStream->pBodyText = OS_findSubstring(pStream->pHeaderText, DOS_EOL_CHARS DOS_EOL_CHARS);
    }
    else if (OS_findSubstring(pStream->pHeaderText, UNIX_EOL_CHARS)) {
	eolLen = strlen(UNIX_EOL_CHARS);
	pStream->pBodyText = OS_findSubstring(pStream->pHeaderText, UNIX_EOL_CHARS UNIX_EOL_CHARS);
    }
    else {
	return NULL;		/* barf */
    }

    if ( pStream->pBodyText ) {
	/* found the body */

	/* Null-terminate the header portion after the first newline */
	for (count = 0; count < eolLen; count++) {
	    ++pStream->pBodyText;
	}
	*pStream->pBodyText = '\0';

	/* Skip past the next newline to the beginning of the body */
	for (count = 0; count < eolLen; count++) {
	    ++pStream->pBodyText;
	}
    }
    else {
	/* No body.  Just point to the end */
	pStream->pBodyText =
	    pStream->pHeaderText + strlen(pStream->pHeaderText);
    }

    /* Initialize body string structure */
    INIT(&bodyString, mail_string,
	 (void *) pStream->pBodyText, strlen(pStream->pBodyText));

    /* Parse the message */
    rfc822_parse_msg(&pStream->pEnv, &pStream->pBody,
		     pStream->pHeaderText, strlen(pStream->pHeaderText),
		     &bodyString, pLocalHostName, tmp);

    if (!(pStream->pEnv->to || pStream->pEnv->cc || pStream->pEnv->bcc))
    {
	char tmpBuf[256];
        sprintf (tmpBuf, "%s, %d: *** No recipient found in RFC 822 message ***\n",
		 __FILE__, __LINE__);
	EH_problem(tmpBuf);
        printf ("Mal-formed message: No recipient found in the message\n");
    }

    /* Give 'em what they came for */
    return pStream;
}

/* Mail close
 * Accepts: mail stream
 * Returns: NIL
 */

MAILSTREAM *
mail_close(MAILSTREAM * pStream)
{
    /* Free the stream structure and associated data (pStream+1) */
    mail_free_envelope(&pStream->pEnv);
    mail_free_body(&pStream->pBody);
    OS_free(pStream);

    return NULL;
}


/* Mail instantiate envelope
 * Returns: new envelope
 */

ENVELOPE *
mail_newenvelope(void)
{
    ENVELOPE *	    pEnv;

    /* Allocate an ENVELOPE structure */
    if ((pEnv = OS_alloc(sizeof(ENVELOPE))) == NULL)
    {
	EH_fatal("mail_newenvelope() out of memory");
    }

    /* Initialize all fields */
    pEnv->remail = NULL;
    pEnv->return_path = NULL;
    pEnv->date = NULL;
    pEnv->subject = NULL;
    pEnv->from = NULL;
    pEnv->sender = NULL;
    pEnv->reply_to = NULL;
    pEnv->to = NULL;
    pEnv->cc = NULL;
    pEnv->bcc = NULL;
    pEnv->in_reply_to = NULL;
    pEnv->message_id = NULL;
    pEnv->newsgroups = NULL;

    return pEnv;
}


/* Mail garbage collect envelope
 * Accepts: pointer to envelope pointer
 */

void
mail_free_env(ENVELOPE * pEnv)
{
    if (pEnv->remail)
    {
	OS_free(pEnv->remail);
    }
	
    mail_free_address (&pEnv->return_path);
	
    if (pEnv->date)
    {
	OS_free(pEnv->date);
    }
	
    mail_free_address(&pEnv->from);
    mail_free_address(&pEnv->sender);
    mail_free_address(&pEnv->reply_to);

    if (pEnv->subject)
    {
	OS_free(pEnv->subject);
    }

    mail_free_address(&pEnv->to);
    mail_free_address(&pEnv->cc);
    mail_free_address(&pEnv->bcc);

    if (pEnv->in_reply_to)
    {
	OS_free(pEnv->in_reply_to);
    }
	
    if (pEnv->message_id)
    {
	OS_free(pEnv->message_id);
    }

    if (pEnv->newsgroups)
    {
	OS_free(pEnv->newsgroups);
    }

    OS_free(pEnv);		/* return envelope to free storage */
}

void
mail_free_envelope(ENVELOPE ** ppEnv)
{
    if (*ppEnv)
    {			/* only free if exists */
	mail_free_env(*ppEnv);
    }

    *ppEnv = NULL;
}


/* Mail instantiate address
 * Returns: new address
 */

ADDRESS *
mail_newaddr(void)
{
    ADDRESS *pAddr;

    if ((pAddr = OS_alloc(sizeof(ADDRESS))) == NULL)
    {
	EH_fatal("mail_newaddr() out of memory");
    }

    /* initialize all fields */
    pAddr->personal = NULL;
    pAddr->adl = NULL;
    pAddr->mailbox = NULL;
    pAddr->host = NULL;
    pAddr->error = NULL;
    pAddr->next = NULL;

    return pAddr;
}



/* Mail garbage collect address
 * Accepts: pointer to address pointer
 */

void
mail_free_address(ADDRESS ** ppAddress)
{
    if (*ppAddress)
    {
	if ((*ppAddress)->personal)
	{
	    OS_free((*ppAddress)->personal);
	}

	if ((*ppAddress)->adl)
	{
	    OS_free((*ppAddress)->adl);
	}

	if ((*ppAddress)->mailbox)
	{
	    OS_free((*ppAddress)->mailbox);
	}

	if ((*ppAddress)->host)
	{
	    OS_free((*ppAddress)->host);
	}

	if ((*ppAddress)->error)
	{
	    OS_free((*ppAddress)->error);
	}

	mail_free_address(&(*ppAddress)->next);
	OS_free(*ppAddress);
	*ppAddress = NULL;
    }
}


/* Mail initialize body
 * Accepts: body
 * Returns: body
 */

BODY *
mail_initbody(BODY * pBody)
{
    pBody->type = TYPETEXT;	/* content type */
    pBody->encoding = ENC7BIT;	/* content encoding */
    pBody->subtype = NULL;
    pBody->id = NULL;
    pBody->description = NULL;
    pBody->parameter = NULL;
    pBody->contents.text = NULL; /* no contents yet */
    pBody->contents.binary = NULL;
    pBody->contents.part = NULL;
    pBody->contents.msg.env = NULL;
    pBody->contents.msg.body = NULL;
    pBody->contents.msg.text = NULL;
    pBody->size.lines = 0;
    pBody->size.bytes = 0;
    pBody->size.ibytes = 0;

    return pBody;
}



/* Mail instantiate body
 * Returns: new body
 */

BODY *
mail_newbody(void)
{
    BODY *	    pBody;

    if ((pBody = OS_alloc(sizeof(BODY))) == NULL)
    {
	EH_fatal("mail_newbody() out of memory");
    }

    return mail_initbody(pBody);
}


/* Mail instantiate body parameter
 * Returns: new body part
 */

PARAMETER *
mail_newbody_parameter(void)
{
    PARAMETER *     pParameter;

    if ((pParameter = OS_alloc(sizeof(PARAMETER))) == NULL)
    {
	EH_fatal("mail_newbody_parameter() out of memory");
    }

    pParameter->attribute = NULL;
    pParameter->value = NULL;
    pParameter->next = NULL;	/* no next yet */

    return pParameter;
}


/* Mail instantiate body part
 * Returns: new body part
 */

PART *
mail_newbody_part(void)
{
    PART * pPart;

    if ((pPart = OS_alloc (sizeof (PART))) == NULL)
    {
	EH_fatal("mail_newbody_part() out of memory");
    }

    mail_initbody (&pPart->body);	/* initialize the body */
    pPart->offset = 0;			/* no offset yet */
    pPart->next = NIL;			/* no next yet */

    return pPart;
}



/* Mail garbage collect body
 * Accepts: pointer to body pointer
 */

void
mail_free_body(BODY ** ppBody)
{
    /* If a body part exists... */
    if (*ppBody)
    {
	/* ... then free the body part data, ... */
	mail_free_body_data(*ppBody);

	/* ... and free the body part structure */
	OS_free(*ppBody);
    }

    *ppBody = NULL;
}


/* Mail garbage collect body data
 * Accepts: body pointer
 */

static void
mail_free_body_data(BODY * pBody)
{
    if (pBody->subtype)
    {
	OS_free(pBody->subtype);
	pBody->subtype = NULL;
    }
    
    mail_free_body_parameter(&pBody->parameter);
    
    if (pBody->id)
    {
	OS_free(pBody->id);
	pBody->id = NULL;
    }
    
    if (pBody->description)
    {
	OS_free(pBody->description);
	pBody->description = NULL;
    }
    
    switch (pBody->type)
    {
    case TYPETEXT:		/* unformatted text */
	if (pBody->contents.text)
	{
	    OS_free(pBody->contents.text);
	    pBody->contents.text = NULL;
	}
	break;
	
    case TYPEMULTIPART:		/* multiple part */
	mail_free_body_part(&pBody->contents.part);
	break;
	
    case TYPEMESSAGE:		/* encapsulated message */
	mail_free_envelope(&pBody->contents.msg.env);
	mail_free_body(&pBody->contents.msg.body);
	if (pBody->contents.msg.text)
	{
	    OS_free(pBody->contents.msg.text);
	    pBody->contents.msg.text = NULL;
	}
	break;
	
    case TYPEAPPLICATION:		/* application data */
    case TYPEAUDIO:		/* audio */
    case TYPEIMAGE:		/* static image */
    case TYPEVIDEO:		/* video */
	if (pBody->contents.binary)
	{
	    OS_free(pBody->contents.binary);
	    pBody->contents.binary = NULL;
	}
	break;
	
    default:
	break;
    }
}


/* Mail garbage collect body parameter
 * Accepts: pointer to body parameter pointer
 */

static void
mail_free_body_parameter(PARAMETER ** ppParameter)
{
    if (*ppParameter)
    {
	if ((*ppParameter)->attribute)
	{
	    OS_free((*ppParameter)->attribute);
	}
      
	if ((*ppParameter)->value)
	{
	    OS_free((*ppParameter)->value);
	}

	mail_free_body_parameter (&(*ppParameter)->next);
	OS_free(*ppParameter);
	*ppParameter = NULL;
    }
}


/* Mail garbage collect body part
 * Accepts: pointer to body part pointer
 */

static void
mail_free_body_part(PART ** ppPart)
{
    if (*ppPart)
    {
	mail_free_body_data (&(*ppPart)->body);
	mail_free_body_part (&(*ppPart)->next);
	OS_free(*ppPart);
	*ppPart = NULL;
    }
}


/* Mail fetch message structure
 * Accepts: mail stream
 *	    message # to fetch
 *	    pointer to return body
 * Returns: envelope of this message, body returned in body value
 *
 * "msgno" is ignored.
 */

ENVELOPE *
mail_fetchstructure(MAILSTREAM * pStream,
		    long msgno,
		    BODY ** ppBody)
{
    if (ppBody != NULL)
    {
	*ppBody = pStream->pBody;
    }

    return pStream->pEnv;
}


/* Mail fetch message header
 * Accepts: mail stream
 *	    message # to fetch
 * Returns: message header in RFC822 format
 *
 * "msgno" is ignored.
 */

char *
mail_fetchheader(MAILSTREAM * pStream,
		 long msgno)
{
    return pStream->pHeaderText;
}


/* Mail fetch message text (body only)
 * Accepts: mail stream
 *	    message # to fetch
 * Returns: message text in RFC822 format
 *
 * "msgno" is ignored.
 */

char *
mail_fetchtext (MAILSTREAM *pStream,
		long msgno)
{
    return pStream->pBodyText;
}
