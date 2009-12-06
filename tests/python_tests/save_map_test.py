#!/usr/bin/env python

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

    mapnik.load_map(map, in_map)
    test_map = "test_map.xml"

    mapnik.save_map(map, test_map)
    new_map = mapnik.Map(256, 256)

    mapnik.load_map(new_map, test_map)

    eq_(open(test_map).read(),mapnik.save_map_to_string(new_map))

    if os.path.exists(test_map):
        os.remove(test_map)
    else:
        # Fail, the map wasn't written
        return False
