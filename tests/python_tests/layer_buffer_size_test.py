#coding=utf8
import os
import mapnik
import cairo
from utilities import execution_path
from nose.tools import *

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_layer_buffer_size_1():
	m = mapnik.Map(512,512)
	mapnik.load_map(m,'../data/good_maps/layer_buffer_size_reduction.xml')
	m.zoom_all()
	im = mapnik.Image(m.width,m.height)
	mapnik.render(m,im)
	actual = '/tmp/mapnik-layer-buffer-size.png'
	expected = 'images/support/mapnik-layer-buffer-size.png'
	im.save(actual)
	expected_im = mapnik.Image.open(expected)
	eq_(im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))

def test_layer_buffer_size_2():
	actual = '/tmp/mapnik-layer-buffer-size-cairo.png'
	expected = 'images/support/mapnik-layer-buffer-size-cairo.png'
	m = mapnik.Map(512,512)
	mapnik.load_map(m,'../data/good_maps/layer_buffer_size_reduction.xml')
	m.zoom_all()
	surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, m.width, m.height)
	mapnik.render(m, surface)
	surface.write_to_png(actual)
	surface.finish()
	expected_im = mapnik.Image.open(expected)
	actual_im = mapnik.Image.open(actual)
	eq_(actual_im.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected (%s)' % (actual,'tests/python_tests/'+ expected))


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
