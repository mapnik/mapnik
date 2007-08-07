
"""scons.Node.Alias

Alias nodes.

This creates a hash of global Aliases (dummy targets).

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Node/Alias.py 0.97.D001 2007/05/17 11:35:19 knight"

import string
import UserDict

import SCons.Errors
import SCons.Node
import SCons.Util

class AliasNameSpace(UserDict.UserDict):
    def Alias(self, name, **kw):
        if isinstance(name, SCons.Node.Alias.Alias):
            return name
        try:
            a = self[name]
        except KeyError:
            a = apply(SCons.Node.Alias.Alias, (name,), kw)
            self[name] = a
        return a

    def lookup(self, name, **kw):
        try:
            return self[name]
        except KeyError:
            return None

class AliasNodeInfo(SCons.Node.NodeInfoBase):
    pass

class AliasBuildInfo(SCons.Node.BuildInfoBase):
    pass

class Alias(SCons.Node.Node):

    NodeInfo = AliasNodeInfo
    BuildInfo = AliasBuildInfo

    def __init__(self, name):
        SCons.Node.Node.__init__(self)
        self.name = name

    def __str__(self):
        return self.name

    really_build = SCons.Node.Node.build
    current = SCons.Node.Node.children_are_up_to_date

    def is_under(self, dir):
        # Make Alias nodes get built regardless of
        # what directory scons was run from. Alias nodes
        # are outside the filesystem:
        return 1

    def get_contents(self):
        """The contents of an alias is the concatenation
        of all the contents of its sources"""
        contents = map(lambda n: n.get_contents(), self.children())
        return string.join(contents, '')

    def sconsign(self):
        """An Alias is not recorded in .sconsign files"""
        pass

    #
    #
    #

    def build(self):
        """A "builder" for aliases."""
        pass

    def convert(self):
        try: del self.builder
        except AttributeError: pass
        self.reset_executor()
        self.build = self.really_build

default_ans = AliasNameSpace()

SCons.Node.arg2nodes_lookups.append(default_ans.lookup)
