#!/usr/bin/env python

from nose.tools import *
from utilities import execution_path
from Queue import Queue
import threading

import os, mapnik2
import sqlite3

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

NUM_THREADS = 10

DB = '../data/sqlite/world.sqlite'

def create_ds():
    ds = mapnik2.SQLite(file=DB,table='world_merc')
    fs = ds.all_features()

if 'sqlite' in mapnik2.DatasourceCache.instance().plugin_names():
    
    def test_rtree_creation():

        index = DB +'.index'
        if os.path.exists(index):
            os.unlink(index)

        threads = []
        for i in range(NUM_THREADS):
            t = threading.Thread(target=create_ds)
            t.start()
            threads.append(t)

        for i in threads:
            i.join()
        
        if os.path.exists(index):
            conn = sqlite3.connect(index)
            cur = conn.cursor()
            cur.execute("Select count(*) from idx_world_merc_GEOMETRY")
            conn.commit()
            eq_(cur.fetchone()[0],245)
            cur.close()
        else:
            print 'index does not exist!'

        if os.path.exists(index):
            ds = mapnik2.SQLite(file=DB,table='world_merc')
            fs = ds.all_features()
            eq_(len(fs),245)
            os.unlink(index)
            ds = mapnik2.SQLite(file=DB,table='world_merc',use_spatial_index=False)
            fs = ds.all_features()
            eq_(len(fs),245)
            eq_(os.path.exists(index),False)

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
    
