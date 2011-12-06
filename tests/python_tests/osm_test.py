#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'osm' in mapnik.DatasourceCache.instance().plugin_names():
    
    # Shapefile initialization
    def test_osm_init():
        ds = mapnik.Osm(file='../data/osm/nodes.osm')
    
        e = ds.envelope()
       
        # these are hardcoded in the plugin… ugh
        assert_almost_equal(e.minx, -180.0)
        assert_almost_equal(e.miny, -90.0)
        assert_almost_equal(e.maxx, 180.0)
        assert_almost_equal(e.maxy, 90)
    
    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.Osm(file='../data/osm/nodes.osm')
        # ugh, more odd stuff hardcoded...
        eq_(len(ds.fields()),5)
        eq_(ds.fields(),['bicycle', 'foot', 'horse', 'name', 'width'])
        eq_(ds.field_types(),['str', 'str', 'str', 'str', 'str'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        fs = ds.features(query)


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
