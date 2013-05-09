#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from utilities import execution_path
from nose.tools import *
import mapnik
import threading

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_svg_loading_from_file():
    svg_file = '../data/svg/octocat.svg'
    svg = mapnik.SVG.open(svg_file)
    # TODO - invalid numbers
    eq_(svg.width(),0)
    eq_(svg.height(),0)
    expected = mapnik.Box2d(0.00700000000001,-4.339,378.46,332.606)
    actual = svg.extent()
    assert_almost_equal(expected.minx,actual.minx, places=7)
    assert_almost_equal(expected.miny,actual.miny, places=7)
    assert_almost_equal(expected.maxx,actual.maxx, places=7)
    assert_almost_equal(expected.maxy,actual.maxy, places=7)

def test_svg_loading_from_string():
    svg_file = '../data/svg/octocat.svg'
    svg = mapnik.SVG.fromstring(open(svg_file,'rb').read())
    # TODO - invalid numbers
    eq_(svg.width(),0)
    eq_(svg.height(),0)
    expected = mapnik.Box2d(0.00700000000001,-4.339,378.46,332.606)
    actual = svg.extent()
    assert_almost_equal(expected.minx,actual.minx, places=7)
    assert_almost_equal(expected.miny,actual.miny, places=7)
    assert_almost_equal(expected.maxx,actual.maxx, places=7)
    assert_almost_equal(expected.maxy,actual.maxy, places=7)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
