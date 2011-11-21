#!/usr/bin/env python

from nose.tools import *
import atexit
import time
from utilities import execution_path
from subprocess import Popen, PIPE
import os, mapnik2

MAPNIK_TEST_DBNAME = 'mapnik-tmp-postgis-test-db'
POSTGIS_TEMPLATE_DBNAME = 'template_postgis'

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def call(cmd,silent=False):
    stdin, stderr = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        return stdin.strip()
    elif not silent:
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
        print 'Notice: skipping postgis tests as basic auth is not correctly set up. Error was: %s' % e.message
        return False

def shp2pgsql_on_path():
    """Test for presence of shp2pgsql on the user path.
    
    We require this program to load test data into a temporarily database.
    """
    try:
        call('shp2pgsql')
        return True
    except RuntimeError, e:
        print 'Notice: skipping postgis tests because shp2pgsql not found. Error was: %s' % e.message
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
        print 'Notice: skipping postgis tests because createdb or dropdb not found. Error was: %s' % e.message
        return False

def postgis_setup():
    call('dropdb %s' % MAPNIK_TEST_DBNAME,silent=True)
    call('createdb -T %s %s' % (POSTGIS_TEMPLATE_DBNAME,MAPNIK_TEST_DBNAME),silent=False)
    call('shp2pgsql -s 3857 -g geom -W LATIN1 ./tests/data/shp/world_merc.shp world_merc | psql -q %s' % MAPNIK_TEST_DBNAME, silent=True)
    call('''psql %s -c "CREATE TABLE \"empty\" (key serial);SELECT AddGeometryColumn('','empty','geom','0','GEOMETRY',4);"''' % MAPNIK_TEST_DBNAME,silent=True)

def postgis_takedown():
    pass
    # fails as the db is in use: https://github.com/mapnik/mapnik/issues/960
    #call('dropdb %s' % MAPNIK_TEST_DBNAME)

if 'postgis' in mapnik2.DatasourceCache.instance().plugin_names() \
        and createdb_and_dropdb_on_path() \
        and psql_can_connect() \
        and shp2pgsql_on_path():
    
    # initialize test database
    postgis_setup()

    def test_feature():
        ds = mapnik2.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='world_merc')
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

    def test_subquery():
        ds = mapnik2.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='(select * from world_merc) as w')
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

        ds = mapnik2.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='(select gid,geom,fips as _fips from world_merc) as w')
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature['gid'],1)
        eq_(feature['_fips'],u'AC')
        eq_(len(feature),2)

    def test_empty_db():
        ds = mapnik2.PostGIS(dbname=MAPNIK_TEST_DBNAME,table='empty')
        fs = ds.featureset()
        feature = fs.next()
        eq_(feature,None)


    atexit.register(postgis_takedown)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
