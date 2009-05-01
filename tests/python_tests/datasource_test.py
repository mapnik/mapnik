#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))
    
def test_field_listing():
    lyr = mapnik.Layer('test')
    lyr.datasource = mapnik.Shapefile(file='../data/shp/poly.shp')
    fields = lyr.datasource.fields()
    eq_(fields, ['AREA', 'EAS_ID', 'PRFEDEA'])