#!/bin/sh

API_DOCS_DIR="mapnik-python-`mapnik-config --version`"

if [ ! -d $API_DOCS_DIR ]
    then 
        echo "creating $API_DOCS_DIR"
    mkdir -p $API_DOCS_DIR
fi

epydoc --no-private \
    --no-frames \
    --no-sourcecode \
    --name mapnik \
    --url http://mapnik.org \
    --css mapnik_epydoc.css mapnik \
    -o $API_DOCS_DIR

exit $?
