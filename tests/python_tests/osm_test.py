#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import eq_
from utilities import execution_path, run_all
import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'osm' in mapnik.DatasourceCache.plugin_names():

    # osm initialization
    def test_osm_init():
        ds = mapnik.Osm(file='../data/osm/nodes.osm')

        e = ds.envelope()

        # these are hardcoded in the plugin… ugh
        eq_(e.minx >= -180.0,True)
        eq_(e.miny >= -90.0,True)
        eq_(e.maxx <= 180.0,True)
        eq_(e.maxy <= 90,True)

    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.Osm(file='../data/osm/nodes.osm')
        eq_(len(ds.fields()),0)
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        ds.features(query)

    def test_that_64bit_int_fields_work():
        ds = mapnik.Osm(file='../data/osm/64bit.osm')
        eq_(len(ds.fields()),4)
        eq_(ds.fields(),['bigint', 'highway', 'junction', 'note'])
        eq_(ds.field_types(),['str', 'str', 'str', 'str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat.to_geojson(),'{"type":"Feature","id":890,"geometry":{"type":"Point","coordinates":[-61.7960248,17.1415874]},"properties":{}}')
        eq_(feat.id(),4294968186)
        eq_(feat['bigint'], None)
        feat = fs.next()
        eq_(feat['bigint'],'9223372036854775807')

    def test_reading_ways():
        ds = mapnik.Osm(file='../data/osm/ways.osm')
        eq_(len(ds.fields()),0)
        eq_(ds.fields(),[])
        eq_(ds.field_types(),[])
        feat = ds.all_features()[4]
        eq_(feat.to_geojson(),'{"type":"Feature","id":1,"geometry":{"type":"LineString","coordinates":[[0,2],[0,-2]]},"properties":{}}')
        eq_(feat.id(),1)


if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
