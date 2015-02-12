#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import eq_,assert_almost_equal
from utilities import execution_path, run_all
import os, mapnik
try:
    import json
except ImportError:
    import simplejson as json

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'geojson' in mapnik.DatasourceCache.plugin_names():

    def test_geojson_init():
        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.geojson')
        e = ds.envelope()
        assert_almost_equal(e.minx, -81.705583, places=7)
        assert_almost_equal(e.miny, 41.480573, places=6)
        assert_almost_equal(e.maxx, -81.705583, places=5)
        assert_almost_equal(e.maxy, 41.480573, places=3)

    def test_geojson_properties():
        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.geojson')
        f = ds.features_at_point(ds.envelope().center()).features[0]
        eq_(len(ds.fields()),9)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

        eq_(f['name'], u'Test')
        eq_(f['int'], 1)
        eq_(f['description'], u'Test: \u005C')
        eq_(f['spaces'], u'this has spaces')
        eq_(f['double'], 1.1)
        eq_(f['boolean'], True)
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')
        eq_(f['array'], u'[[[1],["deux"]],[["\\u0442\\u0440\\u0438","four","\\u4e94"]]]')
        array = json.loads(f['array'])
        eq_(array,[[[1], [u'deux']], [[u'\u0442\u0440\u0438', u'four', u'\u4e94']]])
        eq_(f['object'], u'{"value":{"type":"\\u041c\\u0430pni\\u043a","array":[3,0,"x"]}}')
        object = json.loads(f['object'])
        eq_(object,{u'value': {u'array': [3, 0, u'x'], u'type': u'\u041c\u0430pni\u043a'}})

        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.geojson')
        f = ds.all_features()[0]
        eq_(len(ds.fields()),9)

        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

        eq_(f['name'], u'Test')
        eq_(f['int'], 1)
        eq_(f['description'], u'Test: \u005C')
        eq_(f['spaces'], u'this has spaces')
        eq_(f['double'], 1.1)
        eq_(f['boolean'], True)
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')
        eq_(f['array'], u'[[[1],["deux"]],[["\\u0442\\u0440\\u0438","four","\\u4e94"]]]')
        array = json.loads(f['array'])
        eq_(array,[[[1], [u'deux']], [[u'\u0442\u0440\u0438', u'four', u'\u4e94']]])
        eq_(f['object'], u'{"value":{"type":"\\u041c\\u0430pni\\u043a","array":[3,0,"x"]}}')
        object = json.loads(f['object'])
        eq_(object,{u'value': {u'array': [3, 0, u'x'], u'type': u'\u041c\u0430pni\u043a'}})

    def test_large_geojson_properties():
        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.geojson',cache_features = False)
        f = ds.features_at_point(ds.envelope().center()).features[0]
        eq_(len(ds.fields()),9)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

        eq_(f['name'], u'Test')
        eq_(f['int'], 1)
        eq_(f['description'], u'Test: \u005C')
        eq_(f['spaces'], u'this has spaces')
        eq_(f['double'], 1.1)
        eq_(f['boolean'], True)
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')
        eq_(f['array'], u'[[[1],["deux"]],[["\\u0442\\u0440\\u0438","four","\\u4e94"]]]')
        array = json.loads(f['array'])
        eq_(array,[[[1], [u'deux']], [[u'\u0442\u0440\u0438', u'four', u'\u4e94']]])
        eq_(f['object'], u'{"value":{"type":"\\u041c\\u0430pni\\u043a","array":[3,0,"x"]}}')
        object = json.loads(f['object'])
        eq_(object,{u'value': {u'array': [3, 0, u'x'], u'type': u'\u041c\u0430pni\u043a'}})

        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.geojson')
        f = ds.all_features()[0]
        eq_(len(ds.fields()),9)

        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

        eq_(f['name'], u'Test')
        eq_(f['int'], 1)
        eq_(f['description'], u'Test: \u005C')
        eq_(f['spaces'], u'this has spaces')
        eq_(f['double'], 1.1)
        eq_(f['boolean'], True)
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')
        eq_(f['array'], u'[[[1],["deux"]],[["\\u0442\\u0440\\u0438","four","\\u4e94"]]]')
        array = json.loads(f['array'])
        eq_(array,[[[1], [u'deux']], [[u'\u0442\u0440\u0438', u'four', u'\u4e94']]])
        eq_(f['object'], u'{"value":{"type":"\\u041c\\u0430pni\\u043a","array":[3,0,"x"]}}')
        object = json.loads(f['object'])
        eq_(object,{u'value': {u'array': [3, 0, u'x'], u'type': u'\u041c\u0430pni\u043a'}})

    def test_geojson_from_in_memory_string():
        # will silently fail since it is a geometry and needs to be a featurecollection.
        #ds = mapnik.Datasource(type='geojson',inline='{"type":"LineString","coordinates":[[0,0],[10,10]]}')
        # works since it is a featurecollection
        ds = mapnik.Datasource(type='geojson',inline='{ "type":"FeatureCollection", "features": [ { "type":"Feature", "properties":{"name":"test"}, "geometry": { "type":"LineString","coordinates":[[0,0],[10,10]] } } ]}')
        eq_(len(ds.fields()),1)
        f = ds.all_features()[0]
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.LineString)
        eq_(f['name'], u'test')

#    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.geojson')
        eq_(len(ds.fields()),9)
        # TODO - this sorting is messed up
        #eq_(ds.fields(),['name', 'int', 'double', 'description', 'boolean', 'NOM_FR'])
        #eq_(ds.field_types(),['str', 'int', 'float', 'str', 'bool', 'str'])
# TODO - should geojson plugin throw like others?
#        query = mapnik.Query(ds.envelope())
#        for fld in ds.fields():
#            query.add_property_name(fld)
#        # also add an invalid one, triggering throw
#        query.add_property_name('bogus')
#        fs = ds.features(query)

    def test_parsing_feature_collection_with_top_level_properties():
        ds = mapnik.Datasource(type='geojson',file='../data/json/feature_collection_level_properties.json')
        f = ds.all_features()[0]

        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(f['feat_name'], u'feat_value')

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
