#!/bin/bash

logfile="/home/bas/wahstacktc-validator/`date +%s`.log"
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
cd Debug
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

