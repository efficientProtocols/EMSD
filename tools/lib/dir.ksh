#if [ "${dir_virgin}_" == "_" ] ; then


#
# dir.ksh
#
# Korn shell script that provides pushd/popd/dirs functionality (a la csh)
# Now supports a directory stack dialog via the MKS sh dlg interface
# (See gdir)
#
# WARNING:
#  Do not edit/change this file.
#  It may be replaced on subsequent install of the MKS Toolkit.
#
# $Date: 2001/12/17 21:38:15 $, $Revision: 1.2 $
#

_gdirPrintFunc=print	# _gdirPrintFunc - points to current "print" function
		#          In gdir() we want to redirect error messages to
		#          the dialog's message box
_gdirDirsFunc=dirs	# _gdirDirsFunc  - points to current "dirs" function
		#          In gdir() we want to disable dirs() function

_gdirDialogName=DIRSTACK

# the gdir dialog resource is compiled in the Toolkit's sh.exe
#_gdirResModule=${_gdirResModule:-$(getconf _CS_SHELL)}

# global local variables
_gdirHndl=
_gdirErrBox=105
_gdirEditBox=101

GDIR_MAN="$ROOTDIR/mksnt/tkutil.chm::tkutil/gdir.1.htm"
whence hh.exe 1> /dev/null 2>&1
if [ $? -eq 0 ] ; then
	HH=1
else
	HH=0
fi

function _gdirDlgPrint
{
	typeset opt

	OPTIND=1
	while getopts 'r' opt
	do
	    case $opt in 
	    *) # just ignore all options
	       ;;
	    esac
	done
	shift $((OPTIND-1))  

	# ensure the ErrBox is empty
	dlg settext -d $_gdirHndl -c $_gdirErrBox ""
	# print out the error message
	dlg settext -d $_gdirHndl -c $_gdirErrBox "$1"
}

#
# usage: _gdirPushdir arg
#  where 'arg' can be 0, -#, +# or a dirname
#
function _gdirPushdir
{

	case "$1" in
	    0|+0|-0)
		# selected current directory
		# - all done
		return 0
		;;
	    +[1-9]*)
		pushd "$1"
		;;
	    *)
		# note: this is a directory name
		#       including names that start with a '-'
		#
	
		# let cd expand all the ~'s and such 
		if ! (eval cd ${_EditEntry} ) ; then 
		    $_gdirPrintFunc "$1: directory does not exist"
		    return 1
		else
		    newdir=$( eval cd ${_EditEntry} 2>/dev/null ; pwd  ) 
		    pushd "$newdir"
		fi
		;;
	esac
	return 	# return exit status of last command
}


#
# gdir [ dirname ]
#
# Start up directory stack dialog.
# dirname: directory name to 'pushd' to.
#
function gdir
{
	if [ ! -f "$_gdirResModule" ]; then
		print "Dialog resource not found : $_gdirResModule"
		return 1
	fi

	_gdirDirsFunc=
	_gdirPrintFunc=_gdirDlgPrint
	funcName=" "

	typeset _ListBox=102
	typeset Bdel=3		# Delete Button
	typeset Bbrowse=4	# Browse Button
	typeset msg=
	typeset ctrl=
	typeset sel=
	typeset dir=
	typeset _index=

	dlg load -x -1 -y -1 -r _gdirHndl "$_gdirResModule" $_gdirDialogName > /dev/null 2>&1
	if [ $? -eq 127 ]; then
		printf "Dialogs (dlg) not supported by this shell"
		printf "Must be using MKS Toolkit version 5.1 or newer"
		return 1
	fi
	if [ $? -ne 0 -a $? -ne 2 ]; then
		print "Dialog resource $_gdirDialogName not found in $_gdirResModule"
		return 1
	fi

        dir="${PWD}"
        _to_tilde

	#print out the directory stack in the ListBox
	#
	dlg clear -d $_gdirHndl -c $_ListBox
	for i in "$dir" "${_gdir_pushStack[@]}"; do
		dlg addtext -d $_gdirHndl -c $_ListBox "$i"
	done


	# disable the Delete button
	# until an item is selected in the ListBox,
	#
	dlg enabled -d $_gdirHndl -c $Bdel 0

	if [ "$1" != "" ] ; then
		# display directory name from command line in the EditBox
		dlg settext -d $_gdirHndl -c $_gdirEditBox "$1"
	else
		# select 1st item in directory stack
		dlg setcursel -d $_gdirHndl -i 0 -c $_ListBox
		dlg gettext -d $_gdirHndl -c $_ListBox -i 0 sel
		dlg settext -d $_gdirHndl -c $_gdirEditBox "$sel"
	fi

	# set the initial focus
	dlg setfocus -d $_gdirHndl -c $_gdirEditBox

	while dlg event -d $_gdirHndl msg ctrl
	do

		case "$msg" in
		help)	# F1 to get help
			if [ $HH -ne 0 ] ; then
				wstart hh "$GDIR_MAN"
			fi
			continue
			;;

		select) # item selected from ListBox
			if [ "$ctrl" = $_ListBox ]; then

				# get selected item from ListBox
				# and display it in the EditBox
				dlg getcursel -d $_gdirHndl -c $_ListBox _index
				dlg gettext -d $_gdirHndl -c $_ListBox -i $_index sel
				dlg settext -d $_gdirHndl -c $_gdirEditBox "$sel"

				if [ "$_index" -gt 0 ]; then
					#
					# Enable the Delete Buttons
					#
					dlg enabled -d $_gdirHndl -c $Bdel 1
				else
					dlg enabled -d $_gdirHndl -c $Bdel 0
				fi
			fi

			continue
			;;

		double) # Double click  in ListBox
			# - same as a OK button click
			#

			# get index in ListBox that's currently highlighted  
			dlg getcursel -d $_gdirHndl -c $_ListBox _index

			if [ "$_index" -ge "0" ]; then
				_gdirPushdir "+$_index" || continue
			else
				# should NEVER get here 
				# - how could we get a negative index?
				continue
			fi

			break
			;;

		command) # got a button push

			#clear error box
			dlg settext -d $_gdirHndl -c $_gdirErrBox ""

			case $ctrl in
			1)	# OK Button
				#
				# get contents from EditBox
				dlg gettext -d $_gdirHndl -c $_gdirEditBox _EditEntry

				# if EditBox is empty, then do nothing
				if [ "$_EditEntry" = "" ]; then
					break
				fi

				# check if item selected in ListBox
				dlg getcursel -d $_gdirHndl -c $_ListBox _index
				if [ "$_index" -ge 0 ]; then
					# get string highlighted in ListBox
					dlg gettext -d $_gdirHndl -c $_ListBox -i $_index _ListEntry
				else
					_ListEntry=""
				fi

				# compare ListEntry with EditEntry.
				# If they are not the same then EditEntry 
				# is a new directory to be added to the stack.
				# Otherwise, we 'pushd' to the index 
				# into ListBox.
				# 
				if [ "$_ListEntry" != "$_EditEntry" ]; then
					_gdirPushdir "$_EditEntry" || continue
				else
					_gdirPushdir +"$_index" || continue
				fi
						
				break
			  	;;
			2)	# CANCEL Button
				#
				break
			  	;;

			3)	# DELETE Button
				#
				dlg getcursel -d $_gdirHndl -c $_ListBox _index

				case "$_index" in
				    -*)
					# negative number - ignore
					continue
					;;
				    0)
					#
					# e.g) POPD
					popd
					;;
				    *)
					popd +"$_index"
					;;
				esac
				;;
			4)	# Browse Button
				dirname=`filebox -D`
				if [ $? -eq 0 ]
				then
					dlg settext -d $_gdirHndl -c $_gdirEditBox "$dirname"
					dlg setcursel -d $_gdirHndl -i -1 -c $_ListBox
					dlg setfocus -d $_gdirHndl -c $_gdirEditBox
				fi
				continue
				;;

			esac
			;;
		edit)
			case "$ctrl" in
			101)	
				continue

				;;
			*)
				continue
				;;
			esac
			;;
		*)
			continue
			;;
		esac

		# display current stack in the listbox
		#
		dir="${PWD}"
		_to_tilde

		#print out the current directory stack in the ListBox
		#
		dlg clear -d $_gdirHndl -c $_ListBox
		for i in $dir ${_gdir_pushStack[@]}; do
			dlg addtext -d $_gdirHndl -c $_ListBox "$i"
		done

		# disable the Delete button
		# until an item is selected in the ListBox,
		#
		dlg enabled -d $_gdirHndl -c $Bdel 0

		# The following must be LAST
		# We want to make sure the ListBox entry is highlighted.
		#
		# select 1st item in directory stack
		dlg setcursel -d $_gdirHndl -i 0 -c $_ListBox
		dlg gettext -d $_gdirHndl -c $_ListBox -i 0 sel
		dlg settext -d $_gdirHndl -c $_gdirEditBox "$sel"


	done # while loop

	dlg close -d $_gdirHndl

	# reset print function
	_gdirPrintFunc=print
	_gdirDirsFunc=dirs
	unset funcName
}




#
# korn shell functions for pushd, popd, dirs
#

integer _gdir_pushMax=20 
integer _gdir_pushTop=20

# clear _gdir_pushStack array (in case this file gets sourced multiple times)
unset _gdir_pushStack

#
# Check if 'dir' env. variable is prefixed by $HOME.
# If so, then replace it with '~' character.
#
function _to_tilde
{
    return		# skip this function for now (fix for Win95 Bug)

    dir=${dir#$HOME/}
    case $dir in 
	$HOME)
		dir=\~
		;;
	/*) 
		;;
	 *)	dir=\~/$dir
		;;
    esac
}

#
# usage:  _EVAL  bool_expression
# 
# Returns 0 if bool_expression is true otherwise return 1
#
function _EVAL
{
	set -- $1
	case $[$1$2$3] in
	  1)
		return 0
		;;
	esac
	return 1
}


function dirs
{
    typeset dir="${PWD}"
    _to_tilde
    $_gdirPrintFunc -r -- "$dir ${_gdir_pushStack[@]}"
    return 0
}

function pushd
{
    integer i
    typeset type=0
    typeset dir

    funcName=${funcName:-pushd:}

    case $1 in
	"")	# pushd
	    if _EVAL "$_gdir_pushTop >= $_gdir_pushMax" ; then
		$_gdirPrintFunc "$funcName: No other directory to goto."
		return 1
	    fi
	    type=1
	    dir=${_gdir_pushStack[_gdir_pushTop]}
	    ;;
	+[1-9] | +[1-9][0-9])  	# pushd +n
	    integer i=_gdir_pushTop$1-1	# note, $1 contains a +#
	    if _EVAL "$i >= $_gdir_pushMax"; then
		$_gdirPrintFunc "$funcName Directory stack not that deep."
		return 1
	    fi
	    type=2
	    dir=${_gdir_pushStack[i]}
	    ;;
	*)
	    if _EVAL "$_gdir_pushTop <= 0"; then
		$_gdirPrintFunc "$funcName Directory stack overflow."
		return 1
	    fi
	    ;;
    esac
    case $dir in
	~*)
	    dir=$HOME${dir#\~}
	    ;;
    esac

    dir="${dir:-$1}"

    if ! cd "${dir}" 2>/dev/null ; then  
	$_gdirPrintFunc "unable to change to directory $dir "
	return 1
    fi

    dir="${OLDPWD}"
    _to_tilde
    case $type in
	0)	# pushd name
	    let _gdir_pushTop=_gdir_pushTop-1
	    _gdir_pushStack[_gdir_pushTop]=$dir
	    ;;
	1)	# pushd
	    _gdir_pushStack[_gdir_pushTop]=$dir
	    ;;
	2)	# push +n
	    type=${1#+}
	    let i=_gdir_pushTop-1
	    set -- "${_gdir_pushStack[@]}" "$dir" "${_gdir_pushStack[@]}"
	    shift $type
	    for dir do
		let i=i+1
		if _EVAL "$i >= $_gdir_pushMax"; then
			break
		fi
		_gdir_pushStack[i]=$dir
	    done
	    ;;
    esac
    $_gdirDirsFunc
}


function popd
{
    typeset dir
    funcName=${funcName:-popd:}
    if _EVAL "$_gdir_pushTop >= $_gdir_pushMax"; then
	$_gdirPrintFunc "$funcName Cannot remove current directory."
	return 1
    fi
    case $1 in 
	"")
	    dir=${_gdir_pushStack[_gdir_pushTop]}
	    case $dir in
		~*) dir=$HOME${dir#\~} ;;
	    esac
	    cd "$dir" || return 1
	    ;;
	+[1-9] | +[1-9][0-9])
	    integer i=_gdir_pushTop$1-1
	    if  _EVAL "$i >= $_gdir_pushMax"; then
		$_gdirPrintFunc "$funcName Directory stack not that deep."
		return 1
	    fi
	    while _EVAL "$i > $_gdir_pushTop"; do
		_gdir_pushStack[i]=${_gdir_pushStack[i-1]}
		let i=i-1
	    done
	    ;;
	*)
	    $_gdirPrintFunc "$funcName directory name $1 not allowed."
	    return 1;;
    esac
    unset _gdir_pushStack[_gdir_pushTop]
    let _gdir_pushTop=_gdir_pushTop+1
    $_gdirDirsFunc
    return 0
}


#dir_virgin="not_"
#fi