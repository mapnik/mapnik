#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))
    
def test_field_listing():
    lyr = mapnik2.Layer('test')
    lyr.datasource = mapnik2.Shapefile(file='../data/shp/poly.shp')
    fields = lyr.datasource.fields()
    eq_(fields, ['AREA', 'EAS_ID', 'PRFEDEA'])

def test_total_feature_count_shp():
    lyr = mapnik2.Layer('test')
    lyr.datasource = mapnik2.Shapefile(file='../data/shp/poly.shp')
    features = lyr.datasource.all_features()
    num_feats = len(features)
    eq_(num_feats, 10)

def test_total_feature_count_json():
    lyr = mapnik2.Layer('test')
    lyr.datasource = mapnik2.Ogr(file='../data/json/points.json',layer_by_index=0)
    features = lyr.datasource.all_features()
    num_feats = len(features)
    eq_(num_feats, 5)

def test_reading_json_from_string():
    json = open('../data/json/points.json','r').read()
    lyr = mapnik2.Layer('test')
    lyr.datasource = mapnik2.Ogr(file=json,layer_by_index=0)
    features = lyr.datasource.all_features()
    num_feats = len(features)
    eq_(num_feats, 5)
    
def test_feature_envelope():
    lyr = mapnik2.Layer('test')
    lyr.datasource = mapnik2.Shapefile(file='../data/shp/poly.shp')
    features = lyr.datasource.all_features()
    for feat in features:
        env = feat.envelope()
        contains = lyr.envelope().contains(env)
        eq_(contains, True)
        intersects = lyr.envelope().contains(env)
        eq_(intersects, True)

def test_feature_attributes():
    lyr = mapnik2.Layer('test')
    lyr.datasource = mapnik2.Shapefile(file='../data/shp/poly.shp')
    features = lyr.datasource.all_features()
    feat = features[0]
    attrs = {'PRFEDEA': u'35043411', 'EAS_ID': 168, 'AREA': 215229.266}
    eq_(feat.attributes, attrs)
    eq_(lyr.datasource.fields(),['AREA', 'EAS_ID', 'PRFEDEA'])
    eq_(lyr.datasource.field_types(),['float','int','str'])

def test_hit_grid():
    import os
    from itertools import groupby

    def rle_encode(l):
        """ encode a list of strings with run-length compression """
        return ["%d:%s" % (len(list(group)), name) for name, group in groupby(l)]

    m = mapnik2.Map(256,256);
    mapnik2.load_map(m,'../data/good_maps/agg_poly_gamma_map.xml');
    m.zoom_all()
    join_field = 'NAME'
    fg = [] # feature grid
    for y in range(0, 256, 4):
        for x in range(0, 256, 4):
            featureset = m.query_map_point(0,x,y)
            added = False
            for feature in featureset.features:
                fg.append(feature[join_field])
                added = True
            if not added:
                fg.append('')
    hit_list = '|'.join(rle_encode(fg))
    eq_(hit_list[:16],'730:|2:Greenland')
    eq_(hit_list[-12:],'1:Chile|812:')

if __name__ == '__main__':
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
