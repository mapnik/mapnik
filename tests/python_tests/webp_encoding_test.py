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

if mapnik.has_webp():
    tmp_dir = '/tmp/mapnik-webp/'
    if not os.path.exists(tmp_dir):
       os.makedirs(tmp_dir)

    opts = [
        'webp',
        'webp:method=0',
        'webp:method=6',
        'webp:quality=64',
        'webp:alpha=false',
        'webp:partitions=3',
        'webp:preprocessing=1',
        'webp:partition_limit=50',
        'webp:pass=10',
        'webp:alpha_quality=50',
        'webp:alpha_filtering=2',
        'webp:alpha_compression=0',
        'webp:autofilter=0',
        'webp:filter_type=1:autofilter=1',
        'webp:filter_sharpness=4',
        'webp:filter_strength=50',
        'webp:sns_strength=50',
        'webp:segments=3',
        'webp:target_PSNR=.5',
        'webp:target_size=100'
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
        fails = []
        try:
            for opt in opts:
                im = mapnik.Image(256,256)
                expected = gen_filepath('blank',opt)
                actual = os.path.join(tmp_dir,os.path.basename(expected))
                if generate or not os.path.exists(expected):
                    print 'generating expected image %s' % expected
                    im.save(expected,opt)
                im.save(actual,opt)
                try:
                    expected_bytes = mapnik.Image.open(expected).tostring()
                except RuntimeError:
                    # this will happen if libweb is old, since it cannot open images created by more recent webp
                    print 'warning, cannot open webp expected image (your libwebp is likely too old)'
                    continue
                if mapnik.Image.open(actual).tostring() != expected_bytes:
                    fails.append('%s (actual) not == to %s (expected)' % (actual,expected))

            for opt in opts:
                im = mapnik.Image(256,256)
                im.background = mapnik.Color('green')
                expected = gen_filepath('solid',opt)
                actual = os.path.join(tmp_dir,os.path.basename(expected))
                if generate or not os.path.exists(expected):
                    print 'generating expected image %s' % expected
                    im.save(expected,opt)
                im.save(actual,opt)
                try:
                    expected_bytes = mapnik.Image.open(expected).tostring()
                except RuntimeError:
                    # this will happen if libweb is old, since it cannot open images created by more recent webp
                    print 'warning, cannot open webp expected image (your libwebp is likely too old)'
                    continue
                if mapnik.Image.open(actual).tostring() != expected_bytes:
                    fails.append('%s (actual) not == to %s (expected)' % (actual,expected))

            for opt in opts:
                im = mapnik.Image.open('images/support/transparency/aerial_rgba.png')
                expected = gen_filepath('aerial_rgba',opt)
                actual = os.path.join(tmp_dir,os.path.basename(expected))
                if generate or not os.path.exists(expected):
                    print 'generating expected image %s' % expected
                    im.save(expected,opt)
                im.save(actual,opt)
                try:
                    expected_bytes = mapnik.Image.open(expected).tostring()
                except RuntimeError:
                    # this will happen if libweb is old, since it cannot open images created by more recent webp
                    print 'warning, cannot open webp expected image (your libwebp is likely too old)'
                    continue
                if mapnik.Image.open(actual).tostring() != expected_bytes:
                    fails.append('%s (actual) not == to %s (expected)' % (actual,expected))
            # disabled to avoid failures on ubuntu when using old webp packages
            #eq_(fails,[],'\n'+'\n'.join(fails))
        except RuntimeError, e:
            print e

    def test_transparency_levels():
        try:
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

            t0 = tmp_dir + 'white0-actual.webp'

            # octree
            format = 'webp'
            expected = 'images/support/transparency/white0.webp'
            if generate or not os.path.exists(expected):
                im.save('images/support/transparency/white0.webp')
            im.save(t0,format)
            im_in = mapnik.Image.open(t0)
            t0_len = len(im_in.tostring(format))
            try:
                expected_bytes = mapnik.Image.open(expected).tostring(format)
            except RuntimeError:
                # this will happen if libweb is old, since it cannot open images created by more recent webp
                print 'warning, cannot open webp expected image (your libwebp is likely too old)'
                return
            # disabled to avoid failures on ubuntu when using old webp packages
            #eq_(t0_len,len(expected_bytes))
        except RuntimeError, e:
            print e


if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
