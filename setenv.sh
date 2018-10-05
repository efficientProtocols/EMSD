#! /bin/ksh

# This file should be dotted before 
# use of this environment.

#
# NOTYET, If the path already has the curenv 
# as first entry do not re-append.
# Results/bins should also be added.
# 

systemName=`uname -n`
osType=`uname -s`

case ${osType} in
  'Windows_NT')
	export CURENVBASE=`pwd  | sed -e 's:/:\\\\\\\\:g'`
	#export OPUSMAKECFG="${CURENVBASE}\\tools\\win32make.ini"
	export PATH=${CURENVBASE}\\tools;${PATH}
	;;
  'UWIN-NT')
        export CURENVBASE=`pwd`
	export PATH=${CURENVBASE}/tools/bin:${PATH}
	export PATH=${CURENVBASE}/tools:${PATH}
	;;
  'CYGWIN_NT-5.0'|'CYGWIN_NT-5.1')
        export CURENVBASE=`pwd`
	export PATH=${CURENVBASE}/tools/bin:${PATH}
	export PATH=${CURENVBASE}/tools:${PATH}
	;;
  'Linux')
        export CURENVBASE=`pwd`
	export PATH=${CURENVBASE}/tools/bin:${PATH}
	export PATH=${CURENVBASE}/tools:${PATH}
	;;
  'SunOS')
        export CURENVBASE=`pwd`
	export PATH=${CURENVBASE}/tools/bin:${PATH}
	export PATH=${CURENVBASE}/tools:${PATH}
	;;
esac

echo "CURENVBASE set to ${CURENVBASE}"

if [[ ! -e /bin/osmtKsh ]] ; then
    echo "/bin/osmtKsh is missing."
    echo "/bin/osmtKsh can be a symlink to ksh93, bash, ksh88 or pdksh."
    echo "Create that symlink before proceeding further."
fi


