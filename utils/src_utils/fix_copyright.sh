#!/bin/bash

if [ $# -ne 1 ]
then
  echo "Usage: `basename $0` YEAR"
  exit 1
fi

YEAR=$1
DIRECTORIES=( "include/mapnik" "src" "plugins/input" "demo/c++" "demo/viewer" )

SED="sed -i -e"
COMMAND="s: \* Copyright (C) 20[0-9][0-9].*: \* Copyright (C) $YEAR Artem Pavlenko:g"

for d in "${DIRECTORIES[@]}"
do
  for f in $(find "../../$d" -type f \( -iname '*.hpp' -o -iname '*.cpp' -o -iname '*.h' -o -iname '*.c' \)); do
    $SED "$COMMAND" $f
  done
done
