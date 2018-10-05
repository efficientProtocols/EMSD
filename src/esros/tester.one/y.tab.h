# 
# Copyright (C) 1997-2002  Neda Communications, Inc.
# 
# This file is part of ESRO. An implementation of RFC-2188.
# 
# ESRO is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License (GPL) as 
# published by the Free Software Foundation; either version 2,
# or (at your option) any later version.
# 
# ESRO is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with ESRO; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.  
# 
#
#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

# ifndef YYSTYPE
#  define YYSTYPE int
#  define YYSTYPE_IS_TRIVIAL 1
# endif
# define	LOG	257
# define	LOGFILE	258
# define	QUIET	259
# define	VERBOSE	260
# define	INCLUDE	261
# define	ERRORIND	262
# define	ERRORREQ	263
# define	ERRORCONF	264
# define	INVOKEREQ	265
# define	INVOKEIND	266
# define	RESULTREQ	267
# define	RESULTIND	268
# define	RESULTCONF	269
# define	FAILUREIND	270
# define	STRING	271
# define	NUMBER	272
# define	SAPBIND	273
# define	SAPRELEASE	274
# define	IPADDR	275
# define	SHELL_CMD	276
# define	EVENT	277
# define	ERROR0	278
# define	BASIC	279
# define	PACKED	280
# define	RESERVED2	281
# define	RESERVED3	282
# define	DELAY	283
# define	PATH	284
# define	IGNOREIT	285
# define	NOTICE	286
# define	FAULTS	287
# define	TRANSFAIL	288
# define	LOCRESOURCE	289
# define	USERNOTRESP	290
# define	REMRESOURCE	291
# define	UDP_PC_LOSSY	292
# define	UDP_PC_INHIBIT	293
# define	SEND	294
# define	RECEIVE	295


extern YYSTYPE yylval;

#endif /* not BISON_Y_TAB_H */
