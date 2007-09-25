#!/usr/bin/python

from mapnik import *
import os
import sys
import glob


def test():
#   success = 0
#   failed = 0
#   failed_tests = []

# TODO: write a better test
# 1. construct map in memory
# 2. save map as XML
# 3. load map to a second object
# 4. Compare both map objects

    map = Map(256, 256)
    in_map = "../data/good_maps/osm-styles.xml"
    print "Loading map '" + in_map + "' ... ",
    load_map( map, in_map )
    print "OK"

    test_map = "test_map.xml"
    failed = False;
    try:
        print "Saving map '" + test_map + "' ... ",
        save_map( map, test_map)
        print "OK"
    except:
        print "FAILED"
        failed = True;
        

    if not failed:
        new_map = Map(256, 256)
        try:
            print "Reloading map '" + test_map + "' ... ",
            load_map( new_map, test_map)
            print "OK"
        except UserWarning, what:
            print "FAILED"
            print "Error: ", what
            failed = True;
        except RuntimeError, what:
            print "FAILED"
            print "Error: ", what
            failed = True;
        except:
            print "FAILED"
            failed = True;

    if not failed and os.path.exists( test_map ):
        print "Removing '" + test_map + "'"
        os.remove( test_map )

    print "======================================================="
    print "Status:",
    if failed:
        print "FAILED"
    else:
        print "SUCCESS"
    print "======================================================="
    return not failed





if __name__ == "__main__":
    if test():
        sys.exit( 0 )
    else:
        sys.exit( 1 )
