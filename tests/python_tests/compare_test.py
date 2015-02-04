#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, mapnik
from nose.tools import *
from utilities import execution_path, run_all

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_compare_rgba8():
    im = mapnik.Image(5,5,mapnik.ImageType.rgba8)
    im.fill(mapnik.Color(0,0,0,0))
    eq_(im.compare(im), 0)
    im2 = mapnik.Image(5,5,mapnik.ImageType.rgba8)
    im2.fill(mapnik.Color(0,0,0,0))
    eq_(im.compare(im2), 0)
    eq_(im2.compare(im), 0)
    im2.fill(mapnik.Color(0,0,0,12))
    eq_(im.compare(im2), 25)
    eq_(im.compare(im2, 0, False), 0)
    im3 = mapnik.Image(5,5,mapnik.ImageType.rgba8)
    im3.set_pixel(0,0, mapnik.Color(0,0,0,0))
    im3.set_pixel(0,1, mapnik.Color(1,1,1,1))
    im3.set_pixel(1,0, mapnik.Color(2,2,2,2))
    im3.set_pixel(1,1, mapnik.Color(3,3,3,3))
    eq_(im.compare(im3), 3)
    eq_(im.compare(im3,1),2)
    eq_(im.compare(im3,2),1)
    eq_(im.compare(im3,3),0)

def test_compare_larger_image():
    im = mapnik.Image(5,5)
    im.set_pixel(0,0, mapnik.Color(254, 254, 254, 254))
    im.set_pixel(4,4, mapnik.Color('white'))
    im2 = mapnik.Image(5,5)
    eq_(im2.compare(im,16), 2)

def test_compare_dimensions():
    im = mapnik.Image(2,2)
    im2 = mapnik.Image(3,3)
    eq_(im.compare(im2), 4)
    eq_(im2.compare(im), 9)

def test_compare_gray8():
    im = mapnik.Image(2,2,mapnik.ImageType.gray8)
    im.fill(0)
    eq_(im.compare(im), 0)
    im2 = mapnik.Image(2,2,mapnik.ImageType.gray8)
    im2.fill(0)
    eq_(im.compare(im2), 0)
    eq_(im2.compare(im), 0)
    eq_(im.compare(im2, 0, False), 0)
    im3 = mapnik.Image(2,2,mapnik.ImageType.gray8)
    im3.set_pixel(0,0,0)
    im3.set_pixel(0,1,1)
    im3.set_pixel(1,0,2)
    im3.set_pixel(1,1,3)
    eq_(im.compare(im3),3)
    eq_(im.compare(im3,1),2)
    eq_(im.compare(im3,2),1)
    eq_(im.compare(im3,3),0)

def test_compare_gray16():
    im = mapnik.Image(2,2,mapnik.ImageType.gray16)
    im.fill(0)
    eq_(im.compare(im), 0)
    im2 = mapnik.Image(2,2,mapnik.ImageType.gray16)
    im2.fill(0)
    eq_(im.compare(im2), 0)
    eq_(im2.compare(im), 0)
    eq_(im.compare(im2, 0, False), 0)
    im3 = mapnik.Image(2,2,mapnik.ImageType.gray16)
    im3.set_pixel(0,0,0)
    im3.set_pixel(0,1,1)
    im3.set_pixel(1,0,2)
    im3.set_pixel(1,1,3)
    eq_(im.compare(im3),3)
    eq_(im.compare(im3,1),2)
    eq_(im.compare(im3,2),1)
    eq_(im.compare(im3,3),0)

def test_compare_gray32f():
    im = mapnik.Image(2,2,mapnik.ImageType.gray32f)
    im.fill(0.5)
    eq_(im.compare(im), 0)
    im2 = mapnik.Image(2,2,mapnik.ImageType.gray32f)
    im2.fill(0.5)
    eq_(im.compare(im2), 0)
    eq_(im2.compare(im), 0)
    eq_(im.compare(im2, 0, False), 0)
    im3 = mapnik.Image(2,2,mapnik.ImageType.gray32f)
    im3.set_pixel(0,0,0.5)
    im3.set_pixel(0,1,1.5)
    im3.set_pixel(1,0,2.5)
    im3.set_pixel(1,1,3.5)
    eq_(im.compare(im3),3)
    eq_(im.compare(im3,1.0),2)
    eq_(im.compare(im3,2.0),1)
    eq_(im.compare(im3,3.0),0)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
