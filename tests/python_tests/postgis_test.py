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

insert_sql = """
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;POINT(-2 2)'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;MULTIPOINT(2 1,1 2)'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;LINESTRING(0 0,1 1,1 2)'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;MULTILINESTRING((1 0,0 1,3 2),(3 2,5 4))'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1, 2 1, 2 2, 1 2,1 1))'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;MULTIPOLYGON(((1 1,3 1,3 3,1 3,1 1),(1 1,2 1,2 2,1 2,1 1)), ((-1 -1,-1 -2,-2 -2,-2 -1,-1 -1)))'));
INSERT INTO test(geom) values (GeomFromEWKT('SRID=4326;GEOMETRYCOLLECTION(POLYGON((1 1, 2 1, 2 2, 1 2,1 1)),POINT(2 3),LINESTRING(2 3,3 4))'));
"""

def postgis_setup():
    call('dropdb %s' % MAPNIK_TEST_DBNAME,silent=True)
    call('createdb -T %s %s' % (POSTGIS_TEMPLATE_DBNAME,MAPNIK_TEST_DBNAME),silent=False)
    call('shp2pgsql -s 3857 -g geom -W LATIN1 %s world_merc | psql -q %s' % (SHAPEFILE,MAPNIK_TEST_DBNAME), silent=True)
    call('''psql -q %s -c "CREATE TABLE \"empty\" (key serial);SELECT AddGeometryColumn('','empty','geom','-1','GEOMETRY',4);"''' % MAPNIK_TEST_DBNAME,silent=False)
    call('''psql -q %s -c "create table test(gid serial PRIMARY KEY, geom geometry);%s"''' % (MAPNIK_TEST_DBNAME,insert_sql),silent=False)
def postgis_takedown():
    pass
    # fails as the db is in use: https://github.com/mapnik/mapnik/issues/960
    #call('dropdb %s' % MAPNIK_TEST_DBNAME)

if 'postgis' in mapnik.DatasourceCache.instance().plugin_names() \
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
        feature = fs.next()
        eq_(feature,None)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Collection)

    def test_geometry_detection():
        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test',
                            geometry_field='geom')
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Collection)

        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test',
                            geometry_field='geom',
                            row_limit=1)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Point)

        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test',
                            geometry_field='geom',
                            row_limit=2)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Point)

        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test',
                            geometry_field='geom',
                            row_limit=3)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Point)

        ds = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='test',
                            geometry_field='geom',
                            row_limit=4)
        eq_(ds.describe()['geometry_type'],mapnik.DataGeometryType.Collection)

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


    atexit.register(postgis_takedown)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
