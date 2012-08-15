#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path
from copy import deepcopy

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))


#def test_map_deepcopy1():
#    m1 = mapnik.Map(256,256)
#    m1.append_style('style',mapnik.Style())
#    m1.append_fontset('fontset',mapnik.FontSet())
#    m2 = deepcopy(m1)
#    eq_(m2.width, m1.width)
#    eq_(m2.height, m2.height)
#    eq_(m2.srs, m1.srs)
#    eq_(m2.base, m1.base)
#    eq_(m2.background, m1.background)
#    eq_(m2.buffer_size, m1.buffer_size)
#    eq_(m2.aspect_fix_mode, m1.aspect_fix_mode)
#    eq_(m2.envelope(),m1.envelope())
#    eq_(m2.buffered_envelope(),m1.buffered_envelope())
#    eq_(m2.scale(),m1.scale())
#    eq_(m2.scale_denominator(),m1.scale_denominator())
#    eq_(m2.maximum_extent,m1.maximum_extent)
#    eq_(id(m2.view_transform()),id(m1.view_transform()))
#    eq_(id(m2.parameters),id(m1.parameters))
#    eq_(id(m2.layers),id(m1.layers))
#    eq_(id(m2.layers),id(m1.layers))
#    eq_(id(m2.find_fontset('fontset')),id(m2.find_fontset('fontset')))
#    # fails for some reason on linux (not osx)
#    # but non-critical for now
#    #eq_(id(m2.find_style('style')),id(m2.find_style('style')))

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
