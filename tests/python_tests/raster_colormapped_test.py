#coding=utf8
import os
import mapnik
from utilities import execution_path
from nose.tools import *

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'gdal' in mapnik.DatasourceCache.plugin_names():

    def test_vrt_rendering():
        m = mapnik.Map(512,512)
        mapnik.load_map(m,'../data/good_maps/vrt_colortable.xml')
        m.zoom_all()
        im = mapnik.Image(512,512)
        mapnik.render(m,im)
        actual = '/tmp/vrt_colortable.png'
        expected = 'images/support/vrt_colortable.png'
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

    def test_tif_rendering_nodata():
        m = mapnik.Map(512,512)
        mapnik.load_map(m,'../data/good_maps/tiff_colortable.xml')
        m.zoom_all()
        im = mapnik.Image(512,512)
        mapnik.render(m,im)
        actual = '/tmp/tif_colortable.png'
        expected = 'images/support/tif_colortable.png'
        im.save(actual)
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
