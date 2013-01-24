#!/usr/bin/env python

import os
import mapnik
from nose.tools import *
from utilities import execution_path

datadir = '../data/pngsuite'

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def assert_broken_file(fname):
    assert_raises(RuntimeError, lambda: mapnik.Image.open(fname))

def assert_good_file(fname):
    assert mapnik.Image.open(fname)

def get_pngs(good):
    files = [ x for x in os.listdir(datadir) if x.endswith('.png') ]
    return [ os.path.join(datadir, x) for x in files if good != x.startswith('x') ]

def test_good_pngs():
    for x in get_pngs(True):
        yield assert_good_file, x

def test_broken_pngs():
    for x in get_pngs(False):
        yield assert_broken_file, x

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
