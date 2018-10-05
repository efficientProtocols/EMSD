#!/bin/sh

#set -x

##############################################################################
###
### CONFIGURABLE PARAMETERS
###

# Directory containing the configuration files
configDir=${EMSDRUN}/config

# Determine sending and recipient user name
# Normally, it can just be determined with the "whoami" program
me=`whoami`

# Determine internet-side host name
# Normally, "uname -n" returns this correctly.  Could also be "hostname"
unixHost=`uname -n`

# Determine device-side host name
# This may be the same as the internet-side; depends on sendmail.cf
emsdHost=emsd.`domainname`

###
### The following wont change once we figure out the correct values
###

# Specify the number of lines for a message short enough for
# connectionless ESROS
connectionlessLen=5

# Specify the number of lines long enough to require segmentation, but
# still short enough to use connectionless ESROS
segmentedLen=50

# Specify the number of lines long enough to require
# connection-oriented ESROS
connectionOrientedLen=200

###
### END OF CONFIGURABLE PARAMETERS
###
##############################################################################



##############################################################################
###
### Function to generate and send a single message
###

doMessage ()
{
    # Create the message header and body
    echo "From ${from} ${theDate}" > ${testMsg}
    echo ${theDate} | gawk \
	'/^.*$/ \
	{ \
		print "Date: " $3 " " $2 " " $6-1900 " " $4 " " $5; \
	}' >> ${testMsg}
    echo "From: ${from}" >> ${testMsg}
    echo "To: ${to}" >> ${testMsg}
    echo "Subject: ${subject}" >> ${testMsg}
    echo "" >> ${testMsg}
    gawk -v msgLines=${msgLines} \
	'BEGIN \
	{ \
		printf "Expect %d lines...\n", msgLines; \
		for (i=1; i<=msgLines; i++) \
		{ \
			printf "%04d ", i; \
			printf "abcdefghijklmnopqrstuvwxyz"; \
			printf "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"; \
		} \
	}' >> ${testMsg}

    # Send the message
    messages ${cmd} -f ${from} -c ${configDir} ${to} < ${testMsg}
}

###
### End of message-create-and-send function
###
##############################################################################



##############################################################################
###
### Set some variables that we'll need for this procedure
###

# Get the current date, which we'll massage for our purposes
theDate=`date`

# Create a temporary file name
testMsg=/tmp/testmsg.$$

###
### End of variable initialization
###
##############################################################################



##############################################################################
###
### Create messages to SUBMIT
###

# Generic configuration for submission
from=${me}@${emsdHost}
to=${me}@${unixHost}
subject="Hello INTERNET world"
cmd="-s"

# Create a small test message (connectionless ESROS)
msgLines=${connectionlessLen}
doMessage

# Create a medium sized test message (segmented connectionless ESROS)
msgLines=${segmentedLen}
doMessage

# Create a long test message (connection-oriented ESROS)
# This is not yet implemented, so these are commented out.
#msgLines=${connectionOrientedLen}
#doMessage

###
### End of submission
###
##############################################################################


##############################################################################
###
### Create message to DELIVER
###

# Generic configuration for delivery
from=${me}@${unixHost}
subject="Hello ETWP world"
cmd="-d"

# Create a small test message
to=mohsen@${emsdHost}
msgLines=${connectionlessLen}
doMessage

# Create a medium sized test message (segmented connectionless ESROS)
to=mohsen@${emsdHost}
msgLines=${segmentedLen}
doMessage

# Create a long test message (connection-oriented ESROS)
# This is not yet implemented, so these are commented out.
#msgLines=${connectionOrientedLen}
#doMessage

###
### End of delivery
###
##############################################################################



##############################################################################
###
### All done
###

# Remove our temporary file
rm ${testMsg}

###
### The end.
###
##############################################################################
