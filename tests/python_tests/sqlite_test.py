#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path

import os, mapnik2

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

# Note that without an extent or a spatial index, a sqlite
# datasource will error on creation.  We use this fact to test
# database attachdb and initdb options

def test_attachdb_with_relative_file():
    # The point table and index is in the qgis_spatiallite.sqlite
    # database.  If either is not found, then this fails
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='point',
        attachdb='scratch@qgis_spatiallite.sqlite'
        )

def test_attachdb_with_multiple_files():
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='attachedtest',
        attachdb='scratch1@:memory:,scratch2@:memory:',
        initdb='create table scratch1.attachedtest (the_geom);\n' +
            'create virtual table scratch2.idx_attachedtest_the_geom using rtree(pkid,xmin,xmax,ymin,ymax);\n' +
            'insert into scratch2.idx_attachedtest_the_geom values (1,-7799225.5,-7778571.0,1393264.125,1417719.375)\n'
        )

def test_attachdb_with_absolute_file():
    # The point table and index is in the qgis_spatiallite.sqlite
    # database.  If either is not found, then this fails
    ds = mapnik2.SQLite(file=os.getcwd() + '/../data/sqlite/world.sqlite', 
        table='point',
        attachdb='scratch@qgis_spatiallite.sqlite'
        )

def test_attachdb_with_index():
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='attachedtest',
        attachdb='scratch@:memory:',
        initdb='create table scratch.attachedtest (the_geom);\n' +
            'create virtual table scratch.idx_attachedtest_the_geom using rtree(pkid,xmin,xmax,ymin,ymax);\n' +
            'insert into scratch.idx_attachedtest_the_geom values (1,-7799225.5,-7778571.0,1393264.125,1417719.375)\n'
        )
    
def test_attachdb_with_explicit_index():
    ds = mapnik2.SQLite(file='../data/sqlite/world.sqlite', 
        table='attachedtest',
        index_table='myindex',
        attachdb='scratch@:memory:',
        initdb='create table scratch.attachedtest (the_geom);\n' +
            'create virtual table scratch.myindex using rtree(pkid,xmin,xmax,ymin,ymax);\n' +
            'insert into scratch.myindex values (1,-7799225.5,-7778571.0,1393264.125,1417719.375)\n'
        )


if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
