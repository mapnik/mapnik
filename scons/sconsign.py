#! /usr/bin/env python
#
# SCons - a Software Constructor
#
# Copyright (c) 2001, 2002, 2003, 2004 The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

__revision__ = "/home/scons/scons/branch.0/baseline/src/script/sconsign.py 0.96.1.D001 2004/08/23 09:55:29 knight"

__version__ = "0.96.1"

__build__ = "D001"

__buildsys__ = "casablanca"

__date__ = "2004/08/23 09:55:29"

__developer__ = "knight"

import os
import os.path
import sys
import time

##############################################################################
# BEGIN STANDARD SCons SCRIPT HEADER
#
# This is the cut-and-paste logic so that a self-contained script can
# interoperate correctly with different SCons versions and installation
# locations for the engine.  If you modify anything in this section, you
# should also change other scripts that use this same header.
##############################################################################

# Strip the script directory from sys.path() so on case-insensitive
# (WIN32) systems Python doesn't think that the "scons" script is the
# "SCons" package.  Replace it with our own library directories
# (version-specific first, in case they installed by hand there,
# followed by generic) so we pick up the right version of the build
# engine modules if they're in either directory.

script_dir = sys.path[0]

if script_dir in sys.path:
    sys.path.remove(script_dir)

libs = []

if os.environ.has_key("SCONS_LIB_DIR"):
    libs.append(os.environ["SCONS_LIB_DIR"])

local = 'scons-local-' + __version__
if script_dir:
    local = os.path.join(script_dir, local)
libs.append(local)

scons_version = 'scons-%s' % __version__

prefs = []

if sys.platform == 'win32':
    # sys.prefix is (likely) C:\Python*;
    # check only C:\Python*.
    prefs.append(sys.prefix)
    prefs.append(os.path.join(sys.prefix, 'Lib', 'site-packages'))
else:
    # On other (POSIX) platforms, things are more complicated due to
    # the variety of path names and library locations.  Try to be smart
    # about it.
    if script_dir == 'bin':
        # script_dir is `pwd`/bin;
        # check `pwd`/lib/scons*.
        prefs.append(os.getcwd())
    else:
        if script_dir == '.' or script_dir == '':
            script_dir = os.getcwd()
        head, tail = os.path.split(script_dir)
        if tail == "bin":
            # script_dir is /foo/bin;
            # check /foo/lib/scons*.
            prefs.append(head)

    head, tail = os.path.split(sys.prefix)
    if tail == "usr":
        # sys.prefix is /foo/usr;
        # check /foo/usr/lib/scons* first,
        # then /foo/usr/local/lib/scons*.
        prefs.append(sys.prefix)
        prefs.append(os.path.join(sys.prefix, "local"))
    elif tail == "local":
        h, t = os.path.split(head)
        if t == "usr":
            # sys.prefix is /foo/usr/local;
            # check /foo/usr/local/lib/scons* first,
            # then /foo/usr/lib/scons*.
            prefs.append(sys.prefix)
            prefs.append(head)
        else:
            # sys.prefix is /foo/local;
            # check only /foo/local/lib/scons*.
            prefs.append(sys.prefix)
    else:
        # sys.prefix is /foo (ends in neither /usr or /local);
        # check only /foo/lib/scons*.
        prefs.append(sys.prefix)

    temp = map(lambda x: os.path.join(x, 'lib'), prefs)
    temp.extend(map(lambda x: os.path.join(x,
                                           'lib',
                                           'python' + sys.version[:3],
                                           'site-packages'),
                           prefs))
    prefs = temp

# Look first for 'scons-__version__' in all of our preference libs,
# then for 'scons'.
libs.extend(map(lambda x: os.path.join(x, scons_version), prefs))
libs.extend(map(lambda x: os.path.join(x, 'scons'), prefs))

sys.path = libs + sys.path

##############################################################################
# END STANDARD SCons SCRIPT HEADER
##############################################################################

import cPickle
import imp
import string
import whichdb

import SCons.SConsign

def my_whichdb(filename):
    try:
        f = open(filename + ".dblite", "rb")
        f.close()
        return "SCons.dblite"
    except IOError:
        pass
    return _orig_whichdb(filename)

_orig_whichdb = whichdb.whichdb
whichdb.whichdb = my_whichdb

def my_import(mname):
    if '.' in mname:
        i = string.rfind(mname, '.')
        parent = my_import(mname[:i])
        fp, pathname, description = imp.find_module(mname[i+1:],
                                                    parent.__path__)
    else:
        fp, pathname, description = imp.find_module(mname)
    return imp.load_module(mname, fp, pathname, description)

class Flagger:
    default_value = 1
    def __setitem__(self, item, value):
        self.__dict__[item] = value
        self.default_value = 0
    def __getitem__(self, item):
        return self.__dict__.get(item, self.default_value)

Do_Call = None
Print_Directories = []
Print_Entries = []
Print_Flags = Flagger()
Verbose = 0
Readable = 0

def default_mapper(entry, name):
    try:
        val = eval("entry."+name)
    except:
        val = None
    return str(val)

def map_timestamp(entry, name):
    try:
        timestamp = entry.timestamp
    except AttributeError:
        timestamp = None
    if Readable and timestamp:
        return "'" + time.ctime(timestamp) + "'"
    else:
        return str(timestamp)

def map_bkids(entry, name):
    try:
        bkids = entry.bsources + entry.bdepends + entry.bimplicit
        bkidsigs = entry.bsourcesigs + entry.bdependsigs + entry.bimplicitsigs
    except AttributeError:
        return None
    result = []
    for i in xrange(len(bkids)):
        result.append("%s: %s" % (bkids[i], bkidsigs[i]))
    if result == []:
        return None
    return string.join(result, "\n        ")

map_field = {
    'timestamp' : map_timestamp,
    'bkids'     : map_bkids,
}

map_name = {
    'implicit'  : 'bkids',
}

def printfield(name, entry):
    def field(name, verbose=Verbose, entry=entry):
        if not Print_Flags[name]:
            return None
        fieldname = map_name.get(name, name)
        mapper = map_field.get(fieldname, default_mapper)
        val = mapper(entry, name)
        if verbose:
            val = name + ": " + val
        return val

    fieldlist = ["timestamp", "bsig", "csig"]
    outlist = [name+":"] + filter(None, map(field, fieldlist))
    sep = Verbose and "\n    " or " "
    print string.join(outlist, sep)

    outlist = field("implicit", 0)
    if outlist:
        if Verbose:
            print "    implicit:"
        print "        " + outlist

def printentries(entries):
    if Print_Entries:
        for name in Print_Entries:
            try:
                entry = entries[name]
            except KeyError:
                sys.stderr.write("sconsign: no entry `%s' in `%s'\n" % (name, args[0]))
            else:
                printfield(name, entry)
    else:
        for name, e in entries.items():
            printfield(name, e)

class Do_SConsignDB:
    def __init__(self, dbm_name, dbm):
        self.dbm_name = dbm_name
        self.dbm = dbm

    def __call__(self, fname):
        # The *dbm modules stick their own file suffixes on the names
        # that are passed in.  This is causes us to jump through some
        # hoops here to be able to allow the user
        try:
            # Try opening the specified file name.  Example:
            #   SPECIFIED                  OPENED BY self.dbm.open()
            #   ---------                  -------------------------
            #   .sconsign               => .sconsign.dblite
            #   .sconsign.dblite        => .sconsign.dblite.dblite
            db = self.dbm.open(fname, "r")
        except (IOError, OSError), e:
            print_e = e
            try:
                # That didn't work, so try opening the base name,
                # so that if the actually passed in 'sconsign.dblite'
                # (for example), the dbm module will put the suffix back
                # on for us and open it anyway.
                db = self.dbm.open(os.path.splitext(fname)[0], "r")
            except (IOError, OSError):
                # That didn't work either.  See if the file name
                # they specified just exists (independent of the dbm
                # suffix-mangling).
                try:
                    open(fname, "r")
                except (IOError, OSError), e:
                    # Nope, that file doesn't even exist, so report that
                    # fact back.
                    print_e = e
                sys.stderr.write("sconsign: %s\n" % (print_e))
                return
        except:
            sys.stderr.write("sconsign: ignoring invalid `%s' file `%s'\n" % (self.dbm_name, fname))
            return

        if Print_Directories:
            for dir in Print_Directories:
                try:
                    val = db[dir]
                except KeyError:
                    sys.stderr.write("sconsign: no dir `%s' in `%s'\n" % (dir, args[0]))
                else:
                    self.printentries(dir, val)
        else:
            keys = db.keys()
            keys.sort()
            for dir in keys:
                self.printentries(dir, db[dir])

    def printentries(self, dir, val):
        print '=== ' + dir + ':'
        printentries(cPickle.loads(val))

def Do_SConsignDir(name):
    try:
        fp = open(name, 'rb')
    except (IOError, OSError), e:
        sys.stderr.write("sconsign: %s\n" % (e))
        return
    try:
        sconsign = SCons.SConsign.Dir(fp)
    except:
        sys.stderr.write("sconsign: ignoring invalid .sconsign file `%s'\n" % name)
        return
    printentries(sconsign.entries)

##############################################################################

import getopt

helpstr = """\
Usage: sconsign [OPTIONS] FILE [...]
Options:
  -b, --bsig                  Print build signature information.
  -c, --csig                  Print content signature information.
  -d DIR, --dir=DIR           Print only info about DIR.
  -e ENTRY, --entry=ENTRY     Print only info about ENTRY.
  -f FORMAT, --format=FORMAT  FILE is in the specified FORMAT.
  -h, --help                  Print this message and exit.
  -i, --implicit              Print implicit dependency information.
  -r, --readable              Print timestamps in human-readable form.
  -t, --timestamp             Print timestamp information.
  -v, --verbose               Verbose, describe each field.
"""

opts, args = getopt.getopt(sys.argv[1:], "bcd:e:f:hirtv",
                            ['bsig', 'csig', 'dir=', 'entry=',
                             'format=', 'help', 'implicit',
                             'readable', 'timestamp', 'verbose'])


for o, a in opts:
    if o in ('-b', '--bsig'):
        Print_Flags['bsig'] = 1
    elif o in ('-c', '--csig'):
        Print_Flags['csig'] = 1
    elif o in ('-d', '--dir'):
        Print_Directories.append(a)
    elif o in ('-e', '--entry'):
        Print_Entries.append(a)
    elif o in ('-f', '--format'):
        Module_Map = {'dblite'   : 'SCons.dblite',
                      'sconsign' : None}
        dbm_name = Module_Map.get(a, a)
        if dbm_name:
            try:
                dbm = my_import(dbm_name)
            except:
                sys.stderr.write("sconsign: illegal file format `%s'\n" % a)
                print helpstr
                sys.exit(2)
            Do_Call = Do_SConsignDB(a, dbm)
        else:
            Do_Call = Do_SConsignDir
    elif o in ('-h', '--help'):
        print helpstr
        sys.exit(0)
    elif o in ('-i', '--implicit'):
        Print_Flags['implicit'] = 1
    elif o in ('-r', '--readable'):
        Readable = 1
    elif o in ('-t', '--timestamp'):
        Print_Flags['timestamp'] = 1
    elif o in ('-v', '--verbose'):
        Verbose = 1

if Do_Call:
    for a in args:
        Do_Call(a)
else:
    for a in args:
        dbm_name = whichdb.whichdb(a)
        if dbm_name:
            Map_Module = {'SCons.dblite' : 'dblite'}
            dbm = my_import(dbm_name)
            Do_SConsignDB(Map_Module.get(dbm_name, dbm_name), dbm)(a)
        else:
            Do_SConsignDir(a)

sys.exit(0)
