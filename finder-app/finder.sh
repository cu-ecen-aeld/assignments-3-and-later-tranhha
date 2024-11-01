#!/bin/sh
#Author: Huy Tran
#School: University of Colorado Boulder
#Course: ECEA - Linux System Programming
#finder.sh
if [ $# -ne 2 ]; then
	echo "Please provide 2 arguments filedir and searchstr."
	exit 1
fi

if [ ! -d $1 ]; then
	echo "Error: $1 is not a directory"
	exit 1
fi

filesdir=$1
searchstr=$2

files_count=$(find $filesdir -type f | wc -l)
matching_lines=$(grep -r $searchstr $filesdir | wc -l)

echo "The number of files are $files_count and the number of matching lines are $matching_lines."

