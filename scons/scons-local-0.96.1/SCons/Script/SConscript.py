"""SCons.Script.SConscript

This module defines the Python API provided to SConscript and SConstruct
files.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Script/SConscript.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import SCons
import SCons.Action
import SCons.Builder
import SCons.Defaults
import SCons.Environment
import SCons.Errors
import SCons.Node
import SCons.Node.Alias
import SCons.Node.FS
import SCons.Options
import SCons.Platform
import SCons.SConf
import SCons.Script
import SCons.Tool
import SCons.Util

import os
import os.path
import re
import string
import sys
import traceback
import types
import UserList

launch_dir = os.path.abspath(os.curdir)

def do_nothing(text): pass
HelpFunction = do_nothing

Arguments = {}
ArgList = []
CommandLineTargets = []
DefaultCalled = None
DefaultTargets = []
GlobalDict = {}

class TargetList(UserList.UserList):
    def _do_nothing(self, *args, **kw):
        pass
    def _add_Default(self, list):
        self.extend(list)
    def _clear(self):
        del self[:]
BuildTargets = TargetList()

# global exports set by Export():
global_exports = {}

# chdir flag
sconscript_chdir = 1

# will be set to 1, if we are reading a SConscript
sconscript_reading = 0

def _scons_add_args(alist):
    for arg in alist:
        a, b = string.split(arg, '=', 1)
        Arguments[a] = b
        ArgList.append((a, b))

def _scons_add_targets(tlist):
    if tlist:
        CommandLineTargets.extend(tlist)
        BuildTargets.extend(tlist)
        BuildTargets._add_Default = BuildTargets._do_nothing
        BuildTargets._clear = BuildTargets._do_nothing

def get_calling_namespaces():
    """Return the locals and globals for the function that called
    into this module in the current callstack."""
    try: 1/0
    except ZeroDivisionError: frame = sys.exc_info()[2].tb_frame

    while frame.f_globals.get("__name__") == __name__: frame = frame.f_back

    return frame.f_locals, frame.f_globals


def compute_exports(exports):
    """Compute a dictionary of exports given one of the parameters
    to the Export() function or the exports argument to SConscript()."""

    loc, glob = get_calling_namespaces()

    retval = {}
    try:
        for export in exports:
            if SCons.Util.is_Dict(export):
                retval.update(export)
            else:
                try:
                    retval[export] = loc[export]
                except KeyError:
                    retval[export] = glob[export]
    except KeyError, x:
        raise SCons.Errors.UserError, "Export of non-existent variable '%s'"%x

    return retval


class Frame:
    """A frame on the SConstruct/SConscript call stack"""
    def __init__(self, exports, sconscript):
        self.globals = BuildDefaultGlobals()
        self.retval = None
        self.prev_dir = SCons.Node.FS.default_fs.getcwd()
        self.exports = compute_exports(exports)  # exports from the calling SConscript
        # make sure the sconscript attr is a Node.
        if isinstance(sconscript, SCons.Node.Node):
            self.sconscript = sconscript
        else:
            self.sconscript = SCons.Node.FS.default_fs.File(str(sconscript))

# the SConstruct/SConscript call stack:
stack = []

# For documentation on the methods in this file, see the scons man-page

def Return(*vars):
    retval = []
    try:
        for var in vars:
            for v in string.split(var):
                retval.append(stack[-1].globals[v])
    except KeyError, x:
        raise SCons.Errors.UserError, "Return of non-existent variable '%s'"%x

    if len(retval) == 1:
        stack[-1].retval = retval[0]
    else:
        stack[-1].retval = tuple(retval)

def _SConscript(fs, *files, **kw):
    top = fs.Top
    sd = fs.SConstruct_dir.rdir()
    exports = kw.get('exports', [])

    # evaluate each SConscript file
    results = []
    for fn in files:
        stack.append(Frame(exports,fn))
        old_sys_path = sys.path
        try:
            global sconscript_reading
            sconscript_reading = 1
            if fn == "-":
                exec sys.stdin in stack[-1].globals
            else:
                if isinstance(fn, SCons.Node.Node):
                    f = fn
                else:
                    f = fs.File(str(fn))
                _file_ = None

                # Change directory to the top of the source
                # tree to make sure the os's cwd and the cwd of
                # fs match so we can open the SConscript.
                fs.chdir(top, change_os_dir=1)
                if f.rexists():
                    _file_ = open(f.rstr(), "r")
                elif f.has_src_builder():
                    # The SConscript file apparently exists in a source
                    # code management system.  Build it, but then clear
                    # the builder so that it doesn't get built *again*
                    # during the actual build phase.
                    f.build()
                    f.builder_set(None)
                    s = str(f)
                    if os.path.exists(s):
                        _file_ = open(s, "r")
                if _file_:
                    # Chdir to the SConscript directory.  Use a path
                    # name relative to the SConstruct file so that if
                    # we're using the -f option, we're essentially
                    # creating a parallel SConscript directory structure
                    # in our local directory tree.
                    #
                    # XXX This is broken for multiple-repository cases
                    # where the SConstruct and SConscript files might be
                    # in different Repositories.  For now, cross that
                    # bridge when someone comes to it.
                    ldir = fs.Dir(f.dir.get_path(sd))
                    try:
                        fs.chdir(ldir, change_os_dir=sconscript_chdir)
                    except OSError:
                        # There was no local directory, so we should be
                        # able to chdir to the Repository directory.
                        # Note that we do this directly, not through
                        # fs.chdir(), because we still need to
                        # interpret the stuff within the SConscript file
                        # relative to where we are logically.
                        fs.chdir(ldir, change_os_dir=0)
                        os.chdir(f.rfile().dir.get_abspath())

                    # Append the SConscript directory to the beginning
                    # of sys.path so Python modules in the SConscript
                    # directory can be easily imported.
                    sys.path = [ f.dir.get_abspath() ] + sys.path

                    # This is the magic line that actually reads up and
                    # executes the stuff in the SConscript file.  We
                    # look for the "exec _file_ " from the beginning
                    # of this line to find the right stack frame (the
                    # next one) describing the SConscript file and line
                    # number that creates a node.
                    exec _file_ in stack[-1].globals
                else:
                    SCons.Warnings.warn(SCons.Warnings.MissingSConscriptWarning,
                             "Ignoring missing SConscript '%s'" % f.path)

        finally:
            sconscript_reading = 0
            sys.path = old_sys_path
            frame = stack.pop()
            try:
                fs.chdir(frame.prev_dir, change_os_dir=sconscript_chdir)
            except OSError:
                # There was no local directory, so chdir to the
                # Repository directory.  Like above, we do this
                # directly.
                fs.chdir(frame.prev_dir, change_os_dir=0)
                os.chdir(frame.prev_dir.rdir().get_abspath())

            results.append(frame.retval)

    # if we only have one script, don't return a tuple
    if len(results) == 1:
        return results[0]
    else:
        return tuple(results)

def is_our_exec_statement(line):
    return not line is None and line[:12] == "exec _file_ "

def SConscript_exception(file=sys.stderr):
    """Print an exception stack trace just for the SConscript file(s).
    This will show users who have Python errors where the problem is,
    without cluttering the output with all of the internal calls leading
    up to where we exec the SConscript."""
    exc_type, exc_value, exc_tb = sys.exc_info()
    stack = traceback.extract_tb(exc_tb)
    last_text = ""
    found = 0
    i = 0
    for frame in stack:
        if is_our_exec_statement(last_text):
            found = 1
            break
        i = i + 1
        last_text = frame[3]
    if not found:
        # We did not find our exec statement, so this was actually a bug
        # in SCons itself.  Show the whole stack.
        i = 0
    type = str(exc_type)
    if type[:11] == "exceptions.":
        type = type[11:]
    file.write('%s: %s:\n' % (type, exc_value))
    for fname, line, func, text in stack[i:]:
        file.write('  File "%s", line %d:\n' % (fname, line))
        file.write('    %s\n' % text)

def annotate(node):
    """Annotate a node with the stack frame describing the
    SConscript file and line number that created it."""
    stack = traceback.extract_stack()
    last_text = ""
    for frame in stack:
        # If the script text of the previous frame begins with the
        # magic "exec _file_ " string, then this frame describes the
        # SConscript file and line number that caused this node to be
        # created.  Record the tuple and carry on.
        if is_our_exec_statement(last_text):
            node.creator = frame
            return
        last_text = frame[3]

# The following line would cause each Node to be annotated using the
# above function.  Unfortunately, this is a *huge* performance hit, so
# leave this disabled until we find a more efficient mechanism.
#SCons.Node.Annotate = annotate

class SConsEnvironment(SCons.Environment.Base):
    """An Environment subclass that contains all of the methods that
    are particular to the wrapper SCons interface and which aren't
    (or shouldn't be) part of the build engine itself.

    Note that not all of the methods of this class have corresponding
    global functions, there are some private methods.
    """

    #
    # Private methods of an SConsEnvironment.
    #
    def _exceeds_version(self, major, minor, v_major, v_minor):
        """Return 1 if 'major' and 'minor' are greater than the version
        in 'v_major' and 'v_minor', and 0 otherwise."""
        return (major > v_major or (major == v_major and minor > v_minor))

    def _get_major_minor(self, version_string):
        """Split a version string into major and minor parts.  This
        is complicated by the fact that a version string can be something
        like 3.2b1."""
        version = string.split(string.split(version_string, ' ')[0], '.')
        v_major = int(version[0])
        v_minor = int(re.match('\d+', version[1]).group())
        return v_major, v_minor

    def _get_SConscript_filenames(self, ls, kw):
        """
        Convert the parameters passed to # SConscript() calls into a list
        of files and export variables.  If the parameters are invalid,
        throws SCons.Errors.UserError. Returns a tuple (l, e) where l
        is a list of SConscript filenames and e is a list of exports.
        """
        exports = []

        if len(ls) == 0:
            try:
                dirs = kw["dirs"]
            except KeyError:
                raise SCons.Errors.UserError, \
                      "Invalid SConscript usage - no parameters"

            if not SCons.Util.is_List(dirs):
                dirs = [ dirs ]
            dirs = map(str, dirs)

            name = kw.get('name', 'SConscript')

            files = map(lambda n, name = name: os.path.join(n, name), dirs)

        elif len(ls) == 1:

            files = ls[0]

        elif len(ls) == 2:

            files   = ls[0]
            exports = self.Split(ls[1])

        else:

            raise SCons.Errors.UserError, \
                  "Invalid SConscript() usage - too many arguments"

        if not SCons.Util.is_List(files):
            files = [ files ]

        if kw.get('exports'):
            exports.extend(self.Split(kw['exports']))

        build_dir = kw.get('build_dir')
        if build_dir:
            if len(files) != 1:
                raise SCons.Errors.UserError, \
                    "Invalid SConscript() usage - can only specify one SConscript with a build_dir"
            duplicate = kw.get('duplicate', 1)
            src_dir = kw.get('src_dir')
            if not src_dir:
                src_dir, fname = os.path.split(str(files[0]))
            else:
                if not isinstance(src_dir, SCons.Node.Node):
                    src_dir = self.fs.Dir(src_dir)
                fn = files[0]
                if not isinstance(fn, SCons.Node.Node):
                    fn = self.fs.File(fn)
                if fn.is_under(src_dir):
                    # Get path relative to the source directory.
                    fname = fn.get_path(src_dir)
                else:
                    # Fast way to only get the terminal path component of a Node.
                    fname = fn.get_path(fn.dir)
            self.fs.BuildDir(build_dir, src_dir, duplicate)
            files = [os.path.join(str(build_dir), fname)]

        return (files, exports)

    #
    # Public methods of an SConsEnvironment.  These get
    # entry points in the global name space so they can be called
    # as global functions.
    #

    def Default(self, *targets):
        global DefaultCalled
        global DefaultTargets
        DefaultCalled = 1
        for t in targets:
            if t is None:
                # Delete the elements from the list in-place, don't
                # reassign an empty list to DefaultTargets, so that the
                # DEFAULT_TARGETS variable will still point to the
                # same object we point to.
                del DefaultTargets[:]
                BuildTargets._clear()
            elif isinstance(t, SCons.Node.Node):
                DefaultTargets.append(t)
                BuildTargets._add_Default([t])
            else:
                nodes = self.arg2nodes(t, self.fs.Entry)
                DefaultTargets.extend(nodes)
                BuildTargets._add_Default(nodes)

    def EnsureSConsVersion(self, major, minor):
        """Exit abnormally if the SCons version is not late enough."""
        v_major, v_minor = self._get_major_minor(SCons.__version__)
        if self._exceeds_version(major, minor, v_major, v_minor):
            print "SCons %d.%d or greater required, but you have SCons %s" %(major,minor,SCons.__version__)
            sys.exit(2)

    def EnsurePythonVersion(self, major, minor):
        """Exit abnormally if the Python version is not late enough."""
        try:
            v_major, v_minor, v_micro, release, serial = sys.version_info
        except AttributeError:
            v_major, v_minor = self._get_major_minor(sys.version)
        if self._exceeds_version(major, minor, v_major, v_minor):
            v = string.split(sys.version, " ", 1)[0]
            print "Python %d.%d or greater required, but you have Python %s" %(major,minor,v)
            sys.exit(2)

    def Exit(self, value=0):
        sys.exit(value)

    def Export(self, *vars):
        for var in vars:
            global_exports.update(compute_exports(self.Split(var)))

    def GetLaunchDir(self):
        global launch_dir
        return launch_dir

    def GetOption(self, name):
        name = self.subst(name)
        return SCons.Script.ssoptions.get(name)

    def Help(self, text):
        text = self.subst(text, raw=1)
        HelpFunction(text)

    def Import(self, *vars):
        try:
            for var in vars:
                var = self.Split(var)
                for v in var:
                    if v == '*':
                        stack[-1].globals.update(global_exports)
                        stack[-1].globals.update(stack[-1].exports)
                    else:
                        if stack[-1].exports.has_key(v):
                            stack[-1].globals[v] = stack[-1].exports[v]
                        else:
                            stack[-1].globals[v] = global_exports[v]
        except KeyError,x:
            raise SCons.Errors.UserError, "Import of non-existent variable '%s'"%x

    def SConscript(self, *ls, **kw):
        ls = map(lambda l, self=self: self.subst(l), ls)
        subst_kw = {}
        for key, val in kw.items():
            if SCons.Util.is_String(val):
                val = self.subst(val)
            elif SCons.Util.is_List(val):
                result = []
                for v in val:
                    if SCons.Util.is_String(v):
                        v = self.subst(v)
                    result.append(v)
                val = result
            subst_kw[key] = val

        files, exports = self._get_SConscript_filenames(ls, subst_kw)

        return apply(_SConscript, [self.fs,] + files, {'exports' : exports})

    def SConscriptChdir(self, flag):
        global sconscript_chdir
        sconscript_chdir = flag

    def SetOption(self, name, value):
        name = self.subst(name)
        SCons.Script.ssoptions.set(name, value)

#
#
#
SCons.Environment.Environment = SConsEnvironment

def Options(files=None, args=Arguments):
    return SCons.Options.Options(files, args)

def SetBuildSignatureType(type):
    SCons.Warnings.warn(SCons.Warnings.DeprecatedWarning,
                        "The SetBuildSignatureType() function has been deprecated;\n" +\
                        "\tuse the TargetSignatures() function instead.")
    SCons.Defaults.DefaultEnvironment().TargetSignatures(type)

def SetContentSignatureType(type):
    SCons.Warnings.warn(SCons.Warnings.DeprecatedWarning,
                        "The SetContentSignatureType() function has been deprecated;\n" +\
                        "\tuse the SourceSignatures() function instead.")
    SCons.Defaults.DefaultEnvironment().SourceSignatures(type)

def GetJobs():
    SCons.Warnings.warn(SCons.Warnings.DeprecatedWarning,
                        "The GetJobs() function has been deprecated;\n" +\
                        "\tuse GetOption('num_jobs') instead.")

    return GetOption('num_jobs')

def SetJobs(num):
    SCons.Warnings.warn(SCons.Warnings.DeprecatedWarning,
                        "The SetJobs() function has been deprecated;\n" +\
                        "\tuse SetOption('num_jobs', num) instead.")
    SetOption('num_jobs', num)

def ParseConfig(env, command, function=None):
    SCons.Warnings.warn(SCons.Warnings.DeprecatedWarning,
                        "The ParseConfig() function has been deprecated;\n" +\
                        "\tuse the env.ParseConfig() method instead.")
    return env.ParseConfig(command, function)

#
_DefaultEnvironmentProxy = None

def get_DefaultEnvironmentProxy():
    global _DefaultEnvironmentProxy
    if not _DefaultEnvironmentProxy:
        default_env = SCons.Defaults.DefaultEnvironment()
        _DefaultEnvironmentProxy = SCons.Environment.NoSubstitutionProxy(default_env)
    return _DefaultEnvironmentProxy

class DefaultEnvironmentCall:
    """A class that implements "global function" calls of
    Environment methods by fetching the specified method from the
    DefaultEnvironment's class.  Note that this uses an intermediate
    proxy class instead of calling the DefaultEnvironment method
    directly so that the proxy can override the subst() method and
    thereby prevent expansion of construction variables (since from
    the user's point of view this was called as a global function,
    with no associated construction environment)."""
    def __init__(self, method_name):
        self.method_name = method_name
    def __call__(self, *args, **kw):
        proxy = get_DefaultEnvironmentProxy()
        method = getattr(proxy, self.method_name)
        return apply(method, args, kw)

# The list of global functions to add to the SConscript name space
# that end up calling corresponding methods or Builders in the
# DefaultEnvironment().
GlobalDefaultEnvironmentFunctions = [
    # Methods from the SConsEnvironment class, above.
    'Default',
    'EnsurePythonVersion',
    'EnsureSConsVersion',
    'Exit',
    'Export',
    'GetLaunchDir',
    'GetOption',
    'Help',
    'Import',
    'SConscript',
    'SConscriptChdir',
    'SetOption',

    # Methods from the Environment.Base class.
    'AddPostAction',
    'AddPreAction',
    'Alias',
    'AlwaysBuild',
    'BuildDir',
    'CacheDir',
    'Clean',
    'Command',
    'Depends',
    'Dir',
    'Execute',
    'File',
    'FindFile',
    'Flatten',
    'GetBuildPath',
    'Ignore',
    'Install',
    'InstallAs',
    'Literal',
    'Local',
    'Precious',
    'Repository',
    'SConsignFile',
    'SideEffect',
    'SourceCode',
    'SourceSignatures',
    'Split',
    'TargetSignatures',
    'Value',
]

GlobalDefaultBuilders = [
    # Supported builders.
    'CFile',
    'CXXFile',
    'DVI',
    'Jar',
    'Java',
    'JavaH',
    'Library',
    'M4',
    'MSVSProject',
    'Object',
    'PCH',
    'PDF',
    'PostScript',
    'Program',
    'RES',
    'RMIC',
    'SharedLibrary',
    'SharedObject',
    'StaticLibrary',
    'StaticObject',
    'Tar',
    'TypeLibrary',
    'Zip',
]

for name in GlobalDefaultEnvironmentFunctions + GlobalDefaultBuilders:
    GlobalDict[name] = DefaultEnvironmentCall(name)

def BuildDefaultGlobals():
    """
    Create a dictionary containing all the default globals for
    SConstruct and SConscript files.
    """

    globals = {
        # Global functions that don't get executed through the
        # default Environment.
        'Action'                : SCons.Action.Action,
        'BoolOption'            : SCons.Options.BoolOption,
        'Builder'               : SCons.Builder.Builder,
        'Configure'             : SCons.SConf.SConf,
        'EnumOption'            : SCons.Options.EnumOption,
        'Environment'           : SCons.Environment.Environment,
        'ListOption'            : SCons.Options.ListOption,
        'Options'               : Options,
        'PackageOption'         : SCons.Options.PackageOption,
        'PathOption'            : SCons.Options.PathOption,
        'Platform'              : SCons.Platform.Platform,
        'Return'                : Return,
        'Scanner'               : SCons.Scanner.Base,
        'Tool'                  : SCons.Tool.Tool,
        'WhereIs'               : SCons.Util.WhereIs,

        # Action factories.
        'Chmod'                 : SCons.Defaults.Chmod,
        'Copy'                  : SCons.Defaults.Copy,
        'Delete'                : SCons.Defaults.Delete,
        'Mkdir'                 : SCons.Defaults.Mkdir,
        'Move'                  : SCons.Defaults.Move,
        'Touch'                 : SCons.Defaults.Touch,

        # Other variables we provide.
        'ARGUMENTS'             : Arguments,
        'ARGLIST'               : ArgList,
        'BUILD_TARGETS'         : BuildTargets,
        'COMMAND_LINE_TARGETS'  : CommandLineTargets,
        'DEFAULT_TARGETS'       : DefaultTargets,
    }

    # Functions we might still convert to Environment methods.
    globals['CScan']             = SCons.Defaults.CScan
    globals['DefaultEnvironment'] = SCons.Defaults.DefaultEnvironment

    # Deprecated functions, leave these here for now.
    globals['GetJobs']           = GetJobs
    globals['ParseConfig']       = ParseConfig
    globals['SetBuildSignatureType'] = SetBuildSignatureType
    globals['SetContentSignatureType'] = SetContentSignatureType
    globals['SetJobs']           = SetJobs

    globals.update(GlobalDict)

    return globals
