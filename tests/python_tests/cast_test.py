#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, mapnik
from nose.tools import *
from utilities import execution_path, run_all

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_image_16_8_simple():
    im = mapnik.Image(2,2,mapnik.ImageType.gray16)
    im.set_pixel(0,0, 256)
    im.set_pixel(0,1, 999)
    im.set_pixel(1,0, 5)
    im.set_pixel(1,1, 2)
    im2 = im.cast(mapnik.ImageType.gray8)
    eq_(im2.get_pixel(0,0), 255)
    eq_(im2.get_pixel(0,1), 255)
    eq_(im2.get_pixel(1,0), 5)
    eq_(im2.get_pixel(1,1), 2)
    # Cast back!
    im = im2.cast(mapnik.ImageType.gray16)
    eq_(im.get_pixel(0,0), 255)
    eq_(im.get_pixel(0,1), 255)
    eq_(im.get_pixel(1,0), 5)
    eq_(im.get_pixel(1,1), 2)

def test_image_32f_8_simple():
    im = mapnik.Image(2,2,mapnik.ImageType.gray32f)
    im.set_pixel(0,0, 120.1234)
    im.set_pixel(0,1, -23.4)
    im.set_pixel(1,0, 120.6)
    im.set_pixel(1,1, 360.2)
    im2 = im.cast(mapnik.ImageType.gray8)
    eq_(im2.get_pixel(0,0), 120)
    eq_(im2.get_pixel(0,1), 0)
    eq_(im2.get_pixel(1,0), 120) # Notice this is truncated!
    eq_(im2.get_pixel(1,1), 255)

def test_image_offset_and_scale():
    im = mapnik.Image(2,2,mapnik.ImageType.gray16)
    eq_(im.offset, 0.0)
    eq_(im.scaling, 1.0)
    im.offset = 1.0
    im.scaling = 2.0
    eq_(im.offset, 1.0)
    eq_(im.scaling, 2.0)

def test_image_16_8_scale_and_offset():
    im = mapnik.Image(2,2,mapnik.ImageType.gray16)
    im.set_pixel(0,0, 256)
    im.set_pixel(0,1, 258)
    im.set_pixel(1,0, 99999)
    im.set_pixel(1,1, 615)
    offset = 255
    scaling = 3
    im2 = im.cast(mapnik.ImageType.gray8, offset, scaling)
    eq_(im2.get_pixel(0,0), 0)
    eq_(im2.get_pixel(0,1), 1)
    eq_(im2.get_pixel(1,0), 255)
    eq_(im2.get_pixel(1,1), 120)
    # pixels will be a little off due to offsets in reverting!
    im3 = im2.cast(mapnik.ImageType.gray16)
    eq_(im3.get_pixel(0,0), 255) # Rounding error with ints
    eq_(im3.get_pixel(0,1), 258) # same
    eq_(im3.get_pixel(1,0), 1020) # The other one was way out of range for our scale/offset
    eq_(im3.get_pixel(1,1), 615) # same 

def test_image_16_32f_scale_and_offset():
    im = mapnik.Image(2,2,mapnik.ImageType.gray16)
    im.set_pixel(0,0, 256)
    im.set_pixel(0,1, 258)
    im.set_pixel(1,0, 0)
    im.set_pixel(1,1, 615)
    offset = 255
    scaling = 3.2
    im2 = im.cast(mapnik.ImageType.gray32f, offset, scaling)
    eq_(im2.get_pixel(0,0), 0.3125)
    eq_(im2.get_pixel(0,1), 0.9375)
    eq_(im2.get_pixel(1,0), -79.6875)
    eq_(im2.get_pixel(1,1), 112.5)
    im3 = im2.cast(mapnik.ImageType.gray16)
    eq_(im3.get_pixel(0,0), 256) 
    eq_(im3.get_pixel(0,1), 258)
    eq_(im3.get_pixel(1,0), 0) 
    eq_(im3.get_pixel(1,1), 615) 

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
