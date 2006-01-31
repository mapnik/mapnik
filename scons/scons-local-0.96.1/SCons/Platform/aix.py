"""engine.SCons.Platform.aix

Platform-specific initialization for IBM AIX systems.

There normally shouldn't be any need to import this module directly.  It
will usually be imported through the generic SCons.Platform.Platform()
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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Platform/aix.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os
import string

import posix

def get_xlc(env, xlc, xlc_r, packages):
    # Use the AIX package installer tool lslpp to figure out where a
    # given xl* compiler is installed and what version it is.
    xlcPath = None
    xlcVersion = None

    xlc = env.get('CC', 'xlc')
    for package in packages:
        cmd = "lslpp -fc " + package + " 2>/dev/null | egrep '" + xlc + "([^-_a-zA-Z0-9].*)?$'"
        line = os.popen(cmd).readline()
        if line:
            v, p = string.split(line, ':')[1:3]
            xlcVersion = string.split(v)[1]
            xlcPath = string.split(p)[0]
            xlcPath = xlcPath[:xlcPath.rindex('/')]
            break
    return (xlcPath, xlc, xlc_r, xlcVersion)

def generate(env):
    posix.generate(env)
