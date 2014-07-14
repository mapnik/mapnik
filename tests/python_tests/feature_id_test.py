#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path, run_all
import os, sys, glob, mapnik
import itertools

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def compare_shape_between_mapnik_and_ogr(shapefile,query=None):
    plugins = mapnik.DatasourceCache.plugin_names()
    if 'shape' in plugins and 'ogr' in plugins:
        ds1 = mapnik.Ogr(file=shapefile,layer_by_index=0)
        ds2 = mapnik.Shapefile(file=shapefile)
        if query:
            fs1 = ds1.features(query)
            fs2 = ds2.features(query)
        else:
            fs1 = ds1.featureset()
            fs2 = ds2.featureset()
        count = 0;
        for feat1,feat2 in itertools.izip(fs1,fs2):
            count += 1
            eq_(feat1.id(),feat2.id(),
                '%s : ogr feature id %s "%s" does not equal shapefile feature id %s "%s"'
                  % (count,feat1.id(),str(feat1.attributes), feat2.id(),str(feat2.attributes)))
    return True


def test_shapefile_line_featureset_id():
    compare_shape_between_mapnik_and_ogr('../data/shp/polylines.shp')

def test_shapefile_polygon_featureset_id():
    compare_shape_between_mapnik_and_ogr('../data/shp/poly.shp')

def test_shapefile_polygon_feature_query_id():
    bbox = (15523428.2632, 4110477.6323, -11218494.8310, 7495720.7404)
    query = mapnik.Query(mapnik.Box2d(*bbox))
    if 'ogr' in mapnik.DatasourceCache.plugin_names():
        ds = mapnik.Ogr(file='../data/shp/world_merc.shp',layer_by_index=0)
        for fld in ds.fields():
            query.add_property_name(fld)
        compare_shape_between_mapnik_and_ogr('../data/shp/world_merc.shp',query)

def test_feature_hit_count():
    pass
    #raise Todo("need to optimize multigeom bbox handling in shapeindex: https://github.com/mapnik/mapnik/issues/783")
    # results in different results between shp and ogr!
    #bbox = (-14284551.8434, 2074195.1992, -7474929.8687, 8140237.7628)
    #bbox = (1113194.91,4512803.085,2226389.82,6739192.905)
    #query = mapnik.Query(mapnik.Box2d(*bbox))
    #if 'ogr' in mapnik.DatasourceCache.plugin_names():
    #    ds1 = mapnik.Ogr(file='../data/shp/world_merc.shp',layer_by_index=0)
    #    for fld in ds1.fields():
    #        query.add_property_name(fld)
    #    ds2 = mapnik.Shapefile(file='../data/shp/world_merc.shp')
    #    count1 = len(ds1.features(query).features)
    #    count2 = len(ds2.features(query).features)
    #    eq_(count1,count2,"Feature count differs between OGR driver (%s features) and Shapefile Driver (%s features) when querying the same bbox" % (count1,count2))

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
