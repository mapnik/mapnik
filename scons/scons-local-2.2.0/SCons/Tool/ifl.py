"""SCons.Tool.ifl

Tool-specific initialization for the Intel Fortran compiler.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 The SCons Foundation
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

__revision__ = "src/engine/SCons/Tool/ifl.py issue-2856:2676:d23b7a2f45e8 2012/08/05 15:38:28 garyo"

import SCons.Defaults
from SCons.Scanner.Fortran import FortranScan
from FortranCommon import add_all_to_env

def generate(env):
    """Add Builders and construction variables for ifl to an Environment."""
    fscan = FortranScan("FORTRANPATH")
    SCons.Tool.SourceFileScanner.add_scanner('.i', fscan)
    SCons.Tool.SourceFileScanner.add_scanner('.i90', fscan)

    if 'FORTRANFILESUFFIXES' not in env:
        env['FORTRANFILESUFFIXES'] = ['.i']
    else:
        env['FORTRANFILESUFFIXES'].append('.i')

    if 'F90FILESUFFIXES' not in env:
        env['F90FILESUFFIXES'] = ['.i90']
    else:
        env['F90FILESUFFIXES'].append('.i90')

    add_all_to_env(env)

    env['FORTRAN']        = 'ifl'
    env['SHFORTRAN']      = '$FORTRAN'
    env['FORTRANCOM']     = '$FORTRAN $FORTRANFLAGS $_FORTRANINCFLAGS /c $SOURCES /Fo$TARGET'
    env['FORTRANPPCOM']   = '$FORTRAN $FORTRANFLAGS $CPPFLAGS $_CPPDEFFLAGS $_FORTRANINCFLAGS /c $SOURCES /Fo$TARGET'
    env['SHFORTRANCOM']   = '$SHFORTRAN $SHFORTRANFLAGS $_FORTRANINCFLAGS /c $SOURCES /Fo$TARGET'
    env['SHFORTRANPPCOM'] = '$SHFORTRAN $SHFORTRANFLAGS $CPPFLAGS $_CPPDEFFLAGS $_FORTRANINCFLAGS /c $SOURCES /Fo$TARGET'

def exists(env):
    return env.Detect('ifl')

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
