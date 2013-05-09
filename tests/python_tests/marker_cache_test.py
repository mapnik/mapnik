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

def test_image_put_and_get_image_in_marker_cache():
    cache = mapnik.MarkerCache.instance()
    cache.clear()
    eq_(cache.size(),3)
    image_file = '../data/images/marker.png'
    image = mapnik.Image.open(image_file)
    cache.put(image_file,image)
    new_im = cache.get(image_file)
    eq_(image.tostring(),new_im.tostring())

def test_image_put_and_get_svg_in_marker_cache():
    cache = mapnik.MarkerCache.instance()
    cache.clear()
    eq_(cache.size(),3)
    image_file = '../data/svg/rect.svg'
    image = mapnik.SVG.open(image_file)
    cache.put(image_file,image)
    new_im = cache.get(image_file)
    eq_(image.width(),new_im.width())

def test_marker_cache_override():
    cache = mapnik.MarkerCache.instance()
    cache.clear()
    eq_(cache.size(),3)
    image_file = '../data/images/marker.png'
    image = mapnik.Image.open(image_file)
    cache.put(image_file,image)
    alt_im = mapnik.Image(4,4)
    result = cache.put(image_file,alt_im)
    # putting a item for which a key already exists should return False
    eq_(result,False)
    alt_im_copy = cache.get(image_file)
    eq_(alt_im.tostring(),alt_im_copy.tostring())


def test_threaded_reads_and_writes():
    threads = []
    for i in range(100):
        t = threading.Thread(target=test_image_put_and_clear_marker_cache)
        t.start()
        threads.append(t)
    for t in threads:
        t.join()

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
