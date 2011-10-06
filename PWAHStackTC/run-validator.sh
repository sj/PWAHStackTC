#!/bin/bash

if [ ! "`whoami`" = "bas" ]; then
	echo "Can not run as any user other than 'bas'" >&2
	exit 1
fi

svnusername="wahstacktc-validator"
svnpassword="aORj37uMtQuN2"
svnurl="https://guust.tuxes.nl/svn/bas/studie/0910-afstuderen/Implementations/c++/WAHStackTC"
logfile="/home/bas/wahstacktc-validator/`date +%s`.log"
#useconfiguration="Debug"
useconfiguration="Release"

if [ ! -d "`dirname $logfile`" ]; then
	mkdir "`dirname $logfile`"
fi

tmpdir=`mktemp -d`
cd $tmpdir

date >> $logfile
echo -n "Checking out latest revision in $tmpdir... " >> $logfile
svn --non-interactive --password="$svnpassword" --username=$svnusername co "$svnurl" . > /dev/null
svnres="$?"

if [ ! "$svnres" = "0" ]; then
	echo "FAILED!" >> $logfile
	echo "Checkout failed!" >&2

	rm -rf "$tmpdir"
	exit 1
fi

svn info | grep Revision >> $logfile

if [ ! -d "$useconfiguration" ]; then
	echo "Can not find build configuration dir '$useconfiguration' in checkout directory?!" >> $logfile
	echo "Can not find build configuration dir '$useconfiguration' in checkout directory?!" >&2
	rm -rf "$tmpdir"
	exit 1
fi

echo >> $logfile
echo >> $logfile
echo "======= Build log =======" >> $logfile

cd $useconfiguration
(make all 2>&1) >> $logfile
buildres="$?"
echo >> $logfile
echo >> $logfile

if [ "$buildres" = "0" ]; then
	(./WAHStackTC --run-validator 2>&1) >> $logfile
	validatorres="$?"
fi
rm -rf "$tmpdir"

if [ ! "$buildres" = "0" ] || [ ! "$validatorres" = "0" ]; then
	echo "WAHStackTC validator reported one or more errors!" >&2
	echo "Contents of $logfile:" >&2
	
	cat "$logfile" >&2
fi

