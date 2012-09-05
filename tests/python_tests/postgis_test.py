#!/usr/bin/env python

from nose.tools import *
import atexit
import time
from utilities import execution_path
from subprocess import Popen, PIPE
import os, mapnik

MAPNIK_TEST_DBNAME = 'mapnik-tmp-postgis-test-db'
POSTGIS_TEMPLATE_DBNAME = 'template_postgis'
SHAPEFILE = os.path.join(execution_path('.'),'../data/shp/world_merc.shp')

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def call(cmd,silent=False):
    stdin, stderr = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        return stdin.strip()
    elif not silent and not 'NOTICE' in stderr:
        raise RuntimeError(stderr.strip())

def psql_can_connect():
    """Test ability to connect to a postgis template db with no options.

    Basically, to run these tests your user must have full read
    access over unix sockets without supplying a password. This
    keeps these tests simple and focused on postgis not on postgres
    auth issues.
    """
    try:
        call('psql %s -c "select postgis_version()"' % POSTGIS_TEMPLATE_DBNAME)
        return True
    except RuntimeError, e:
        print 'Notice: skipping postgis tests (connection)'
        return False

def shp2pgsql_on_path():
    """Test for presence of shp2pgsql on the user path.

    We require this program to load test data into a temporarily database.
    """
    try:
        call('shp2pgsql')
        return True
    except RuntimeError, e:
        print 'Notice: skipping postgis tests (shp2pgsql)'
        return False

def createdb_and_dropdb_on_path():
    """Test for presence of dropdb/createdb on user path.

    We require these programs to setup and teardown the testing db.
    """
    try:
        call('createdb --help')
        call('dropdb --help')
        return True
    except RuntimeError, e:
        print 'Notice: skipping postgis tests (createdb/dropdb)'
        return False

insert_table_1 = """
CREATE TABLE test(gid serial PRIMARY KEY, geom geometry);
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;POINT(-2 2)'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;MULTIPOINT(2 1,1 2)'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;LINESTRING(0 0,1 1,1 2)'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;MULTILINESTRING((1 0,0 1,3 2),(3 2,5 4))'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1, 2 1, 2 2, 1 2,1 1))'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;MULTIPOLYGON(((1 1,3 1,3 3,1 3,1 1),(1 1,2 1,2 2,1 2,1 1)), ((-1 -1,-1 -2,-2 -2,-2 -1,-1 -1)))'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;GEOMETRYCOLLECTION(POLYGON((1 1, 2 1, 2 2, 1 2,1 1)),POINT(2 3),LINESTRING(2 3,3 4))'));
"""

insert_table_2 = """
CREATE TABLE test2(manual_id int4 PRIMARY KEY, geom geometry);
INSERT INTO test2(manual_id, geom) values (0, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test2(manual_id, geom) values (1, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test2(manual_id, geom) values (1000, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test2(manual_id, geom) values (-1000, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test2(manual_id, geom) values (2147483647, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test2(manual_id, geom) values (-2147483648, GeomFromEWKT('SRID=4326;POINT(0 0)'));
"""

insert_table_3 = """
CREATE TABLE test3(non_id bigint, manual_id int4, geom geometry);
INSERT INTO test3(non_id, manual_id, geom) values (9223372036854775807, 0, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test3(non_id, manual_id, geom) values (9223372036854775807, 1, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test3(non_id, manual_id, geom) values (9223372036854775807, 1000, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test3(non_id, manual_id, geom) values (9223372036854775807, -1000, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test3(non_id, manual_id, geom) values (9223372036854775807, 2147483647, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test3(non_id, manual_id, geom) values (9223372036854775807, -2147483648, GeomFromEWKT('SRID=4326;POINT(0 0)'));
"""

insert_table_4 = """
CREATE TABLE test4(non_id int4, manual_id int8 PRIMARY KEY, geom geometry);
INSERT INTO test4(non_id, manual_id, geom) values (0, 0, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test4(non_id, manual_id, geom) values (0, 1, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test4(non_id, manual_id, geom) values (0, 1000, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test4(non_id, manual_id, geom) values (0, -1000, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test4(non_id, manual_id, geom) values (0, 2147483647, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test4(non_id, manual_id, geom) values (0, -2147483648, GeomFromEWKT('SRID=4326;POINT(0 0)'));
"""

insert_table_5 = """
CREATE TABLE test5(non_id int4, manual_id numeric PRIMARY KEY, geom geometry);
INSERT INTO test5(non_id, manual_id, geom) values (0, -1, GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test5(non_id, manual_id, geom) values (0, 1, GeomFromEWKT('SRID=4326;POINT(0 0)'));
"""

insert_table_5b = '''
CREATE TABLE "tableWithMixedCase"(gid serial PRIMARY KEY, geom geometry);
INSERT INTO "tableWithMixedCase"(geom) values (ST_MakePoint(0,0));
INSERT INTO "tableWithMixedCase"(geom) values (ST_MakePoint(0,1));
INSERT INTO "tableWithMixedCase"(geom) values (ST_MakePoint(1,0));
INSERT INTO "tableWithMixedCase"(geom) values (ST_MakePoint(1,1));
'''

insert_table_6 = '''
CREATE TABLE test6(first_id int4, second_id int4,PRIMARY KEY (first_id,second_id), geom geometry);
INSERT INTO test6(first_id, second_id, geom) values (0, 0, GeomFromEWKT('SRID=4326;POINT(0 0)'));
'''

insert_table_7 = '''
CREATE TABLE test7(gid serial PRIMARY KEY, geom geometry);
INSERT INTO test7(gid, geom) values (1, GeomFromEWKT('SRID=4326;GEOMETRYCOLLECTION(MULTILINESTRING((10 10,20 20,10 40),(40 40,30 30,40 20,30 10)),LINESTRING EMPTY)'));
'''


def postgis_setup():
    call('dropdb %s' % MAPNIK_TEST_DBNAME,silent=True)
    call('createdb -T %s %s' % (POSTGIS_TEMPLATE_DBNAME,MAPNIK_TEST_DBNAME),silent=False)
    call('shp2pgsql -s 3857 -g geom -W LATIN1 %s world_merc | psql -q %s' % (SHAPEFILE,MAPNIK_TEST_DBNAME), silent=True)
    call('''psql -q %s -c "CREATE TABLE \"empty\" (key serial);SELECT AddGeometryColumn('','empty','geom','-1','GEOMETRY',4);"''' % MAPNIK_TEST_DBNAME,silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_1),silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_2),silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_3),silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_4),silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_5),silent=False)
    call("""psql -q %s -c '%s'""" % (MAPNIK_TEST_DBNAME,insert_table_5b),silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_6),silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_7),silent=False)

def postgis_takedown():
    pass
    # fails as the db is in use: https://github.com/mapnik/mapnik/issues/960
    #call('dropdb %s' % MAPNIK_TEST_DBNAME)

if 'postgis' in mapnik.DatasourceCache.plugin_names() \
        and createdb_and_dropdb_on_path() \
        and psql_can_connect() \
        and shp2pgsql_on_path():

    # initialize test database
    postgis_setup()

    def test_feature():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='world_merc')
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['gid'],1)
        eq_(feature['fips'],u'AC')
        eq_(feature['iso2'],u'AG')
        eq_(feature['iso3'],u'ATG')
        eq_(feature['un'],28)
        eq_(feature['name'],u'Antigua and Barbuda')
        eq_(feature['area'],44)
        eq_(feature['pop2005'],83039)
        eq_(feature['region'],19)
        eq_(feature['subregion'],29)
        eq_(feature['lon'],-61.783)
        eq_(feature['lat'],17.078)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Polygon)

    def test_subquery():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='(select * from world_merc) as w')
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['gid'],1)
        eq_(feature['fips'],u'AC')
        eq_(feature['iso2'],u'AG')
        eq_(feature['iso3'],u'ATG')
        eq_(feature['un'],28)
        eq_(feature['name'],u'Antigua and Barbuda')
        eq_(feature['area'],44)
        eq_(feature['pop2005'],83039)
        eq_(feature['region'],19)
        eq_(feature['subregion'],29)
        eq_(feature['lon'],-61.783)
        eq_(feature['lat'],17.078)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Polygon)

        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='(select gid,geom,fips as _fips from world_merc) as w')
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['gid'],1)
        eq_(feature['_fips'],u'AC')
        eq_(len(feature),2)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Polygon)

    def test_empty_db():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='empty')
        fs = ds.featureset()
        feature = None
        try:
            feature = fs.next()
        except StopIteration:
            pass
        eq_(feature,None)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Collection)

    def test_geometry_detection():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test',
                            geometry_field='geom')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Collection)

        # will fail with postgis 2.0 because it automatically adds a geometry_columns entry
        #ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test',
        #                   geometry_field='geom',
        #                    row_limit=1)
        #eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Point)

    @raises(RuntimeError)
    def test_that_nonexistant_query_field_throws(**kwargs):
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='empty')
        eq_(len(ds.fields()),1)
        eq_(ds.fields(),['key'])
        eq_(ds.field_types(),['int'])
        query = mapnik.Query(ds.envelope())
        for fld in ds.fields():
            query.add_property_name(fld)
        # also add an invalid one, triggering throw
        query.add_property_name('bogus')
        fs = ds.features(query)

    def test_auto_detection_of_unique_feature_id_32_bit():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test2',
                            geometry_field='geom',
                            autodetect_key_field=True)
        fs = ds.featureset()
        eq_(fs.next()['manual_id'],0)
        eq_(fs.next()['manual_id'],1)
        eq_(fs.next()['manual_id'],1000)
        eq_(fs.next()['manual_id'],-1000)
        eq_(fs.next()['manual_id'],2147483647)
        eq_(fs.next()['manual_id'],-2147483648)

        fs = ds.featureset()
        eq_(fs.next().id(),0)
        eq_(fs.next().id(),1)
        eq_(fs.next().id(),1000)
        eq_(fs.next().id(),-1000)
        eq_(fs.next().id(),2147483647)
        eq_(fs.next().id(),-2147483648)

    def test_auto_detection_will_fail_since_no_primary_key():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test3',
                            geometry_field='geom',
                            autodetect_key_field=False)
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat['manual_id'],0)
        # will fail: https://github.com/mapnik/mapnik/issues/895
        #eq_(feat['non_id'],9223372036854775807)
        eq_(fs.next()['manual_id'],1)
        eq_(fs.next()['manual_id'],1000)
        eq_(fs.next()['manual_id'],-1000)
        eq_(fs.next()['manual_id'],2147483647)
        eq_(fs.next()['manual_id'],-2147483648)

        # since no valid primary key will be detected the fallback
        # is auto-incrementing counter
        fs = ds.featureset()
        eq_(fs.next().id(),1)
        eq_(fs.next().id(),2)
        eq_(fs.next().id(),3)
        eq_(fs.next().id(),4)
        eq_(fs.next().id(),5)
        eq_(fs.next().id(),6)

    @raises(RuntimeError)
    def test_auto_detection_will_fail_and_should_throw():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test3',
                            geometry_field='geom',
                            autodetect_key_field=True)
        fs = ds.featureset()

    def test_auto_detection_of_unique_feature_id_64_bit():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test4',
                            geometry_field='geom',
                            autodetect_key_field=True)
        fs = ds.featureset()
        eq_(fs.next()['manual_id'],0)
        eq_(fs.next()['manual_id'],1)
        eq_(fs.next()['manual_id'],1000)
        eq_(fs.next()['manual_id'],-1000)
        eq_(fs.next()['manual_id'],2147483647)
        eq_(fs.next()['manual_id'],-2147483648)

        fs = ds.featureset()
        eq_(fs.next().id(),0)
        eq_(fs.next().id(),1)
        eq_(fs.next().id(),1000)
        eq_(fs.next().id(),-1000)
        eq_(fs.next().id(),2147483647)
        eq_(fs.next().id(),-2147483648)

    def test_disabled_auto_detection_and_subquery():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='''(select geom, 'a'::varchar as name from test2) as t''',
                            geometry_field='geom',
                            autodetect_key_field=False)
        fs = ds.featureset()
        feat = fs.next()
        eq_(feat.id(),1)
        eq_(feat['name'],'a')
        feat = fs.next()
        eq_(feat.id(),2)
        eq_(feat['name'],'a')
        feat = fs.next()
        eq_(feat.id(),3)
        eq_(feat['name'],'a')
        feat = fs.next()
        eq_(feat.id(),4)
        eq_(feat['name'],'a')
        feat = fs.next()
        eq_(feat.id(),5)
        eq_(feat['name'],'a')
        feat = fs.next()
        eq_(feat.id(),6)
        eq_(feat['name'],'a')

    def test_auto_detection_and_subquery_including_key():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='''(select geom, manual_id from test2) as t''',
                            geometry_field='geom',
                            autodetect_key_field=True)
        fs = ds.featureset()
        eq_(fs.next()['manual_id'],0)
        eq_(fs.next()['manual_id'],1)
        eq_(fs.next()['manual_id'],1000)
        eq_(fs.next()['manual_id'],-1000)
        eq_(fs.next()['manual_id'],2147483647)
        eq_(fs.next()['manual_id'],-2147483648)

        fs = ds.featureset()
        eq_(fs.next().id(),0)
        eq_(fs.next().id(),1)
        eq_(fs.next().id(),1000)
        eq_(fs.next().id(),-1000)
        eq_(fs.next().id(),2147483647)
        eq_(fs.next().id(),-2147483648)

    @raises(RuntimeError)
    def test_auto_detection_of_invalid_numeric_primary_key():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='''(select geom, manual_id::numeric from test2) as t''',
                            geometry_field='geom',
                            autodetect_key_field=True)

    @raises(RuntimeError)
    def test_auto_detection_of_invalid_multiple_keys():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='''test6''',
                            geometry_field='geom',
                            autodetect_key_field=True)

    @raises(RuntimeError)
    def test_auto_detection_of_invalid_multiple_keys_subquery():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='''(select first_id,second_id,geom from test6) as t''',
                            geometry_field='geom',
                            autodetect_key_field=True)

    def test_manually_specified_feature_id_field():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test4',
                            geometry_field='geom',
                            key_field='manual_id',
                            autodetect_key_field=True)
        fs = ds.featureset()
        eq_(fs.next()['manual_id'],0)
        eq_(fs.next()['manual_id'],1)
        eq_(fs.next()['manual_id'],1000)
        eq_(fs.next()['manual_id'],-1000)
        eq_(fs.next()['manual_id'],2147483647)
        eq_(fs.next()['manual_id'],-2147483648)

        fs = ds.featureset()
        eq_(fs.next().id(),0)
        eq_(fs.next().id(),1)
        eq_(fs.next().id(),1000)
        eq_(fs.next().id(),-1000)
        eq_(fs.next().id(),2147483647)
        eq_(fs.next().id(),-2147483648)

    def test_numeric_type_feature_id_field():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test5',
                            geometry_field='geom',
                            autodetect_key_field=False)
        fs = ds.featureset()
        eq_(fs.next()['manual_id'],-1)
        eq_(fs.next()['manual_id'],1)

        fs = ds.featureset()
        eq_(fs.next().id(),1)
        eq_(fs.next().id(),2)

    def test_querying_table_with_mixed_case():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='"tableWithMixedCase"',
                            geometry_field='geom',
                            autodetect_key_field=True)
        fs = ds.featureset()
        for id in range(1,5):
            eq_(fs.next().id(),id)

    def test_querying_subquery_with_mixed_case():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='(SeLeCt * FrOm "tableWithMixedCase") as MixedCaseQuery',
                            geometry_field='geom',
                            autodetect_key_field=True)
        fs = ds.featureset()
        for id in range(1,5):
            eq_(fs.next().id(),id)

    def test_bbox_token_in_subquery1():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='''
           (SeLeCt * FrOm "tableWithMixedCase" where geom && !bbox! ) as MixedCaseQuery''',
                            geometry_field='geom',
                            autodetect_key_field=True)
        fs = ds.featureset()
        for id in range(1,5):
            eq_(fs.next().id(),id)

    def test_bbox_token_in_subquery2():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='''
           (SeLeCt * FrOm "tableWithMixedCase" where ST_Intersects(geom,!bbox!) ) as MixedCaseQuery''',
                            geometry_field='geom',
                            autodetect_key_field=True)
        fs = ds.featureset()
        for id in range(1,5):
            eq_(fs.next().id(),id)

    def test_empty_geom():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test7',
                            geometry_field='geom')
        fs = ds.featureset()
        eq_(fs.next()['gid'],1)

    atexit.register(postgis_takedown)

if __name__ == "__main__":
    setup()
    #test_auto_detection_and_subquery()
    [eval(run)() for run in dir() if 'test_' in run]
