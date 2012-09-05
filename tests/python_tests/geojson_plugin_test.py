#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'geojson' in mapnik.DatasourceCache.plugin_names():

    def test_geojson_init():
        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.json')
        e = ds.envelope()
        assert_almost_equal(e.minx, -81.705583, places=7)
        assert_almost_equal(e.miny, 41.480573, places=6)
        assert_almost_equal(e.maxx, -81.705583, places=5)
        assert_almost_equal(e.maxy, 41.480573, places=3)

    def test_geojson_properties():
        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.json')
        f = ds.features_at_point(s.envelope().center()).features[0]

        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

        eq_(f['name'], u'test')
        eq_(f['description'], u'Test: \u005C')
        eq_(f['int'], 1)
        eq_(f['double'], u'Quebec')
        eq_(f['boolean'], True)
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')

    def test_geojson_properties():
        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.json')
        f = ds.all_features()[0]

        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

        eq_(f['name'], u'Test')
        eq_(f['int'], 1)
        eq_(f['double'], 1.1)
        eq_(f['boolean'], True)
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')
        eq_(f['spaces'], u'this has spaces')
        eq_(f['description'], u'Test: \u005C')

#    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.Datasource(type='geojson',file='../data/json/escaped.json')
        eq_(len(ds.fields()),7)
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

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
