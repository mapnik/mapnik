"""SCons.Scanner

The Scanner package for the SCons software construction utility.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Scanner/__init__.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import re

import SCons.Node.FS
import SCons.Sig
import SCons.Util


class _Null:
    pass

# This is used instead of None as a default argument value so None can be
# used as an actual argument value.
_null = _Null

def Scanner(function, *args, **kw):
    """Public interface factory function for creating different types
    of Scanners based on the different types of "functions" that may
    be supplied."""
    if SCons.Util.is_Dict(function):
        return apply(Selector, (function,) + args, kw)
    else:
        return apply(Base, (function,) + args, kw)

class FindPathDirs:
    """A class to bind a specific *PATH variable name and the fs object
    to a function that will return all of the *path directories."""
    def __init__(self, variable, fs):
        self.variable = variable
        self.fs = fs
    def __call__(self, env, dir, argument=None):
        try:
            path = env[self.variable]
        except KeyError:
            return ()

        return tuple(self.fs.Rsearchall(env.subst_path(path),
                                        must_exist = 0,
                                        clazz = SCons.Node.FS.Dir,
                                        cwd = dir))

class Base:
    """
    The base class for dependency scanners.  This implements
    straightforward, single-pass scanning of a single file.
    """

    def __init__(self,
                 function,
                 name = "NONE",
                 argument = _null,
                 skeys = [],
                 path_function = None,
                 node_class = SCons.Node.FS.Entry,
                 node_factory = SCons.Node.FS.default_fs.File,
                 scan_check = None,
                 recursive = None):
        """
        Construct a new scanner object given a scanner function.

        'function' - a scanner function taking two or three
        arguments and returning a list of strings.

        'name' - a name for identifying this scanner object.

        'argument' - an optional argument that, if specified, will be
        passed to both the scanner function and the path_function.

        'skeys' - an optional list argument that can be used to determine
        which scanner should be used for a given Node. In the case of File
        nodes, for example, the 'skeys' would be file suffixes.

        'path_function' - a function that takes one to three arguments
        (a construction environment, optional directory, and optional
        argument for this instance) and returns a tuple of the
        directories that can be searched for implicit dependency files.

        'node_class' - the class of Nodes which this scan will return.
        If node_class is None, then this scanner will not enforce any
        Node conversion and will return the raw results from the
        underlying scanner function.

        'node_factory' - the factory function to be called to translate
        the raw results returned by the scanner function into the
        expected node_class objects.

        'scan_check' - a function to be called to first check whether
        this node really needs to be scanned.

        'recursive' - specifies that this scanner should be invoked
        recursively on the implicit dependencies it returns (the
        canonical example being #include lines in C source files).

        The scanner function's first argument will be the name of a file
        that should be scanned for dependencies, the second argument will
        be an Environment object, the third argument will be the value
        passed into 'argument', and the returned list should contain the
        Nodes for all the direct dependencies of the file.

        Examples:

        s = Scanner(my_scanner_function)

        s = Scanner(function = my_scanner_function)

        s = Scanner(function = my_scanner_function, argument = 'foo')

        """

        # Note: this class could easily work with scanner functions that take
        # something other than a filename as an argument (e.g. a database
        # node) and a dependencies list that aren't file names. All that
        # would need to be changed is the documentation.

        self.function = function
        self.path_function = path_function
        self.name = name
        self.argument = argument
        self.skeys = skeys
        self.node_class = node_class
        self.node_factory = node_factory
        self.scan_check = scan_check
        self.recursive = recursive

    def path(self, env, dir = None):
        if not self.path_function:
            return ()
        if not self.argument is _null:
            return self.path_function(env, dir, self.argument)
        else:
            return self.path_function(env, dir)

    def __call__(self, node, env, path = ()):
        """
        This method scans a single object. 'node' is the node
        that will be passed to the scanner function, and 'env' is the
        environment that will be passed to the scanner function. A list of
        direct dependency nodes for the specified node will be returned.
        """
        if self.scan_check and not self.scan_check(node, env):
            return []

        if not self.argument is _null:
            list = self.function(node, env, path, self.argument)
        else:
            list = self.function(node, env, path)
        kw = {}
        if hasattr(node, 'dir'):
            kw['directory'] = node.dir
        nodes = []
        for l in list:
            if self.node_class and not isinstance(l, self.node_class):
                l = apply(self.node_factory, (l,), kw)
            nodes.append(l)
        return nodes

    def __cmp__(self, other):
        return cmp(self.__dict__, other.__dict__)

    def __hash__(self):
        return hash(repr(self))

    def add_skey(self, skey):
        """Add a skey to the list of skeys"""
        self.skeys.append(skey)

    def get_skeys(self, env=None):
        if SCons.Util.is_String(self.skeys):
            return env.subst_list(self.skeys)[0]
        return self.skeys

    def select(self, node):
        return self


class Selector(Base):
    """
    A class for selecting a more specific scanner based on the
    scanner_key() (suffix) for a specific Node.
    """
    def __init__(self, dict, *args, **kw):
        Base.__init__(self, (None,)+args, kw)
        self.dict = dict

    def __call__(self, node, env, path = ()):
        return self.select(node)(node, env, path)

    def select(self, node):
        try:
            return self.dict[node.scanner_key()]
        except KeyError:
            return None

    def add_scanner(self, skey, scanner):
        self.dict[skey] = scanner


class Current(Base):
    """
    A class for scanning files that are source files (have no builder)
    or are derived files and are current (which implies that they exist,
    either locally or in a repository).
    """

    def __init__(self, *args, **kw):
        def current_check(node, env):
            c = not node.has_builder() or node.current(env.get_calculator())
            return c
        kw['scan_check'] = current_check
        apply(Base.__init__, (self,) + args, kw)

class Classic(Current):
    """
    A Scanner subclass to contain the common logic for classic CPP-style
    include scanning, but which can be customized to use different
    regular expressions to find the includes.

    Note that in order for this to work "out of the box" (without
    overriding the find_include() method), the regular expression passed
    to the constructor must return the name of the include file in group
    0.
    """

    def __init__(self, name, suffixes, path_variable, regex,
                 fs=SCons.Node.FS.default_fs, *args, **kw):

        self.cre = re.compile(regex, re.M)
        self.fs = fs

        def _scan(node, env, path, self=self, fs=fs):
            return self.scan(node, env, path)

        kw['function'] = _scan
        kw['path_function'] = FindPathDirs(path_variable, fs)
        kw['recursive'] = 1
        kw['skeys'] = suffixes
        kw['name'] = name

        apply(Current.__init__, (self,) + args, kw)

    def find_include(self, include, source_dir, path):
        n = SCons.Node.FS.find_file(include, (source_dir,) + path, self.fs.File)
        return n, include

    def scan(self, node, env, path=()):
        node = node.rfile()

        if not node.exists():
            return []

        # cache the includes list in node so we only scan it once:
        if node.includes != None:
            includes = node.includes
        else:
            includes = self.cre.findall(node.get_contents())
            node.includes = includes

        nodes = []
        source_dir = node.get_dir()
        for include in includes:
            n, i = self.find_include(include, source_dir, path)

            if not n is None:
                nodes.append(n)
            else:
                SCons.Warnings.warn(SCons.Warnings.DependencyWarning,
                                    "No dependency generated for file: %s (included from: %s) -- file not found" % (i, node))

        # Schwartzian transform from the Python FAQ Wizard
        def st(List, Metric):
            def pairing(element, M = Metric):
                return (M(element), element)
            def stripit(pair):
                return pair[1]
            paired = map(pairing, List)
            paired.sort()
            return map(stripit, paired)

        def normalize(node):
            # We don't want the order of includes to be
            # modified by case changes on case insensitive OSes, so
            # normalize the case of the filename here:
            # (see test/win32pathmadness.py for a test of this)
            return SCons.Node.FS._my_normcase(str(node))

        transformed = st(nodes, normalize)
        # print "Classic: " + str(node) + " => " + str(map(lambda x: str(x),list(transformed)))
        return transformed

class ClassicCPP(Classic):
    """
    A Classic Scanner subclass which takes into account the type of
    bracketing used to include the file, and uses classic CPP rules
    for searching for the files based on the bracketing.

    Note that in order for this to work, the regular expression passed
    to the constructor must return the leading bracket in group 0, and
    the contained filename in group 1.
    """
    def find_include(self, include, source_dir, path):
        if include[0] == '"':
            n = SCons.Node.FS.find_file(include[1],
                                        (source_dir,) + path,
                                        self.fs.File)
        else:
            n = SCons.Node.FS.find_file(include[1],
                                        path + (source_dir,),
                                        self.fs.File)
        return n, include[1]
