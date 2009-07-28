#!/usr/bin/env python

import os
import mapnik
from nose.tools import *
from utilities import execution_path,Todo

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def _pycairo_surface(type,sym):
    if mapnik.has_pycairo():
        import cairo
        test_cairo_file = 'test.%s' % type
        m = mapnik.Map(256,256)
        mapnik.load_map(m,'../data/good_maps/%s_symbolizer.xml' % sym)
        surface = getattr(cairo,'%sSurface' % type.upper())(test_cairo_file, m.width,m.height)
        mapnik.render(m, surface)
        surface.finish()        
        
        if os.path.exists(test_cairo_file):
            os.remove(test_cairo_file)
            return True
        else:
            # Fail, the file wasn't written
            return False

def test_pycairo_svg_surface():
    return _pycairo_surface('svg','point')

def test_pycairo_svg_surface():
    return _pycairo_surface('svg','building')
    
def test_pycairo_svg_surface():
    return _pycairo_surface('svg','polygon')

def test_pycairo_svg_surface():
    return _pycairo_surface('pdf','point')

def test_pycairo_svg_surface():
    return _pycairo_surface('pdf','building')
    
def test_pycairo_svg_surface():
    return _pycairo_surface('pdf','polygon')

def test_pycairo_svg_surface():
    return _pycairo_surface('ps','point')

def test_pycairo_svg_surface():
    return _pycairo_surface('ps','building')
    
def test_pycairo_svg_surface():
    return _pycairo_surface('ps','polygon')
