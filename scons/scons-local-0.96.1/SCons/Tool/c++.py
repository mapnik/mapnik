"""SCons.Tool.c++

Tool-specific initialization for generic Posix C++ compilers.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.
"""

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/c++.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os.path

import SCons.Tool
import SCons.Defaults
import SCons.Util

compilers = ['CC', 'c++']

CXXSuffixes = ['.cpp', '.cc', '.cxx', '.c++', '.C++']
if SCons.Util.case_sensitive_suffixes('.c', '.C'):
    CXXSuffixes.append('.C')

def iscplusplus(source):
    if not source:
        # Source might be None for unusual cases like SConf.
        return 0
    for s in source:
        if s.sources:
            ext = os.path.splitext(str(s.sources[0]))[1]
            if ext in CXXSuffixes:
                return 1
    return 0

def generate(env):
    """
    Add Builders and construction variables for Visual Age C++ compilers
    to an Environment.
    """
    static_obj, shared_obj = SCons.Tool.createObjBuilders(env)

    for suffix in CXXSuffixes:
        static_obj.add_action(suffix, SCons.Defaults.CXXAction)
        shared_obj.add_action(suffix, SCons.Defaults.ShCXXAction)
        static_obj.add_emitter(suffix, SCons.Defaults.StaticObjectEmitter)
        shared_obj.add_emitter(suffix, SCons.Defaults.SharedObjectEmitter)
        
    env['CXX']        = 'c++'
    env['CXXFLAGS']   = SCons.Util.CLVar('$CCFLAGS')
    env['CXXCOM']     = '$CXX $CXXFLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS -c -o $TARGET $SOURCES'
    env['SHCXX']      = '$CXX'
    env['SHCXXFLAGS'] = SCons.Util.CLVar('$CXXFLAGS')
    env['SHCXXCOM']   = '$SHCXX $SHCXXFLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS -c -o $TARGET $SOURCES'

    env['CPPDEFPREFIX']  = '-D'
    env['CPPDEFSUFFIX']  = ''
    env['INCPREFIX']  = '-I'
    env['INCSUFFIX']  = ''
    env['SHOBJSUFFIX'] = '.os'
    env['OBJSUFFIX'] = '.o'
    env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 0

    env['CXXFILESUFFIX'] = '.cc'

def exists(env):
    return env.Detect(compilers)
