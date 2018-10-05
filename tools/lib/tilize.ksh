
function tildize {
    if [[ $1 == $HOME* ]]; then
	print "Exec case1"
	print "\~/${1#HOME}"
	return 0
    fi
    awk 'BEGIN {FS=":"} {print $1, $6}' /etc/passwd
	while read user homedir; do
	    if [[ $homedir != / && $1 == ${homedir}?(/*) ]]; then
		print "Exec case2"
		print "\~$user/${1#$homedir}> "
		return 0
	    fi
        done
    print "Exec case3"
    print "$1"
    return 1
}
    
