#!/bin/sh
#Author: Huy Tran
#School: University of Colorado Boulder
#Course: ECEA - Linux System Programming
#writer.sh

if [ $# -ne 2 ]; then
	echo Please provide 2 arguments writefile and writestr.
	exit 1
fi

writefile=$1
writestr=$2

dir=$(dirname $writefile)
if [ ! -d $dir ]; then
	mkdir -p $dir
	if [ $? -ne 0 ]; then
		echo "Huy Error: Cannot create directory $dir"
		exit 1
	fi
fi

echo "$writestr" > $writefile

if [ $? -ne 0 ]; then
	echo Error: Cannot create or write to $writefile.
	exit 1
fi
