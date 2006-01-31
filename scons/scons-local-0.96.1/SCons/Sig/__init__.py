"""SCons.Sig

The Signature package for the scons software construction utility.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Sig/__init__.py 0.96.1.D001 2004/08/23 09:55:29 knight"

try:
    import MD5
    default_module = MD5
except ImportError:
    import TimeStamp
    default_module = TimeStamp

# XXX We should move max_drift into Node/FS.py,
# since it's really something about files.
default_max_drift = 2*24*60*60

class SConsignEntry:
    """The old SConsignEntry format.
    We keep this around to handle conversions from old .sconsign files."""
    timestamp = None
    bsig = None
    csig = None
    implicit = None

class Calculator:
    """
    Encapsulates signature calculations and .sconsign file generating
    for the build engine.
    """

    def __init__(self, module=default_module, max_drift=default_max_drift):
        """
        Initialize the calculator.

        module - the signature module to use for signature calculations
        max_drift - the maximum system clock drift used to determine when to
          cache content signatures. A negative value means to never cache
          content signatures. (defaults to 2 days)
        """
        self.module = module
        self.max_drift = max_drift

default_calc = Calculator()
