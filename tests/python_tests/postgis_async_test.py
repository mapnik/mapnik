#!/usr/bin/env python

from nose.tools import *
import sys
import time
from utilities import execution_path
from subprocess import Popen, PIPE
import os, mapnik

MAPNIK_TEST_DBNAME = 'mapnik-tmp-postgis-async-test-db'
POSTGIS_TEMPLATE_DBNAME = 'template_postgis'

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
CREATE TABLE test1(gid serial PRIMARY KEY, label varchar(40), geom geometry);
INSERT INTO test1(label,geom) values ('label_1',GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test1(label,geom) values ('label_2',GeomFromEWKT('SRID=4326;POINT(-2 2)'));
INSERT INTO test1(label,geom) values ('label_3',GeomFromEWKT('SRID=4326;MULTIPOINT(2 1,1 2)'));
INSERT INTO test1(label,geom) values ('label_4',GeomFromEWKT('SRID=4326;LINESTRING(0 0,1 1,1 2)'));
INSERT INTO test1(label,geom) values ('label_5',GeomFromEWKT('SRID=4326;MULTILINESTRING((1 0,0 1,3 2),(3 2,5 4))'));
INSERT INTO test1(label,geom) values ('label_6',GeomFromEWKT('SRID=4326;POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1, 2 1, 2 2, 1 2,1 1))'));
INSERT INTO test1(label,geom) values ('label_7',GeomFromEWKT('SRID=4326;MULTIPOLYGON(((1 1,3 1,3 3,1 3,1 1),(1 1,2 1,2 2,1 2,1 1)), ((-1 -1,-1 -2,-2 -2,-2 -1,-1 -1)))'));
INSERT INTO test1(label,geom) values ('label_8',GeomFromEWKT('SRID=4326;GEOMETRYCOLLECTION(POLYGON((1 1, 2 1, 2 2, 1 2,1 1)),POINT(2 3),LINESTRING(2 3,3 4))'));
"""

insert_table_2 = """
CREATE TABLE test2(gid serial PRIMARY KEY, label varchar(40), geom geometry);
INSERT INTO test2(label,geom) values ('label_1',GeomFromEWKT('SRID=4326;POINT(0 0)'));
INSERT INTO test2(label,geom) values ('label_2',GeomFromEWKT('SRID=4326;POINT(-2 2)'));
INSERT INTO test2(label,geom) values ('label_3',GeomFromEWKT('SRID=4326;MULTIPOINT(2 1,1 2)'));
INSERT INTO test2(label,geom) values ('label_4',GeomFromEWKT('SRID=4326;LINESTRING(0 0,1 1,1 2)'));
INSERT INTO test2(label,geom) values ('label_5',GeomFromEWKT('SRID=4326;MULTILINESTRING((1 0,0 1,3 2),(3 2,5 4))'));
INSERT INTO test2(label,geom) values ('label_6',GeomFromEWKT('SRID=4326;POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1, 2 1, 2 2, 1 2,1 1))'));
INSERT INTO test2(label,geom) values ('label_7',GeomFromEWKT('SRID=4326;MULTIPOLYGON(((1 1,3 1,3 3,1 3,1 1),(1 1,2 1,2 2,1 2,1 1)), ((-1 -1,-1 -2,-2 -2,-2 -1,-1 -1)))'));
INSERT INTO test2(label,geom) values ('label_8',GeomFromEWKT('SRID=4326;GEOMETRYCOLLECTION(POLYGON((1 1, 2 1, 2 2, 1 2,1 1)),POINT(2 3),LINESTRING(2 3,3 4))'));
"""

def postgis_setup():
    call('dropdb %s' % MAPNIK_TEST_DBNAME,silent=True)
    call('createdb -T %s %s' % (POSTGIS_TEMPLATE_DBNAME,MAPNIK_TEST_DBNAME),silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_1),silent=False)
    call('''psql -q %s -c "%s"''' % (MAPNIK_TEST_DBNAME,insert_table_2),silent=False)

if 'postgis' in mapnik.DatasourceCache.plugin_names() \
        and createdb_and_dropdb_on_path() \
        and psql_can_connect():

    # initialize test database
    postgis_setup()

    def test_psql_error_should_not_break_connection_pool():
        # Bad request, will trig an error when returning result
        ds_bad = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table="""(SELECT geom as geom,label::int from public.test1) as failure_table""",
                            max_async_connection=5,geometry_field='geom',srid=4326,trace=False)

        # Good request 
        ds_good = mapnik.PostGIS(dbname=MAPNIK_TEST_DBNAME,table="test1",
                            max_async_connection=5,geometry_field='geom',srid=4326,trace=False)

        # This will/should trig a PSQL error
        failed = False
        try:
            fs = ds_bad.featureset()
            for feature in fs:
                pass
        except RuntimeError:
            failed = True

        assert_true(failed)
        
        # Should be ok
        fs = ds_good.featureset()
        for feature in fs:
            pass        



if __name__ == "__main__":
    setup()
    run_all(eval(x) for x in dir() if x.startswith("test_"))
