#!/usr/bin/ksh

#stringToFind="include <libisl"
#stringToReplace="include <isl"
stringToFind="LIBISL__"
stringToReplace="ISL__"
tempFileName="temp_file.txt"

for file in `find . -type f -name "*.?xx"`; do
	echo "Processing $file ... \c"
	sed -e "s@$stringToFind@$stringToReplace@g" $file > $tempFileName
	mv $tempFileName $file
	echo "OK"
done
