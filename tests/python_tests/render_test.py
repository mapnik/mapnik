#!/usr/bin/env python

from nose.tools import *

import os, mapnik

def test_simplest_render():
    m = mapnik.Map(256, 256)
    i = mapnik.Image(m.width, m.height)

    mapnik.render(m, i)

    s = i.tostring()

    eq_(s, 256 * 256 * '\x00\x00\x00\x00')

def test_render_image_to_string():
    i = mapnik.Image(256, 256)
    
    i.background = mapnik.Color('black')
    
    s = i.tostring()

    eq_(s, 256 * 256 * '\x00\x00\x00\xff')

    s = i.tostring('png')

def test_render_image_to_file():
    i = mapnik.Image(256, 256)
    
    i.background = mapnik.Color('black')

    i.save('test.jpg')
    i.save('test.png', 'png')

    if os.path.exists('test.jpg'):
        os.remove('test.jpg')
    else:
        return False
    
    if os.path.exists('test.png'):
        os.remove('test.png')
    else:
        return False
