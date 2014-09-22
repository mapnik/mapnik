#coding=utf8
import os
import mapnik
from utilities import execution_path, run_all
from nose.tools import *

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'sqlite' in mapnik.DatasourceCache.plugin_names():

    # the negative buffer on the layer should
    # override the postive map buffer leading
    # only one point to be rendered in the map
    def test_layer_buffer_size_1():
        m = mapnik.Map(512,512)
        eq_(m.buffer_size,0)
        mapnik.load_map(m,'../data/good_maps/layer_buffer_size_reduction.xml')
        eq_(m.buffer_size,256)
        eq_(m.layers[0].buffer_size,-150)
        m.zoom_all()
        im = mapnik.Image(m.width,m.height)
        mapnik.render(m,im)
        actual = '/tmp/mapnik-layer-buffer-size.png'
        expected = 'images/support/mapnik-layer-buffer-size.png'
        im.save(actual,"png32")
        expected_im = mapnik.Image.open(expected)
        eq_(im.tostring('png32'),expected_im.tostring('png32'), 'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))


if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
