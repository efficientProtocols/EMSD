#
module=$1
#echo ${module}
#echo `${module} -l`

sed -e "s/XXX_NOTICE_LEN_XXX/`${module} -l`/" -e "s/XXX_NOTICE_CRC_XXX/`${module} -c`/" ${module}.c
