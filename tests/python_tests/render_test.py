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

def get_paired_images(w,h,mapfile):
    tmp_map = 'tmp_map.xml'
    m = mapnik.Map(w,h)
    mapnik.load_map(m,mapfile)
    i = mapnik.Image(w,h)
    m.zoom_all()
    mapnik.render(m,i)
    mapnik.save_map(m,tmp_map)
    m2 = mapnik.Map(w,h)
    mapnik.load_map(m2,tmp_map)
    i2 = mapnik.Image(w,h)
    m2.zoom_all()
    mapnik.render(m2,i2)
    os.remove(tmp_map)
    return i,i2    

def test_render_from_serialization():
    i,i2 = get_paired_images(100,100,'../data/good_maps/building_symbolizer.xml')
    eq_(i.tostring(),i2.tostring())

    i,i2 = get_paired_images(100,100,'../data/good_maps/polygon_symbolizer.xml')
    eq_(i.tostring(),i2.tostring())
