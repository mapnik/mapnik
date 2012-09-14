#!/bin/bash

# batch format *.{hpp,cpp} files

MAPNIK_DIR=`pwd`
DIRS="$MAPNIK_DIR/plugins $MAPNIK_DIR/demo/c++ $MAPNIK_DIR/src $MAPNIK_DIR/include $MAPNIK_DIR/bindings $MAPNIK_DIR/utils $MAPNIK_DIR/tests"
EMACS="emacs"

for file in $(find $DIRS -name '*.cpp' -o -name '*.hpp')
do
    $EMACS -batch $file -l $MAPNIK_DIR/utils/format_source_files/mapnik_format.el -f mapnik-format-function
done


