#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, mapnik

from nose.tools import *
from utilities import execution_path

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_query_init():
    bbox = (-180, -90, 180, 90)
    query = mapnik.Query(mapnik.Box2d(*bbox))
    r = query.resolution
    assert_almost_equal(r[0], 1.0, places=7)
    assert_almost_equal(r[1], 1.0, places=7)

# Converting *from* tuples *to* resolutions is not yet supported
@raises(TypeError)
def test_query_resolution():
    bbox = (-180, -90, 180, 90)
    init_res = (4.5, 6.7)
    query = mapnik.Query(mapnik.Box2d(*bbox), init_res)
    r = query.resolution
    assert_almost_equal(r[0], init_res[0], places=7)
    assert_almost_equal(r[1], init_res[1], places=7)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
