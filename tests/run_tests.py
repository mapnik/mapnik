#!/usr/bin/env python

import sys

try:
    import nose
except ImportError:
    sys.stderr.write("Unable to run python tests: the third party 'nose' module is required\nTo install 'nose' do:\n\tsudo pip install nose (or on debian systems: apt-get install python-nose\n")
    sys.exit(1)
    
from python_tests.utilities import TodoPlugin
from nose.plugins.doctests import Doctest

import nose, sys, os, getopt

def usage():
    print("test.py -h | --help")
    print("test.py [-q | -v] [-p | --prefix <path>]")

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hvqp:", ["help", "prefix="])
    except getopt.GetoptError,err:
        print(str(err))
        usage()
        sys.exit(2)

    prefix = None
    verbose = False
    quiet = False

    for o, a in opts:
        if o == "-q":
            quiet = True
        elif o == "-v":
            verbose = True
        elif o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-p", "--prefix"):
            prefix = a
        else:
            assert False, "Unhandled option"

    if quiet and verbose:
        usage()
        sys.exit(2)

    if prefix:
        # Allow python to find libraries for testing on the buildbot
        sys.path.insert(0, os.path.join(prefix, "lib/python%s/site-packages" % sys.version[:3]))

    import mapnik

    if not quiet:
        print("- mapnik path: %s" % mapnik.__file__)
        if hasattr(mapnik,'_mapnik'):
           print("- _mapnik.so path: %s" % mapnik._mapnik.__file__)
        print("- Input plugins path: %s" % mapnik.inputpluginspath)
        print("- Font path: %s" % mapnik.fontscollectionpath)
        print('')
        print("- Running nosetests:")
        print('')

    argv = [__file__, '--exe', '--with-todo', '--with-doctest', '--doctest-tests']

    if not quiet:
        argv.append('-v')

    if verbose:
        # 3 * '-v' gets us debugging information from nose
        argv.append('-v')
        argv.append('-v')

    dirname = os.path.dirname(sys.argv[0]) 
    argv.extend(['-w', dirname+'/python_tests'])

    if not nose.run(argv=argv, plugins=[TodoPlugin(), Doctest()]):
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main()
