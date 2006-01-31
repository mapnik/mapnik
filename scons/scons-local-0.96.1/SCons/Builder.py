"""SCons.Builder

Builder object subsystem.

A Builder object is a callable that encapsulates information about how
to execute actions to create a target Node (file) from source Nodes
(files), and how to create those dependencies for tracking.

The main entry point here is the Builder() factory method.  This provides
a procedural interface that creates the right underlying Builder object
based on the keyword arguments supplied and the types of the arguments.

The goal is for this external interface to be simple enough that the
vast majority of users can create new Builders as necessary to support
building new types of files in their configurations, without having to
dive any deeper into this subsystem.

The base class here is BuilderBase.  This is a concrete base class which
does, in fact, represent most Builder objects that we (or users) create.

There is (at present) one subclasses:

    MultiStepBuilder

        This is a Builder that knows how to "chain" Builders so that
        users can specify a source file that requires multiple steps
        to turn into a target file.  A canonical example is building a
        program from yacc input file, which requires invoking a builder
        to turn the .y into a .c, the .c into a .o, and the .o into an
        executable program.

There is also two proxies that look like Builders:

    CompositeBuilder

        This proxies for a Builder with an action that is actually a
        dictionary that knows how to map file suffixes to a specific
        action.  This is so that we can invoke different actions
        (compilers, compile options) for different flavors of source
        files.

    ListBuilder

        This proxies for a Builder *invocation* where the target
        is a list of files, not a single file.

Builders and their proxies have the following public interface methods
used by other modules:

    __call__()
        THE public interface.  Calling a Builder object (with the
        use of internal helper methods) sets up the target and source
        dependencies, appropriate mapping to a specific action, and the
        environment manipulation necessary for overridden construction
        variable.  This also takes care of warning about possible mistakes
        in keyword arguments.

    targets()
        Returns the list of targets for a specific builder instance.

    add_emitter()
        Adds an emitter for a specific file suffix, used by some Tool
        modules to specify that (for example) a yacc invocation on a .y
        can create a .h *and* a .c file.

    add_action()
        Adds an action for a specific file suffix, heavily used by
        Tool modules to add their specific action(s) for turning
        a source file into an object file to the global static
        and shared object file Builders.

There are the following methods for internal use within this module:

    _execute()
        The internal method that handles the heavily lifting when a
        Builder is called.  This is used so that the __call__() methods
        can set up warning about possible mistakes in keyword-argument
        overrides, and *then* execute all of the steps necessary so that
        the warnings only occur once.

    get_name()
        Returns the Builder's name within a specific Environment,
        primarily used to try to return helpful information in error
        messages.

    adjust_suffix()
    get_prefix()
    get_suffix()
    get_src_suffix()
    set_src_suffix()
        Miscellaneous stuff for handling the prefix and suffix
        manipulation we use in turning source file names into target
        file names.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Builder.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import UserDict
import UserList

import SCons.Action
from SCons.Debug import logInstanceCreation
from SCons.Errors import InternalError, UserError
import SCons.Executor
import SCons.Node.FS
import SCons.Util
import SCons.Warnings

class _Null:
    pass

_null = _Null

class DictCmdGenerator(SCons.Util.Selector):
    """This is a callable class that can be used as a
    command generator function.  It holds on to a dictionary
    mapping file suffixes to Actions.  It uses that dictionary
    to return the proper action based on the file suffix of
    the source file."""

    def src_suffixes(self):
        return self.keys()

    def add_action(self, suffix, action):
        """Add a suffix-action pair to the mapping.
        """
        self[suffix] = action

    def __call__(self, target, source, env, for_signature):
        ext = None
        for src in map(str, source):
            my_ext = SCons.Util.splitext(src)[1]
            if ext and my_ext != ext:
                raise UserError("While building `%s' from `%s': Cannot build multiple sources with different extensions: %s, %s" % (repr(map(str, target)), src, ext, my_ext))
            ext = my_ext

        if not ext:
            raise UserError("While building `%s': Cannot deduce file extension from source files: %s" % (repr(map(str, target)), repr(map(str, source))))

        try:
            ret = SCons.Util.Selector.__call__(self, env, source)
        except KeyError, e:
            raise UserError("Ambiguous suffixes after environment substitution: %s == %s == %s" % (e[0], e[1], e[2]))
        if ret is None:
            raise UserError("While building `%s': Don't know how to build a file with suffix `%s'." % (repr(map(str, target)), ext))
        return ret

class CallableSelector(SCons.Util.Selector):
    """A callable dictionary that will, in turn, call the value it
    finds if it can."""
    def __call__(self, env, source):
        value = SCons.Util.Selector.__call__(self, env, source)
        if callable(value):
            value = value(env, source)
        return value

class DictEmitter(SCons.Util.Selector):
    """A callable dictionary that maps file suffixes to emitters.
    When called, it finds the right emitter in its dictionary for the
    suffix of the first source file, and calls that emitter to get the
    right lists of targets and sources to return.  If there's no emitter
    for the suffix in its dictionary, the original target and source are
    returned.
    """
    def __call__(self, target, source, env):
        emitter = SCons.Util.Selector.__call__(self, env, source)
        if emitter:
            target, source = emitter(target, source, env)
        return (target, source)

class ListEmitter(UserList.UserList):
    """A callable list of emitters that calls each in sequence,
    returning the result.
    """
    def __call__(self, target, source, env):
        for e in self.data:
            target, source = e(target, source, env)
        return (target, source)

# These are a common errors when calling a Builder;
# they are similar to the 'target' and 'source' keyword args to builders,
# so we issue warnings when we see them.  The warnings can, of course,
# be disabled.
misleading_keywords = {
    'targets'   : 'target',
    'sources'   : 'source',
}

class OverrideWarner(UserDict.UserDict):
    """A class for warning about keyword arguments that we use as
    overrides in a Builder call.

    This class exists to handle the fact that a single MultiStepBuilder
    call can actually invoke multiple builders as a result of a single
    user-level Builder call.  This class only emits the warnings once,
    no matter how many Builders are invoked.
    """
    def __init__(self, dict):
        UserDict.UserDict.__init__(self, dict)
        self.already_warned = None
    def warn(self):
        if self.already_warned:
            return
        for k in self.keys():
            try:
                alt = misleading_keywords[k]
            except KeyError:
                pass
            else:
                SCons.Warnings.warn(SCons.Warnings.MisleadingKeywordsWarning,
                                    "Did you mean to use `%s' instead of `%s'?" % (alt, k))
        self.already_warned = 1

def Builder(**kw):
    """A factory for builder objects."""
    composite = None
    if kw.has_key('generator'):
        if kw.has_key('action'):
            raise UserError, "You must not specify both an action and a generator."
        kw['action'] = SCons.Action.CommandGenerator(kw['generator'])
        del kw['generator']
    elif kw.has_key('action') and SCons.Util.is_Dict(kw['action']):
        composite = DictCmdGenerator(kw['action'])
        kw['action'] = SCons.Action.CommandGenerator(composite)
        kw['src_suffix'] = composite.src_suffixes()

    if kw.has_key('emitter'):
        emitter = kw['emitter']
        if SCons.Util.is_String(emitter):
            # This allows users to pass in an Environment
            # variable reference (like "$FOO") as an emitter.
            # We will look in that Environment variable for
            # a callable to use as the actual emitter.
            var = SCons.Util.get_environment_var(emitter)
            if not var:
                raise UserError, "Supplied emitter '%s' does not appear to refer to an Environment variable" % emitter
            kw['emitter'] = EmitterProxy(var)
        elif SCons.Util.is_Dict(emitter):
            kw['emitter'] = DictEmitter(emitter)
        elif SCons.Util.is_List(emitter):
            kw['emitter'] = ListEmitter(emitter)

    if kw.has_key('src_builder'):
        ret = apply(MultiStepBuilder, (), kw)
    else:
        ret = apply(BuilderBase, (), kw)

    if not composite is None:
        ret = CompositeBuilder(ret, composite)

    return ret

def _init_nodes(builder, env, overrides, tlist, slist):
    """Initialize lists of target and source nodes with all of
    the proper Builder information.
    """

    # First, figure out if there are any errors in the way the targets
    # were specified.
    for t in tlist:
        if t.side_effect:
            raise UserError, "Multiple ways to build the same target were specified for: %s" % str(t)
        if t.has_builder():
            if not t.env is None and not t.env is env:
                t_contents = t.builder.action.get_contents(tlist, slist, t.env)
                contents = t.builder.action.get_contents(tlist, slist, env)

                if t_contents == contents:
                    SCons.Warnings.warn(SCons.Warnings.DuplicateEnvironmentWarning,
                                        "Two different environments were specified for target %s,\n\tbut they appear to have the same action: %s"%(str(t), t.builder.action.strfunction(tlist, slist, t.env)))

                else:
                    raise UserError, "Two environments with different actions were specified for the same target: %s"%str(t)

            elif t.overrides != overrides:
                raise UserError, "Two different sets of overrides were specified for the same target: %s"%str(t)

            elif builder.target_scanner and t.target_scanner and builder.target_scanner != t.target_scanner:
                raise UserError, "Two different scanners were specified for the same target: %s"%str(t)

            if builder.multi:
                if t.builder != builder:
                    if isinstance(t.builder, ListBuilder) and isinstance(builder, ListBuilder) and t.builder.builder == builder.builder:
                        raise UserError, "Two different target sets have a target in common: %s"%str(t)
                    else:
                        raise UserError, "Two different builders (%s and %s) were specified for the same target: %s"%(t.builder.get_name(env), builder.get_name(env), str(t))
            elif t.sources != slist:
                raise UserError, "Multiple ways to build the same target were specified for: %s" % str(t)

    if builder.single_source:
        if len(slist) > 1:
            raise UserError, "More than one source given for single-source builder: targets=%s sources=%s" % (map(str,tlist), map(str,slist))

    # The targets are fine, so find or make the appropriate Executor to
    # build this particular list of targets from this particular list of
    # sources.
    executor = None
    if builder.multi:
        try:
            executor = tlist[0].get_executor(create = 0)
        except AttributeError:
            pass
        else:
            executor.add_sources(slist)
    if executor is None:
        executor = SCons.Executor.Executor(builder.action,
                                           env or builder.env,
                                           [builder.overrides, overrides],
                                           tlist,
                                           slist)

    # Now set up the relevant information in the target Nodes themselves.
    for t in tlist:
        t.overrides = overrides
        t.cwd = SCons.Node.FS.default_fs.getcwd()
        t.builder_set(builder)
        t.env_set(env)
        t.add_source(slist)
        t.set_executor(executor)
        if builder.target_scanner:
            t.target_scanner = builder.target_scanner
        if t.source_scanner is None:
            t.source_scanner = builder.source_scanner

    # Add backup source scanners from the environment to the source
    # nodes.  This may not be necessary if the node will have a real
    # source scanner added later (which is why these are the "backup"
    # source scanners, not the real ones), but because source nodes may
    # be used multiple times for different targets, it ends up being
    # more efficient to do this calculation once here, as opposed to
    # delaying it until later when we potentially have to calculate it
    # over and over and over.
    for s in slist:
        if s.source_scanner is None and s.backup_source_scanner is None:
            s.backup_source_scanner = env.get_scanner(s.scanner_key())

class EmitterProxy:
    """This is a callable class that can act as a
    Builder emitter.  It holds on to a string that
    is a key into an Environment dictionary, and will
    look there at actual build time to see if it holds
    a callable.  If so, we will call that as the actual
    emitter."""
    def __init__(self, var):
        self.var = SCons.Util.to_String(var)

    def __call__(self, target, source, env):
        emitter = self.var

        # Recursively substitute the variable.
        # We can't use env.subst() because it deals only
        # in strings.  Maybe we should change that?
        while SCons.Util.is_String(emitter) and env.has_key(emitter):
            emitter = env[emitter]
        if callable(emitter):
            target, source = emitter(target, source, env)
        elif SCons.Util.is_List(emitter):
            for e in emitter:
                target, source = e(target, source, env)

        return (target, source)


    def __cmp__(self, other):
        return cmp(self.var, other.var)

class BuilderBase:
    """Base class for Builders, objects that create output
    nodes (files) from input nodes (files).
    """

    def __init__(self,  action = None,
                        prefix = '',
                        suffix = '',
                        src_suffix = '',
                        target_factory = SCons.Node.FS.default_fs.File,
                        source_factory = SCons.Node.FS.default_fs.File,
                        target_scanner = None,
                        source_scanner = None,
                        emitter = None,
                        multi = 0,
                        env = None,
                        single_source = 0,
                        **overrides):
        if __debug__: logInstanceCreation(self, 'BuilderBase')
        self.action = SCons.Action.Action(action)
        self.multi = multi
        if SCons.Util.is_Dict(prefix):
            prefix = CallableSelector(prefix)
        self.prefix = prefix
        if SCons.Util.is_Dict(suffix):
            suffix = CallableSelector(suffix)
        self.suffix = suffix
        self.env = env
        self.single_source = single_source
        if overrides.has_key('overrides'):
            SCons.Warnings.warn(SCons.Warnings.DeprecatedWarning,
                "The \"overrides\" keyword to Builder() creation has been deprecated;\n" +\
                "\tspecify the items as keyword arguments to the Builder() call instead.")
            overrides.update(overrides['overrides'])
            del overrides['overrides']
        self.overrides = overrides

        self.set_src_suffix(src_suffix)

        self.target_factory = target_factory
        self.source_factory = source_factory
        self.target_scanner = target_scanner
        self.source_scanner = source_scanner

        self.emitter = emitter

    def __nonzero__(self):
        raise InternalError, "Do not test for the Node.builder attribute directly; use Node.has_builder() instead"

    def get_name(self, env):
        """Attempts to get the name of the Builder.

        Look at the BUILDERS variable of env, expecting it to be a
        dictionary containing this Builder, and return the key of the
        dictionary."""

        try:
            index = env['BUILDERS'].values().index(self)
            return env['BUILDERS'].keys()[index]
        except (AttributeError, KeyError, ValueError):
            return str(self.__class__)

    def __cmp__(self, other):
        return cmp(self.__dict__, other.__dict__)

    def splitext(self, path):
        return SCons.Util.splitext(path)

    def _create_nodes(self, env, overwarn, target = None, source = None):
        """Create and return lists of target and source nodes.
        """
        def _adjustixes(files, pre, suf):
            if not files:
                return []
            result = []
            if not SCons.Util.is_List(files):
                files = [files]

            for f in files:
                if SCons.Util.is_String(f):
                    f = SCons.Util.adjustixes(f, pre, suf)
                result.append(f)
            return result

        overwarn.warn()

        env = env.Override(overwarn.data)

        src_suf = self.get_src_suffix(env)

        source = _adjustixes(source, None, src_suf)
        slist = env.arg2nodes(source, self.source_factory)

        pre = self.get_prefix(env, slist)
        suf = self.get_suffix(env, slist)

        if target is None:
            try:
                t_from_s = slist[0].target_from_source
            except AttributeError:
                raise UserError("Do not know how to create a target from source `%s'" % slist[0])
            tlist = [ t_from_s(pre, suf, self.splitext) ]
        else:
            target = _adjustixes(target, pre, suf)
            tlist = env.arg2nodes(target, self.target_factory)

        if self.emitter:
            # The emitter is going to do str(node), but because we're
            # being called *from* a builder invocation, the new targets
            # don't yet have a builder set on them and will look like
            # source files.  Fool the emitter's str() calls by setting
            # up a temporary builder on the new targets.
            new_targets = []
            for t in tlist:
                if not t.is_derived():
                    t.builder = self
                    new_targets.append(t)

            target, source = self.emitter(target=tlist, source=slist, env=env)

            # Now delete the temporary builders that we attached to any
            # new targets, so that _init_nodes() doesn't do weird stuff
            # to them because it thinks they already have builders.
            for t in new_targets:
                if t.builder is self:
                    # Only delete the temporary builder if the emitter
                    # didn't change it on us.
                    t.builder = None

            # Have to call arg2nodes yet again, since it is legal for
            # emitters to spit out strings as well as Node instances.
            slist = env.arg2nodes(source, self.source_factory)
            tlist = env.arg2nodes(target, self.target_factory)

        return tlist, slist

    def _execute(self, env, target = None, source = _null, overwarn={}):
        if source is _null:
            source = target
            target = None

        if(self.single_source and
           SCons.Util.is_List(source) and
           len(source) > 1 and
           target is None):
            result = []
            if target is None: target = [None]*len(source)
            for k in range(len(source)):
                t = self._execute(env, target[k], source[k], overwarn)
                if SCons.Util.is_List(t):
                    result.extend(t)
                else:
                    result.append(t)
            return result
        
        tlist, slist = self._create_nodes(env, overwarn, target, source)

        if len(tlist) == 1:
            builder = self
        else:
            builder = ListBuilder(self, env, tlist)
        _init_nodes(builder, env, overwarn.data, tlist, slist)

        return tlist

    def __call__(self, env, target = None, source = _null, **kw):
        return self._execute(env, target, source, OverrideWarner(kw))

    def adjust_suffix(self, suff):
        if suff and not suff[0] in [ '.', '_', '$' ]:
            return '.' + suff
        return suff

    def get_prefix(self, env, sources=[]):
        prefix = self.prefix
        if callable(prefix):
            prefix = prefix(env, sources)
        return env.subst(prefix)

    def get_suffix(self, env, sources=[]):
        suffix = self.suffix
        if callable(suffix):
            suffix = suffix(env, sources)
        else:
            suffix = self.adjust_suffix(suffix)
        return env.subst(suffix)

    def src_suffixes(self, env):
        return map(lambda x, s=self, e=env: e.subst(s.adjust_suffix(x)),
                   self.src_suffix)

    def set_src_suffix(self, src_suffix):
        if not src_suffix:
            src_suffix = []
        elif not SCons.Util.is_List(src_suffix):
            src_suffix = [ src_suffix ]
        self.src_suffix = src_suffix

    def get_src_suffix(self, env):
        """Get the first src_suffix in the list of src_suffixes."""
        ret = self.src_suffixes(env)
        if not ret:
            return ''
        return ret[0]

    def targets(self, node):
        """Return the list of targets for this builder instance.

        For most normal builders, this is just the supplied node.
        """
        return [ node ]

    def add_emitter(self, suffix, emitter):
        """Add a suffix-emitter mapping to this Builder.

        This assumes that emitter has been initialized with an
        appropriate dictionary type, and will throw a TypeError if
        not, so the caller is responsible for knowing that this is an
        appropriate method to call for the Builder in question.
        """
        self.emitter[suffix] = emitter

class ListBuilder(SCons.Util.Proxy):
    """A Proxy to support building an array of targets (for example,
    foo.o and foo.h from foo.y) from a single Action execution.
    """

    def __init__(self, builder, env, tlist):
        if __debug__: logInstanceCreation(self)
        SCons.Util.Proxy.__init__(self, builder)
        self.builder = builder
        self.target_scanner = builder.target_scanner
        self.source_scanner = builder.source_scanner
        self.env = env
        self.tlist = tlist
        self.multi = builder.multi
        self.single_source = builder.single_source

    def targets(self, node):
        """Return the list of targets for this builder instance.
        """
        return self.tlist

    def __cmp__(self, other):
        return cmp(self.__dict__, other.__dict__)

    def get_name(self, env):
        """Attempts to get the name of the Builder."""

        return "ListBuilder(%s)" % self.builder.get_name(env)

class MultiStepBuilder(BuilderBase):
    """This is a builder subclass that can build targets in
    multiple steps.  The src_builder parameter to the constructor
    accepts a builder that is called to build sources supplied to
    this builder.  The targets of that first build then become
    the sources of this builder.

    If this builder has a src_suffix supplied, then the src_builder
    builder is NOT invoked if the suffix of a source file matches
    src_suffix.
    """
    def __init__(self,  src_builder,
                        action = None,
                        prefix = '',
                        suffix = '',
                        src_suffix = '',
                        target_factory = SCons.Node.FS.default_fs.File,
                        source_factory = SCons.Node.FS.default_fs.File,
                        target_scanner = None,
                        source_scanner = None,
                        emitter=None,
                        single_source=0):
        if __debug__: logInstanceCreation(self)
        BuilderBase.__init__(self, action, prefix, suffix, src_suffix,
                             target_factory, source_factory,
                             target_scanner, source_scanner, emitter,
                             single_source = single_source)
        if not SCons.Util.is_List(src_builder):
            src_builder = [ src_builder ]
        self.src_builder = src_builder
        self.sdict = {}
        self.cached_src_suffixes = {} # source suffixes keyed on id(env)

    def _execute(self, env, target = None, source = _null, overwarn={}):
        if source is _null:
            source = target
            target = None

        slist = env.arg2nodes(source, self.source_factory)
        final_sources = []

        try:
            sdict = self.sdict[id(env)]
        except KeyError:
            sdict = {}
            self.sdict[id(env)] = sdict
            for bld in self.src_builder:
                if SCons.Util.is_String(bld):
                    try:
                        bld = env['BUILDERS'][bld]
                    except KeyError:
                        continue
                for suf in bld.src_suffixes(env):
                    sdict[suf] = bld

        src_suffixes = self.src_suffixes(env)

        for snode in slist:
            try:
                get_suffix = snode.get_suffix
            except AttributeError:
                ext = self.splitext(str(snode))
            else:
                ext = get_suffix()
            try:
                subsidiary_builder = sdict[ext]
            except KeyError:
                final_sources.append(snode)
            else:
                tgt = subsidiary_builder._execute(env, None, snode, overwarn)
                # If the subsidiary Builder returned more than one target,
                # then filter out any sources that this Builder isn't
                # capable of building.
                if len(tgt) > 1:
                    tgt = filter(lambda x, self=self, suf=src_suffixes:
                                 self.splitext(SCons.Util.to_String(x))[1] in suf,
                                 tgt)
                final_sources.extend(tgt)

        return BuilderBase._execute(self, env, target, final_sources, overwarn)

    def get_src_builders(self, env):
        """Return all the src_builders for this Builder.

        This is essentially a recursive descent of the src_builder "tree."
        """
        ret = []
        for bld in self.src_builder:
            if SCons.Util.is_String(bld):
                # All Environments should have a BUILDERS
                # variable, so no need to check for it.
                try:
                    bld = env['BUILDERS'][bld]
                except KeyError:
                    continue
            ret.append(bld)
        return ret

    def src_suffixes(self, env):
        """Return a list of the src_suffix attributes for all
        src_builders of this Builder.
        """
        try:
            return self.cached_src_suffixes[id(env)]
        except KeyError:
            suffixes = BuilderBase.src_suffixes(self, env)
            for builder in self.get_src_builders(env):
                suffixes.extend(builder.src_suffixes(env))
            self.cached_src_suffixes[id(env)] = suffixes
            return suffixes

class CompositeBuilder(SCons.Util.Proxy):
    """A Builder Proxy whose main purpose is to always have
    a DictCmdGenerator as its action, and to provide access
    to the DictCmdGenerator's add_action() method.
    """

    def __init__(self, builder, cmdgen):
        if __debug__: logInstanceCreation(self)
        SCons.Util.Proxy.__init__(self, builder)

        # cmdgen should always be an instance of DictCmdGenerator.
        self.cmdgen = cmdgen
        self.builder = builder

    def add_action(self, suffix, action):
        self.cmdgen.add_action(suffix, action)
        self.set_src_suffix(self.cmdgen.src_suffixes())

    def __cmp__(self, other):
        return cmp(self.__dict__, other.__dict__)
