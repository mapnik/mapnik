"""SCons.Action

This encapsulates information about executing any sort of action that
can build one or more target Nodes (typically files) from one or more
source Nodes (also typically files) given a specific Environment.

The base class here is ActionBase.  The base class supplies just a few
OO utility methods and some generic methods for displaying information
about an Action in response to the various commands that control printing.

The heavy lifting is handled by subclasses for the different types of
actions we might execute:

    CommandAction
    CommandGeneratorAction
    FunctionAction
    ListAction

The subclasses supply the following public interface methods used by
other modules:

    __call__()
        THE public interface, "calling" an Action object executes the
        command or Python function.  This also takes care of printing
        a pre-substitution command for debugging purposes.

    get_contents()
        Fetches the "contents" of an Action for signature calculation.
        This is what the Sig/*.py subsystem uses to decide if a target
        needs to be rebuilt because its action changed.

    genstring()
        Returns a string representation of the Action *without* command
        substitution, but allows a CommandGeneratorAction to generate
        the right action based on the specified target, source and env.
        This is used by the Signature subsystem (through the Executor)
        to compare the actions used to build a target last time and
        this time.

Subclasses also supply the following methods for internal use within
this module:

    __str__()
        Returns a string representation of the Action *without* command
        substitution.  This is used by the __call__() methods to display
        the pre-substitution command whenever the --debug=presub option
        is used.

    strfunction()
        Returns a substituted string representation of the Action.
        This is used by the ActionBase.show() command to display the
        command/function that will be executed to generate the target(s).

    execute()
        The internal method that really, truly, actually handles the
        execution of a command or Python function.  This is used so
        that the __call__() methods can take care of displaying any
        pre-substitution representations, and *then* execute an action
        without worrying about the specific Actions involved.

There is a related independent ActionCaller class that looks like a
regular Action, and which serves as a wrapper for arbitrary functions
that we want to let the user specify the arguments to now, but actually
execute later (when an out-of-date check determines that it's needed to
be executed, for example).  Objects of this class are returned by an
ActionFactory class that provides a __call__() method as a convenient
way for wrapping up the functions.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Action.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os
import os.path
import re
import string
import sys

from SCons.Debug import logInstanceCreation
import SCons.Errors
import SCons.Util

class _Null:
    pass

_null = _Null

print_actions = 1
execute_actions = 1
print_actions_presub = 0

default_ENV = None

def rfile(n):
    try:
        return n.rfile()
    except AttributeError:
        return n

def _actionAppend(act1, act2):
    # This function knows how to slap two actions together.
    # Mainly, it handles ListActions by concatenating into
    # a single ListAction.
    a1 = Action(act1)
    a2 = Action(act2)
    if a1 is None or a2 is None:
        raise TypeError, "Cannot append %s to %s" % (type(act1), type(act2))
    if isinstance(a1, ListAction):
        if isinstance(a2, ListAction):
            return ListAction(a1.list + a2.list)
        else:
            return ListAction(a1.list + [ a2 ])
    else:
        if isinstance(a2, ListAction):
            return ListAction([ a1 ] + a2.list)
        else:
            return ListAction([ a1, a2 ])

class CommandGenerator:
    """
    Wraps a command generator function so the Action() factory
    function can tell a generator function from a function action.
    """
    def __init__(self, generator):
        self.generator = generator

    def __add__(self, other):
        return _actionAppend(self, other)

    def __radd__(self, other):
        return _actionAppend(other, self)

def _do_create_action(act, *args, **kw):
    """This is the actual "implementation" for the
    Action factory method, below.  This handles the
    fact that passing lists to Action() itself has
    different semantics than passing lists as elements
    of lists.

    The former will create a ListAction, the latter
    will create a CommandAction by converting the inner
    list elements to strings."""

    if isinstance(act, ActionBase):
        return act
    if SCons.Util.is_List(act):
        return apply(CommandAction, (act,)+args, kw)
    if isinstance(act, CommandGenerator):
        return apply(CommandGeneratorAction, (act.generator,)+args, kw)
    if callable(act):
        return apply(FunctionAction, (act,)+args, kw)
    if SCons.Util.is_String(act):
        var=SCons.Util.get_environment_var(act)
        if var:
            # This looks like a string that is purely an Environment
            # variable reference, like "$FOO" or "${FOO}".  We do
            # something special here...we lazily evaluate the contents
            # of that Environment variable, so a user could put something
            # like a function or a CommandGenerator in that variable
            # instead of a string.
            lcg = LazyCmdGenerator(var)
            return apply(CommandGeneratorAction, (lcg,)+args, kw)
        commands = string.split(str(act), '\n')
        if len(commands) == 1:
            return apply(CommandAction, (commands[0],)+args, kw)
        else:
            listCmdActions = map(lambda x: CommandAction(x), commands)
            return apply(ListAction, (listCmdActions,)+args, kw)
    return None

def Action(act, strfunction=_null, varlist=[], presub=_null):
    """A factory for action objects."""
    if SCons.Util.is_List(act):
        acts = map(lambda x, s=strfunction, v=varlist, ps=presub:
                          _do_create_action(x, strfunction=s, varlist=v, presub=ps),
                   act)
        acts = filter(lambda x: not x is None, acts)
        if len(acts) == 1:
            return acts[0]
        else:
            return ListAction(acts, strfunction=strfunction, varlist=varlist, presub=presub)
    else:
        return _do_create_action(act, strfunction=strfunction, varlist=varlist, presub=presub)

class ActionBase:
    """Base class for actions that create output objects."""
    def __init__(self, strfunction=_null, presub=_null, **kw):
        if not strfunction is _null:
            self.strfunction = strfunction
        if presub is _null:
            self.presub = print_actions_presub
        else:
            self.presub = presub

    def __cmp__(self, other):
        return cmp(self.__dict__, other.__dict__)

    def __call__(self, target, source, env,
                               errfunc=None,
                               presub=_null,
                               show=_null,
                               execute=_null):
        if not SCons.Util.is_List(target):
            target = [target]
        if not SCons.Util.is_List(source):
            source = [source]
        if presub is _null:  presub = self.presub
        if show is _null:  show = print_actions
        if execute is _null:  execute = execute_actions
        if presub:
            t = string.join(map(str, target), 'and')
            l = string.join(self.presub_lines(env), '\n  ')
            out = "Building %s with action(s):\n  %s\n" % (t, l)
            sys.stdout.write(out)
        if show and self.strfunction:
            s = self.strfunction(target, source, env)
            if s:
                sys.stdout.write(s + '\n')
        if execute:
            stat = self.execute(target, source, env)
            if stat and errfunc:
                errfunc(stat)
            return stat
        else:
            return 0

    def presub_lines(self, env):
        # CommandGeneratorAction needs a real environment
        # in order to return the proper string here, since
        # it may call LazyCmdGenerator, which looks up a key
        # in that env.  So we temporarily remember the env here,
        # and CommandGeneratorAction will use this env
        # when it calls its __generate method.
        self.presub_env = env
        lines = string.split(str(self), '\n')
        self.presub_env = None      # don't need this any more
        return lines

    def genstring(self, target, source, env):
        return str(self)

    def get_actions(self):
        return [self]

    def __add__(self, other):
        return _actionAppend(self, other)

    def __radd__(self, other):
        return _actionAppend(other, self)

def _string_from_cmd_list(cmd_list):
    """Takes a list of command line arguments and returns a pretty
    representation for printing."""
    cl = []
    for arg in map(str, cmd_list):
        if ' ' in arg or '\t' in arg:
            arg = '"' + arg + '"'
        cl.append(arg)
    return string.join(cl)

class CommandAction(ActionBase):
    """Class for command-execution actions."""
    def __init__(self, cmd, **kw):
        # Cmd list can actually be a list or a single item...basically
        # anything that we could pass in as the first arg to
        # Environment.subst_list().
        if __debug__: logInstanceCreation(self)
        apply(ActionBase.__init__, (self,), kw)
        self.cmd_list = cmd

    def __str__(self):
        return str(self.cmd_list)

    def strfunction(self, target, source, env):
        cmd_list = env.subst_list(self.cmd_list, 0, target, source)
        return string.join(map(_string_from_cmd_list, cmd_list), "\n")

    def execute(self, target, source, env):
        """Execute a command action.

        This will handle lists of commands as well as individual commands,
        because construction variable substitution may turn a single
        "command" into a list.  This means that this class can actually
        handle lists of commands, even though that's not how we use it
        externally.
        """
        import SCons.Util

        escape = env.get('ESCAPE', lambda x: x)

        if env.has_key('SHELL'):
            shell = env['SHELL']
        else:
            raise SCons.Errors.UserError('Missing SHELL construction variable.')

        # for SConf support (by now): check, if we want to pipe the command
        # output to somewhere else
        if env.has_key('PIPE_BUILD'):
            pipe_build = 1
            if env.has_key('PSPAWN'):
                pspawn = env['PSPAWN']
            else:
                raise SCons.Errors.UserError('Missing PSPAWN construction variable.')
            if env.has_key('PSTDOUT'):
                pstdout = env['PSTDOUT']
            else:
                raise SCons.Errors.UserError('Missing PSTDOUT construction variable.')
            if env.has_key('PSTDERR'):
                pstderr = env['PSTDERR']
            else:
                raise SCons.Errors.UserError('Missing PSTDOUT construction variable.')
        else:
            pipe_build = 0
            if env.has_key('SPAWN'):
                spawn = env['SPAWN']
            else:
                raise SCons.Errors.UserError('Missing SPAWN construction variable.')

        cmd_list = env.subst_list(self.cmd_list, 0, target, source)
        for cmd_line in cmd_list:
            if len(cmd_line):
                try:
                    ENV = env['ENV']
                except KeyError:
                    global default_ENV
                    if not default_ENV:
                        import SCons.Environment
                        default_ENV = SCons.Environment.Environment()['ENV']
                    ENV = default_ENV

                # ensure that the ENV values are all strings:
                for key, value in ENV.items():
                    if SCons.Util.is_List(value):
                        # If the value is a list, then we assume
                        # it is a path list, because that's a pretty
                        # common list like value to stick in an environment
                        # variable:
                        value = SCons.Util.flatten(value)
                        ENV[key] = string.join(map(str, value), os.pathsep)
                    elif not SCons.Util.is_String(value):
                        # If it isn't a string or a list, then
                        # we just coerce it to a string, which
                        # is proper way to handle Dir and File instances
                        # and will produce something reasonable for
                        # just about everything else:
                        ENV[key] = str(value)

                # Escape the command line for the command
                # interpreter we are using
                cmd_line = SCons.Util.escape_list(cmd_line, escape)
                if pipe_build:
                    ret = pspawn( shell, escape, cmd_line[0], cmd_line,
                                  ENV, pstdout, pstderr )
                else:
                    ret = spawn(shell, escape, cmd_line[0], cmd_line, ENV)
                if ret:
                    return ret
        return 0

    def get_contents(self, target, source, env, dict=None):
        """Return the signature contents of this action's command line.

        This strips $(-$) and everything in between the string,
        since those parts don't affect signatures.
        """
        cmd = self.cmd_list
        if SCons.Util.is_List(cmd):
            cmd = string.join(map(str, cmd))
        else:
            cmd = str(cmd)
        return env.subst_target_source(cmd, SCons.Util.SUBST_SIG, target, source, dict)

class CommandGeneratorAction(ActionBase):
    """Class for command-generator actions."""
    def __init__(self, generator, **kw):
        if __debug__: logInstanceCreation(self)
        apply(ActionBase.__init__, (self,), kw)
        self.generator = generator

    def __generate(self, target, source, env, for_signature):
        # ensure that target is a list, to make it easier to write
        # generator functions:
        if not SCons.Util.is_List(target):
            target = [target]

        ret = self.generator(target=target, source=source, env=env, for_signature=for_signature)
        gen_cmd = Action(ret)
        if not gen_cmd:
            raise SCons.Errors.UserError("Object returned from command generator: %s cannot be used to create an Action." % repr(ret))
        return gen_cmd

    def strfunction(self, target, source, env):
        if not SCons.Util.is_List(source):
            source = [source]
        rsources = map(rfile, source)
        act = self.__generate(target, source, env, 0)
        if act.strfunction:
            return act.strfunction(target, rsources, env)
        else:
            return None

    def __str__(self):
        try:
            env = self.presub_env or {}
        except AttributeError:
            env = {}
        act = self.__generate([], [], env, 0)
        return str(act)

    def genstring(self, target, source, env):
        return str(self.__generate(target, source, env, 0))

    def execute(self, target, source, env):
        rsources = map(rfile, source)
        act = self.__generate(target, source, env, 0)
        return act.execute(target, source, env)

    def get_contents(self, target, source, env, dict=None):
        """Return the signature contents of this action's command line.

        This strips $(-$) and everything in between the string,
        since those parts don't affect signatures.
        """
        return self.__generate(target, source, env, 1).get_contents(target, source, env, dict=None)

class LazyCmdGenerator:
    """This is not really an Action, although it kind of looks like one.
    This is really a simple callable class that acts as a command
    generator.  It holds on to a key into an Environment dictionary,
    then waits until execution time to see what type it is, then tries
    to create an Action out of it."""
    def __init__(self, var):
        if __debug__: logInstanceCreation(self)
        self.var = SCons.Util.to_String(var)

    def strfunction(self, target, source, env):
        try:
            return env[self.var]
        except KeyError:
            # The variable reference substitutes to nothing.
            return ''

    def __str__(self):
        return 'LazyCmdGenerator: %s'%str(self.var)

    def __call__(self, target, source, env, for_signature):
        try:
            return env[self.var]
        except KeyError:
            # The variable reference substitutes to nothing.
            return ''

    def __cmp__(self, other):
        return cmp(self.__dict__, other.__dict__)

class FunctionAction(ActionBase):
    """Class for Python function actions."""

    def __init__(self, execfunction, **kw):
        if __debug__: logInstanceCreation(self)
        self.execfunction = execfunction
        apply(ActionBase.__init__, (self,), kw)
        self.varlist = kw.get('varlist', [])

    def function_name(self):
        try:
            return self.execfunction.__name__
        except AttributeError:
            try:
                return self.execfunction.__class__.__name__
            except AttributeError:
                return "unknown_python_function"

    def strfunction(self, target, source, env):
        def array(a):
            def quote(s):
                return '"' + str(s) + '"'
            return '[' + string.join(map(quote, a), ", ") + ']'
        name = self.function_name()
        tstr = array(target)
        sstr = array(source)
        return "%s(%s, %s)" % (name, tstr, sstr)

    def __str__(self):
        return "%s(env, target, source)" % self.function_name()

    def execute(self, target, source, env):
        rsources = map(rfile, source)
        return self.execfunction(target=target, source=rsources, env=env)

    def get_contents(self, target, source, env, dict=None):
        """Return the signature contents of this callable action.

        By providing direct access to the code object of the
        function, Python makes this extremely easy.  Hooray!
        """
        try:
            # "self.execfunction" is a function.
            contents = str(self.execfunction.func_code.co_code)
        except AttributeError:
            # "self.execfunction" is a callable object.
            try:
                contents = str(self.execfunction.__call__.im_func.func_code.co_code)
            except AttributeError:
                try:
                    # See if execfunction will do the heavy lifting for us.
                    gc = self.execfunction.get_contents
                except AttributeError:
                    # This is weird, just do the best we can.
                    contents = str(self.execfunction)
                else:
                    contents = gc(target, source, env, dict)
        return contents + env.subst(string.join(map(lambda v: '${'+v+'}',
                                                     self.varlist)))

class ListAction(ActionBase):
    """Class for lists of other actions."""
    def __init__(self, list, **kw):
        if __debug__: logInstanceCreation(self)
        apply(ActionBase.__init__, (self,), kw)
        self.list = map(lambda x: Action(x), list)

    def get_actions(self):
        return self.list

    def __str__(self):
        s = []
        for l in self.list:
            s.append(str(l))
        return string.join(s, "\n")

    def strfunction(self, target, source, env):
        s = []
        for l in self.list:
            if l.strfunction:
                x = l.strfunction(target, source, env)
                if not SCons.Util.is_List(x):
                    x = [x]
                s.extend(x)
        return string.join(s, "\n")

    def execute(self, target, source, env):
        for l in self.list:
            r = l.execute(target, source, env)
            if r:
                return r
        return 0

    def get_contents(self, target, source, env, dict=None):
        """Return the signature contents of this action list.

        Simple concatenation of the signatures of the elements.
        """
        dict = SCons.Util.subst_dict(target, source)
        return string.join(map(lambda x, t=target, s=source, e=env, d=dict:
                                      x.get_contents(t, s, e, d),
                               self.list),
                           "")

class ActionCaller:
    """A class for delaying calling an Action function with specific
    (positional and keyword) arguments until the Action is actually
    executed.

    This class looks to the rest of the world like a normal Action object,
    but what it's really doing is hanging on to the arguments until we
    have a target, source and env to use for the expansion.
    """
    def __init__(self, parent, args, kw):
        self.parent = parent
        self.args = args
        self.kw = kw
    def get_contents(self, target, source, env, dict=None):
        actfunc = self.parent.actfunc
        try:
            # "self.actfunc" is a function.
            contents = str(actfunc.func_code.co_code)
        except AttributeError:
            # "self.actfunc" is a callable object.
            try:
                contents = str(actfunc.__call__.im_func.func_code.co_code)
            except AttributeError:
                # No __call__() method, so it might be a builtin
                # or something like that.  Do the best we can.
                contents = str(actfunc)
        return contents
    def subst_args(self, target, source, env):
        return map(lambda x, e=env, t=target, s=source:
                          e.subst(x, 0, t, s),
                   self.args)
    def subst_kw(self, target, source, env):
        kw = {}
        for key in self.kw.keys():
            kw[key] = env.subst(self.kw[key], 0, target, source)
        return kw
    def __call__(self, target, source, env):
        args = self.subst_args(target, source, env)
        kw = self.subst_kw(target, source, env)
        return apply(self.parent.actfunc, args, kw)
    def strfunction(self, target, source, env):
        args = self.subst_args(target, source, env)
        kw = self.subst_kw(target, source, env)
        return apply(self.parent.strfunc, args, kw)

class ActionFactory:
    """A factory class that will wrap up an arbitrary function
    as an SCons-executable Action object.

    The real heavy lifting here is done by the ActionCaller class.
    We just collect the (positional and keyword) arguments that we're
    called with and give them to the ActionCaller object we create,
    so it can hang onto them until it needs them.
    """
    def __init__(self, actfunc, strfunc):
        self.actfunc = actfunc
        self.strfunc = strfunc
    def __call__(self, *args, **kw):
        ac = ActionCaller(self, args, kw)
        return Action(ac, strfunction=ac.strfunction)
