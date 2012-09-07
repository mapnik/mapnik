#coding=utf8
import os
import mapnik
from utilities import execution_path
from nose.tools import *

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'csv' in mapnik.DatasourceCache.plugin_names():
    def test_marker_ellipse_render1():
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/marker_ellipse_transform.xml')
        m.zoom_all()
        im = mapnik.Image(m.width,m.height)
        mapnik.render(m,im)
        actual = '/tmp/mapnik-marker-ellipse-render1.png'
        expected = 'images/support/mapnik-marker-ellipse-render1.png'
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))

    def test_marker_ellipse_render2():
        # currently crashes https://github.com/mapnik/mapnik/issues/1365
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/marker_ellipse_transform2.xml')
        m.zoom_all()
        im = mapnik.Image(m.width,m.height)
        mapnik.render(m,im)
        actual = '/tmp/mapnik-marker-ellipse-render2.png'
        expected = 'images/support/mapnik-marker-ellipse-render2.png'
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
