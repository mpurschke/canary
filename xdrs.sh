#! /bin/bash

FILE="$1"

if [ -z "$FILE" ] ; then

    root -l xdrs.C\(0\)
else

    root -l xdrs.C\(\"$FILE\"\)
fi
