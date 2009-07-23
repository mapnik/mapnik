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

def test_total_feature_count():
    lyr = mapnik.Layer('test')
    lyr.datasource = mapnik.Shapefile(file='../data/shp/poly.shp')
    features = lyr.datasource.all_features()
    num_feats = len(features)
    eq_(num_feats, 10)

def test_feature_envelope():
    lyr = mapnik.Layer('test')
    lyr.datasource = mapnik.Shapefile(file='../data/shp/poly.shp')
    features = lyr.datasource.all_features()
    for feat in features:
        env = feat.envelope()
        contains = lyr.envelope().contains(env)
        eq_(contains, True)
        intersects = lyr.envelope().contains(env)
        eq_(intersects, True)

def test_feature_attributes():
    lyr = mapnik.Layer('test')
    lyr.datasource = mapnik.Shapefile(file='../data/shp/poly.shp')
    features = lyr.datasource.all_features()
    feat = features[0]
    attrs = {'PRFEDEA': u'35043411', 'EAS_ID': 168, 'AREA': 215229.266}
    eq_(feat.attributes, attrs)
    eq_(lyr.datasource.fields(),['AREA', 'EAS_ID', 'PRFEDEA'])
    eq_(lyr.datasource.field_types(),[float,int,str])
    