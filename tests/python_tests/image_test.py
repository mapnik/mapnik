#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os, mapnik
from timeit import Timer, time
from nose.tools import *
from utilities import execution_path

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))


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
    #eq_(len(mapnik.Image.fromstring(im1.tostring('tiff')).tostring()),length)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
