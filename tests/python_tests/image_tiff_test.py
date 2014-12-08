#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os, mapnik
from timeit import Timer, time
from nose.tools import *
from utilities import execution_path, run_all

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_tiff_rgba8_compare():
    filepath1 = '../data/images/24989_rgb_uint8.tif'
    filepath2 = '/tmp/mapnik-tiff-rgba8.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))

def test_tiff_uint8_compare():
    filepath1 = '../data/images/24989_ndvi_uint8.tif'
    filepath2 = '/tmp/mapnik-tiff-uint8.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))

def test_tiff_uint16_compare():
    filepath1 = '../data/images/24989_ndvi_uint16.tif'
    filepath2 = '/tmp/mapnik-tiff-uint16.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))

def test_tiff_float32_compare():
    filepath1 = '../data/images/24989_ndvi_float32.tif'
    filepath2 = '/tmp/mapnik-tiff-float32.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))
if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
