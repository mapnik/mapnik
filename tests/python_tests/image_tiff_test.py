#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os, mapnik
import hashlib
from timeit import Timer, time
from nose.tools import *
from utilities import execution_path, run_all

def hashstr(var):
    return hashlib.md5(var).hexdigest()

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_tiff_round_trip_scanline():
    filepath = '/tmp/mapnik-tiff-io-scanline.tiff'
    im = mapnik.Image(255,267)
    im.fill(mapnik.Color('rgba(12,255,128,.5)'))
    org_str = hashstr(im.tostring())
    im.save(filepath,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    eq_(hashstr(im.tostring()), org_str)
    # This won't be the same the first time around because the im is not premultiplied and im2 is
    assert_not_equal(hashstr(im.tostring()),hashstr(im2.tostring()))
    assert_not_equal(hashstr(im.tostring('tiff:method=scanline')),hashstr(im2.tostring('tiff:method=scanline')))
    # Now premultiply
    im.premultiply()
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=scanline')),hashstr(im2.tostring('tiff:method=scanline')))
    eq_(hashstr(im2.tostring()),hashstr(im3.tostring()))
    eq_(hashstr(im2.tostring('tiff:method=scanline')),hashstr(im3.tostring('tiff:method=scanline')))

def test_tiff_round_trip_stripped():
    filepath = '/tmp/mapnik-tiff-io-stripped.tiff'
    im = mapnik.Image(255,267)
    im.fill(mapnik.Color('rgba(12,255,128,.5)'))
    org_str = hashstr(im.tostring())
    im.save(filepath,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath)
    im2.save('/tmp/mapnik-tiff-io-stripped2.tiff','tiff:method=stripped')
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    # Because one will end up with UNASSOC alpha tag which internally the TIFF reader will premultiply, the first to string will not be the same due to the
    # difference in tags.
    assert_not_equal(hashstr(im.tostring()),hashstr(im2.tostring()))
    assert_not_equal(hashstr(im.tostring('tiff:method=stripped')),hashstr(im2.tostring('tiff:method=stripped')))
    # Now if we premultiply they will be exactly the same
    im.premultiply()
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=stripped')),hashstr(im2.tostring('tiff:method=stripped')))
    eq_(hashstr(im2.tostring()),hashstr(im3.tostring()))
    # Both of these started out premultiplied, so this round trip should be exactly the same!
    eq_(hashstr(im2.tostring('tiff:method=stripped')),hashstr(im3.tostring('tiff:method=stripped')))

def test_tiff_round_trip_rows_stripped():
    filepath = '/tmp/mapnik-tiff-io-rows_stripped.tiff'
    filepath2 = '/tmp/mapnik-tiff-io-rows_stripped2.tiff'
    im = mapnik.Image(255,267)
    im.fill(mapnik.Color('rgba(12,255,128,.5)'))
    c = im.get_pixel_color(0,0)
    eq_(c.r, 12)
    eq_(c.g, 255)
    eq_(c.b, 128)
    eq_(c.a, 128)
    eq_(c.get_premultiplied(), False)
    im.save(filepath,'tiff:method=stripped:rows_per_strip=8')
    im2 = mapnik.Image.open(filepath)
    c2 = im2.get_pixel_color(0,0)
    eq_(c2.r, 6)
    eq_(c2.g, 128)
    eq_(c2.b, 64)
    eq_(c2.a, 128)
    eq_(c2.get_premultiplied(), True)
    im2.save(filepath2,'tiff:method=stripped:rows_per_strip=8')
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    # Because one will end up with UNASSOC alpha tag which internally the TIFF reader will premultiply, the first to string will not be the same due to the
    # difference in tags. 
    assert_not_equal(hashstr(im.tostring()),hashstr(im2.tostring()))
    assert_not_equal(hashstr(im.tostring('tiff:method=stripped:rows_per_strip=8')),hashstr(im2.tostring('tiff:method=stripped:rows_per_strip=8')))
    # Now premultiply the first image and they will be the same!
    im.premultiply()
    eq_(hashstr(im.tostring('tiff:method=stripped:rows_per_strip=8')),hashstr(im2.tostring('tiff:method=stripped:rows_per_strip=8')))
    eq_(hashstr(im2.tostring()),hashstr(im3.tostring()))
    # Both of these started out premultiplied, so this round trip should be exactly the same!
    eq_(hashstr(im2.tostring('tiff:method=stripped:rows_per_strip=8')),hashstr(im3.tostring('tiff:method=stripped:rows_per_strip=8')))

def test_tiff_round_trip_buffered_tiled():
    filepath = '/tmp/mapnik-tiff-io-buffered-tiled.tiff'
    filepath2 = '/tmp/mapnik-tiff-io-buffered-tiled2.tiff'
    filepath3 = '/tmp/mapnik-tiff-io-buffered-tiled3.tiff'
    im = mapnik.Image(255,267)
    im.fill(mapnik.Color('rgba(33,255,128,.5)'))
    c = im.get_pixel_color(0,0)
    eq_(c.r, 33)
    eq_(c.g, 255)
    eq_(c.b, 128)
    eq_(c.a, 128)
    eq_(c.get_premultiplied(), False)
    im.save(filepath,'tiff:method=tiled:tile_width=32:tile_height=32')
    im2 = mapnik.Image.open(filepath)
    c2 = im2.get_pixel_color(0,0)
    eq_(c2.r, 17)
    eq_(c2.g, 128)
    eq_(c2.b, 64)
    eq_(c2.a, 128)
    eq_(c2.get_premultiplied(), True)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    im2.save(filepath2, 'tiff:method=tiled:tile_width=32:tile_height=32')
    im3.save(filepath3, 'tiff:method=tiled:tile_width=32:tile_height=32')
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    # Because one will end up with UNASSOC alpha tag which internally the TIFF reader will premultiply, the first to string will not be the same due to the
    # difference in tags.
    assert_not_equal(hashstr(im.tostring()),hashstr(im2.tostring()))
    assert_not_equal(hashstr(im.tostring('tiff:method=tiled:tile_width=32:tile_height=32')),hashstr(im2.tostring('tiff:method=tiled:tile_width=32:tile_height=32')))
    # Now premultiply the first image and they should be the same
    im.premultiply()
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=tiled:tile_width=32:tile_height=32')),hashstr(im2.tostring('tiff:method=tiled:tile_width=32:tile_height=32')))
    eq_(hashstr(im2.tostring()),hashstr(im3.tostring()))
    # Both of these started out premultiplied, so this round trip should be exactly the same!
    eq_(hashstr(im2.tostring('tiff:method=tiled:tile_width=32:tile_height=32')),hashstr(im3.tostring('tiff:method=tiled:tile_width=32:tile_height=32')))

def test_tiff_round_trip_tiled():
    filepath = '/tmp/mapnik-tiff-io-tiled.tiff'
    im = mapnik.Image(256,256)
    im.fill(mapnik.Color('rgba(1,255,128,.5)'))
    im.save(filepath,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath)
    im3 = mapnik.Image.fromstring(open(filepath,'r').read())
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(im.width(),im3.width())
    eq_(im.height(),im3.height())
    # Because one will end up with UNASSOC alpha tag which internally the TIFF reader will premultiply, the first to string will not be the same due to the
    # difference in tags.
    assert_not_equal(hashstr(im.tostring()),hashstr(im2.tostring()))
    assert_not_equal(hashstr(im.tostring('tiff:method=tiled')),hashstr(im2.tostring('tiff:method=tiled')))
    # Now premultiply the first image and they will be exactly the same.
    im.premultiply()
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=tiled')),hashstr(im2.tostring('tiff:method=tiled')))
    eq_(hashstr(im2.tostring()),hashstr(im3.tostring()))
    # Both of these started out premultiplied, so this round trip should be exactly the same!
    eq_(hashstr(im2.tostring('tiff:method=tiled')),hashstr(im3.tostring('tiff:method=tiled')))


def test_tiff_rgb8_compare():
    filepath1 = '../data/tiff/ndvi_256x256_rgb8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgb8.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff')),hashstr(im2.tostring('tiff')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.rgba8).tostring("tiff")),True)

def test_tiff_rgba8_compare_scanline():
    filepath1 = '../data/tiff/ndvi_256x256_rgba8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgba8-scanline.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=scanline')),hashstr(im2.tostring('tiff:method=scanline')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.rgba8).tostring("tiff")),True)

def test_tiff_rgba8_compare_stripped():
    filepath1 = '../data/tiff/ndvi_256x256_rgba8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgba8-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=stripped')),hashstr(im2.tostring('tiff:method=stripped')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.rgba8).tostring("tiff")),True)

def test_tiff_rgba8_compare_tiled():
    filepath1 = '../data/tiff/ndvi_256x256_rgba8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-rgba8-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=tiled')),hashstr(im2.tostring('tiff:method=tiled')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.rgba8).tostring("tiff")),True)

def test_tiff_gray8_compare_scanline():
    filepath1 = '../data/tiff/ndvi_256x256_gray8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray8-scanline.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=scanline')),hashstr(im2.tostring('tiff:method=scanline')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray8).tostring("tiff")),True)

def test_tiff_gray8_compare_stripped():
    filepath1 = '../data/tiff/ndvi_256x256_gray8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray8-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=stripped')),hashstr(im2.tostring('tiff:method=stripped')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray8).tostring("tiff")),True)

def test_tiff_gray8_compare_tiled():
    filepath1 = '../data/tiff/ndvi_256x256_gray8_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray8-tiled.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=tiled')),hashstr(im2.tostring('tiff:method=tiled')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray8).tostring("tiff")),True)

def test_tiff_gray16_compare_scanline():
    filepath1 = '../data/tiff/ndvi_256x256_gray16_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray16-scanline.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=scanline')),hashstr(im2.tostring('tiff:method=scanline')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray16).tostring("tiff")),True)

def test_tiff_gray16_compare_stripped():
    filepath1 = '../data/tiff/ndvi_256x256_gray16_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray16-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=stripped')),hashstr(im2.tostring('tiff:method=stripped')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray16).tostring("tiff")),True)

def test_tiff_gray16_compare_tiled():
    filepath1 = '../data/tiff/ndvi_256x256_gray16_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray16-tiled.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=tiled')),hashstr(im2.tostring('tiff:method=tiled')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray16).tostring("tiff")),True)

def test_tiff_gray32f_compare_scanline():
    filepath1 = '../data/tiff/ndvi_256x256_gray32f_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray32f-scanline.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=scanline')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=scanline')),hashstr(im2.tostring('tiff:method=scanline')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray32f).tostring("tiff")),True)

def test_tiff_gray32f_compare_stripped():
    filepath1 = '../data/tiff/ndvi_256x256_gray32f_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray32f-stripped.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=stripped')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=stripped')),hashstr(im2.tostring('tiff:method=stripped')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray32f).tostring("tiff")),True)

def test_tiff_gray32f_compare_tiled():
    filepath1 = '../data/tiff/ndvi_256x256_gray32f_striped.tif'
    filepath2 = '/tmp/mapnik-tiff-gray32f-tiled.tiff'
    im = mapnik.Image.open(filepath1)
    im.save(filepath2,'tiff:method=tiled')
    im2 = mapnik.Image.open(filepath2)
    eq_(im.width(),im2.width())
    eq_(im.height(),im2.height())
    eq_(hashstr(im.tostring()),hashstr(im2.tostring()))
    eq_(hashstr(im.tostring('tiff:method=tiled')),hashstr(im2.tostring('tiff:method=tiled')))
    # should not be a blank image
    eq_(hashstr(im.tostring("tiff")) != hashstr(mapnik.Image(im.width(),im.height(),mapnik.ImageType.gray32f).tostring("tiff")),True)

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
