"""SCons.Tool.BitKeeper.py

Tool-specific initialization for the BitKeeper source code control
system.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/BitKeeper.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os.path

import SCons.Builder
import SCons.Util

def generate(env):
    """Add a Builder factory function and construction variables for
    BitKeeper to an Environment."""

    def BitKeeperFactory(env=env):
        """ """
        return SCons.Builder.Builder(action = "$BITKEEPERCOM", env = env)

    #setattr(env, 'BitKeeper', BitKeeperFactory)
    env.BitKeeper = BitKeeperFactory

    env['BITKEEPER']         = 'bk'
    env['BITKEEPERGET']      = '$BITKEEPER get'
    env['BITKEEPERGETFLAGS'] = SCons.Util.CLVar('')
    env['BITKEEPERCOM']      = '$BITKEEPERGET $BITKEEPERGETFLAGS $TARGET'

def exists(env):
    return env.Detect('bk')
