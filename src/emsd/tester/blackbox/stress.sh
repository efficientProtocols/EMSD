#!/bin/sh

#set -x

sleepPeriod=60

cmd=./tester.sh


  #while false; do
  while true; do
	echo running $cmd
	$cmd
	echo sleeping for $sleepPeriod
	sleep $sleepPeriod
  done
