#!/usr/bin/env python
# -*- coding: utf-8 -*-

import glob
from nose.tools import *
from utilities import execution_path

import os, mapnik
# make the tests silent since we intentially test error conditions that are noisy
mapnik.logger.set_severity(mapnik.severity_type.None)

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

if 'csv' in mapnik.DatasourceCache.plugin_names():

    def get_csv_ds(filename):
        return mapnik.Datasource(type='csv',file=os.path.join('../data/csv/',filename),quiet=True)

    def test_broken_files(visual=False):
        broken = glob.glob("../data/csv/fails/*.*")
        broken.extend(glob.glob("../data/csv/warns/*.*"))

        # Add a filename that doesn't exist
        broken.append("../data/csv/fails/does_not_exist.csv")

        for csv in broken:
            throws = False
            if visual:
                try:
                    ds = mapnik.Datasource(type='csv',file=csv,strict=True,quiet=True)
                    print '\x1b[33mfailed\x1b[0m',csv
                except Exception:
                    print '\x1b[1;32m✓ \x1b[0m', csv

    def test_good_files(visual=False):
        good_files = glob.glob("../data/csv/*.*")
        good_files.extend(glob.glob("../data/csv/warns/*.*"))

        for csv in good_files:
            if visual:
                try:
                    ds = mapnik.Datasource(type='csv',file=csv,quiet=True)
                    print '\x1b[1;32m✓ \x1b[0m', csv
                except Exception:
                    print '\x1b[33mfailed\x1b[0m',csv

    def test_lon_lat_detection(**kwargs):
        ds = get_csv_ds('lon_lat.csv')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['lon','lat'])
        eq_(ds.field_types(),['int','int'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        fs = ds.features(query)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        feat = fs.next()
        attr = {'lon': 0, 'lat': 0}
        eq_(feat.attributes,attr)

    def test_lon_lat_detection(**kwargs):
        ds = get_csv_ds('lng_lat.csv')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['lng','lat'])
        eq_(ds.field_types(),['int','int'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        fs = ds.features(query)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        feat = fs.next()
        attr = {'lng': 0, 'lat': 0}
        eq_(feat.attributes,attr)

    def test_type_detection(**kwargs):
        ds = get_csv_ds('nypd.csv')
        eq_(ds.fields(),['Precinct','Phone','Address','City','geo_longitude','geo_latitude','geo_accuracy'])
        eq_(ds.field_types(),['str','str','str','str','float','float','str'])
        feat = ds.featureset().next()
        attr = {'City': u'New York, NY', 'geo_accuracy': u'house', 'Phone': u'(212) 334-0711', 'Address': u'19 Elizabeth Street', 'Precinct': u'5th Precinct', 'geo_longitude': -70, 'geo_latitude': 40}
        eq_(feat.attributes,attr)
        eq_(len(ds.all_features()),2)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_skipping_blank_rows(**kwargs):
        ds = get_csv_ds('blank_rows.csv')
        eq_(ds.fields(),['x','y','name'])
        eq_(ds.field_types(),['int','int','str'])
        eq_(len(ds.all_features()),2)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_empty_rows(**kwargs):
        ds = get_csv_ds('empty_rows.csv')
        eq_(len(ds.fields()),10)
        eq_(len(ds.field_types()),10)
        eq_(ds.fields(),['x', 'y', 'text', 'date', 'integer', 'boolean', 'float', 'time', 'datetime', 'empty_column'])
        eq_(ds.field_types(),['int', 'int', 'str', 'str', 'int', 'str', 'float', 'str', 'str', 'str'])
        fs = ds.featureset()
        attr = {'x': 0, 'empty_column': u'', 'text': u'a b', 'float': 1.0, 'datetime': u'1971-01-01T04:14:00', 'y': 0, 'boolean': u'True', 'time': u'04:14:00', 'date': u'1971-01-01', 'integer': 40}
        first = True
        for feat in fs:
            if first:
                first=False
                eq_(feat.attributes,attr)
            eq_(len(feat),10)
            eq_(feat['empty_column'],u'')

        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_slashes(**kwargs):
        ds = get_csv_ds('has_attributes_with_slashes.csv')
        eq_(len(ds.fields()),3)
        fs = ds.all_features()
        eq_(fs[0].attributes,{'x':0,'y':0,'name':u'a/a'})
        eq_(fs[1].attributes,{'x':1,'y':4,'name':u'b/b'})
        eq_(fs[2].attributes,{'x':10,'y':2.5,'name':u'c/c'})
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_wkt_field(**kwargs):
        ds = get_csv_ds('wkt.csv')
        eq_(len(ds.fields()),1)
        eq_(ds.fields(),['type'])
        eq_(ds.field_types(),['str'])
        fs = ds.all_features()
        eq_(len(fs[0].geometries()),1)
        eq_(fs[0].geometries()[0].type(),mapnik.DataGeometryType.Point)
        eq_(len(fs[1].geometries()),1)
        eq_(fs[1].geometries()[0].type(),mapnik.DataGeometryType.LineString)
        eq_(len(fs[2].geometries()),1)
        eq_(fs[2].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        eq_(len(fs[3].geometries()),1) # one geometry, two parts
        eq_(fs[3].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        eq_(len(fs[4].geometries()),4)
        eq_(fs[4].geometries()[0].type(),mapnik.DataGeometryType.Point)
        eq_(len(fs[5].geometries()),2)
        eq_(fs[5].geometries()[0].type(),mapnik.DataGeometryType.LineString)
        eq_(len(fs[6].geometries()),2)
        eq_(fs[6].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        eq_(len(fs[7].geometries()),2)
        eq_(fs[7].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Collection)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_handling_of_missing_header(**kwargs):
        ds = get_csv_ds('missing_header.csv')
        eq_(len(ds.fields()),6)
        eq_(ds.fields(),['one','two','x','y','_4','aftermissing'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['_4'],'missing')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_handling_of_headers_that_are_numbers(**kwargs):
        ds = get_csv_ds('numbers_for_headers.csv')
        eq_(len(ds.fields()),5)
        eq_(ds.fields(),['x','y','1990','1991','1992'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['1990'],1)
        eq_(feat['1991'],2)
        eq_(feat['1992'],3)

        eq_(mapnik.Expression("[1991]=2").evaluate(feat),True)

    def test_quoted_numbers(**kwargs):
        ds = get_csv_ds('points.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','label'])
        fs = ds.all_features()
        eq_(fs[0]['label'],"0,0")
        eq_(fs[1]['label'],"5,5")
        eq_(fs[2]['label'],"0,5")
        eq_(fs[3]['label'],"5,0")
        eq_(fs[4]['label'],"2.5,2.5")
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_reading_windows_newlines(**kwargs):
        ds = get_csv_ds('windows_newlines.csv')
        eq_(len(ds.fields()),3)
        feats = ds.all_features()
        eq_(len(feats),1)
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],1)
        eq_(feat['y'],10)
        eq_(feat['z'],9999.9999)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_reading_mac_newlines(**kwargs):
        ds = get_csv_ds('mac_newlines.csv')
        eq_(len(ds.fields()),3)
        feats = ds.all_features()
        eq_(len(feats),1)
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],1)
        eq_(feat['y'],10)
        eq_(feat['z'],9999.9999)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def check_newlines(filename):
        ds = get_csv_ds(filename)
        eq_(len(ds.fields()),3)
        feats = ds.all_features()
        eq_(len(feats),1)
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['line'],'many\n  lines\n  of text\n  with unix newlines')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_mixed_mac_unix_newlines(**kwargs):
        check_newlines('mac_newlines_with_unix_inline.csv')

    def test_mixed_mac_unix_newlines_escaped(**kwargs):
        check_newlines('mac_newlines_with_unix_inline_escaped.csv')

    # To hard to support this case
    #def test_mixed_unix_windows_newlines(**kwargs):
    #    check_newlines('unix_newlines_with_windows_inline.csv')

    # To hard to support this case
    #def test_mixed_unix_windows_newlines_escaped(**kwargs):
    #    check_newlines('unix_newlines_with_windows_inline_escaped.csv')

    def test_mixed_windows_unix_newlines(**kwargs):
        check_newlines('windows_newlines_with_unix_inline.csv')

    def test_mixed_windows_unix_newlines_escaped(**kwargs):
        check_newlines('windows_newlines_with_unix_inline_escaped.csv')

    def test_tabs(**kwargs):
        ds = get_csv_ds('tabs_in_csv.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','z'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],-122)
        eq_(feat['y'],48)
        eq_(feat['z'],0)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_separator_pipes(**kwargs):
        ds = get_csv_ds('pipe_delimiters.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','z'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['z'],'hello')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_separator_semicolon(**kwargs):
        ds = get_csv_ds('semicolon_delimiters.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','z'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['z'],'hello')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_that_null_and_bool_keywords_are_empty_strings(**kwargs):
        ds = get_csv_ds('nulls_and_booleans_as_strings.csv')
        eq_(len(ds.fields()),4)
        eq_(ds.fields(),['x','y','null','boolean'])
        eq_(ds.field_types(),['int','int','str','str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['null'],'null')
        eq_(feat['boolean'],'true')
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['null'],'')
        eq_(feat['boolean'],'false')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = get_csv_ds('lon_lat.csv')
        eq_(len(ds.fields()),2)
        eq_(ds.fields(),['lon','lat'])
        eq_(ds.field_types(),['int','int'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        fs = ds.features(query)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

    def test_that_leading_zeros_mean_strings(**kwargs):
        ds = get_csv_ds('leading_zeros.csv')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','fips'])
        eq_(ds.field_types(),['int','int','str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['fips'],'001')
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['fips'],'003')
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['fips'],'005')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

    def test_advanced_geometry_detection(**kwargs):
        ds = get_csv_ds('point_wkt.csv')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Point)
        ds = get_csv_ds('poly_wkt.csv')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Polygon)
        ds = get_csv_ds('multi_poly_wkt.csv')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Polygon)
        ds = get_csv_ds('line_wkt.csv')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.LineString)

    def test_creation_of_csv_from_in_memory_string(**kwargs):
        csv_string = '''
           wkt,Name
          "POINT (120.15 48.47)","Winthrop, WA"
          ''' # csv plugin will test lines <= 10 chars for being fully blank
        ds = mapnik.Datasource(**{"type":"csv","inline":csv_string})
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Point)
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['Name'],u"Winthrop, WA")

    def validate_geojson_datasource(ds):
        eq_(len(ds.fields()),1)
        eq_(ds.fields(),['type'])
        eq_(ds.field_types(),['str'])
        fs = ds.all_features()
        eq_(len(fs[0].geometries()),1)
        eq_(fs[0].geometries()[0].type(),mapnik.DataGeometryType.Point)
        eq_(len(fs[1].geometries()),1)
        eq_(fs[1].geometries()[0].type(),mapnik.DataGeometryType.LineString)
        eq_(len(fs[2].geometries()),1)
        eq_(fs[2].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        eq_(len(fs[3].geometries()),1) # one geometry, two parts
        eq_(fs[3].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        eq_(len(fs[4].geometries()),4)
        eq_(fs[4].geometries()[0].type(),mapnik.DataGeometryType.Point)
        eq_(len(fs[5].geometries()),2)
        eq_(fs[5].geometries()[0].type(),mapnik.DataGeometryType.LineString)
        eq_(len(fs[6].geometries()),2)
        eq_(fs[6].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        eq_(len(fs[7].geometries()),2)
        eq_(fs[7].geometries()[0].type(),mapnik.DataGeometryType.Polygon)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Collection)
        eq_(desc['name'],'csv')
        eq_(desc['type'],mapnik.DataType.Vector)
        eq_(desc['encoding'],'utf-8')

    def test_json_field1(**kwargs):
        ds = get_csv_ds('geojson_double_quote_escape.csv')
        validate_geojson_datasource(ds)

    def test_json_field2(**kwargs):
        ds = get_csv_ds('geojson_single_quote.csv')
        validate_geojson_datasource(ds)

    def test_json_field3(**kwargs):
        ds = get_csv_ds('geojson_2x_double_quote_filebakery_style.csv')
        validate_geojson_datasource(ds)

    def test_that_blank_undelimited_rows_are_still_parsed(**kwargs):
        ds = get_csv_ds('more_headers_than_column_values.csv')
        eq_(len(ds.fields()),5)
        eq_(ds.fields(),['x','y','one', 'two','three'])
        eq_(ds.field_types(),['int','int','str','str','str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['one'],'')
        eq_(feat['two'],'')
        eq_(feat['three'],'')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)

    @raises(RuntimeError)
    def test_that_fewer_headers_than_rows_throws(**kwargs):
        # this has invalid header # so throw
        ds = get_csv_ds('more_column_values_than_headers.csv')

    def test_that_feature_id_only_incremented_for_valid_rows(**kwargs):
        ds = mapnik.Datasource(type='csv',
                               file=os.path.join('../data/csv/warns','feature_id_counting.csv'),
                               quiet=True)
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','id'])
        eq_(ds.field_types(),['int','int','int'])
        fs = ds.featureset()
        # first
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['id'],1)
        # second, should have skipped bogus one
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['id'],2)
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(len(ds.all_features()),2)

    def test_dynamically_defining_headers1(**kwargs):
        ds = mapnik.Datasource(type='csv',
                               file=os.path.join('../data/csv/fails','needs_headers_two_lines.csv'),
                               quiet=True,
                               headers='x,y,name')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','name'])
        eq_(ds.field_types(),['int','int','str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['name'],'data_name')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(len(ds.all_features()),2)

    def test_dynamically_defining_headers2(**kwargs):
        ds = mapnik.Datasource(type='csv',
                               file=os.path.join('../data/csv/fails','needs_headers_one_line.csv'),
                               quiet=True,
                               headers='x,y,name')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','name'])
        eq_(ds.field_types(),['int','int','str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['name'],'data_name')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(len(ds.all_features()),1)

    def test_dynamically_defining_headers3(**kwargs):
        ds = mapnik.Datasource(type='csv',
                               file=os.path.join('../data/csv/fails','needs_headers_one_line_no_newline.csv'),
                               quiet=True,
                               headers='x,y,name')
        eq_(len(ds.fields()),3)
        eq_(ds.fields(),['x','y','name'])
        eq_(ds.field_types(),['int','int','str'])
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['x'],0)
        eq_(feat['y'],0)
        eq_(feat['name'],'data_name')
        desc = ds.describe()
        eq_(desc['geometry_type'],mapnik.DataGeometryType.Point)
        eq_(len(ds.all_features()),1)

if __name__ == "__main__":
    setup()
    [eval(run)(visual=True) for run in dir() if 'test_' in run]
