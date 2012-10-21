#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path

import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'ogr' in mapnik.DatasourceCache.plugin_names():

    # Shapefile initialization
    def test_shapefile_init():
        s = mapnik.Ogr(file='../../demo/data/boundaries.shp',layer_by_index=0)

        e = s.envelope()

        assert_almost_equal(e.minx, -11121.6896651, places=7)
        assert_almost_equal(e.miny, -724724.216526, places=6)
        assert_almost_equal(e.maxx, 2463000.67866, places=5)
        assert_almost_equal(e.maxy, 1649661.267, places=3)

    # Shapefile properties
    def test_shapefile_properties():
        # NOTE: encoding is latin1 but gdal >= 1.9 should now expose utf8 encoded features
        # See SHAPE_ENCODING for overriding: http://gdal.org/ogr/drv_shapefile.html
        # So: failure for the NOM_FR field is expected for older gdal
        ds = mapnik.Ogr(file='../../demo/data/boundaries.shp',layer_by_index=0)
        f = ds.features_at_point(ds.envelope().center()).features[0]
        eq_(ds.geometry_type(),mapnik.DataGeometryType.Polygon)

        eq_(f['CGNS_FID'], u'6f733341ba2011d892e2080020a0f4c9')
        eq_(f['COUNTRY'], u'CAN')
        eq_(f['F_CODE'], u'FA001')
        eq_(f['NAME_EN'], u'Quebec')
        # this seems to break if icu data linking is not working
        eq_(f['NOM_FR'], u'Qu\xe9bec')
        eq_(f['NOM_FR'], u'Québec')
        eq_(f['Shape_Area'], 1512185733150.0)
        eq_(f['Shape_Leng'], 19218883.724300001)

    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.Ogr(file='../data/shp/world_merc.shp',layer_by_index=0)
        eq_(len(ds.fields()),11)
        eq_(ds.fields(),['FIPS', 'ISO2', 'ISO3', 'UN', 'NAME', 'AREA', 'POP2005', 'REGION', 'SUBREGION', 'LON', 'LAT'])
        eq_(ds.field_types(),['str', 'str', 'str', 'int', 'str', 'int', 'int', 'int', 'int', 'float', 'float'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        fs = ds.features(query)

    def test_handling_of_null_features():
        ds = mapnik.Ogr(file='../data/json/null_feature.json',layer_by_index=0)
        fs = ds.all_features()
        eq_(len(fs),1)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
