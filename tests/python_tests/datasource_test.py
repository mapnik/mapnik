#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, run_all
import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_that_datasources_exist():
    if len(mapnik.DatasourceCache.plugin_names()) == 0:
        print '***NOTICE*** - no datasource plugins have been loaded'

def test_field_listing():
    if 'shape' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.Shapefile(file='../data/shp/poly.shp')
        fields = ds.fields()
        eq_(fields, ['AREA', 'EAS_ID', 'PRFEDEA'])
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Polygon)
        eq_(desc['name'],'shape')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

def test_total_feature_count_shp():
    if 'shape' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.Shapefile(file='../data/shp/poly.shp')
        features = ds.all_features()
        num_feats = len(features)
        eq_(num_feats, 10)

def test_total_feature_count_json():
    if 'ogr' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.Ogr(file='../data/json/points.geojson',layer_by_index=0)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'ogr')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')
        features = ds.all_features()
        num_feats = len(features)
        eq_(num_feats, 5)

def test_sqlite_reading():
    if 'sqlite' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.SQLite(file='../data/sqlite/world.sqlite',table_by_index=0)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Polygon)
        eq_(desc['name'],'sqlite')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')
        features = ds.all_features()
        num_feats = len(features)
        eq_(num_feats, 245)

def test_reading_json_from_string():
    json = open('../data/json/points.geojson','r').read()
    if 'ogr' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.Ogr(file=json,layer_by_index=0)
        features = ds.all_features()
        num_feats = len(features)
        eq_(num_feats, 5)

def test_feature_envelope():
    if 'shape' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.Shapefile(file='../data/shp/poly.shp')
        features = ds.all_features()
        for feat in features:
            env = feat.envelope()
            contains = ds.envelope().contains(env)
            eq_(contains, True)
            intersects = ds.envelope().contains(env)
            eq_(intersects, True)

def test_feature_attributes():
    if 'shape' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.Shapefile(file='../data/shp/poly.shp')
        features = ds.all_features()
        feat = features[0]
        attrs = {'PRFEDEA': u'35043411', 'EAS_ID': 168, 'AREA': 215229.266}
        eq_(feat.attributes, attrs)
        eq_(ds.fields(),['AREA', 'EAS_ID', 'PRFEDEA'])
        eq_(ds.field_types(),['float','int','str'])

def test_ogr_layer_by_sql():
    if 'ogr' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.Ogr(file='../data/shp/poly.shp', layer_by_sql='SELECT * FROM poly WHERE EAS_ID = 168')
        features = ds.all_features()
        num_feats = len(features)
        eq_(num_feats, 1)

def test_hit_grid():
    import os
    from itertools import groupby

    def rle_encode(l):
        """ encode a list of strings with run-length compression """
        return ["%d:%s" % (len(list(group)), name) for name, group in groupby(l)]

    m = mapnik.Map(256,256);
    try:
        mapnik.load_map(m,'../data/good_maps/agg_poly_gamma_map.xml');
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
    except RuntimeError, e:
        # only test datasources that we have installed
        if not 'Could not create datasource' in str(e):
            raise RuntimeError(str(e))


if __name__ == '__main__':
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
