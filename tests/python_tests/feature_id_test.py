#!/usr/bin/env python

from nose.tools import *

from utilities import execution_path

import os, sys, glob, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test_shapefile_feature_id():

    ds = mapnik2.Shapefile(file='../data/shp/polylines.shp')
    fs = ds.featureset()
    eq_(1, fs.next().id())
    eq_(2, fs.next().id())
    count = 0
    while True:
        feature = fs.next()
        if not feature:
            break
        count = feature.id()
    eq_(count,11)

    ds = mapnik2.Ogr(file='../data/shp/polylines.shp',layer_by_index=0)
    fs = ds.featureset()
    eq_(1, fs.next().id())
    eq_(2, fs.next().id())
    count = 0
    while True:
        feature = fs.next()
        if not feature:
            break
        count = feature.id()
    eq_(count,11)



if __name__ == "__main__":
    setup()
    test_shapefile_feature_id()