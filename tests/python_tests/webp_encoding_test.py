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

tmp_dir = '/tmp/mapnik-webp/'
if not os.path.exists(tmp_dir):
   os.makedirs(tmp_dir)

opts = [
'webp',
'webp:q=64',
]


def gen_filepath(name,format):
    return os.path.join('images/support/encoding-opts',name+'-'+format.replace(":","+")+'.webp')

def test_quality_threshold():
    im = mapnik.Image(256,256)
    im.tostring('webp:quality=99.99000')
    im.tostring('webp:quality=0')
    im.tostring('webp:quality=0.001')

@raises(RuntimeError)
def test_quality_threshold_invalid():
    im = mapnik.Image(256,256)
    im.tostring('webp:quality=101')

@raises(RuntimeError)
def test_quality_threshold_invalid2():
    im = mapnik.Image(256,256)
    im.tostring('webp:quality=-1')

generate = False

def test_expected_encodings():
    im = mapnik.Image(256,256)
    for opt in opts:
        expected = gen_filepath('solid',opt)
        actual = os.path.join(tmp_dir,os.path.basename(expected))
        if generate or not os.path.exists(expected):
            print 'generating expected image %s' % expected
            im.save(expected,opt)
        im.save(actual,opt)
        eq_(mapnik.Image.open(actual).tostring(),
            mapnik.Image.open(expected).tostring(),
            '%s (actual) not == to %s (expected)' % (actual,expected))

    for opt in opts:
        expected = gen_filepath('blank',opt)
        actual = os.path.join(tmp_dir,os.path.basename(expected))
        if generate or not os.path.exists(expected):
            print 'generating expected image %s' % expected
            im.save(expected,opt)
        im.save(actual,opt)
        eq_(mapnik.Image.open(actual).tostring(),
            mapnik.Image.open(expected).tostring(),
            '%s (actual) not == to %s (expected)' % (actual,expected))

def test_transparency_levels():
    # create partial transparency image
    im = mapnik.Image(256,256)
    im.background = mapnik.Color('rgba(255,255,255,.5)')
    c2 = mapnik.Color('rgba(255,255,0,.2)')
    c3 = mapnik.Color('rgb(0,255,255)')
    for y in range(0,im.height()/2):
        for x in range(0,im.width()/2):
            im.set_pixel(x,y,c2)
    for y in range(im.height()/2,im.height()):
        for x in range(im.width()/2,im.width()):
            im.set_pixel(x,y,c3)

    t0 = tmp_dir + 'white0.webp'

    # octree
    format = 'webp'
    expected = 'images/support/transparency/white0.webp'
    if generate or not os.path.exists(expected):
        im.save('images/support/transparency/white0.webp')
    im.save(t0,format)
    im_in = mapnik.Image.open(t0)
    t0_len = len(im_in.tostring(format))
    eq_(t0_len,len(mapnik.Image.open(expected).tostring(format)))


if __name__ == "__main__":
    setup()
    run_all(eval(x) for x in dir() if x.startswith("test_"))