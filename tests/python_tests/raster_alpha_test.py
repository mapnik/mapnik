#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'gdal' in mapnik.DatasourceCache.plugin_names():

    def test_map_alpha_compare():
        m = mapnik.Map(600,400)
        mapnik.load_map(m,'../data/good_maps/raster-alpha.xml')
        m.zoom_all()
        actual = '/tmp/mapnik-raster-alpha.png'
        expected = 'images/support/raster-alpha.png'
        im = mapnik.Image(m.width,m.height)
        mapnik.render(m,im)
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

    def test_map_alpha_gradient_compare():
        m = mapnik.Map(600,400)
        mapnik.load_map(m,'../data/good_maps/raster-alpha-gradient.xml')
        m.zoom_all()
        actual = '/tmp/mapnik-raster-alpha-gradient.png'
        expected = 'images/support/raster-alpha-gradient.png'
        im = mapnik.Image(m.width,m.height)
        mapnik.render(m,im)
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

    # there should be no gray edges on raster
    # https://github.com/mapnik/mapnik/issues/1471
    def test_edge_scaling_with_nodata():
        m = mapnik.Map(600,400)
        mapnik.load_map(m,'../data/good_maps/raster-nodata-edge.xml')
        m.zoom_all()
        actual = '/tmp/mapnik-raster-nodata-edge.png'
        expected = 'images/support/raster-nodata-edge.png'
        im = mapnik.Image(m.width,m.height)
        mapnik.render(m,im)
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
