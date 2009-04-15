#!/usr/bin/python

from nose.tools import *
from utilities import execution_path

import os, sys, glob, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

def test():
    # TODO: Write a better test
    # 1. Construct map in memory
    # 2. Save map as XML
    # 3. Load map to a second object
    # 4. Compare both map objects
    map = mapnik.Map(256, 256)
    in_map = "../data/good_maps/osm-styles.xml"

    print "Loading map '%s' ... " % in_map

    mapnik.load_map(map, in_map)
    test_map = "test_map.xml"

    print "Saving map '%s' ... " % test_map

    mapnik.save_map(map, test_map)
    new_map = mapnik.Map(256, 256)

    print "Reloading map '%s' ... " % test_map

    mapnik.load_map(new_map, test_map)

    if os.path.exists(test_map):
        print "Removing '%s'" % test_map
        os.remove(test_map)
    else:
        # Fail, the map wasn't written
        return False
