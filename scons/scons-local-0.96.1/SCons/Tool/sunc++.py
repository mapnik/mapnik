"""SCons.Tool.sunc++

Tool-specific initialization for C++ on SunOS / Solaris.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/sunc++.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os.path

cplusplus = __import__('c++', globals(), locals(), [])

# use the package installer tool lslpp to figure out where cppc and what
# version of it is installed
def get_cppc(env):
    cppcPath = env.get('CXX', None)
    cppcVersion = None

    for package in ['SPROcpl']:
        cmd = "pkginfo -l " + package + " 2>/dev/null | grep '^ *VERSION:'"
        line = os.popen(cmd).readline()
        if line:
            cppcVersion = line.split()[-1]
            cmd = "pkgchk -l " + package + " | grep '^Pathname:.*/bin/CC$' | grep -v '/SC[0-9]*\.[0-9]*/'"
            line = os.popen(cmd).readline()
            cppcPath = os.path.dirname(line.split()[-1])
            break
    return (cppcPath, 'CC', 'CC', cppcVersion)

def generate(env):
    """Add Builders and construction variables for SUN PRO C++ to an Environment."""
    path, cxx, shcxx, version = get_cppc(env)
    if path:
        cxx = os.path.join(path, cxx)
        shcxx = os.path.join(path, shcxx)

    cplusplus.generate(env)

    env['CXX'] = cxx
    env['SHCXX'] = shcxx
    env['CXXVERSION'] = version
    env['SHOBJSUFFIX'] = '.os'
    
def exists(env):
    path, cxx, shcxx, version = get_cppc(env)
    if path and cxx:
        cppc = os.path.join(path, cxx)
        if os.path.exists(cppc):
            return cppc
    return None
