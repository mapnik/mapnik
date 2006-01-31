"""SCons.Environment

Base class for construction Environments.  These are
the primary objects used to communicate dependency and
construction information to the build engine.

Keyword arguments supplied when the construction Environment
is created are construction variables used to initialize the
Environment 
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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Environment.py 0.96.1.D001 2004/08/23 09:55:29 knight"


import copy
import os
import os.path
import string
from UserDict import UserDict

import SCons.Action
import SCons.Builder
from SCons.Debug import logInstanceCreation
import SCons.Defaults
import SCons.Errors
import SCons.Node
import SCons.Node.Alias
import SCons.Node.FS
import SCons.Node.Python
import SCons.Platform
import SCons.SConsign
import SCons.Sig
import SCons.Sig.MD5
import SCons.Sig.TimeStamp
import SCons.Tool
import SCons.Util
import SCons.Warnings

class _Null:
    pass

_null = _Null

CleanTargets = {}
CalculatorArgs = {}

# Pull UserError into the global name space for the benefit of
# Environment().SourceSignatures(), which has some import statements
# which seem to mess up its ability to reference SCons directly.
UserError = SCons.Errors.UserError

def installFunc(target, source, env):
    """Install a source file into a target using the function specified
    as the INSTALL construction variable."""
    try:
        install = env['INSTALL']
    except KeyError:
        raise SCons.Errors.UserError('Missing INSTALL construction variable.')
    return install(target[0].path, source[0].path, env)

def installString(target, source, env):
    return 'Install file: "%s" as "%s"' % (source[0], target[0])

installAction = SCons.Action.Action(installFunc, installString)

InstallBuilder = SCons.Builder.Builder(action=installAction)

def alias_builder(env, target, source):
    pass

AliasBuilder = SCons.Builder.Builder(action = alias_builder,
                                     target_factory = SCons.Node.Alias.default_ans.Alias,
                                     source_factory = SCons.Node.FS.default_fs.Entry,
                                     multi = 1)

def our_deepcopy(x):
   """deepcopy lists and dictionaries, and just copy the reference
   for everything else."""
   if SCons.Util.is_Dict(x):
       copy = {}
       for key in x.keys():
           copy[key] = our_deepcopy(x[key])
   elif SCons.Util.is_List(x):
       copy = map(our_deepcopy, x)
       try:
           copy = x.__class__(copy)
       except AttributeError:
           pass
   else:
       copy = x
   return copy

def apply_tools(env, tools, toolpath):
    if tools:
        # Filter out null tools from the list.
        tools = filter(None, tools)
        for tool in tools:
            if SCons.Util.is_String(tool):
                env.Tool(tool, toolpath)
            else:
                tool(env)

# These names are controlled by SCons; users should never set or override
# them.  This warning can optionally be turned off, but scons will still
# ignore the illegal variable names even if it's off.
reserved_construction_var_names = \
    ['TARGET', 'TARGETS', 'SOURCE', 'SOURCES']

def copy_non_reserved_keywords(dict):
    result = our_deepcopy(dict)
    for k in result.keys():
        if k in reserved_construction_var_names:
            SCons.Warnings.warn(SCons.Warnings.ReservedVariableWarning,
                                "Ignoring attempt to set reserved variable `%s'" % k)
            del result[k]
    return result

class BuilderWrapper:
    """Wrapper class that associates an environment with a Builder at
    instantiation."""
    def __init__(self, env, builder):
        self.env = env
        self.builder = builder

    def __call__(self, *args, **kw):
        return apply(self.builder, (self.env,) + args, kw)

    # This allows a Builder to be executed directly
    # through the Environment to which it's attached.
    # In practice, we shouldn't need this, because
    # builders actually get executed through a Node.
    # But we do have a unit test for this, and can't
    # yet rule out that it would be useful in the
    # future, so leave it for now.
    def execute(self, **kw):
        kw['env'] = self.env
        apply(self.builder.execute, (), kw)

class BuilderDict(UserDict):
    """This is a dictionary-like class used by an Environment to hold
    the Builders.  We need to do this because every time someone changes
    the Builders in the Environment's BUILDERS dictionary, we must
    update the Environment's attributes."""
    def __init__(self, dict, env):
        # Set self.env before calling the superclass initialization,
        # because it will end up calling our other methods, which will
        # need to point the values in this dictionary to self.env.
        self.env = env
        UserDict.__init__(self, dict)

    def __setitem__(self, item, val):
        UserDict.__setitem__(self, item, val)
        try:
            self.setenvattr(item, val)
        except AttributeError:
            # Have to catch this because sometimes __setitem__ gets
            # called out of __init__, when we don't have an env
            # attribute yet, nor do we want one!
            pass

    def setenvattr(self, item, val):
        """Set the corresponding environment attribute for this Builder.

        If the value is already a BuilderWrapper, we pull the builder
        out of it and make another one, so that making a copy of an
        existing BuilderDict is guaranteed separate wrappers for each
        Builder + Environment pair."""
        try:
            builder = val.builder
        except AttributeError:
            builder = val
        setattr(self.env, item, BuilderWrapper(self.env, builder))

    def __delitem__(self, item):
        UserDict.__delitem__(self, item)
        delattr(self.env, item)

    def update(self, dict):
        for i, v in dict.items():
            self.__setitem__(i, v)

class Base:
    """Base class for construction Environments.  These are
    the primary objects used to communicate dependency and
    construction information to the build engine.

    Keyword arguments supplied when the construction Environment
    is created are construction variables used to initialize the
    Environment.
    """

    #######################################################################
    # This is THE class for interacting with the SCons build engine,
    # and it contains a lot of stuff, so we're going to try to keep this
    # a little organized by grouping the methods.
    #######################################################################

    #######################################################################
    # Methods that make an Environment act like a dictionary.  These have
    # the expected standard names for Python mapping objects.  Note that
    # we don't actually make an Environment a subclass of UserDict for
    # performance reasons.  Note also that we only supply methods for
    # dictionary functionality that we actually need and use.
    #######################################################################

    def __init__(self,
                 platform=None,
                 tools=None,
                 toolpath=[],
                 options=None,
                 **kw):
        if __debug__: logInstanceCreation(self)
        self.fs = SCons.Node.FS.default_fs
        self.ans = SCons.Node.Alias.default_ans
        self.lookup_list = SCons.Node.arg2nodes_lookups
        self._dict = our_deepcopy(SCons.Defaults.ConstructionEnvironment)

        self._dict['__env__'] = self
        self._dict['BUILDERS'] = BuilderDict(self._dict['BUILDERS'], self)

        if platform is None:
            platform = self._dict.get('PLATFORM', None)
            if platform is None:
                platform = SCons.Platform.Platform()
        if SCons.Util.is_String(platform):
            platform = SCons.Platform.Platform(platform)
        self._dict['PLATFORM'] = str(platform)
        platform(self)

        # Apply the passed-in variables before calling the tools,
        # because they may use some of them:
        apply(self.Replace, (), kw)
        
        # Update the environment with the customizable options
        # before calling the tools, since they may use some of the options: 
        if options:
            options.Update(self)

        if tools is None:
            tools = self._dict.get('TOOLS', None)
            if tools is None:
                tools = ['default']
        apply_tools(self, tools, toolpath)

        # Reapply the passed in variables after calling the tools,
        # since they should overide anything set by the tools:
        apply(self.Replace, (), kw)

        # Update the environment with the customizable options
        # after calling the tools, since they should override anything
        # set by the tools:
        if options:
            options.Update(self)

    def __cmp__(self, other):
        # Since an Environment now has an '__env__' construction variable
        # that refers to itself, delete that variable to avoid infinite
        # loops when comparing the underlying dictionaries in some Python
        # versions (*cough* 1.5.2 *cough*)...
        sdict = self._dict.copy()
        del sdict['__env__']
        odict = other._dict.copy()
        del odict['__env__']
        return cmp(sdict, odict)

    def __getitem__(self, key):
        return self._dict[key]

    def __setitem__(self, key, value):
        if key in reserved_construction_var_names:
            SCons.Warnings.warn(SCons.Warnings.ReservedVariableWarning,
                                "Ignoring attempt to set reserved variable `%s'" % key)
        elif key == 'BUILDERS':
            try:
                bd = self._dict[key]
                for k in bd.keys():
                    del bd[k]
            except KeyError:
                self._dict[key] = BuilderDict(kwbd, self)
            self._dict[key].update(value)
        elif key == 'SCANNERS':
            self._dict[key] = value
            self.scanner_map_delete()
        else:
            if not SCons.Util.is_valid_construction_var(key):
                raise SCons.Errors.UserError, "Illegal construction variable `%s'" % key
            self._dict[key] = value

    def __delitem__(self, key):
        del self._dict[key]

    def items(self):
        "Emulates the items() method of dictionaries."""
        return self._dict.items()

    def has_key(self, key):
        return self._dict.has_key(key)

    def get(self, key, default=None):
        "Emulates the get() method of dictionaries."""
        return self._dict.get(key, default)

    #######################################################################
    # Utility methods that are primarily for internal use by SCons.
    # These begin with lower-case letters.  Note that the subst() method
    # is actually already out of the closet and used by people.
    #######################################################################

    def arg2nodes(self, args, node_factory=_null, lookup_list=_null):
        if node_factory is _null:
            node_factory = self.fs.File
        if lookup_list is _null:
            lookup_list = self.lookup_list

        if not args:
            return []

        if SCons.Util.is_List(args):
            args = SCons.Util.flatten(args)
        else:
            args = [args]

        nodes = []
        for v in args:
            if SCons.Util.is_String(v):
                n = None
                for l in lookup_list:
                    n = l(v)
                    if not n is None:
                        break
                if not n is None:
                    if SCons.Util.is_String(n):
                        n = self.subst(n, raw=1)
                        if node_factory:
                            n = node_factory(n)
                    if SCons.Util.is_List(n):
                        nodes.extend(n)
                    else:
                        nodes.append(n)
                elif node_factory:
                    v = node_factory(self.subst(v, raw=1))
                    if SCons.Util.is_List(v):
                        nodes.extend(v)
                    else:
                        nodes.append(v)
            else:
                nodes.append(v)
    
        return nodes

    def get_calculator(self):
        try:
            return self._calculator
        except AttributeError:
            try:
                module = self._calc_module
                c = apply(SCons.Sig.Calculator, (module,), CalculatorArgs)
            except AttributeError:
                # Note that we're calling get_calculator() here, so the
                # DefaultEnvironment() must have a _calc_module attribute
                # to avoid infinite recursion.
                c = SCons.Defaults.DefaultEnvironment().get_calculator()
            self._calculator = c
            return c

    def get_builder(self, name):
        """Fetch the builder with the specified name from the environment.
        """
        try:
            return self._dict['BUILDERS'][name]
        except KeyError:
            return None

    def get_scanner(self, skey):
        """Find the appropriate scanner given a key (usually a file suffix).
        """
        try:
            sm = self.scanner_map
        except AttributeError:
            try:
                scanners = self._dict['SCANNERS']
            except KeyError:
                self.scanner_map = {}
                return None
            else:
                self.scanner_map = sm = {}
                # Reverse the scanner list so that, if multiple scanners
                # claim they can scan the same suffix, earlier scanners
                # in the list will overwrite later scanners, so that
                # the result looks like a "first match" to the user.
                if not SCons.Util.is_List(scanners):
                    scanners = [scanners]
                scanners.reverse()
                for scanner in scanners:
                    for k in scanner.get_skeys(self):
                        sm[k] = scanner
        try:
            return sm[skey]
        except KeyError:
            return None

    def scanner_map_delete(self, kw=None):
        """Delete the cached scanner map (if we need to).
        """
        if not kw is None and not kw.has_key('SCANNERS'):
            return
        try:
            del self.scanner_map
        except AttributeError:
            pass

    def subst(self, string, raw=0, target=None, source=None, dict=None, conv=None):
        """Recursively interpolates construction variables from the
        Environment into the specified string, returning the expanded
        result.  Construction variables are specified by a $ prefix
        in the string and begin with an initial underscore or
        alphabetic character followed by any number of underscores
        or alphanumeric characters.  The construction variable names
        may be surrounded by curly braces to separate the name from
        trailing characters.
        """
        return SCons.Util.scons_subst(string, self, raw, target, source, dict, conv)

    def subst_kw(self, kw, raw=0, target=None, source=None, dict=None):
        nkw = {}
        for k, v in kw.items():
            k = self.subst(k, raw, target, source, dict)
            if SCons.Util.is_String(v):
                v = self.subst(v, raw, target, source, dict)
            nkw[k] = v
        return nkw

    def subst_list(self, string, raw=0, target=None, source=None, dict=None, conv=None):
        """Calls through to SCons.Util.scons_subst_list().  See
        the documentation for that function."""
        return SCons.Util.scons_subst_list(string, self, raw, target, source, dict, conv)


    def subst_path(self, path):
        """Substitute a path list, turning EntryProxies into Nodes
        and leaving Nodes (and other objects) as-is."""

        if not SCons.Util.is_List(path):
            path = [path]

        def s(obj):
            """This is the "string conversion" routine that we have our
            substitutions use to return Nodes, not strings.  This relies
            on the fact that an EntryProxy object has a get() method that
            returns the underlying Node that it wraps, which is a bit of
            architectural dependence that we might need to break or modify
            in the future in response to additional requirements."""
            try:
                get = obj.get
            except AttributeError:
                pass
            else:
                obj = get()
            return obj

        r = []
        for p in path:
            if SCons.Util.is_String(p):
                p = self.subst(p, conv=s)
                if SCons.Util.is_List(p):
                    if len(p) == 1:
                        p = p[0]
                    else:
                        # We have an object plus a string, or multiple
                        # objects that we need to smush together.  No choice
                        # but to make them into a string.
                        p = string.join(map(SCons.Util.to_String, p), '')
            else:
                p = s(p)
            r.append(p)
        return r

    subst_target_source = subst

    def _update(self, dict):
        """Update an environment's values directly, bypassing the normal
        checks that occur when users try to set items.
        """
        self._dict.update(dict)

    def use_build_signature(self):
        try:
            return self._build_signature
        except AttributeError:
            b = SCons.Defaults.DefaultEnvironment()._build_signature
            self._build_signature = b
            return b

    #######################################################################
    # Public methods for manipulating an Environment.  These begin with
    # upper-case letters.  The essential characteristic of methods in
    # this section is that they do *not* have corresponding same-named
    # global functions.  For example, a stand-alone Append() function
    # makes no sense, because Append() is all about appending values to
    # an Environment's construction variables.
    #######################################################################

    def Append(self, **kw):
        """Append values to existing construction variables
        in an Environment.
        """
        kw = copy_non_reserved_keywords(kw)
        for key, val in kw.items():
            # It would be easier on the eyes to write this using
            # "continue" statements whenever we finish processing an item,
            # but Python 1.5.2 apparently doesn't let you use "continue"
            # within try:-except: blocks, so we have to nest our code.
            try:
                orig = self._dict[key]
            except KeyError:
                # No existing variable in the environment, so just set
                # it to the new value.
                self._dict[key] = val
            else:
                try:
                    # Most straightforward:  just try to add them
                    # together.  This will work in most cases, when the
                    # original and new values are of compatible types.
                    self._dict[key] = orig + val
                except TypeError:
                    try:
                        # Try to update a dictionary value with another.
                        # If orig isn't a dictionary, it won't have an
                        # update() method; if val isn't a dictionary,
                        # it won't have a keys() method.  Either way,
                        # it's an AttributeError.
                        orig.update(val)
                    except AttributeError:
                        try:
                            # Check if the original is a list.
                            add_to_orig = orig.append
                        except AttributeError:
                            # The original isn't a list, but the new
                            # value is (by process of elimination),
                            # so insert the original in the new value
                            # (if there's one to insert) and replace
                            # the variable with it.
                            if orig:
                                val.insert(0, orig)
                            self._dict[key] = val
                        else:
                            # The original is a list, so append the new
                            # value to it (if there's a value to append).
                            if val:
                                add_to_orig(val)
        self.scanner_map_delete(kw)

    def AppendENVPath(self, name, newpath, envname = 'ENV', sep = os.pathsep):
        """Append path elements to the path 'name' in the 'ENV'
        dictionary for this environment.  Will only add any particular
        path once, and will normpath and normcase all paths to help
        assure this.  This can also handle the case where the env
        variable is a list instead of a string.
        """

        orig = ''
        if self._dict.has_key(envname) and self._dict[envname].has_key(name):
            orig = self._dict[envname][name]

        nv = SCons.Util.AppendPath(orig, newpath, sep)
            
        if not self._dict.has_key(envname):
            self._dict[envname] = {}

        self._dict[envname][name] = nv

    def AppendUnique(self, **kw):
        """Append values to existing construction variables
        in an Environment, if they're not already there.
        """
        kw = copy_non_reserved_keywords(kw)
        for key, val in kw.items():
            if not self._dict.has_key(key):
                self._dict[key] = val
            elif SCons.Util.is_Dict(self._dict[key]) and \
                 SCons.Util.is_Dict(val):
                self._dict[key].update(val)
            elif SCons.Util.is_List(val):
                dk = self._dict[key]
                if not SCons.Util.is_List(dk):
                    dk = [dk]
                val = filter(lambda x, dk=dk: x not in dk, val)
                self._dict[key] = dk + val
            else:
                dk = self._dict[key]
                if SCons.Util.is_List(dk):
                    if not val in dk:
                        self._dict[key] = dk + val
                else:
                    self._dict[key] = self._dict[key] + val
        self.scanner_map_delete(kw)

    def Copy(self, tools=None, toolpath=[], **kw):
        """Return a copy of a construction Environment.  The
        copy is like a Python "deep copy"--that is, independent
        copies are made recursively of each objects--except that
        a reference is copied when an object is not deep-copyable
        (like a function).  There are no references to any mutable
        objects in the original Environment.
        """
        clone = copy.copy(self)
        clone._dict = our_deepcopy(self._dict)
        clone['__env__'] = clone
        try:
            cbd = clone._dict['BUILDERS']
            clone._dict['BUILDERS'] = BuilderDict(cbd, clone)
        except KeyError:
            pass
        
        apply_tools(clone, tools, toolpath)

        # Apply passed-in variables after the new tools.
        kw = copy_non_reserved_keywords(kw)
        new = {}
        for key, value in kw.items():
            new[key] = SCons.Util.scons_subst_once(value, self, key)
        apply(clone.Replace, (), new)
        return clone

    def Detect(self, progs):
        """Return the first available program in progs.
        """
        if not SCons.Util.is_List(progs):
            progs = [ progs ]
        for prog in progs:
            path = self.WhereIs(prog)
            if path: return prog
        return None

    def Dictionary(self, *args):
	if not args:
	    return self._dict
	dlist = map(lambda x, s=self: s._dict[x], args)
	if len(dlist) == 1:
	    dlist = dlist[0]
	return dlist

    def FindIxes(self, paths, prefix, suffix):
        """
        Search a list of paths for something that matches the prefix and suffix.

        paths - the list of paths or nodes.
        prefix - construction variable for the prefix.
        suffix - construction variable for the suffix.
        """

        suffix = self.subst('$'+suffix)
        prefix = self.subst('$'+prefix)

        for path in paths:
            dir,name = os.path.split(str(path))
            if name[:len(prefix)] == prefix and name[-len(suffix):] == suffix: 
                return path

    def Override(self, overrides):
        """
        Produce a modified environment whose variables
        are overriden by the overrides dictionaries.

        overrides - a dictionary that will override
        the variables of this environment.

        This function is much more efficient than Copy()
        or creating a new Environment because it doesn't do
        a deep copy of the dictionary, and doesn't do a copy
        at all if there are no overrides.
        """

        if overrides:
            env = copy.copy(self)
            env._dict = copy.copy(self._dict)
            env['__env__'] = env
            overrides = copy_non_reserved_keywords(overrides)
            new = {}
            for key, value in overrides.items():
                new[key] = SCons.Util.scons_subst_once(value, self, key)
            env._dict.update(new)
            return env
        else:
            return self

    def ParseConfig(self, command, function=None):
        """
        Use the specified function to parse the output of the command
        in order to modify the current environment. The 'command' can
        be a string or a list of strings representing a command and
        it's arguments. 'Function' is an optional argument that takes
        the environment and the output of the command. If no function is
        specified, the output will be treated as the output of a typical
        'X-config' command (i.e. gtk-config) and used to append to the
        ASFLAGS, CCFLAGS, CPPFLAGS, CPPPATH, LIBPATH, LIBS, LINKFLAGS
        and CCFLAGS variables.
        """

        # the default parse function
        def parse_conf(env, output):
            dict = {
                'ASFLAGS'       : [],
                'CCFLAGS'       : [],
                'CPPFLAGS'      : [],
                'CPPPATH'       : [],
                'LIBPATH'       : [],
                'LIBS'          : [],
                'LINKFLAGS'     : [],
            }
            static_libs = []
    
            params = string.split(output)
            for arg in params:
                if arg[0] != '-':
                    static_libs.append(arg)
                elif arg[:2] == '-L':
                    dict['LIBPATH'].append(arg[2:])
                elif arg[:2] == '-l':
                    dict['LIBS'].append(arg[2:])
                elif arg[:2] == '-I':
                    dict['CPPPATH'].append(arg[2:])
                elif arg[:4] == '-Wa,':
                    dict['ASFLAGS'].append(arg)
                elif arg[:4] == '-Wl,':
                    dict['LINKFLAGS'].append(arg)
                elif arg[:4] == '-Wp,':
                    dict['CPPFLAGS'].append(arg)
                elif arg == '-pthread':
                    dict['CCFLAGS'].append(arg)
                    dict['LINKFLAGS'].append(arg)
                else:
                    dict['CCFLAGS'].append(arg)
            apply(env.Append, (), dict)
            return static_libs
    
        if function is None:
            function = parse_conf
        if type(command) is type([]):
            command = string.join(command)
        command = self.subst(command)
        return function(self, os.popen(command).read())

    def Platform(self, platform):
        platform = self.subst(platform)
        return SCons.Platform.Platform(platform)(self)

    def Prepend(self, **kw):
        """Prepend values to existing construction variables
        in an Environment.
        """
        kw = copy_non_reserved_keywords(kw)
        for key, val in kw.items():
            # It would be easier on the eyes to write this using
            # "continue" statements whenever we finish processing an item,
            # but Python 1.5.2 apparently doesn't let you use "continue"
            # within try:-except: blocks, so we have to nest our code.
            try:
                orig = self._dict[key]
            except KeyError:
                # No existing variable in the environment, so just set
                # it to the new value.
                self._dict[key] = val
            else:
                try:
                    # Most straightforward:  just try to add them
                    # together.  This will work in most cases, when the
                    # original and new values are of compatible types.
                    self._dict[key] = val + orig
                except TypeError:
                    try:
                        # Try to update a dictionary value with another.
                        # If orig isn't a dictionary, it won't have an
                        # update() method; if val isn't a dictionary,
                        # it won't have a keys() method.  Either way,
                        # it's an AttributeError.
                        orig.update(val)
                    except AttributeError:
                        try:
                            # Check if the added value is a list.
                            add_to_val = val.append
                        except AttributeError:
                            # The added value isn't a list, but the
                            # original is (by process of elimination),
                            # so insert the the new value in the original
                            # (if there's one to insert).
                            if val:
                                orig.insert(0, val)
                        else:
                            # The added value is a list, so append
                            # the original to it (if there's a value
                            # to append).
                            if orig:
                                add_to_val(orig)
                            self._dict[key] = val
        self.scanner_map_delete(kw)

    def PrependENVPath(self, name, newpath, envname = 'ENV', sep = os.pathsep):
        """Prepend path elements to the path 'name' in the 'ENV'
        dictionary for this environment.  Will only add any particular
        path once, and will normpath and normcase all paths to help
        assure this.  This can also handle the case where the env
        variable is a list instead of a string.
        """

        orig = ''
        if self._dict.has_key(envname) and self._dict[envname].has_key(name):
            orig = self._dict[envname][name]

        nv = SCons.Util.PrependPath(orig, newpath, sep)
            
        if not self._dict.has_key(envname):
            self._dict[envname] = {}

        self._dict[envname][name] = nv

    def PrependUnique(self, **kw):
        """Append values to existing construction variables
        in an Environment, if they're not already there.
        """
        kw = copy_non_reserved_keywords(kw)
        for key, val in kw.items():
            if not self._dict.has_key(key):
                self._dict[key] = val
            elif SCons.Util.is_Dict(self._dict[key]) and \
                 SCons.Util.is_Dict(val):
                self._dict[key].update(val)
            elif SCons.Util.is_List(val):
                dk = self._dict[key]
                if not SCons.Util.is_List(dk):
                    dk = [dk]
                val = filter(lambda x, dk=dk: x not in dk, val)
                self._dict[key] = val + dk
            else:
                dk = self._dict[key]
                if SCons.Util.is_List(dk):
                    if not val in dk:
                        self._dict[key] = val + dk
                else:
                    self._dict[key] = val + dk
        self.scanner_map_delete(kw)

    def Replace(self, **kw):
        """Replace existing construction variables in an Environment
        with new construction variables and/or values.
        """
        try:
            kwbd = our_deepcopy(kw['BUILDERS'])
            del kw['BUILDERS']
            self.__setitem__('BUILDERS', kwbd)
        except KeyError:
            pass
        kw = copy_non_reserved_keywords(kw)
        self._dict.update(our_deepcopy(kw))
        self.scanner_map_delete(kw)

    def ReplaceIxes(self, path, old_prefix, old_suffix, new_prefix, new_suffix):
        """
        Replace old_prefix with new_prefix and old_suffix with new_suffix.

        env - Environment used to interpolate variables.
        path - the path that will be modified.
        old_prefix - construction variable for the old prefix.
        old_suffix - construction variable for the old suffix.
        new_prefix - construction variable for the new prefix.
        new_suffix - construction variable for the new suffix.
        """
        old_prefix = self.subst('$'+old_prefix)
        old_suffix = self.subst('$'+old_suffix)

        new_prefix = self.subst('$'+new_prefix)
        new_suffix = self.subst('$'+new_suffix)

        dir,name = os.path.split(str(path))
        if name[:len(old_prefix)] == old_prefix:
            name = name[len(old_prefix):]
        if name[-len(old_suffix):] == old_suffix:
            name = name[:-len(old_suffix)]
        return os.path.join(dir, new_prefix+name+new_suffix)

    def Tool(self, tool, toolpath=[]):
        tool = self.subst(tool)
        return SCons.Tool.Tool(tool, map(self.subst, toolpath))(self)

    def WhereIs(self, prog, path=None, pathext=None, reject=[]):
        """Find prog in the path.  
        """
        if path is None:
            try:
                path = self['ENV']['PATH']
            except KeyError:
                pass
        elif SCons.Util.is_String(path):
            path = self.subst(path)
        if pathext is None:
            try:
                pathext = self['ENV']['PATHEXT']
            except KeyError:
                pass
        elif SCons.Util.is_String(pathext):
            pathext = self.subst(pathext)
        path = SCons.Util.WhereIs(prog, path, pathext, reject)
        if path: return path
        return None

    #######################################################################
    # Public methods for doing real "SCons stuff" (manipulating
    # dependencies, setting attributes on targets, etc.).  These begin
    # with upper-case letters.  The essential characteristic of methods
    # in this section is that they all *should* have corresponding
    # same-named global functions.
    #######################################################################

    def Action(self, *args, **kw):
        nargs = self.subst(args)
        nkw = self.subst_kw(kw)
        return apply(SCons.Action.Action, nargs, nkw)

    def AddPreAction(self, files, action):
        nodes = self.arg2nodes(files, self.fs.Entry)
        action = SCons.Action.Action(action)
        for n in nodes:
            n.add_pre_action(action)
        return nodes
    
    def AddPostAction(self, files, action):
        nodes = self.arg2nodes(files, self.fs.Entry)
        action = SCons.Action.Action(action)
        for n in nodes:
            n.add_post_action(action)
        return nodes

    def Alias(self, target, *source, **kw):
        if not SCons.Util.is_List(target):
            target = [target]
        tlist = []
        for t in target:
            if not isinstance(t, SCons.Node.Alias.Alias):
                t = self.arg2nodes(self.subst(t), self.ans.Alias)[0]
            tlist.append(t)
        try:
            s = kw['source']
        except KeyError:
            try:
                s = source[0]
            except IndexError:
                s = None
        if s:
            if not SCons.Util.is_List(s):
                s = [s]
            s = filter(None, s)
            s = self.arg2nodes(s, self.fs.Entry)
            for t in tlist:
                AliasBuilder(self, t, s)
        return tlist

    def AlwaysBuild(self, *targets):
        tlist = []
        for t in targets:
            tlist.extend(self.arg2nodes(t, self.fs.File))
        for t in tlist:
            t.set_always_build()
        return tlist

    def BuildDir(self, build_dir, src_dir, duplicate=1):
        build_dir = self.arg2nodes(build_dir, self.fs.Dir)[0]
        src_dir = self.arg2nodes(src_dir, self.fs.Dir)[0]
        self.fs.BuildDir(build_dir, src_dir, duplicate)

    def Builder(self, **kw):
        nkw = self.subst_kw(kw)
        return apply(SCons.Builder.Builder, [], nkw)

    def CacheDir(self, path):
        self.fs.CacheDir(self.subst(path))

    def Clean(self, targets, files):
        global CleanTargets
        tlist = self.arg2nodes(targets, self.fs.Entry)
        flist = self.arg2nodes(files, self.fs.Entry)
        for t in tlist:
            try:
                CleanTargets[t].extend(flist)
            except KeyError:
                CleanTargets[t] = flist

    def Configure(self, *args, **kw):
        nargs = [self]
        if args:
            nargs = nargs + self.subst_list(args)[0]
        nkw = self.subst_kw(kw)
        try:
            nkw['custom_tests'] = self.subst_kw(nkw['custom_tests'])
        except KeyError:
            pass
        return apply(SCons.SConf.SConf, nargs, nkw)

    def Command(self, target, source, action, **kw):
        """Builds the supplied target files from the supplied
        source files using the supplied action.  Action may
        be any type that the Builder constructor will accept
        for an action."""
        nkw = self.subst_kw(kw)
        nkw['action'] = action
        nkw['source_factory'] = self.fs.Entry
        bld = apply(SCons.Builder.Builder, (), nkw)
        return bld(self, target, source)

    def Depends(self, target, dependency):
        """Explicity specify that 'target's depend on 'dependency'."""
        tlist = self.arg2nodes(target, self.fs.Entry)
        dlist = self.arg2nodes(dependency, self.fs.Entry)
        for t in tlist:
            t.add_dependency(dlist)
        return tlist

    def Dir(self, name, *args, **kw):
        """
        """
        return apply(self.fs.Dir, (self.subst(name),) + args, kw)

    def Environment(self, **kw):
        return apply(SCons.Environment.Environment, [], self.subst_kw(kw))

    def Execute(self, action, *args, **kw):
        """Directly execute an action through an Environment
        """
        action = apply(self.Action, (action,) + args, kw)
        return action([], [], self)

    def File(self, name, *args, **kw):
        """
        """
        return apply(self.fs.File, (self.subst(name),) + args, kw)

    def FindFile(self, file, dirs):
        file = self.subst(file)
        nodes = self.arg2nodes(dirs, self.fs.Dir)
        return SCons.Node.FS.find_file(file, nodes, self.fs.File)

    def Flatten(self, sequence):
        return SCons.Util.flatten(sequence)

    def GetBuildPath(self, files):
        result = map(str, self.arg2nodes(files, self.fs.Entry))
        if SCons.Util.is_List(files):
            return result
        else:
            return result[0]

    def Ignore(self, target, dependency):
        """Ignore a dependency."""
        tlist = self.arg2nodes(target, self.fs.Entry)
        dlist = self.arg2nodes(dependency, self.fs.Entry)
        for t in tlist:
            t.add_ignore(dlist)
        return tlist

    def Install(self, dir, source):
        """Install specified files in the given directory."""
        try:
            dnodes = self.arg2nodes(dir, self.fs.Dir)
        except TypeError:
            raise SCons.Errors.UserError, "Target `%s' of Install() is a file, but should be a directory.  Perhaps you have the Install() arguments backwards?" % str(dir)
        try:
            sources = self.arg2nodes(source, self.fs.File)
        except TypeError:
            if SCons.Util.is_List(source):
                raise SCons.Errors.UserError, "Source `%s' of Install() contains one or more non-files.  Install() source must be one or more files." % repr(map(str, source))
            else:
                raise SCons.Errors.UserError, "Source `%s' of Install() is not a file.  Install() source must be one or more files." % str(source)
        tgt = []
        for dnode in dnodes:
            for src in sources:
                target = self.fs.File(src.name, dnode)
                tgt.extend(InstallBuilder(self, target, src))
        return tgt

    def InstallAs(self, target, source):
        """Install sources as targets."""
        sources = self.arg2nodes(source, self.fs.File)
        targets = self.arg2nodes(target, self.fs.File)
        result = []
        for src, tgt in map(lambda x, y: (x, y), sources, targets):
            result.extend(InstallBuilder(self, tgt, src))
        return result

    def Literal(self, string):
        return SCons.Util.Literal(string)

    def Local(self, *targets):
        ret = []
        for targ in targets:
            if isinstance(targ, SCons.Node.Node):
                targ.set_local()
                ret.append(targ)
            else:
                for t in self.arg2nodes(targ, self.fs.Entry):
                   t.set_local()
                   ret.append(t)
        return ret

    def Precious(self, *targets):
        tlist = []
        for t in targets:
            tlist.extend(self.arg2nodes(t, self.fs.Entry))
        for t in tlist:
            t.set_precious()
        return tlist

    def Repository(self, *dirs, **kw):
        dirs = self.arg2nodes(list(dirs), self.fs.Dir)
        apply(self.fs.Repository, dirs, kw)

    def Scanner(self, *args, **kw):
        nargs = []
        for arg in args:
            if SCons.Util.is_String(arg):
                arg = self.subst(arg)
            nargs.append(arg)
        nkw = self.subst_kw(kw)
        return apply(SCons.Scanner.Scanner, nargs, nkw)

    def SConsignFile(self, name=".sconsign", dbm_module=None):
        name = self.subst(name)
        if not os.path.isabs(name):
            name = os.path.join(str(self.fs.SConstruct_dir), name)
        SCons.SConsign.File(name, dbm_module)

    def SideEffect(self, side_effect, target):
        """Tell scons that side_effects are built as side 
        effects of building targets."""
        side_effects = self.arg2nodes(side_effect, self.fs.Entry)
        targets = self.arg2nodes(target, self.fs.Entry)

        for side_effect in side_effects:
            if side_effect.multiple_side_effect_has_builder():
                raise SCons.Errors.UserError, "Multiple ways to build the same target were specified for: %s" % str(side_effect)
            side_effect.add_source(targets)
            side_effect.side_effect = 1
            self.Precious(side_effect)
            for target in targets:
                target.side_effects.append(side_effect)
        return side_effects

    def SourceCode(self, entry, builder):
        """Arrange for a source code builder for (part of) a tree."""
        entries = self.arg2nodes(entry, self.fs.Entry)
        for entry in entries:
            entry.set_src_builder(builder)
        return entries

    def SourceSignatures(self, type):
        type = self.subst(type)
        if type == 'MD5':
            import SCons.Sig.MD5
            self._calc_module = SCons.Sig.MD5
        elif type == 'timestamp':
            import SCons.Sig.TimeStamp
            self._calc_module = SCons.Sig.TimeStamp
        else:
            raise UserError, "Unknown source signature type '%s'"%type

    def Split(self, arg):
        """This function converts a string or list into a list of strings
        or Nodes.  This makes things easier for users by allowing files to
        be specified as a white-space separated list to be split.
        The input rules are:
            - A single string containing names separated by spaces. These will be
              split apart at the spaces.
            - A single Node instance
            - A list containing either strings or Node instances. Any strings
              in the list are not split at spaces.
        In all cases, the function returns a list of Nodes and strings."""
        if SCons.Util.is_List(arg):
            return map(self.subst, arg)
        elif SCons.Util.is_String(arg):
            return string.split(self.subst(arg))
        else:
            return [self.subst(arg)]

    def TargetSignatures(self, type):
        type = self.subst(type)
        if type == 'build':
            self._build_signature = 1
        elif type == 'content':
            self._build_signature = 0
        else:
            raise SCons.Errors.UserError, "Unknown target signature type '%s'"%type

    def Value(self, value):
        """
        """
        return SCons.Node.Python.Value(value)

# The entry point that will be used by the external world
# to refer to a construction environment.  This allows the wrapper
# interface to extend a construction environment for its own purposes
# by subclassing SCons.Environment.Base and then assigning the
# class to SCons.Environment.Environment.

Environment = Base

# An entry point for returning a proxy subclass instance that overrides
# the subst*() methods so they don't actually perform construction
# variable substitution.  This is specifically intended to be the shim
# layer in between global function calls (which don't want construction
# variable substitution) and the DefaultEnvironment() (which would
# substitute variables if left to its own devices)."""
#
# We have to wrap this in a function that allows us to delay definition of
# the class until it's necessary, so that when it subclasses Environment
# it will pick up whatever Environment subclass the wrapper interface
# might have assigned to SCons.Environment.Environment.

def NoSubstitutionProxy(subject):
    class _NoSubstitutionProxy(Environment):
        def __init__(self, subject):
            self.__dict__['__subject'] = subject
        def __getattr__(self, name):
            return getattr(self.__dict__['__subject'], name)
        def __setattr__(self, name, value):
            return setattr(self.__dict__['__subject'], name, value)
        def raw_to_mode(self, dict):
            try:
                raw = dict['raw']
            except KeyError:
                pass
            else:
                del dict['raw']
                dict['mode'] = raw
        def subst(self, string, *args, **kwargs):
            return string
        def subst_kw(self, kw, *args, **kwargs):
            return kw
        def subst_list(self, string, *args, **kwargs):
            nargs = (string, self,) + args
            nkw = kwargs.copy()
            nkw['gvars'] = {}
            self.raw_to_mode(nkw)
            return apply(SCons.Util.scons_subst_list, nargs, nkw)
        def subst_target_source(self, string, *args, **kwargs):
            nargs = (string, self,) + args
            nkw = kwargs.copy()
            nkw['gvars'] = {}
            self.raw_to_mode(nkw)
            return apply(SCons.Util.scons_subst, nargs, nkw)
    return _NoSubstitutionProxy(subject)
