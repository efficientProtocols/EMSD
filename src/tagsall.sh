#!/bin/sh
# RCS: $Id: tagsall.sh,v 1.1 1999/11/14 07:51:48 mohsen Exp $

MODULES="ae-if/include \
  ae-if/tern-hw \
  ae-if/tern-tst" 

TAGSFILE=TAGS.All

rm $TAGSFILE

find $MODULES -print | egrep '\.c$|\.h$' \
	 | egrep -v 'SCCS|RCS' |  egrep -v '/s\.|/p\.' | \
	 xargs etags --defines --typedefs  --append -o $TAGSFILE -t


