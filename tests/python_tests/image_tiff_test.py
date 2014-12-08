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
    filepath1 = '../data/tiff/ndvi_256x256_rgba8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgba8.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))

def test_tiff_gray8_compare():
    filepath1 = '../data/tiff/ndvi_256x256_gray8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray8.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))

def test_tiff_gray16_compare():
    filepath1 = '../data/tiff/ndvi_256x256_gray16_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray16.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))

def test_tiff_gray32f_compare():
    filepath1 = '../data/tiff/ndvi_256x256_gray32f_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray32f.tiff'
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
