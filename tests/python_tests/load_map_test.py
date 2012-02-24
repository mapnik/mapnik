#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, sys, glob, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

# We expect these files to not raise any
# exceptions at all
def assert_loads_successfully(file):
    m = mapnik.Map(512, 512)

    try:
        strict = True
        mapnik.load_map(m, file, strict)

        # libxml2 is not smart about paths, and clips the last directory off
        # of a path if it does not end in a trailing slash
        base_path = os.path.dirname(file) + '/'
        mapnik.load_map_from_string(m,open(file,'rb').read(),strict,base_path)
    except RuntimeError, e:
        # only test datasources that we have installed
        if not 'Could not create datasource' in str(e):
            raise RuntimeError(e)


# We expect these files to raise a RuntimeError
# and fail if there isn't one (or a different type
# of exception)
@raises(RuntimeError)
def assert_raises_runtime_error(file):
    m = mapnik.Map(512, 512)

    strict = True
    mapnik.load_map(m, file, strict)

def test_broken_files():
    broken_files = glob.glob("../data/broken_maps/*.xml")

    # Add a filename that doesn't exist 
    broken_files.append("../data/broken/does_not_exist.xml")

    for file in broken_files:
        yield assert_raises_runtime_error, file

def test_good_files():
    good_files = glob.glob("../data/good_maps/*.xml")

    for file in good_files:
        yield assert_loads_successfully, file

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
