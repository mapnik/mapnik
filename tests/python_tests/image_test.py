#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os, mapnik
from timeit import Timer, time
from nose.tools import *
from utilities import execution_path, run_all, get_unique_colors

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_image_premultiply():
    im = mapnik.Image(256,256)
    eq_(im.premultiplied(),False)
    # Premultiply should return true that it worked
    eq_(im.premultiply(), True)
    eq_(im.premultiplied(),True)
    # Premultipling again should return false as nothing should happen
    eq_(im.premultiply(), False)
    eq_(im.premultiplied(),True)
    # Demultiply should return true that it worked
    eq_(im.demultiply(), True)
    eq_(im.premultiplied(),False)
    # Demultiply again should not work and return false as it did nothing
    eq_(im.demultiply(), False)
    eq_(im.premultiplied(),False)

def test_image_premultiply_values():
    im = mapnik.Image(256,256)
    im.background(mapnik.Color(16, 33, 255, 128))
    im.premultiply()
    c = im.get_pixel_color(0,0)
    eq_(c.r, 8)
    eq_(c.g, 17)
    eq_(c.b, 128)
    eq_(c.a, 128)
    im.demultiply()
    # Do to the nature of this operation the result will not be exactly the same
    c = im.get_pixel_color(0,0)
    eq_(c.r,15)
    eq_(c.g,33)
    eq_(c.b,255)
    eq_(c.a,128)

def test_background():
    im = mapnik.Image(256,256)
    eq_(im.premultiplied(), False)
    im.background(mapnik.Color(32,64,125,128))
    eq_(im.premultiplied(), False)
    c = im.get_pixel_color(0,0)
    eq_(c.get_premultiplied(), False)
    eq_(c.r,32)
    eq_(c.g,64)
    eq_(c.b,125)
    eq_(c.a,128)
    # Now again with a premultiplied alpha
    im.background(mapnik.Color(32,64,125,128,True))
    eq_(im.premultiplied(), True)
    c = im.get_pixel_color(0,0)
    eq_(c.get_premultiplied(), True)
    eq_(c.r,32)
    eq_(c.g,64)
    eq_(c.b,125)
    eq_(c.a,128)

def test_set_and_get_pixel():
    # Create an image that is not premultiplied
    im = mapnik.Image(256,256)
    c0 = mapnik.Color(16,33,255,128)
    c0_pre = mapnik.Color(16,33,255,128, True)
    im.set_pixel(0,0,c0)
    im.set_pixel(1,1,c0_pre)
    # No differences for non premultiplied pixels
    c1_int = mapnik.Color(im.get_pixel(0,0))
    eq_(c0.r, c1_int.r)
    eq_(c0.g, c1_int.g)
    eq_(c0.b, c1_int.b)
    eq_(c0.a, c1_int.a)
    c1 = im.get_pixel_color(0,0)
    eq_(c0.r, c1.r)
    eq_(c0.g, c1.g)
    eq_(c0.b, c1.b)
    eq_(c0.a, c1.a)
    # The premultiplied Color should be demultiplied before being applied.
    c0_pre.demultiply()
    c1_int = mapnik.Color(im.get_pixel(1,1))
    eq_(c0_pre.r, c1_int.r)
    eq_(c0_pre.g, c1_int.g)
    eq_(c0_pre.b, c1_int.b)
    eq_(c0_pre.a, c1_int.a)
    c1 = im.get_pixel_color(1,1)
    eq_(c0_pre.r, c1.r)
    eq_(c0_pre.g, c1.g)
    eq_(c0_pre.b, c1.b)
    eq_(c0_pre.a, c1.a)
    
    # Now create a new image that is premultiplied
    im = mapnik.Image(256,256, mapnik.ImageType.rgba8, True, True)
    c0 = mapnik.Color(16,33,255,128)
    c0_pre = mapnik.Color(16,33,255,128, True)
    im.set_pixel(0,0,c0)
    im.set_pixel(1,1,c0_pre)
    # It should have put pixels that are the same as premultiplied so premultiply c0
    c0.premultiply()
    c1_int = mapnik.Color(im.get_pixel(0,0))
    eq_(c0.r, c1_int.r)
    eq_(c0.g, c1_int.g)
    eq_(c0.b, c1_int.b)
    eq_(c0.a, c1_int.a)
    c1 = im.get_pixel_color(0,0)
    eq_(c0.r, c1.r)
    eq_(c0.g, c1.g)
    eq_(c0.b, c1.b)
    eq_(c0.a, c1.a)
    # The premultiplied Color should be the same though
    c1_int = mapnik.Color(im.get_pixel(1,1))
    eq_(c0_pre.r, c1_int.r)
    eq_(c0_pre.g, c1_int.g)
    eq_(c0_pre.b, c1_int.b)
    eq_(c0_pre.a, c1_int.a)
    c1 = im.get_pixel_color(1,1)
    eq_(c0_pre.r, c1.r)
    eq_(c0_pre.g, c1.g)
    eq_(c0_pre.b, c1.b)
    eq_(c0_pre.a, c1.a)

def test_pixel_overflow():
    im = mapnik.Image(4,4,mapnik.ImageType.gray8)
    im.set_pixel(0,0,256)
    eq_(im.get_pixel(0,0),255)

def test_pixel_underflow():
    im = mapnik.Image(4,4,mapnik.ImageType.gray8)
    im.set_pixel(0,0,-1)
    eq_(im.get_pixel(0,0),0)

@raises(IndexError)
def test_set_pixel_out_of_range_1():
    im = mapnik.Image(4,4)
    c = mapnik.Color('blue')
    im.set_pixel(5,5,c)

@raises(OverflowError)
def test_set_pixel_out_of_range_2():
    im = mapnik.Image(4,4)
    c = mapnik.Color('blue')
    im.set_pixel(-1,1,c)

@raises(IndexError)
def test_get_pixel_out_of_range_1():
    im = mapnik.Image(4,4)
    c = im.get_pixel(5,5)

@raises(OverflowError)
def test_get_pixel_out_of_range_2():
    im = mapnik.Image(4,4)
    c = im.get_pixel(-1,1)

@raises(IndexError)
def test_get_pixel_color_out_of_range_1():
    im = mapnik.Image(4,4)
    c = im.get_pixel_color(5,5)

@raises(OverflowError)
def test_get_pixel_color_out_of_range_2():
    im = mapnik.Image(4,4)
    c = im.get_pixel_color(-1,1)
    
def test_set_color_to_alpha():
    im = mapnik.Image(256,256)
    im.background(mapnik.Color('rgba(12,12,12,255)'))
    eq_(get_unique_colors(im), ['rgba(12,12,12,255)'])
    im.set_color_to_alpha(mapnik.Color('rgba(12,12,12,0)'))
    eq_(get_unique_colors(im), ['rgba(0,0,0,0)'])

@raises(RuntimeError)
def test_negative_image_dimensions():
    # TODO - this may have regressed in https://github.com/mapnik/mapnik/commit/4f3521ac24b61fc8ae8fd344a16dc3a5fdf15af7
    im = mapnik.Image(-40,40)

def test_jpeg_round_trip():
    filepath = '/tmp/mapnik-jpeg-io.jpeg'
    im = mapnik.Image(255,267)
    im.background(mapnik.Color('rgba(1,2,3,.5)'))
    im.save(filepath,'jpeg')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('jpeg')),len(im2.tostring('jpeg')))
    eq_(len(im.tostring()),len(im3.tostring()))
    eq_(len(im.tostring('jpeg')),len(im3.tostring('jpeg')))

def test_png_round_trip():
    filepath = '/tmp/mapnik-png-io.png'
    im = mapnik.Image(255,267)
    im.background(mapnik.Color('rgba(1,2,3,.5)'))
    im.save(filepath,'png')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('png')),len(im2.tostring('png')))
    eq_(len(im.tostring('png8')),len(im2.tostring('png8')))
    eq_(len(im.tostring()),len(im3.tostring()))
    eq_(len(im.tostring('png')),len(im3.tostring('png')))
    eq_(len(im.tostring('png8')),len(im3.tostring('png8')))

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
