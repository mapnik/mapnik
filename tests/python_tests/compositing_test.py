#encoding: utf8

from nose.tools import *
import os,sys
from utilities import execution_path
from utilities import Todo
import mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_compare_images():
    b = mapnik.Image.open('./images/support/b.png')
    for name in mapnik.CompositeOp.names:
        a = mapnik.Image.open('./images/support/a.png')
        a.composite(b,getattr(mapnik.CompositeOp,name))
        actual = '/tmp/mapnik-comp-op-test-' + name + '.png'
        expected = 'images/composited/' + name + '.png'
        a.save(actual)
        expected_im = mapnik.Image.open(expected)
        # compare them
        eq_(a.tostring(),expected_im.tostring(), 'failed comparing actual (%s) and expected(%s)' % (actual,'tests/python_tests/'+ expected))

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
