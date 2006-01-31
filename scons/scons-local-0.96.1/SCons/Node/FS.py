"""scons.Node.FS

File system nodes.

These Nodes represent the canonical external objects that people think
of when they think of building software: files and directories.

This initializes a "default_fs" Node with an FS at the current directory
for its own purposes, and for use by scripts or modules looking for the
canonical default.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Node/FS.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os
import os.path
import shutil
import stat
import string
import sys
import time
import cStringIO

import SCons.Action
from SCons.Debug import logInstanceCreation
import SCons.Errors
import SCons.Node
import SCons.Sig.MD5
import SCons.Util
import SCons.Warnings

#
# We stringify these file system Nodes a lot.  Turning a file system Node
# into a string is non-trivial, because the final string representation
# can depend on a lot of factors:  whether it's a derived target or not,
# whether it's linked to a repository or source directory, and whether
# there's duplication going on.  The normal technique for optimizing
# calculations like this is to memoize (cache) the string value, so you
# only have to do the calculation once.
#
# A number of the above factors, however, can be set after we've already
# been asked to return a string for a Node, because a Repository() or
# BuildDir() call or the like may not occur until later in SConscript
# files.  So this variable controls whether we bother trying to save
# string values for Nodes.  The wrapper interface can set this whenever
# they're done mucking with Repository and BuildDir and the other stuff,
# to let this module know it can start returning saved string values
# for Nodes.
#
Save_Strings = None

def save_strings(val):
    global Save_Strings
    Save_Strings = val

#
# SCons.Action objects for interacting with the outside world.
#
# The Node.FS methods in this module should use these actions to
# create and/or remove files and directories; they should *not* use
# os.{link,symlink,unlink,mkdir}(), etc., directly.
#
# Using these SCons.Action objects ensures that descriptions of these
# external activities are properly displayed, that the displays are
# suppressed when the -s (silent) option is used, and (most importantly)
# the actions are disabled when the the -n option is used, in which case
# there should be *no* changes to the external file system(s)...
#

def _copy_func(src, dest):
    shutil.copy2(src, dest)
    st=os.stat(src)
    os.chmod(dest, stat.S_IMODE(st[stat.ST_MODE]) | stat.S_IWRITE)

Valid_Duplicates = ['hard-soft-copy', 'soft-hard-copy',
                    'hard-copy', 'soft-copy', 'copy']

Link_Funcs = [] # contains the callables of the specified duplication style

def set_duplicate(duplicate):
    # Fill in the Link_Funcs list according to the argument
    # (discarding those not available on the platform).

    # Set up the dictionary that maps the argument names to the
    # underlying implementations.  We do this inside this function,
    # not in the top-level module code, so that we can remap os.link
    # and os.symlink for testing purposes.
    try:
        _hardlink_func = os.link
    except AttributeError:
        _hardlink_func = None

    try:
        _softlink_func = os.symlink
    except AttributeError:
        _softlink_func = None

    link_dict = {
        'hard' : _hardlink_func,
        'soft' : _softlink_func,
        'copy' : _copy_func
    }

    if not duplicate in Valid_Duplicates:
        raise SCons.Errors.InternalError, ("The argument of set_duplicate "
                                           "should be in Valid_Duplicates")
    global Link_Funcs
    Link_Funcs = []
    for func in string.split(duplicate,'-'):
        if link_dict[func]:
            Link_Funcs.append(link_dict[func])

def LinkFunc(target, source, env):
    # Relative paths cause problems with symbolic links, so
    # we use absolute paths, which may be a problem for people
    # who want to move their soft-linked src-trees around. Those
    # people should use the 'hard-copy' mode, softlinks cannot be
    # used for that; at least I have no idea how ...
    src = source[0].abspath
    dest = target[0].abspath
    dir, file = os.path.split(dest)
    if dir and not os.path.isdir(dir):
        os.makedirs(dir)
    if not Link_Funcs:
        # Set a default order of link functions.
        set_duplicate('hard-soft-copy')
    # Now link the files with the previously specified order.
    for func in Link_Funcs:
        try:
            func(src,dest)
            break
        except OSError:
            if func == Link_Funcs[-1]:
                # exception of the last link method (copy) are fatal
                raise
            else:
                pass
    return 0

Link = SCons.Action.Action(LinkFunc, None)
def LocalString(target, source, env):
    return 'Local copy of %s from %s' % (target[0], source[0])

LocalCopy = SCons.Action.Action(LinkFunc, LocalString)

def UnlinkFunc(target, source, env):
    t = target[0]
    t.fs.unlink(t.abspath)
    return 0

Unlink = SCons.Action.Action(UnlinkFunc, None)

def MkdirFunc(target, source, env):
    t = target[0]
    p = t.abspath
    if not t.fs.exists(p):
        t.fs.mkdir(p)
    return 0

Mkdir = SCons.Action.Action(MkdirFunc, None, presub=None)

MkdirBuilder = None

def get_MkdirBuilder():
    global MkdirBuilder
    if MkdirBuilder is None:
        import SCons.Builder
        # "env" will get filled in by Executor.get_build_env()
        # calling SCons.Defaults.DefaultEnvironment() when necessary.
        MkdirBuilder = SCons.Builder.Builder(action = Mkdir,
                                             env = None,
                                             explain = None)
    return MkdirBuilder

def CacheRetrieveFunc(target, source, env):
    t = target[0]
    fs = t.fs
    cachedir, cachefile = t.cachepath()
    if fs.exists(cachefile):
        fs.copy2(cachefile, t.path)
        st = fs.stat(cachefile)
        fs.chmod(t.path, stat.S_IMODE(st[stat.ST_MODE]) | stat.S_IWRITE)
        return 0
    return 1

def CacheRetrieveString(target, source, env):
    t = target[0]
    cachedir, cachefile = t.cachepath()
    if t.fs.exists(cachefile):
        return "Retrieved `%s' from cache" % t.path
    return None

CacheRetrieve = SCons.Action.Action(CacheRetrieveFunc, CacheRetrieveString)

CacheRetrieveSilent = SCons.Action.Action(CacheRetrieveFunc, None)

def CachePushFunc(target, source, env):
    t = target[0]
    fs = t.fs
    cachedir, cachefile = t.cachepath()
    if fs.exists(cachefile):
        # Don't bother copying it if it's already there.
        return

    if not fs.isdir(cachedir):
        fs.makedirs(cachedir)

    tempfile = cachefile+'.tmp'
    try:
        fs.copy2(t.path, tempfile)
        fs.rename(tempfile, cachefile)
        st = fs.stat(t.path)
        fs.chmod(cachefile, stat.S_IMODE(st[stat.ST_MODE]) | stat.S_IWRITE)
    except OSError:
        # It's possible someone else tried writing the file at the same
        # time we did.  Print a warning but don't stop the build, since
        # it doesn't affect the correctness of the build.
        SCons.Warnings.warn(SCons.Warnings.CacheWriteErrorWarning,
                            "Unable to copy %s to cache. Cache file is %s"
                                % (str(target), cachefile))
        return

CachePush = SCons.Action.Action(CachePushFunc, None)

class _Null:
    pass

_null = _Null()

DefaultSCCSBuilder = None
DefaultRCSBuilder = None

def get_DefaultSCCSBuilder():
    global DefaultSCCSBuilder
    if DefaultSCCSBuilder is None:
        import SCons.Builder
        # "env" will get filled in by Executor.get_build_env()
        # calling SCons.Defaults.DefaultEnvironment() when necessary.
        DefaultSCCSBuilder = SCons.Builder.Builder(action = '$SCCSCOM',
                                                   env = None)
    return DefaultSCCSBuilder

def get_DefaultRCSBuilder():
    global DefaultRCSBuilder
    if DefaultRCSBuilder is None:
        import SCons.Builder
        # "env" will get filled in by Executor.get_build_env()
        # calling SCons.Defaults.DefaultEnvironment() when necessary.
        DefaultRCSBuilder = SCons.Builder.Builder(action = '$RCS_COCOM',
                                                  env = None)
    return DefaultRCSBuilder

#
class ParentOfRoot:
    """
    An instance of this class is used as the parent of the root of a
    filesystem (POSIX) or drive (Win32). This isn't actually a node,
    but it looks enough like one so that we don't have to have
    special purpose code everywhere to deal with dir being None. 
    This class is an instance of the Null object pattern.
    """
    def __init__(self):
        self.abspath = ''
        self.path = ''
        self.name=''
        self.duplicate=0
        self.srcdir=None
        self.build_dirs=[]
        
    def is_under(self, dir):
        return 0

    def up(self):
        return None

    def getRepositories(self):
        return []

    def get_dir(self):
        return None

    def src_builder(self):
        return _null

    def entry_abspath(self, name):
        return name

    def entry_path(self, name):
        return name

# Cygwin's os.path.normcase pretends it's on a case-sensitive filesystem.
_is_cygwin = sys.platform == "cygwin"
if os.path.normcase("TeSt") == os.path.normpath("TeSt") and not _is_cygwin:
    def _my_normcase(x):
        return x
else:
    def _my_normcase(x):
        return string.upper(x)

class EntryProxy(SCons.Util.Proxy):
    def __get_abspath(self):
        entry = self.get()
        return SCons.Util.SpecialAttrWrapper(entry.get_abspath(),
                                             entry.name + "_abspath")

    def __get_filebase(self):
        name = self.get().name
        return SCons.Util.SpecialAttrWrapper(SCons.Util.splitext(name)[0],
                                             name + "_filebase")

    def __get_suffix(self):
        name = self.get().name
        return SCons.Util.SpecialAttrWrapper(SCons.Util.splitext(name)[1],
                                             name + "_suffix")

    def __get_file(self):
        name = self.get().name
        return SCons.Util.SpecialAttrWrapper(name, name + "_file")

    def __get_base_path(self):
        """Return the file's directory and file name, with the
        suffix stripped."""
        entry = self.get()
        return SCons.Util.SpecialAttrWrapper(SCons.Util.splitext(entry.get_path())[0],
                                             entry.name + "_base")

    def __get_posix_path(self):
        """Return the path with / as the path separator,
        regardless of platform."""
        if os.sep == '/':
            return self
        else:
            entry = self.get()
            r = string.replace(entry.get_path(), os.sep, '/')
            return SCons.Util.SpecialAttrWrapper(r, entry.name + "_posix")

    def __get_win32_path(self):
        """Return the path with \ as the path separator,
        regardless of platform."""
        if os.sep == '\\':
            return self
        else:
            entry = self.get()
            r = string.replace(entry.get_path(), os.sep, '\\')
            return SCons.Util.SpecialAttrWrapper(r, entry.name + "_win32")

    def __get_srcnode(self):
        return EntryProxy(self.get().srcnode())

    def __get_srcdir(self):
        """Returns the directory containing the source node linked to this
        node via BuildDir(), or the directory of this node if not linked."""
        return EntryProxy(self.get().srcnode().dir)

    def __get_rsrcnode(self):
        return EntryProxy(self.get().srcnode().rfile())

    def __get_rsrcdir(self):
        """Returns the directory containing the source node linked to this
        node via BuildDir(), or the directory of this node if not linked."""
        return EntryProxy(self.get().srcnode().rfile().dir)

    def __get_dir(self):
        return EntryProxy(self.get().dir)
    
    dictSpecialAttrs = { "base"     : __get_base_path,
                         "posix"    : __get_posix_path,
                         "win32"    : __get_win32_path,
                         "srcpath"  : __get_srcnode,
                         "srcdir"   : __get_srcdir,
                         "dir"      : __get_dir,
                         "abspath"  : __get_abspath,
                         "filebase" : __get_filebase,
                         "suffix"   : __get_suffix,
                         "file"     : __get_file,
                         "rsrcpath" : __get_rsrcnode,
                         "rsrcdir"  : __get_rsrcdir,
                       }

    def __getattr__(self, name):
        # This is how we implement the "special" attributes
        # such as base, posix, srcdir, etc.
        try:
            return self.dictSpecialAttrs[name](self)
        except KeyError:
            try:
                attr = SCons.Util.Proxy.__getattr__(self, name)
            except AttributeError:
                entry = self.get()
                classname = string.split(str(entry.__class__), '.')[-1]
                raise AttributeError, "%s instance '%s' has no attribute '%s'" % (classname, entry.name, name)
            return attr

class Base(SCons.Node.Node):
    """A generic class for file system entries.  This class is for
    when we don't know yet whether the entry being looked up is a file
    or a directory.  Instances of this class can morph into either
    Dir or File objects by a later, more precise lookup.

    Note: this class does not define __cmp__ and __hash__ for
    efficiency reasons.  SCons does a lot of comparing of
    Node.FS.{Base,Entry,File,Dir} objects, so those operations must be
    as fast as possible, which means we want to use Python's built-in
    object identity comparisons.
    """

    def __init__(self, name, directory, fs):
        """Initialize a generic Node.FS.Base object.
        
        Call the superclass initialization, take care of setting up
        our relative and absolute paths, identify our parent
        directory, and indicate that this node should use
        signatures."""
        if __debug__: logInstanceCreation(self, 'Node.FS.Base')
        SCons.Node.Node.__init__(self)

        self.name = name
        self.fs = fs
        self.relpath = {self : '.'}

        assert directory, "A directory must be provided"

        self.abspath = directory.entry_abspath(name)
        if directory.path == '.':
            self.path = name
        else:
            self.path = directory.entry_path(name)

        self.dir = directory
        self.cwd = None # will hold the SConscript directory for target nodes
        self.duplicate = directory.duplicate

    def clear(self):
        """Completely clear a Node.FS.Base object of all its cached
        state (so that it can be re-evaluated by interfaces that do
        continuous integration builds).
        """
        SCons.Node.Node.clear(self)
        try:
            delattr(self, '_exists')
        except AttributeError:
            pass
        try:
            delattr(self, '_rexists')
        except AttributeError:
            pass
        try:
            delattr(self, '_str_val')
        except AttributeError:
            pass
        self.relpath = {self : '.'}

    def get_dir(self):
        return self.dir

    def get_suffix(self):
        return SCons.Util.splitext(self.name)[1]

    def rfile(self):
        return self

    def __str__(self):
        """A Node.FS.Base object's string representation is its path
        name."""
        try:
            return self._str_val
        except AttributeError:
            global Save_Strings
            if self.duplicate or self.is_derived():
                str_val = self.get_path()
            else:
                str_val = self.srcnode().get_path()
            if Save_Strings:
                self._str_val = str_val
            return str_val

    rstr = __str__

    def exists(self):
        try:
            return self._exists
        except AttributeError:
            self._exists = self.fs.exists(self.abspath)
            return self._exists

    def rexists(self):
        try:
            return self._rexists
        except AttributeError:
            self._rexists = self.rfile().exists()
            return self._rexists

    def is_under(self, dir):
        if self is dir:
            return 1
        else:
            return self.dir.is_under(dir)

    def set_local(self):
        self._local = 1

    def srcnode(self):
        """If this node is in a build path, return the node
        corresponding to its source file.  Otherwise, return
        ourself."""
        try:
            return self._srcnode
        except AttributeError:
            dir=self.dir
            name=self.name
            while dir:
                if dir.srcdir:
                    self._srcnode = self.fs.Entry(name, dir.srcdir,
                                                  klass=self.__class__)
                    return self._srcnode
                name = dir.name + os.sep + name
                dir=dir.get_dir()
            self._srcnode = self
            return self._srcnode

    def get_path(self, dir=None):
        """Return path relative to the current working directory of the
        Node.FS.Base object that owns us."""
        if not dir:
            dir = self.fs.getcwd()
        try:
            return self.relpath[dir]
        except KeyError:
            path_elems = []
            d = self
            while d != dir and not isinstance(d, ParentOfRoot):
                path_elems.append(d.name)
                d = d.dir
            path_elems.reverse()
            ret = string.join(path_elems, os.sep)
            self.relpath[dir] = ret
            return ret
            
    def set_src_builder(self, builder):
        """Set the source code builder for this node."""
        self.sbuilder = builder
        if not self.has_builder():
            self.builder_set(builder)

    def src_builder(self):
        """Fetch the source code builder for this node.

        If there isn't one, we cache the source code builder specified
        for the directory (which in turn will cache the value from its
        parent directory, and so on up to the file system root).
        """
        try:
            scb = self.sbuilder
        except AttributeError:
            scb = self.dir.src_builder()
            self.sbuilder = scb
        return scb

    def get_abspath(self):
        """Get the absolute path of the file."""
        return self.abspath

    def for_signature(self):
        # Return just our name.  Even an absolute path would not work,
        # because that can change thanks to symlinks or remapped network
        # paths.
        return self.name

    def get_subst_proxy(self):
        try:
            return self._proxy
        except AttributeError:
            ret = EntryProxy(self)
            self._proxy = ret
            return ret

class Entry(Base):
    """This is the class for generic Node.FS entries--that is, things
    that could be a File or a Dir, but we're just not sure yet.
    Consequently, the methods in this class really exist just to
    transform their associated object into the right class when the
    time comes, and then call the same-named method in the transformed
    class."""

    def rfile(self):
        """We're a generic Entry, but the caller is actually looking for
        a File at this point, so morph into one."""
        self.__class__ = File
        self._morph()
        self.clear()
        return File.rfile(self)

    def get_found_includes(self, env, scanner, target):
        """If we're looking for included files, it's because this Entry
        is really supposed to be a File itself."""
        node = self.rfile()
        return node.get_found_includes(env, scanner, target)

    def scanner_key(self):
        return self.get_suffix()

    def get_contents(self):
        """Fetch the contents of the entry.
        
        Since this should return the real contents from the file
        system, we check to see into what sort of subclass we should
        morph this Entry."""
        if self.fs.isfile(self.abspath):
            self.__class__ = File
            self._morph()
            return File.get_contents(self)
        if self.fs.isdir(self.abspath):
            self.__class__ = Dir
            self._morph()
            return Dir.get_contents(self)
        if self.fs.islink(self.abspath):
            return ''             # avoid errors for dangling symlinks
        raise AttributeError

    def exists(self):
        """Return if the Entry exists.  Check the file system to see
        what we should turn into first.  Assume a file if there's no
        directory."""
        if self.fs.isdir(self.abspath):
            self.__class__ = Dir
            self._morph()
            return Dir.exists(self)
        else:
            self.__class__ = File
            self._morph()
            self.clear()
            return File.exists(self)

    def calc_signature(self, calc=None):
        """Return the Entry's calculated signature.  Check the file
        system to see what we should turn into first.  Assume a file if
        there's no directory."""
        if self.fs.isdir(self.abspath):
            self.__class__ = Dir
            self._morph()
            return Dir.calc_signature(self, calc)
        else:
            self.__class__ = File
            self._morph()
            self.clear()
            return File.calc_signature(self, calc)

    def must_be_a_Dir(self):
        """Called to make sure a Node is a Dir.  Since we're an
        Entry, we can morph into one."""
        self.__class__ = Dir
        self._morph()

# This is for later so we can differentiate between Entry the class and Entry
# the method of the FS class.
_classEntry = Entry


class LocalFS:
    # This class implements an abstraction layer for operations involving
    # a local file system.  Essentially, this wraps any function in
    # the os, os.path or shutil modules that we use to actually go do
    # anything with or to the local file system.
    #
    # Note that there's a very good chance we'll refactor this part of
    # the architecture in some way as we really implement the interface(s)
    # for remote file system Nodes.  For example, the right architecture
    # might be to have this be a subclass instead of a base class.
    # Nevertheless, we're using this as a first step in that direction.
    #
    # We're not using chdir() yet because the calling subclass method
    # needs to use os.chdir() directly to avoid recursion.  Will we
    # really need this one?
    #def chdir(self, path):
    #    return os.chdir(path)
    def chmod(self, path, mode):
        return os.chmod(path, mode)
    def copy2(self, src, dst):
        return shutil.copy2(src, dst)
    def exists(self, path):
        return os.path.exists(path)
    def getmtime(self, path):
        return os.path.getmtime(path)
    def isdir(self, path):
        return os.path.isdir(path)
    def isfile(self, path):
        return os.path.isfile(path)
    def link(self, src, dst):
        return os.link(src, dst)
    def listdir(self, path):
        return os.listdir(path)
    def makedirs(self, path):
        return os.makedirs(path)
    def mkdir(self, path):
        return os.mkdir(path)
    def rename(self, old, new):
        return os.rename(old, new)
    def stat(self, path):
        return os.stat(path)
    def symlink(self, src, dst):
        return os.symlink(src, dst)
    def open(self, path):
        return open(path)
    def unlink(self, path):
        return os.unlink(path)

    if hasattr(os, 'symlink'):
        def islink(self, path):
            return os.path.islink(path)
        def exists_or_islink(self, path):
            return os.path.exists(path) or os.path.islink(path)
    else:
        def islink(self, path):
            return 0                    # no symlinks
        exists_or_islink = exists

#class RemoteFS:
#    # Skeleton for the obvious methods we might need from the
#    # abstraction layer for a remote filesystem.
#    def upload(self, local_src, remote_dst):
#        pass
#    def download(self, remote_src, local_dst):
#        pass


class FS(LocalFS):
    def __init__(self, path = None):
        """Initialize the Node.FS subsystem.

        The supplied path is the top of the source tree, where we
        expect to find the top-level build file.  If no path is
        supplied, the current directory is the default.

        The path argument must be a valid absolute path.
        """
        if __debug__: logInstanceCreation(self)
        self.Top = None
        if path == None:
            self.pathTop = os.getcwd()
        else:
            self.pathTop = path
        self.Root = {}
        self.SConstruct_dir = None
        self.CachePath = None
        self.cache_force = None
        self.cache_show = None

    def set_toplevel_dir(self, path):
        assert not self.Top, "You can only set the top-level path on an FS object that has not had its File, Dir, or Entry methods called yet."
        self.pathTop = path

    def set_SConstruct_dir(self, dir):
        self.SConstruct_dir = dir
        
    def __setTopLevelDir(self):
        if not self.Top:
            self.Top = self.__doLookup(Dir, os.path.normpath(self.pathTop))
            self.Top.path = '.'
            self._cwd = self.Top
        
    def getcwd(self):
        self.__setTopLevelDir()
        return self._cwd

    def __checkClass(self, node, klass):
        if isinstance(node, klass) or klass == Entry:
            return node
        if node.__class__ == Entry:
            node.__class__ = klass
            node._morph()
            return node
        raise TypeError, "Tried to lookup %s '%s' as a %s." % \
              (node.__class__.__name__, node.path, klass.__name__)
        
    def __doLookup(self, fsclass, name, directory = None, create = 1):
        """This method differs from the File and Dir factory methods in
        one important way: the meaning of the directory parameter.
        In this method, if directory is None or not supplied, the supplied
        name is expected to be an absolute path.  If you try to look up a
        relative path with directory=None, then an AssertionError will be
        raised."""

        if not name:
            # This is a stupid hack to compensate for the fact
            # that the POSIX and Win32 versions of os.path.normpath()
            # behave differently.  In particular, in POSIX:
            #   os.path.normpath('./') == '.'
            # in Win32
            #   os.path.normpath('./') == ''
            #   os.path.normpath('.\\') == ''
            #
            # This is a definite bug in the Python library, but we have
            # to live with it.
            name = '.'
        path_comp = string.split(name, os.sep)
        drive, path_first = os.path.splitdrive(path_comp[0])
        if not path_first:
            # Absolute path
            drive = _my_normcase(drive)
            try:
                directory = self.Root[drive]
            except KeyError:
                if not create:
                    raise SCons.Errors.UserError
                directory = RootDir(drive, ParentOfRoot(), self)
                self.Root[drive] = directory
            path_comp = path_comp[1:]
        else:
            path_comp = [ path_first, ] + path_comp[1:]

        if not path_comp:
            path_comp = ['']
            
        # Lookup the directory
        for path_name in path_comp[:-1]:
            path_norm = _my_normcase(path_name)
            try:
                d = directory.entries[path_norm]
            except KeyError:
                if not create:
                    raise SCons.Errors.UserError

                # look at the actual filesystem and make sure there isn't
                # a file already there
                path = directory.entry_path(path_name)
                if self.isfile(path):
                    raise TypeError, \
                          "File %s found where directory expected." % path

                dir_temp = Dir(path_name, directory, self)
                directory.entries[path_norm] = dir_temp
                directory.add_wkid(dir_temp)
                directory = dir_temp
            else:
                d.must_be_a_Dir()
                directory = d

        entry_norm = _my_normcase(path_comp[-1])
        try:
            e = directory.entries[entry_norm]
        except KeyError:
            if not create:
                raise SCons.Errors.UserError

            # make sure we don't create File nodes when there is actually
            # a directory at that path on the disk, and vice versa
            path = directory.entry_path(path_comp[-1])
            if fsclass == File:
                if self.isdir(path):
                    raise TypeError, \
                          "Directory %s found where file expected." % path
            elif fsclass == Dir:
                if self.isfile(path):
                    raise TypeError, \
                          "File %s found where directory expected." % path
            
            result = fsclass(path_comp[-1], directory, self)
            directory.entries[entry_norm] = result 
            directory.add_wkid(result)
        else:
            result = self.__checkClass(e, fsclass)
        return result 

    def __transformPath(self, name, directory):
        """Take care of setting up the correct top-level directory,
        usually in preparation for a call to doLookup().

        If the path name is prepended with a '#', then it is unconditionally
        interpreted as relative to the top-level directory of this FS.

        If directory is None, and name is a relative path,
        then the same applies.
        """
        self.__setTopLevelDir()
        if name and name[0] == '#':
            directory = self.Top
            name = name[1:]
            if name and (name[0] == os.sep or name[0] == '/'):
                # Correct such that '#/foo' is equivalent
                # to '#foo'.
                name = name[1:]
            name = os.path.join('.', os.path.normpath(name))
        elif not directory:
            directory = self._cwd
        return (os.path.normpath(name), directory)

    def chdir(self, dir, change_os_dir=0):
        """Change the current working directory for lookups.
        If change_os_dir is true, we will also change the "real" cwd
        to match.
        """
        self.__setTopLevelDir()
        curr=self._cwd
        try:
            if not dir is None:
                self._cwd = dir
                if change_os_dir:
                    os.chdir(dir.abspath)
        except OSError:
            self._cwd = curr
            raise

    def Entry(self, name, directory = None, create = 1, klass=None):
        """Lookup or create a generic Entry node with the specified name.
        If the name is a relative path (begins with ./, ../, or a file
        name), then it is looked up relative to the supplied directory
        node, or to the top level directory of the FS (supplied at
        construction time) if no directory is supplied.
        """

        if not klass:
            klass = Entry

        if isinstance(name, Base):
            return self.__checkClass(name, klass)
        else:
            if directory and not isinstance(directory, Dir):
                directory = self.Dir(directory)
            name, directory = self.__transformPath(name, directory)
            return self.__doLookup(klass, name, directory, create)
    
    def File(self, name, directory = None, create = 1):
        """Lookup or create a File node with the specified name.  If
        the name is a relative path (begins with ./, ../, or a file name),
        then it is looked up relative to the supplied directory node,
        or to the top level directory of the FS (supplied at construction
        time) if no directory is supplied.

        This method will raise TypeError if a directory is found at the
        specified path.
        """

        return self.Entry(name, directory, create, File)
    
    def Dir(self, name, directory = None, create = 1):
        """Lookup or create a Dir node with the specified name.  If
        the name is a relative path (begins with ./, ../, or a file name),
        then it is looked up relative to the supplied directory node,
        or to the top level directory of the FS (supplied at construction
        time) if no directory is supplied.

        This method will raise TypeError if a normal file is found at the
        specified path.
        """

        return self.Entry(name, directory, create, Dir)
    
    def BuildDir(self, build_dir, src_dir, duplicate=1):
        """Link the supplied build directory to the source directory
        for purposes of building files."""
        
        self.__setTopLevelDir()
        if not isinstance(src_dir, SCons.Node.Node):
            src_dir = self.Dir(src_dir)
        if not isinstance(build_dir, SCons.Node.Node):
            build_dir = self.Dir(build_dir)
        if not src_dir.is_under(self.Top):
            raise SCons.Errors.UserError, "Source directory must be under top of build tree."
        if src_dir.is_under(build_dir):
            raise SCons.Errors.UserError, "Source directory cannot be under build directory."
        if build_dir.srcdir:
            if build_dir.srcdir == src_dir:
                return # We already did this.
            raise SCons.Errors.UserError, "'%s' already has a source directory: '%s'."%(build_dir, build_dir.srcdir)
        build_dir.link(src_dir, duplicate)

    def Repository(self, *dirs):
        """Specify Repository directories to search."""
        for d in dirs:
            if not isinstance(d, SCons.Node.Node):
                d = self.Dir(d)
            self.__setTopLevelDir()
            self.Top.addRepository(d)

    def Rsearch(self, path, clazz=_classEntry, cwd=None):
        """Search for something in a Repository.  Returns the first
        one found in the list, or None if there isn't one."""
        if isinstance(path, SCons.Node.Node):
            return path
        else:
            name, d = self.__transformPath(path, cwd)
            n = self.__doLookup(clazz, name, d)
            if n.exists():
                return n
            if isinstance(n, Dir):
                # If n is a Directory that has Repositories directly
                # attached to it, then any of those is a valid Repository
                # path.  Return the first one that exists.
                reps = filter(lambda x: x.exists(), n.getRepositories())
                if len(reps):
                    return reps[0]
            d = n.get_dir()
            name = n.name
            # Search repositories of all directories that this file is under.
            while d:
                for rep in d.getRepositories():
                    try:
                        rnode = self.__doLookup(clazz, name, rep)
                        # Only find the node if it exists and it is not
			# a derived file.  If for some reason, we are
			# explicitly building a file IN a Repository, we
			# don't want it to show up in the build tree.
			# This is usually the case with BuildDir().
			# We only want to find pre-existing files.
                        if rnode.exists() and \
                           (isinstance(rnode, Dir) or not rnode.is_derived()):
                            return rnode
                    except TypeError:
                        pass # Wrong type of node.
                # Prepend directory name
                name = d.name + os.sep + name
                # Go up one directory
                d = d.get_dir()
        return None

    def Rsearchall(self, pathlist, must_exist=1, clazz=_classEntry, cwd=None):
        """Search for a list of somethings in the Repository list."""
        ret = []
        if SCons.Util.is_String(pathlist):
            pathlist = string.split(pathlist, os.pathsep)
        if not SCons.Util.is_List(pathlist):
            pathlist = [pathlist]
        for path in filter(None, pathlist):
            if isinstance(path, SCons.Node.Node):
                ret.append(path)
            else:
                name, d = self.__transformPath(path, cwd)
                n = self.__doLookup(clazz, name, d)
                if not must_exist or n.exists():
                    ret.append(n)
                if isinstance(n, Dir):
                    # If this node is a directory, then any repositories
                    # attached to this node can be repository paths.
                    ret.extend(filter(lambda x, me=must_exist, clazz=clazz: isinstance(x, clazz) and (not me or x.exists()),
                                      n.getRepositories()))
                    
                d = n.get_dir()
                name = n.name
                # Search repositories of all directories that this file
                # is under.
                while d:
                    for rep in d.getRepositories():
                        try:
                            rnode = self.__doLookup(clazz, name, rep)
                            # Only find the node if it exists (or
                            # must_exist is zero) and it is not a
                            # derived file.  If for some reason, we
                            # are explicitly building a file IN a
                            # Repository, we don't want it to show up in
                            # the build tree.  This is usually the case
                            # with BuildDir().  We only want to find
                            # pre-existing files.
                            if (not must_exist or rnode.exists()) and \
                               (not rnode.is_derived() or isinstance(rnode, Dir)):
                                ret.append(rnode)
                        except TypeError:
                            pass # Wrong type of node.
                    # Prepend directory name
                    name = d.name + os.sep + name
                    # Go up one directory
                    d = d.get_dir()
        return ret

    def CacheDir(self, path):
        self.CachePath = path

    def build_dir_target_climb(self, dir, tail):
        """Create targets in corresponding build directories

        Climb the directory tree, and look up path names
        relative to any linked build directories we find.
        """
        targets = []
        message = None
        while dir:
            for bd in dir.build_dirs:
                p = apply(os.path.join, [bd.path] + tail)
                targets.append(self.Entry(p))
            tail = [dir.name] + tail
            dir = dir.up()
        if targets:
            message = "building associated BuildDir targets: %s" % string.join(map(str, targets))
        return targets, message

class DummyExecutor:
    """Dummy executor class returned by Dir nodes to bamboozle SCons
    into thinking we are an actual derived node, where our sources are
    our directory entries."""
    def cleanup(self):
        pass
    def get_raw_contents(self):
        return ''
    def get_contents(self):
        return ''
    def get_timestamp(self):
        return 0
    def get_build_env(self):
        return None

class Dir(Base):
    """A class for directories in a file system.
    """

    def __init__(self, name, directory, fs):
        if __debug__: logInstanceCreation(self, 'Node.FS.Dir')
        Base.__init__(self, name, directory, fs)
        self._morph()

    def _morph(self):
        """Turn a file system Node (either a freshly initialized directory
        object or a separate Entry object) into a proper directory object.

        Set up this directory's entries and hook it into the file
        system tree.  Specify that directories (this Node) don't use
        signatures for calculating whether they're current."""

        self.repositories = []
        self.srcdir = None

        self.entries = {}
        self.entries['.'] = self
        self.entries['..'] = self.dir
        self.cwd = self
        self.builder = get_MkdirBuilder()
        self.searched = 0
        self._sconsign = None
        self.build_dirs = []

    def __clearRepositoryCache(self, duplicate=None):
        """Called when we change the repository(ies) for a directory.
        This clears any cached information that is invalidated by changing
        the repository."""

        for node in self.entries.values():
            if node != self.dir:
                if node != self and isinstance(node, Dir):
                    node.__clearRepositoryCache(duplicate)
                else:
                    try:
                        del node._srcreps
                    except AttributeError:
                        pass
                    try:
                        del node._rfile
                    except AttributeError:
                        pass
                    try:
                        del node._rexists
                    except AttributeError:
                        pass
                    try:
                        del node._exists
                    except AttributeError:
                        pass
                    try:
                        del node._srcnode
                    except AttributeError:
                        pass
                    try:
                        del node._str_val
                    except AttributeError:
                        pass
                    if duplicate != None:
                        node.duplicate=duplicate
    
    def __resetDuplicate(self, node):
        if node != self:
            node.duplicate = node.get_dir().duplicate

    def Entry(self, name):
        """Create an entry node named 'name' relative to this directory."""
        return self.fs.Entry(name, self)

    def Dir(self, name):
        """Create a directory node named 'name' relative to this directory."""
        return self.fs.Dir(name, self)

    def File(self, name):
        """Create a file node named 'name' relative to this directory."""
        return self.fs.File(name, self)

    def link(self, srcdir, duplicate):
        """Set this directory as the build directory for the
        supplied source directory."""
        self.srcdir = srcdir
        self.duplicate = duplicate
        self.__clearRepositoryCache(duplicate)
        srcdir.build_dirs.append(self)

    def getRepositories(self):
        """Returns a list of repositories for this directory."""
        if self.srcdir and not self.duplicate:
            try:
                return self._srcreps
            except AttributeError:
                self._srcreps = self.fs.Rsearchall(self.srcdir.path,
                                                   clazz=Dir,
                                                   must_exist=0,
                                                   cwd=self.fs.Top) \
                                + self.repositories
                return self._srcreps
        return self.repositories

    def addRepository(self, dir):
        if not dir in self.repositories and dir != self:
            self.repositories.append(dir)
            self.__clearRepositoryCache()

    def up(self):
        return self.entries['..']

    def root(self):
        if not self.entries['..']:
            return self
        else:
            return self.entries['..'].root()

    def scan(self):
        if not self.implicit is None:
            return
        self.implicit = []
        self.implicit_dict = {}
        self._children_reset()
        try:
            for filename in self.fs.listdir(self.abspath):
                if filename != '.sconsign':
                    self.Entry(filename)
        except OSError:
            # Directory does not exist.  No big deal
            pass
        keys = filter(lambda k: k != '.' and k != '..', self.entries.keys())
        kids = map(lambda x, s=self: s.entries[x], keys)
        def c(one, two):
            return cmp(one.abspath, two.abspath)
        kids.sort(c)
        self._add_child(self.implicit, self.implicit_dict, kids)

    def get_actions(self):
        """A null "builder" for directories."""
        return []

    def build(self, **kw):
        """A null "builder" for directories."""
        global MkdirBuilder
        if not self.builder is MkdirBuilder:
            apply(SCons.Node.Node.build, [self,], kw)

    def multiple_side_effect_has_builder(self):
        global MkdirBuilder
        return not self.builder is MkdirBuilder and self.has_builder()

    def alter_targets(self):
        """Return any corresponding targets in a build directory.
        """
        return self.fs.build_dir_target_climb(self, [])

    def scanner_key(self):
        """A directory does not get scanned."""
        return None

    def get_contents(self):
        """Return aggregate contents of all our children."""
        contents = cStringIO.StringIO()
        for kid in self.children():
            contents.write(kid.get_contents())
        return contents.getvalue()
    
    def prepare(self):
        pass

    def current(self, calc=None):
        """If all of our children were up-to-date, then this
        directory was up-to-date, too."""
        if not self.builder is MkdirBuilder and not self.exists():
            return 0
        state = 0
        for kid in self.children():
            s = kid.get_state()
            if s and (not state or s > state):
                state = s
        import SCons.Node
        if state == 0 or state == SCons.Node.up_to_date:
            return 1
        else:
            return 0

    def rdir(self):
        try:
            return self._rdir
        except AttributeError:
            self._rdir = self
            if not self.exists():
                n = self.fs.Rsearch(self.path, clazz=Dir, cwd=self.fs.Top)
                if n:
                    self._rdir = n
            return self._rdir

    def sconsign(self):
        """Return the .sconsign file info for this directory,
        creating it first if necessary."""
        if not self._sconsign:
            import SCons.SConsign
            self._sconsign = SCons.SConsign.ForDirectory(self)
        return self._sconsign

    def srcnode(self):
        """Dir has a special need for srcnode()...if we
        have a srcdir attribute set, then that *is* our srcnode."""
        if self.srcdir:
            return self.srcdir
        return Base.srcnode(self)

    def get_timestamp(self):
        """Return the latest timestamp from among our children"""
        stamp = 0
        for kid in self.children():
            if kid.get_timestamp() > stamp:
                stamp = kid.get_timestamp()
        return stamp

    def entry_abspath(self, name):
        return self.abspath + os.sep + name

    def entry_path(self, name):
        return self.path + os.sep + name

    def must_be_a_Dir(self):
        """Called to make sure a Node is a Dir.  Since we're already
        one, this is a no-op for us."""
        pass

class RootDir(Dir):
    """A class for the root directory of a file system.

    This is the same as a Dir class, except that the path separator
    ('/' or '\\') is actually part of the name, so we don't need to
    add a separator when creating the path names of entries within
    this directory.
    """
    def __init__(self, name, directory, fs):
        if __debug__: logInstanceCreation(self, 'Node.FS.RootDir')
        Base.__init__(self, name, directory, fs)
        self.path = self.path + os.sep
        self.abspath = self.abspath + os.sep
        self._morph()

    def entry_abspath(self, name):
        return self.abspath + name

    def entry_path(self, name):
        return self.path + name

class BuildInfo:
    bsig = None
    def __cmp__(self, other):
        try:
            return cmp(self.bsig, other.bsig)
        except AttributeError:
            return 1

class File(Base):
    """A class for files in a file system.
    """
    def __init__(self, name, directory, fs):
        if __debug__: logInstanceCreation(self, 'Node.FS.File')
        Base.__init__(self, name, directory, fs)
        self._morph()

    def Entry(self, name):
        """Create an entry node named 'name' relative to
        the SConscript directory of this file."""
        return self.fs.Entry(name, self.cwd)

    def Dir(self, name):
        """Create a directory node named 'name' relative to
        the SConscript directory of this file."""
        return self.fs.Dir(name, self.cwd)

    def File(self, name):
        """Create a file node named 'name' relative to
        the SConscript directory of this file."""
        return self.fs.File(name, self.cwd)

    def RDirs(self, pathlist):
        """Search for a list of directories in the Repository list."""
        return self.fs.Rsearchall(pathlist, clazz=Dir, must_exist=0,
                                  cwd=self.cwd)

    def generate_build_dict(self):
        """Return an appropriate dictionary of values for building
        this File."""
        return {'Dir' : self.Dir,
                'File' : self.File,
                'RDirs' : self.RDirs}

    def _morph(self):
        """Turn a file system node into a File object."""
        self.scanner_paths = {}
        self.found_includes = {}
        if not hasattr(self, '_local'):
            self._local = 0

    def root(self):
        return self.dir.root()

    def scanner_key(self):
        return self.get_suffix()

    def get_contents(self):
        if not self.rexists():
            return ''
        return open(self.rfile().abspath, "rb").read()

    def get_timestamp(self):
        if self.rexists():
            return self.fs.getmtime(self.rfile().abspath)
        else:
            return 0

    def store_info(self, obj):
        # Merge our build information into the already-stored entry.
        # This accomodates "chained builds" where a file that's a target
        # in one build (SConstruct file) is a source in a different build.
        # See test/chained-build.py for the use case.
        entry = self.get_stored_info()
        for key, val in obj.__dict__.items():
            entry.__dict__[key] = val
        self.dir.sconsign().set_entry(self.name, entry)

    def get_stored_info(self):
        try:
            stored = self.dir.sconsign().get_entry(self.name)
            if isinstance(stored, BuildInfo):
                return stored
            # The stored build information isn't a BuildInfo object.
            # This probably means it's an old SConsignEntry from SCons
            # 0.95 or before.  The relevant attribute names are the same,
            # though, so just copy the attributes over to an object of
            # the correct type.
            binfo = BuildInfo()
            for key, val in stored.__dict__.items():
                setattr(binfo, key, val)
            return binfo
        except:
            return BuildInfo()

    def get_stored_implicit(self):
        binfo = self.get_stored_info()
        try:
            return binfo.bimplicit
        except AttributeError:
            return None

    def get_found_includes(self, env, scanner, target):
        """Return the included implicit dependencies in this file.
        Cache results so we only scan the file once regardless of
        how many times this information is requested."""
        if not scanner:
            return []

        try:
            path = target.scanner_paths[scanner]
        except AttributeError:
            # The target had no scanner_paths attribute, which means
            # it's an Alias or some other node that's not actually a
            # file.  In that case, back off and use the path for this
            # node itself.
            try:
                path = self.scanner_paths[scanner]
            except KeyError:
                path = scanner.path(env, self.cwd)
                self.scanner_paths[scanner] = path
        except KeyError:
            path = scanner.path(env, target.cwd)
            target.scanner_paths[scanner] = path

        try:
            includes = self.found_includes[path]
        except KeyError:
            includes = scanner(self, env, path)
            self.found_includes[path] = includes

        return includes

    def _createDir(self):
        # ensure that the directories for this node are
        # created.

        listDirs = []
        parent=self.dir
        while parent:
            if parent.exists():
                break
            listDirs.append(parent)
            p = parent.up()
            if isinstance(p, ParentOfRoot):
                raise SCons.Errors.StopError, parent.path
            parent = p
        listDirs.reverse()
        for dirnode in listDirs:
            try:
                # Don't call dirnode.build(), call the base Node method
                # directly because we definitely *must* create this
                # directory.  The dirnode.build() method will suppress
                # the build if it's the default builder.
                SCons.Node.Node.build(dirnode)
                # The build() action may or may not have actually
                # created the directory, depending on whether the -n
                # option was used or not.  Delete the _exists and
                # _rexists attributes so they can be reevaluated.
                try:
                    delattr(dirnode, '_exists')
                except AttributeError:
                    pass
                try:
                    delattr(dirnode, '_rexists')
                except AttributeError:
                    pass
            except OSError:
                pass

    def retrieve_from_cache(self):
        """Try to retrieve the node's content from a cache

        This method is called from multiple threads in a parallel build,
        so only do thread safe stuff here. Do thread unsafe stuff in
        built().

        Returns true iff the node was successfully retrieved.
        """
        b = self.is_derived()
        if not b and not self.has_src_builder():
            return None
        if b and self.fs.CachePath:
            if self.fs.cache_show:
                if CacheRetrieveSilent(self, [], None) == 0:
                    self.build(presub=0, execute=0)
                    return 1
            elif CacheRetrieve(self, [], None) == 0:
                return 1
        return None

    def built(self):
        """Called just after this node is sucessfully built."""
        # Push this file out to cache before the superclass Node.built()
        # method has a chance to clear the build signature, which it
        # will do if this file has a source scanner.
        if self.fs.CachePath and self.fs.exists(self.path):
            CachePush(self, [], None)
        SCons.Node.Node.built(self)
        self.found_includes = {}
        try:
            delattr(self, '_exists')
        except AttributeError:
            pass
        try:
            delattr(self, '_rexists')
        except AttributeError:
            pass

    def visited(self):
        if self.fs.CachePath and self.fs.cache_force and self.fs.exists(self.path):
            CachePush(self, None, None)

    def has_src_builder(self):
        """Return whether this Node has a source builder or not.

        If this Node doesn't have an explicit source code builder, this
        is where we figure out, on the fly, if there's a transparent
        source code builder for it.

        Note that if we found a source builder, we also set the
        self.builder attribute, so that all of the methods that actually
        *build* this file don't have to do anything different.
        """
        try:
            scb = self.sbuilder
        except AttributeError:
            if self.rexists():
                scb = None
            else:
                scb = self.dir.src_builder()
                if scb is _null:
                    scb = None
                    dir = self.dir.path
                    sccspath = os.path.join('SCCS', 's.' + self.name)
                    if dir != '.':
                        sccspath = os.path.join(dir, sccspath)
                    if self.fs.exists(sccspath):
                        scb = get_DefaultSCCSBuilder()
                    else:
                        rcspath = os.path.join('RCS', self.name + ',v')
                        if dir != '.':
                            rcspath = os.path.join(dir, rcspath)
                        if os.path.exists(rcspath):
                            scb = get_DefaultRCSBuilder()
                self.builder = scb
            self.sbuilder = scb
        return not scb is None

    def alter_targets(self):
        """Return any corresponding targets in a build directory.
        """
        if self.is_derived():
            return [], None
        return self.fs.build_dir_target_climb(self.dir, [self.name])

    def is_pseudo_derived(self):
        return self.has_src_builder()
    
    def prepare(self):
        """Prepare for this file to be created."""
        SCons.Node.Node.prepare(self)

        if self.get_state() != SCons.Node.up_to_date:
            if self.exists():
                if self.is_derived() and not self.precious:
                    try:
                        Unlink(self, [], None)
                    except OSError, e:
                        raise SCons.Errors.BuildError(node = self,
                                                      errstr = e.strerror)
                    try:
                        delattr(self, '_exists')
                    except AttributeError:
                        pass
            else:
                try:
                    self._createDir()
                except SCons.Errors.StopError, drive:
                    desc = "No drive `%s' for target `%s'." % (drive, self)
                    raise SCons.Errors.StopError, desc

    def remove(self):
        """Remove this file."""
        if self.fs.exists_or_islink(self.path):
            self.fs.unlink(self.path)
            return 1
        return None

    def exists(self):
        # Duplicate from source path if we are set up to do this.
        if self.duplicate and not self.is_derived() and not self.linked:
            src=self.srcnode().rfile()
            if src.exists() and src.abspath != self.abspath:
                self._createDir()
                try:
                    Unlink(self, None, None)
                except OSError:
                    pass
                try:
                    Link(self, src, None)
                except IOError, e:
                    desc = "Cannot duplicate `%s' in `%s': %s." % (src, self.dir, e.strerror)
                    raise SCons.Errors.StopError, desc
                self.linked = 1
                # The Link() action may or may not have actually
                # created the file, depending on whether the -n
                # option was used or not.  Delete the _exists and
                # _rexists attributes so they can be reevaluated.
                try:
                    delattr(self, '_exists')
                except AttributeError:
                    pass
                try:
                    delattr(self, '_rexists')
                except AttributeError:
                    pass
        return Base.exists(self)

    def new_binfo(self):
        return BuildInfo()

    def del_cinfo(self):
        try:
            del self.binfo.csig
        except AttributeError:
            pass
        try:
            del self.binfo.timestamp
        except AttributeError:
            pass

    def calc_csig(self, calc=None):
        """
        Generate a node's content signature, the digested signature
        of its content.

        node - the node
        cache - alternate node to use for the signature cache
        returns - the content signature
        """
        if calc is None:
            calc = self.calculator()

        try:
            return self.binfo.csig
        except AttributeError:
            pass
        
        if calc.max_drift >= 0:
            old = self.get_stored_info()
        else:
            old = BuildInfo()

        try:
            mtime = self.get_timestamp()
        except:
            mtime = 0
            raise SCons.Errors.UserError, "no such %s" % self

        try:
            if (old.timestamp and old.csig and old.timestamp == mtime):
                # use the signature stored in the .sconsign file
                csig = old.csig
            else:
                csig = calc.module.signature(self)
        except AttributeError:
            csig = calc.module.signature(self)

        if calc.max_drift >= 0 and (time.time() - mtime) > calc.max_drift:
            try:
                binfo = self.binfo
            except AttributeError:
                binfo = self.binfo = self.new_binfo()
            binfo.csig = csig
            binfo.timestamp = mtime
            self.store_info(binfo)

        return csig

    def current(self, calc=None, scan=1):
        self.binfo = self.gen_binfo(calc)
        if self.always_build:
            return None
        if not self.exists():
            # The file doesn't exist locally...
            r = self.rfile()
            if r != self:
                # ...but there is one in a Repository...
                old = r.get_stored_info()
                if old == self.binfo:
                    # ...and it's even up-to-date...
                    if self._local:
                        # ...and they'd like a local copy.
                        LocalCopy(self, r, None)
                        self.store_info(self.binfo)
                    return 1
            self._rfile = self
            return None
        else:
            old = self.get_stored_info()
            return (old == self.binfo)

    def rfile(self):
        try:
            return self._rfile
        except AttributeError:
            self._rfile = self
            if not self.exists():
                n = self.fs.Rsearch(self.path, clazz=File,
                                    cwd=self.fs.Top)
                if n:
                    self._rfile = n
            return self._rfile

    def rstr(self):
        return str(self.rfile())

    def cachepath(self):
        if not self.fs.CachePath:
            return None, None
        if self.binfo.bsig is None:
            raise SCons.Errors.InternalError, "cachepath(%s) found a bsig of None" % self.path
        # Add the path to the cache signature, because multiple
        # targets built by the same action will all have the same
        # build signature, and we have to differentiate them somehow.
        cache_sig = SCons.Sig.MD5.collect([self.binfo.bsig, self.path])
        subdir = string.upper(cache_sig[0])
        dir = os.path.join(self.fs.CachePath, subdir)
        return dir, os.path.join(dir, cache_sig)

    def target_from_source(self, prefix, suffix, splitext=SCons.Util.splitext):
        return self.dir.File(prefix + splitext(self.name)[0] + suffix)

    def must_be_a_Dir(self):
        """Called to make sure a Node is a Dir.  Since we're already a
        File, this is a TypeError..."""
        raise TypeError, "Tried to lookup File '%s' as a Dir." % self.path

default_fs = FS()


def find_file(filename, paths, node_factory = default_fs.File):
    """
    find_file(str, [Dir()]) -> [nodes]

    filename - a filename to find
    paths - a list of directory path *nodes* to search in

    returns - the node created from the found file.

    Find a node corresponding to either a derived file or a file
    that exists already.

    Only the first file found is returned, and none is returned
    if no file is found.
    """
    retval = None
    for dir in paths:
        try:
            node = node_factory(filename, dir)
            # Return true if the node exists or is a derived node.
            if node.is_derived() or \
               node.is_pseudo_derived() or \
               (isinstance(node, SCons.Node.FS.Base) and node.exists()):
                retval = node
                break
        except TypeError:
            # If we find a directory instead of a file, we don't care
            pass

    return retval

def find_files(filenames, paths, node_factory = default_fs.File):
    """
    find_files([str], [Dir()]) -> [nodes]

    filenames - a list of filenames to find
    paths - a list of directory path *nodes* to search in

    returns - the nodes created from the found files.

    Finds nodes corresponding to either derived files or files
    that exist already.

    Only the first file found is returned for each filename,
    and any files that aren't found are ignored.
    """
    nodes = map(lambda x, paths=paths, node_factory=node_factory:
                       find_file(x, paths, node_factory),
                filenames)
    return filter(lambda x: x != None, nodes)
