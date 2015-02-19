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

def test_color_init():
    c = mapnik.Color(12, 128, 255)
    eq_(c.r, 12)
    eq_(c.g, 128)
    eq_(c.b, 255)
    eq_(c.a, 255)
    eq_(False, c.get_premultiplied())
    c = mapnik.Color(16, 32, 64, 128)
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    eq_(False, c.get_premultiplied())
    c = mapnik.Color(16, 32, 64, 128,True)
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    eq_(True, c.get_premultiplied())
    c = mapnik.Color('rgba(16,32,64,0.5)')
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    eq_(False, c.get_premultiplied())
    c = mapnik.Color('rgba(16,32,64,0.5)', True)
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    eq_(True, c.get_premultiplied())
    hex_str = '#10204080'
    c = mapnik.Color(hex_str)
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    eq_(hex_str, c.to_hex_string())
    eq_(False, c.get_premultiplied())
    c = mapnik.Color(hex_str, True)
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    eq_(hex_str, c.to_hex_string())
    eq_(True, c.get_premultiplied())
    rgba_int = 2151686160
    c = mapnik.Color(rgba_int)
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    eq_(rgba_int, c.packed())
    eq_(False, c.get_premultiplied())
    c = mapnik.Color(rgba_int, True)
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    eq_(rgba_int, c.packed())
    eq_(True, c.get_premultiplied())

def test_color_properties():
    c = mapnik.Color(16, 32, 64, 128)
    eq_(c.r, 16)
    eq_(c.g, 32)
    eq_(c.b, 64)
    eq_(c.a, 128)
    c.r = 17
    eq_(c.r, 17)
    c.g = 33
    eq_(c.g, 33)
    c.b = 65
    eq_(c.b, 65)
    c.a = 128
    eq_(c.a, 128)

def test_color_premultiply():
    c = mapnik.Color(16, 33, 255, 128)
    eq_(c.premultiply(), True)
    eq_(c.r, 8)
    eq_(c.g, 17)
    eq_(c.b, 128)
    eq_(c.a, 128)
    # Repeating it again should do nothing
    eq_(c.premultiply(), False)
    eq_(c.r, 8)
    eq_(c.g, 17)
    eq_(c.b, 128)
    eq_(c.a, 128)
    c.demultiply()
    c.demultiply()
    # This will not return the same values as before but we expect that
    eq_(c.r,15)
    eq_(c.g,33)
    eq_(c.b,255)
    eq_(c.a,128)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
