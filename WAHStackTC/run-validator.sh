#!/bin/bash

logfile="/home/bas/wahstacktc-validator/`date +%s`.log"
#useconfiguration="Debug"
useconfiguration="Release"

if [ ! -d "$useconfiguration" ]; then
	echo "Can not find build configuration dir '$useconfiguration' in current directory?!" >&2
	exit 1
fi

if [ ! -d "`dirname $logfile`" ]; then
	mkdir "`dirname $logfile`"
fi

date >> $logfile
svn up &> /dev/null
svn info src | grep Revision >> $logfile
echo >> $logfile
echo >> $logfile
echo "======= Build log =======" >> $logfile

tmplogfile=`mktemp`
cd $useconfiguration
make all &> $tmplogfile
buildres="$?"
cat $tmplogfile >> $logfile
echo >> $logfile
echo >> $logfile

if [ "$buildres" = "0" ]; then
	./WAHStackTC --run-validator &> $tmplogfile
	validatorres="$?"
	cat $tmplogfile >> $logfile
fi
rm $tmplogfile

if [ ! "$buildres" = "0" ] || [ ! "$validatorres" = "0" ]; then
	echo "WAHStackTC validator reported one or more errors!" >&2
	echo "Contents of $logfile:" >&2
	
	cat "$logfile" >&2
fi

