#!/bin/sh

function purgeLicenseAndHeaderGuards
{
	# sed removes the license text and the #ifndef..#define statements
	# head removes the last 4 lines which include corresponding #endif and empty lines
	cat HippoMocks/detail/$1 | sed -e '1,23d' | head -n -4  > .unifiertemp/$1.purged
}

function replaceIncludeWithFile
{
	# this one inserts the sub-header after the include directive
	sed -i "/#include \"detail\/$1\"/r .unifiertemp\/$1.purged" .unifiertemp/unified_hippomocks.h
	# this one removes the include directive
	sed -i "/#include \"detail\/$1\"/d" .unifiertemp/unified_hippomocks.h
}

function processSubHeader
{
	purgeLicenseAndHeaderGuards $1
	replaceIncludeWithFile $1
}

mkdir .unifiertemp
cp HippoMocks/hippomocks.h .unifiertemp/unified_hippomocks.h

processSubHeader replace.h
processSubHeader reporter.h
processSubHeader reverse.h
processSubHeader defaultreporter.h
processSubHeader func_index.h

if [[ $1 == --comsupport ]] || [[ $1 == -c ]]
then
	#Remove the #pragma once and hippomock.h include directive
	cat HippoMocks/comsupport.h | sed -e '1,2d' | sed -e '2,1d' > .unifiertemp/comsupport.h.purged

	#Remove the last three lines including the #endif for the header guard
	head -n -3 .unifiertemp/unified_hippomocks.h > HippoMocks/unified_hippomocks.h

	#Add some empty lines for readability
	echo -ne "\n\n" >> HippoMocks/unified_hippomocks.h

	# Append the purged comsupport contents
	cat .unifiertemp/comsupport.h.purged >> HippoMocks/unified_hippomocks.h

	# Append the previously removed #endif for the header guard
	echo -ne "\n\n#endif\n\n" >> HippoMocks/unified_hippomocks.h
fi

# Clear the working directory, obviously.
rm -rf .unifiertemp
