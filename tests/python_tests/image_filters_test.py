#!/usr/bin/env python

from nose.tools import *
import os, mapnik

def test_append():
    s = mapnik.Style()
    eq_(s.image_filters,'')
    s.image_filters = 'gray'
    eq_(s.image_filters,'gray')
    s.image_filters = 'sharpen'
    eq_(s.image_filters,'sharpen')

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
