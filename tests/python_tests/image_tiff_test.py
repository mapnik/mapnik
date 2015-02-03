#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, mapnik
from nose.tools import eq_
from utilities import execution_path, run_all

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_tiff_round_trip_scanline():
    filepath = '/tmp/mapnik-tiff-io-scanline.tiff'
    im = mapnik.Image(255,267)
    im.background = mapnik.Color('rgba(1,2,3,.5)')
    org_str = len(im.tostring())
    im.save(filepath,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    eq_(len(im.tostring()), org_str)
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=scanline')),len(im2.tostring('tiff:method=scanline')))
    eq_(len(im.tostring()),len(im3.tostring()))
    eq_(len(im.tostring('tiff:method=scanline')),len(im3.tostring('tiff:method=scanline')))

def test_tiff_round_trip_stripped():
    filepath = '/tmp/mapnik-tiff-io-stripped.tiff'
    im = mapnik.Image(255,267)
    im.background = mapnik.Color('rgba(1,2,3,.5)')
    org_str = len(im.tostring())
    im.save(filepath,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    eq_(len(im.tostring()), org_str)
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=stripped')),len(im2.tostring('tiff:method=stripped')))
    eq_(len(im.tostring()),len(im3.tostring()))
    eq_(len(im.tostring('tiff:method=stripped')),len(im3.tostring('tiff:method=stripped')))

def test_tiff_round_trip_rows_stripped():
    filepath = '/tmp/mapnik-tiff-io-stripped.tiff'
    im = mapnik.Image(255,267)
    im.background = mapnik.Color('rgba(1,2,3,.5)')
    org_str = len(im.tostring())
    im.save(filepath,'tiff:method=stripped:rows_per_strip=8')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    eq_(len(im.tostring()), org_str)
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=stripped:rows_per_strip=8')),len(im2.tostring('tiff:method=stripped:rows_per_strip=8')))
    eq_(len(im.tostring()),len(im3.tostring()))
    eq_(len(im.tostring('tiff:method=stripped:rows_per_strip=8')),len(im3.tostring('tiff:method=stripped:rows_per_strip=8')))

def test_tiff_round_trip_buffered_tiled():
    filepath = '/tmp/mapnik-tiff-io-buffered-tiled.tiff'
    filepath2 = '/tmp/mapnik-tiff-io-buffered-tiled2.tiff'
    im = mapnik.Image(255,267)
    #im = mapnik.Image(256,256)
    im.background = mapnik.Color('rgba(1,2,3,.5)')
    im.save(filepath,'tiff:method=tiled:tile_width=32:tile_height=32')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    im2.save(filepath2, 'tiff:method=tiled:tile_width=32:tile_height=32')
    im4 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    eq_(len(im2.tostring()),len(im4.tostring()))
    eq_(len(im2.tostring('tiff:method=tiled:tile_width=32:tile_height=32')),len(im4.tostring('tiff:method=tiled:tile_width=32:tile_height=32')))
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=tiled:tile_width=32:tile_height=32')),len(im2.tostring('tiff:method=tiled:tile_width=32:tile_height=32')))
    eq_(len(im.tostring()),len(im3.tostring()))
    eq_(len(im.tostring('tiff:method=tiled:tile_width=32:tile_height=32')),len(im3.tostring('tiff:method=tiled:tile_width=32:tile_height=32')))

def test_tiff_round_trip_tiled():
    filepath = '/tmp/mapnik-tiff-io-tiled.tiff'
    im = mapnik.Image(256,256)
    im.background = mapnik.Color('rgba(1,2,3,.5)')
    im.save(filepath,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=tiled')),len(im2.tostring('tiff:method=tiled')))
    eq_(len(im.tostring()),len(im3.tostring()))
    eq_(len(im.tostring('tiff:method=tiled')),len(im3.tostring('tiff:method=tiled')))


def test_tiff_rgb8_compare():
    filepath1 = '../data/tiff/ndvi_256x256_rgb8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgb8.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff')),len(im2.tostring('tiff')))
    # should not be a blank image
    eq_(len(im.tostring("png")) != len(mapnik.Image(im.width(),im.height()).tostring("png")),True)

def test_tiff_rgba8_compare_scanline():
    filepath1 = '../data/tiff/ndvi_256x256_rgba8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgba8-scanline.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=scanline')),len(im2.tostring('tiff:method=scanline')))
    # should not be a blank image
    eq_(len(im.tostring("png")) != len(mapnik.Image(im.width(),im.height()).tostring("png")),True)

def test_tiff_rgba8_compare_stripped():
    filepath1 = '../data/tiff/ndvi_256x256_rgba8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgba8-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=stripped')),len(im2.tostring('tiff:method=stripped')))
    # should not be a blank image
    eq_(len(im.tostring("png")) != len(mapnik.Image(im.width(),im.height()).tostring("png")),True)

def test_tiff_rgba8_compare_tiled():
    filepath1 = '../data/tiff/ndvi_256x256_rgba8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgba8-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=tiled')),len(im2.tostring('tiff:method=tiled')))
    # should not be a blank image
    eq_(len(im.tostring("png")) != len(mapnik.Image(im.width(),im.height()).tostring("png")),True)

def test_tiff_gray8_compare_scanline():
    filepath1 = '../data/tiff/ndvi_256x256_gray8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray8-scanline.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=scanline')),len(im2.tostring('tiff:method=scanline')))

def test_tiff_gray8_compare_stripped():
    filepath1 = '../data/tiff/ndvi_256x256_gray8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray8-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=stripped')),len(im2.tostring('tiff:method=stripped')))

def test_tiff_gray8_compare_tiled():
    filepath1 = '../data/tiff/ndvi_256x256_gray8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray8-tiled.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=tiled')),len(im2.tostring('tiff:method=tiled')))

def test_tiff_gray16_compare_scanline():
    filepath1 = '../data/tiff/ndvi_256x256_gray16_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray16-scanline.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=scanline')),len(im2.tostring('tiff:method=scanline')))

def test_tiff_gray16_compare_stripped():
    filepath1 = '../data/tiff/ndvi_256x256_gray16_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray16-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=stripped')),len(im2.tostring('tiff:method=stripped')))

def test_tiff_gray16_compare_tiled():
    filepath1 = '../data/tiff/ndvi_256x256_gray16_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray16-tiled.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=tiled')),len(im2.tostring('tiff:method=tiled')))

def test_tiff_gray32f_compare_scanline():
    filepath1 = '../data/tiff/ndvi_256x256_gray32f_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray32f-scanline.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=scanline')),len(im2.tostring('tiff:method=scanline')))

def test_tiff_gray32f_compare_stripped():
    filepath1 = '../data/tiff/ndvi_256x256_gray32f_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray32f-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=stripped')),len(im2.tostring('tiff:method=stripped')))

def test_tiff_gray32f_compare_tiled():
    filepath1 = '../data/tiff/ndvi_256x256_gray32f_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray32f-tiled.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(len(im.tostring()),len(im2.tostring()))
    eq_(len(im.tostring('tiff:method=tiled')),len(im2.tostring('tiff:method=tiled')))

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
