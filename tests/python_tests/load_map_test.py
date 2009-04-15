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

    print "Loading %s" % (file)

    strict = True
    mapnik.load_map(m, file, strict)

# We expect these files to raise a UserWarning
# and fail if there isn't one (or a different type
# of exception)
@raises(UserWarning)
def assert_raises_userwarning(file):
    m = mapnik.Map(512, 512)

    print "Loading %s" % (file)

    strict = True
    mapnik.load_map(m, file, strict)

def test_broken_files():
    broken_files = glob.glob("../data/broken_maps/*.xml")

    # Add a filename that doesn't exist 
    broken_files.append("../data/broken/does_not_exist.xml")

    for file in broken_files:
        yield assert_raises_userwarning, file

def test_good_files():
    good_files = glob.glob("../data/good_maps/*.xml")

    for file in good_files:
        yield assert_loads_successfully, file
