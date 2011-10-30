#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'shape' in mapnik2.DatasourceCache.instance().plugin_names():
    
    # Shapefile initialization
    def test_shapefile_init():
        s = mapnik2.Shapefile(file='../../demo/data/boundaries')
    
        e = s.envelope()
       
        assert_almost_equal(e.minx, -11121.6896651, places=7)
        assert_almost_equal(e.miny, -724724.216526, places=6)
        assert_almost_equal(e.maxx, 2463000.67866, places=5)
        assert_almost_equal(e.maxy, 1649661.267, places=3)
    
    # Shapefile properties
    def test_shapefile_properties():
        s = mapnik2.Shapefile(file='../../demo/data/boundaries', encoding='latin1')
        f = s.features_at_point(s.envelope().center()).features[0]
    
        eq_(f['CGNS_FID'], u'6f733341ba2011d892e2080020a0f4c9')
        eq_(f['COUNTRY'], u'CAN')
        eq_(f['F_CODE'], u'FA001')
        eq_(f['NAME_EN'], u'Quebec')
        # this seems to break if icu data linking is not working
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')
        eq_(f['Shape_Area'], 1512185733150.0)
        eq_(f['Shape_Leng'], 19218883.724300001)
    
        # Check that the deprecated interface still works,
        # remove me once the deprecated code is cleaned up
        eq_(f.properties['Shape_Leng'], 19218883.724300001)


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
