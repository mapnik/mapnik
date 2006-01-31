"""engine.SCons.Options.PathOption

This file defines an option type for SCons implementing 'package
activation'.

To be used whenever a 'package' may be enabled/disabled and the
package path may be specified.

Usage example:

  Examples:
      x11=no   (disables X11 support)
      x11=yes  (will search for the package installation dir)
      x11=/usr/local/X11 (will check this path for existance)

  To replace autoconf's --with-xxx=yyy 

  opts = Options()

  opts = Options()
  opts.Add(PathOption('qtdir',
                      'where the root of Qt is installed',
                      qtdir))
  opts.Add(PathOption('qt_includes',
                      'where the Qt includes are installed',
                      '$qtdir/includes'))
  opts.Add(PathOption('qt_libraries',
                      'where the Qt library is installed',
                      '$qtdir/lib'))

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Options/PathOption.py 0.96.1.D001 2004/08/23 09:55:29 knight"

__all__ = ('PathOption',)

import os

import SCons.Errors

def _validator(key, val, env):
    """
    """
    # todo: write validator, check for path
    if not os.path.exists(val):
        raise SCons.Errors.UserError(
            'Path does not exist for option %s: %s' % (key, val))


def PathOption(key, help, default):
    # NB: searchfunc is currenty undocumented and unsupported
    """
    The input parameters describe a 'path list' option, thus they
    are returned with the correct converter and validator appended. The
    result is usable for input to opts.Add() .

    A 'package list' option may either be 'all', 'none' or a list of
    package names (seperated by space).
    """
    return (key, '%s ( /path/to/%s )' % (help, key), default,
            _validator, None)

