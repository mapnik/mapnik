"""SCons.Sig.MD5

The MD5 signature package for the SCons software construction
utility.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Sig/MD5.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import imp
import string

# Force Python to load the builtin "md5" module.  If we do this with a
# normal import statement, then case-insensitive systems (Win32) get
# confused and thinks there's a case mismatch with *this* MD5.py module.
file, name, desc = imp.find_module('md5')
try:
    md5 = imp.load_module('md5', file, name, desc)
finally:
    if file:
        file.close()

def current(new, old):
    """Return whether a new signature is up-to-date with
    respect to an old signature.
    """
    return new == old

def hexdigest(s):
    """Return a signature as a string of hex characters.
    """
    # NOTE:  This routine is a method in the Python 2.0 interface
    # of the native md5 module, but we want SCons to operate all
    # the way back to at least Python 1.5.2, which doesn't have it.
    h = string.hexdigits
    r = ''
    for c in s:
	i = ord(c)
	r = r + h[(i >> 4) & 0xF] + h[i & 0xF]
    return r

def collect(signatures):
    """
    Collect a list of signatures into an aggregate signature.

    signatures - a list of signatures
    returns - the aggregate signature
    """
    if len(signatures) == 1:
	return signatures[0]
    else:
        contents = string.join(signatures, ', ')
	return hexdigest(md5.new(contents).digest())

def signature(obj):
    """Generate a signature for an object
    """
    try:
        gc = obj.get_contents
    except AttributeError:
        raise AttributeError, "unable to fetch contents of '%s'" % str(obj)
    return hexdigest(md5.new(str(gc())).digest())

def to_string(signature):
    """Convert a signature to a string"""
    return signature

def from_string(string):
    """Convert a string to a signature"""
    return string
