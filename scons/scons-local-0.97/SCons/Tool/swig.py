"""SCons.Tool.swig

Tool-specific initialization for swig.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 The SCons Foundation
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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/swig.py 0.97.D001 2007/05/17 11:35:19 knight"

import SCons.Action
import SCons.Defaults
import SCons.Tool
import SCons.Util
from SCons.Scanner import Scanner
import os
import re

SwigAction = SCons.Action.Action('$SWIGCOM', '$SWIGCOMSTR')

def swigSuffixEmitter(env, source):
    if '-c++' in SCons.Util.CLVar(env.subst("$SWIGFLAGS")):
        return '$SWIGCXXFILESUFFIX'
    else:
        return '$SWIGCFILESUFFIX'

_reInclude = re.compile(r'%include\s+(\S+)')
_reModule = re.compile(r'%module\s+(.+)')

def recurse(path, searchPath):
    global _reInclude
    f = open(path)
    try: contents = f.read()
    finally: f.close()

    found = []
    # Better code for when we drop Python 1.5.2.
    #for m in _reInclude.finditer(contents):
    #    fname = m.group(1)
    for fname in _reInclude.findall(contents):
        for dpath in searchPath:
            absPath = os.path.join(dpath, fname)
            if os.path.isfile(absPath):
                found.append(absPath)
                break

    # Equivalent code for when we drop Python 1.5.2.
    #for f in [f for f in found if os.path.splitext(f)[1] == ".i"]:
    #    found += recurse(f, searchPath)
    for f in filter(lambda f: os.path.splitext(f)[1] == ".i", found):
        found = found + recurse(f, searchPath)
    return found

def _scanSwig(node, env, path):
    r = recurse(str(node), [os.path.abspath(os.path.dirname(str(node))), os.path.abspath(os.path.join("include", "swig"))])
    return r

def _swigEmitter(target, source, env):
    for src in source:
        src = str(src)
        mname = None
        flags = SCons.Util.CLVar(env.subst("$SWIGFLAGS"))
        if "-python" in flags and "-noproxy" not in flags:
            f = open(src)
            try:
                for l in f.readlines():
                    m = _reModule.match(l)
                    if m:
                        mname = m.group(1)
            finally:
                f.close()
            if mname is not None:
                target.append(mname + ".py")
    return (target, source)

def generate(env):
    """Add Builders and construction variables for swig to an Environment."""
    c_file, cxx_file = SCons.Tool.createCFileBuilders(env)

    c_file.suffix['.i'] = swigSuffixEmitter
    cxx_file.suffix['.i'] = swigSuffixEmitter

    c_file.add_action('.i', SwigAction)
    c_file.add_emitter('.i', _swigEmitter)
    cxx_file.add_action('.i', SwigAction)
    cxx_file.add_emitter('.i', _swigEmitter)

    env['SWIG']              = 'swig'
    env['SWIGFLAGS']         = SCons.Util.CLVar('')
    env['SWIGCFILESUFFIX']   = '_wrap$CFILESUFFIX'
    env['SWIGCXXFILESUFFIX'] = '_wrap$CXXFILESUFFIX'
    env['SWIGCOM']           = '$SWIG $SWIGFLAGS -o $TARGET $SOURCES'
    env.Append(SCANNERS=Scanner(function=_scanSwig, skeys=[".i"]))

def exists(env):
    return env.Detect(['swig'])
