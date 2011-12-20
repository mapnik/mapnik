#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path
from copy import deepcopy

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_loading_fontset_from_map():
    m = mapnik.Map(256,256)
    mapnik.load_map(m,'../data/good_maps/fontset.xml')
    fs = m.find_fontset('book-fonts')
    eq_(len(fs.names),3)
    eq_(list(fs.names),['DejaVu Sans Book','DejaVu Sans Oblique', 'does not exist'])


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
