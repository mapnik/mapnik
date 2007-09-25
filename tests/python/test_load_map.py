#!/usr/bin/python

from mapnik import *
import sys
import glob

def testGood( file ) :
    print "Testing good file '" + file + "' ... ",
    
    m = Map(512, 512)
    try:
        load_map(m, file, True)
    except RuntimeError, what:
        print "FAILED"
        print what
        return False
    except:
        print "FAILED"
        return False
    else:
        print "OK"
        return True


def testBroken( file ) :
    print "Testing broken file '" + file + "' ... ",

    m = Map(512, 512)
    try:
        strict = True
        load_map(m, file, strict)
    except UserWarning, what:
        print "OK"
        print "=== Error Message ============="
        print what
        print 
        return True
    except RuntimeError, what:
        print "FAILED (not a UserWarning)"
        print "=== Error Message ============="
        print what
        print 
    else:
        print "FAILED"

    return False


def test():
    success = 0
    failed = 0
    failed_tests = []

    broken_files = glob.glob("../data/broken_maps/*.xml")
    # eh, can't glob this ... :-)
    broken_files.append( "../data/broken/does_not_exist.xml" ) 
    for file in broken_files:
        if testBroken( file ):
            success += 1
        else:
            failed += 1
            failed_tests.append( file )

    good_files = glob.glob("../data/good_maps/*.xml")
    for file in good_files:
        if testGood( file ):
            success += 1
        else:
            failed += 1
            failed_tests.append( file )

    print "======================================================="
    print "Status:",
    if failed:
        print "FAILED"
        print "Errors in: ", failed_tests
    else:
        print "SUCCESS"
    print "Success:", success, "Failed:", failed, "Total:", success + failed
    print "======================================================="
    if failed:
        return False
    else:
        return True


if __name__ == "__main__":
    if test():
        sys.exit( 0 )
    else:
        sys.exit( 1 )
