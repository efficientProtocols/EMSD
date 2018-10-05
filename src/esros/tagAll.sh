#!/bin/sh
# RCS: $Id: tagAll.sh,v 1.1.1.1 2002/10/08 18:57:31 mohsen Exp $

MODULES="./upq_bsd ./prvdr_sh ./prvdr ./erop_eng ./user_sh ./ops_xmpl ./include  ./udp_pco ./pdu_log ../ocp/gf ../ocp/include"

TAGSFILE=TAGS.All

/bin/rm $TAGSFILE

find $MODULES -print | egrep '\.c$|\.h$' \
	 | egrep -v 'SCCS|RCS' |  egrep -v '/s\.|/p\.' | \
	 xargs etags --defines --typedefs  --append -o $TAGSFILE -t

