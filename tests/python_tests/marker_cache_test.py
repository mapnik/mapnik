#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from utilities import execution_path
from nose.tools import *
import mapnik

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

def test_svg_put_to_marker_cache():
    cache = mapnik.MarkerCache.instance()
    cache.clear()
    eq_(cache.size(),3)
    svg_file = '../data/svg/octocat.svg'
    svg = mapnik.SVG.open(svg_file)
    cache.put(svg_file,svg)
    eq_(cache.size(),4)
    eq_(svg_file in cache.keys(),True)

def test_svg_put_and_clear_marker_cache():
    cache = mapnik.MarkerCache.instance()
    cache.clear()
    eq_(cache.size(),3)
    svg_file = '../data/svg/octocat.svg'
    svg = mapnik.SVG.open(svg_file)
    cache.put(svg_file,svg)
    eq_(cache.size(),4)
    eq_(svg_file in cache.keys(),True)
    cache.clear()
    eq_(svg_file in cache.keys(),False)
    eq_(cache.size(),3)

def test_svg_put_and_remove_marker_cache():
    cache = mapnik.MarkerCache.instance()
    cache.clear()
    eq_(cache.size(),3)
    svg_file = '../data/svg/octocat.svg'
    svg = mapnik.SVG.open(svg_file)
    cache.put(svg_file,svg)
    eq_(cache.size(),4)
    eq_(svg_file in cache.keys(),True)
    cache.remove(svg_file)
    eq_(svg_file in cache.keys(),False)
    eq_(cache.size(),3)

def test_image_put_and_clear_marker_cache():
    cache = mapnik.MarkerCache.instance()
    cache.clear()
    eq_(cache.size(),3)
    image_file = '../data/images/marker.png'
    image = mapnik.Image.open(image_file)
    cache.put(image_file,image)
    eq_(cache.size(),4)
    eq_(image_file in cache.keys(),True)
    cache.clear()
    eq_(image_file in cache.keys(),False)
    eq_(cache.size(),3)

def test_image_put_and_clear_marker_cache():
    cache = mapnik.MarkerCache.instance()
    cache.clear()
    eq_(cache.size(),3)
    image_file = '../data/images/marker.png'
    image = mapnik.Image.open(image_file)
    cache.put(image_file,image)
    eq_(cache.size(),4)
    eq_(image_file in cache.keys(),True)
    eq_(cache.remove(image_file),True)
    # removing twice should return False for no successful removal
    eq_(cache.remove(image_file),False)
    eq_(image_file in cache.keys(),False)
    eq_(cache.size(),3)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
