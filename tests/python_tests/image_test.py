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

def test_image_premultiply():
    im = mapnik.Image(256,256)
    eq_(im.premultiplied(),False)
    im.premultiply()
    eq_(im.premultiplied(),True)
    im.demultiply()
    eq_(im.premultiplied(),False)

# Disabled for now since this breaks hard if run against
# a mapnik version that does not have the fix
#@raises(RuntimeError)
#def test_negative_image_dimensions():
    #im = mapnik.Image(-40,40)

def test_tiff_round_trip():
    filepath = '/tmp/mapnik-tiff-io.tiff'
    im = mapnik.Image(255,267)
    im.background = mapnik.Color('rgba(1,2,3,.5)')
    im.save(filepath,'tiff')
    im2 = mapnik.Image.open(filepath)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))

def test_jpeg_round_trip():
    filepath = '/tmp/mapnik-jpeg-io.jpeg'
    im = mapnik.Image(255,267)
    im.background = mapnik.Color('rgba(1,2,3,.5)')
    im.save(filepath,'jpeg')
    im2 = mapnik.Image.open(filepath)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('jpeg')),len(im2.tostring('jpeg')))

def test_png_round_trip():
    filepath = '/tmp/mapnik-png-io.png'
    im = mapnik.Image(255,267)
    im.background = mapnik.Color('rgba(1,2,3,.5)')
    im.save(filepath,'png')
    im2 = mapnik.Image.open(filepath)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('png')),len(im2.tostring('png')))
    eq_(len(im.tostring('png8')),len(im2.tostring('png8')))

def test_image_open_from_string():
    filepath = '../data/images/dummy.png'
    im1 = mapnik.Image.open(filepath)
    im2 = mapnik.Image.fromstring(open(filepath,'rb').read())
    eq_(im1.width(),im2.width())
    length = len(im1.tostring())
    eq_(length,len(im2.tostring()))
    eq_(len(mapnik.Image.fromstring(im1.tostring('png')).tostring()),length)
    eq_(len(mapnik.Image.fromstring(im1.tostring('jpeg')).tostring()),length)
    eq_(len(mapnik.Image.frombuffer(buffer(im1.tostring('png'))).tostring()),length)
    eq_(len(mapnik.Image.frombuffer(buffer(im1.tostring('jpeg'))).tostring()),length)

    # TODO - https://github.com/mapnik/mapnik/issues/1831
    eq_(len(mapnik.Image.fromstring(im1.tostring('tiff')).tostring()),length)
    eq_(len(mapnik.Image.frombuffer(buffer(im1.tostring('tiff'))).tostring()),length)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
