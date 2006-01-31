"""SCons.Util

Various utility functions go here.

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

__revision__ = "/home/scons/scons/branch.0/branch.96/baseline/src/engine/SCons/Util.py 0.96.1.D001 2004/08/23 09:55:29 knight"


import copy
import os
import os.path
import re
import stat
import string
import sys
import types
import UserDict
import UserList

import SCons.Errors

try:
    from UserString import UserString
except ImportError:
    # "Borrowed" from the Python 2.2 UserString module
    # and modified slightly for use with SCons.
    class UserString:
        def __init__(self, seq):
            if is_String(seq):
                self.data = seq
            elif isinstance(seq, UserString):
                self.data = seq.data[:]
            else:
                self.data = str(seq)
        def __str__(self): return str(self.data)
        def __repr__(self): return repr(self.data)
        def __int__(self): return int(self.data)
        def __long__(self): return long(self.data)
        def __float__(self): return float(self.data)
        def __complex__(self): return complex(self.data)
        def __hash__(self): return hash(self.data)

        def __cmp__(self, string):
            if isinstance(string, UserString):
                return cmp(self.data, string.data)
            else:
                return cmp(self.data, string)
        def __contains__(self, char):
            return char in self.data

        def __len__(self): return len(self.data)
        def __getitem__(self, index): return self.__class__(self.data[index])
        def __getslice__(self, start, end):
            start = max(start, 0); end = max(end, 0)
            return self.__class__(self.data[start:end])

        def __add__(self, other):
            if isinstance(other, UserString):
                return self.__class__(self.data + other.data)
            elif is_String(other):
                return self.__class__(self.data + other)
            else:
                return self.__class__(self.data + str(other))
        def __radd__(self, other):
            if is_String(other):
                return self.__class__(other + self.data)
            else:
                return self.__class__(str(other) + self.data)
        def __mul__(self, n):
            return self.__class__(self.data*n)
        __rmul__ = __mul__

_altsep = os.altsep
if _altsep is None and sys.platform == 'win32':
    # My ActivePython 2.0.1 doesn't set os.altsep!  What gives?
    _altsep = '/'
if _altsep:
    def rightmost_separator(path, sep, _altsep=_altsep):
        rfind = string.rfind
        return max(rfind(path, sep), rfind(path, _altsep))
else:
    rightmost_separator = string.rfind

# First two from the Python Cookbook, just for completeness.
# (Yeah, yeah, YAGNI...)
def containsAny(str, set):
    """Check whether sequence str contains ANY of the items in set."""
    for c in set:
        if c in str: return 1
    return 0

def containsAll(str, set):
    """Check whether sequence str contains ALL of the items in set."""
    for c in set:
        if c not in str: return 0
    return 1

def containsOnly(str, set):
    """Check whether sequence str contains ONLY items in set."""
    for c in str:
        if c not in set: return 0
    return 1

def splitext(path):
    "Same as os.path.splitext() but faster."
    sep = rightmost_separator(path, os.sep)
    dot = string.rfind(path, '.')
    # An ext is only real if it has at least one non-digit char
    if dot > sep and not containsOnly(path[dot:], "0123456789."):
        return path[:dot],path[dot:]
    else:
        return path,""

def updrive(path):
    """
    Make the drive letter (if any) upper case.
    This is useful because Windows is inconsitent on the case
    of the drive letter, which can cause inconsistencies when
    calculating command signatures.
    """
    drive, rest = os.path.splitdrive(path)
    if drive:
        path = string.upper(drive) + rest
    return path

#
# Generic convert-to-string functions that abstract away whether or
# not the Python we're executing has Unicode support.  The wrapper
# to_String_for_signature() will use a for_signature() method if the
# specified object has one.
#
if hasattr(types, 'UnicodeType'):
    def to_String(s):
        if isinstance(s, UserString):
            t = type(s.data)
        else:
            t = type(s)
        if t is types.UnicodeType:
            return unicode(s)
        else:
            return str(s)
else:
    to_String = str

def to_String_for_signature(obj):
    try:
        f = obj.for_signature
    except:
        return to_String(obj)
    else:
        return f()

# Indexed by the SUBST_* constants below.
_strconv = [to_String, to_String, to_String_for_signature]

class Literal:
    """A wrapper for a string.  If you use this object wrapped
    around a string, then it will be interpreted as literal.
    When passed to the command interpreter, all special
    characters will be escaped."""
    def __init__(self, lstr):
        self.lstr = lstr

    def __str__(self):
        return self.lstr

    def escape(self, escape_func):
        return escape_func(self.lstr)

    def for_signature(self):
        return self.lstr

    def is_literal(self):
        return 1

class SpecialAttrWrapper:
    """This is a wrapper for what we call a 'Node special attribute.'
    This is any of the attributes of a Node that we can reference from
    Environment variable substitution, such as $TARGET.abspath or
    $SOURCES[1].filebase.  We implement the same methods as Literal
    so we can handle special characters, plus a for_signature method,
    such that we can return some canonical string during signature
    calculation to avoid unnecessary rebuilds."""

    def __init__(self, lstr, for_signature=None):
        """The for_signature parameter, if supplied, will be the
        canonical string we return from for_signature().  Else
        we will simply return lstr."""
        self.lstr = lstr
        if for_signature:
            self.forsig = for_signature
        else:
            self.forsig = lstr

    def __str__(self):
        return self.lstr

    def escape(self, escape_func):
        return escape_func(self.lstr)

    def for_signature(self):
        return self.forsig

    def is_literal(self):
        return 1

class CallableComposite(UserList.UserList):
    """A simple composite callable class that, when called, will invoke all
    of its contained callables with the same arguments."""
    def __call__(self, *args, **kwargs):
        retvals = map(lambda x, args=args, kwargs=kwargs: apply(x,
                                                                args,
                                                                kwargs),
                      self.data)
        if self.data and (len(self.data) == len(filter(callable, retvals))):
            return self.__class__(retvals)
        return NodeList(retvals)

class NodeList(UserList.UserList):
    """This class is almost exactly like a regular list of Nodes
    (actually it can hold any object), with one important difference.
    If you try to get an attribute from this list, it will return that
    attribute from every item in the list.  For example:

    >>> someList = NodeList([ '  foo  ', '  bar  ' ])
    >>> someList.strip()
    [ 'foo', 'bar' ]
    """
    def __nonzero__(self):
        return len(self.data) != 0

    def __str__(self):
        return string.join(map(str, self.data))

    def __getattr__(self, name):
        if not self.data:
            # If there is nothing in the list, then we have no attributes to
            # pass through, so raise AttributeError for everything.
            raise AttributeError, "NodeList has no attribute: %s" % name

        # Return a list of the attribute, gotten from every element
        # in the list
        attrList = map(lambda x, n=name: getattr(x, n), self.data)

        # Special case.  If the attribute is callable, we do not want
        # to return a list of callables.  Rather, we want to return a
        # single callable that, when called, will invoke the function on
        # all elements of this list.
        if self.data and (len(self.data) == len(filter(callable, attrList))):
            return CallableComposite(attrList)
        return self.__class__(attrList)

_valid_var = re.compile(r'[_a-zA-Z]\w*$')
_get_env_var = re.compile(r'^\$([_a-zA-Z]\w*|{[_a-zA-Z]\w*})$')

def is_valid_construction_var(varstr):
    """Return if the specified string is a legitimate construction
    variable.
    """
    return _valid_var.match(varstr)

def get_environment_var(varstr):
    """Given a string, first determine if it looks like a reference
    to a single environment variable, like "$FOO" or "${FOO}".
    If so, return that variable with no decorations ("FOO").
    If not, return None."""
    mo=_get_env_var.match(to_String(varstr))
    if mo:
        var = mo.group(1)
        if var[0] == '{':
            return var[1:-1]
        else:
            return var
    else:
        return None

def quote_spaces(arg):
    """Generic function for putting double quotes around any string that
    has white space in it."""
    if ' ' in arg or '\t' in arg:
        return '"%s"' % arg
    else:
        return str(arg)

class CmdStringHolder(UserString):
    """This is a special class used to hold strings generated by
    scons_subst() and scons_subst_list().  It defines a special method
    escape().  When passed a function with an escape algorithm for a
    particular platform, it will return the contained string with the
    proper escape sequences inserted.

    This should really be a subclass of UserString, but that module
    doesn't exist in Python 1.5.2."""
    def __init__(self, cmd, literal=None):
        UserString.__init__(self, cmd)
        self.literal = literal

    def is_literal(self):
        return self.literal

    def escape(self, escape_func, quote_func=quote_spaces):
        """Escape the string with the supplied function.  The
        function is expected to take an arbitrary string, then
        return it with all special characters escaped and ready
        for passing to the command interpreter.

        After calling this function, the next call to str() will
        return the escaped string.
        """

        if self.is_literal():
            return escape_func(self.data)
        elif ' ' in self.data or '\t' in self.data:
            return quote_func(self.data)
        else:
            return self.data

class DisplayEngine:
    def __init__(self):
        self.__call__ = self.print_it

    def print_it(self, text):
        sys.stdout.write(text + '\n')

    def dont_print(self, text):
        pass

    def set_mode(self, mode):
        if mode:
            self.__call__ = self.print_it
        else:
            self.__call__ = self.dont_print

def escape_list(list, escape_func):
    """Escape a list of arguments by running the specified escape_func
    on every object in the list that has an escape() method."""
    def escape(obj, escape_func=escape_func):
        try:
            e = obj.escape
        except AttributeError:
            return obj
        else:
            return e(escape_func)
    return map(escape, list)

class NLWrapper:
    """A wrapper class that delays turning a list of sources or targets
    into a NodeList until it's needed.  The specified function supplied
    when the object is initialized is responsible for turning raw nodes
    into proxies that implement the special attributes like .abspath,
    .source, etc.  This way, we avoid creating those proxies just
    "in case" someone is going to use $TARGET or the like, and only
    go through the trouble if we really have to.

    In practice, this might be a wash performance-wise, but it's a little
    cleaner conceptually...
    """

    def __init__(self, list, func):
        self.list = list
        self.func = func
    def _create_nodelist(self):
        try:
            return self.nodelist
        except AttributeError:
            list = self.list
            if list is None:
                list = []
            elif not is_List(list):
                list = [list]
            # The map(self.func) call is what actually turns
            # a list into appropriate proxies.
            self.nodelist = NodeList(map(self.func, list))
        return self.nodelist

class Targets_or_Sources(UserList.UserList):
    """A class that implements $TARGETS or $SOURCES expansions by in turn
    wrapping a NLWrapper.  This class handles the different methods used
    to access the list, calling the NLWrapper to create proxies on demand.

    Note that we subclass UserList.UserList purely so that the is_List()
    function will identify an object of this class as a list during
    variable expansion.  We're not really using any UserList.UserList
    methods in practice.
    """
    def __init__(self, nl):
        self.nl = nl
    def __getattr__(self, attr):
        nl = self.nl._create_nodelist()
        return getattr(nl, attr)
    def __getitem__(self, i):
        nl = self.nl._create_nodelist()
        return nl[i]
    def __getslice__(self, i, j):
        nl = self.nl._create_nodelist()
        i = max(i, 0); j = max(j, 0)
        return nl[i:j]
    def __str__(self):
        nl = self.nl._create_nodelist()
        return str(nl)
    def __repr__(self):
        nl = self.nl._create_nodelist()
        return repr(nl)

class Target_or_Source:
    """A class that implements $TARGET or $SOURCE expansions by in turn
    wrapping a NLWrapper.  This class handles the different methods used
    to access an individual proxy Node, calling the NLWrapper to create
    a proxy on demand.
    """
    def __init__(self, nl):
        self.nl = nl
    def __getattr__(self, attr):
        nl = self.nl._create_nodelist()
        try:
            nl0 = nl[0]
        except IndexError:
            # If there is nothing in the list, then we have no attributes to
            # pass through, so raise AttributeError for everything.
            raise AttributeError, "NodeList has no attribute: %s" % attr
        return getattr(nl0, attr)
    def __str__(self):
        nl = self.nl._create_nodelist()
        try:
            nl0 = nl[0]
        except IndexError:
            return ''
        return str(nl0)
    def __repr__(self):
        nl = self.nl._create_nodelist()
        try:
            nl0 = nl[0]
        except IndexError:
            return ''
        return repr(nl0)

def subst_dict(target, source):
    """Create a dictionary for substitution of special
    construction variables.

    This translates the following special arguments:

    target - the target (object or array of objects),
             used to generate the TARGET and TARGETS
             construction variables

    source - the source (object or array of objects),
             used to generate the SOURCES and SOURCE
             construction variables
    """
    dict = {}

    if target:
        tnl = NLWrapper(target, lambda x: x.get_subst_proxy())
        dict['TARGETS'] = Targets_or_Sources(tnl)
        dict['TARGET'] = Target_or_Source(tnl)

    if source:
        snl = NLWrapper(source, lambda x: x.rfile().get_subst_proxy())
        dict['SOURCES'] = Targets_or_Sources(snl)
        dict['SOURCE'] = Target_or_Source(snl)

    return dict

# Constants for the "mode" parameter to scons_subst_list() and
# scons_subst().  SUBST_RAW gives the raw command line.  SUBST_CMD
# gives a command line suitable for passing to a shell.  SUBST_SIG
# gives a command line appropriate for calculating the signature
# of a command line...if this changes, we should rebuild.
SUBST_CMD = 0
SUBST_RAW = 1
SUBST_SIG = 2

_rm = re.compile(r'\$[()]')
_remove = re.compile(r'\$\(([^\$]|\$[^\(])*?\$\)')

# Indexed by the SUBST_* constants above.
_regex_remove = [ _rm, None, _remove ]

# This regular expression splits a string into the following types of
# arguments for use by the scons_subst() and scons_subst_list() functions:
#
#       "$$"
#       "$("
#       "$)"
#       "$variable"             [must begin with alphabetic or underscore]
#       "${any stuff}"
#       "   "                   [white space]
#       "non-white-space"       [without any dollar signs]
#       "$"                     [single dollar sign]
#
_separate_args = re.compile(r'(\$[\$\(\)]|\$[_a-zA-Z][\.\w]*|\${[^}]*}|\s+|[^\s\$]+|\$)')

# This regular expression is used to replace strings of multiple white
# space characters in the string result from the scons_subst() function.
_space_sep = re.compile(r'[\t ]+(?![^{]*})')

def scons_subst(strSubst, env, mode=SUBST_RAW, target=None, source=None, dict=None, conv=None, gvars=None):
    """Expand a string containing construction variable substitutions.

    This is the work-horse function for substitutions in file names
    and the like.  The companion scons_subst_list() function (below)
    handles separating command lines into lists of arguments, so see
    that function if that's what you're looking for.
    """
    class StringSubber:
        """A class to construct the results of a scons_subst() call.

        This binds a specific construction environment, mode, target and
        source with two methods (substitute() and expand()) that handle
        the expansion.
        """
        def __init__(self, env, mode, target, source, conv, gvars):
            self.env = env
            self.mode = mode
            self.target = target
            self.source = source
            self.conv = conv
            self.gvars = gvars

        def expand(self, s, lvars):
            """Expand a single "token" as necessary, returning an
            appropriate string containing the expansion.

            This handles expanding different types of things (strings,
            lists, callables) appropriately.  It calls the wrapper
            substitute() method to re-expand things as necessary, so that
            the results of expansions of side-by-side strings still get
            re-evaluated separately, not smushed together.
            """
            if is_String(s):
                try:
                    s0, s1 = s[:2]
                except (IndexError, ValueError):
                    return s
                if s0 == '$':
                    if s1 == '$':
                        return '$'
                    elif s1 in '()':
                        return s
                    else:
                        key = s[1:]
                        if key[0] == '{':
                            key = key[1:-1]
                        try:
                            s = eval(key, self.gvars, lvars)
                        except (IndexError, NameError, TypeError):
                            return ''
                        except SyntaxError,e:
                            if self.target:
                                raise SCons.Errors.BuildError, (self.target[0], "Syntax error `%s' trying to evaluate `%s'" % (e,s))
                            else:
                                raise SCons.Errors.UserError, "Syntax error `%s' trying to evaluate `%s'" % (e,s)
                        else:
                            # Before re-expanding the result, handle
                            # recursive expansion by copying the local
                            # variable dictionary and overwriting a null
                            # string for the value of the variable name
                            # we just expanded.
                            lv = lvars.copy()
                            var = string.split(key, '.')[0]
                            lv[var] = ''
                            return self.substitute(s, lv)
                else:
                    return s
            elif is_List(s):
                r = []
                for l in s:
                    r.append(self.conv(self.substitute(l, lvars)))
                return string.join(r)
            elif callable(s):
                s = s(target=self.target,
                     source=self.source,
                     env=self.env,
                     for_signature=(self.mode != SUBST_CMD))
                return self.substitute(s, lvars)
            elif s is None:
                return ''
            else:
                return s

        def substitute(self, args, lvars):
            """Substitute expansions in an argument or list of arguments.

            This serves as a wrapper for splitting up a string into
            separate tokens.
            """
            if is_String(args) and not isinstance(args, CmdStringHolder):
                args = _separate_args.findall(args)
                result = []
                for a in args:
                    result.append(self.conv(self.expand(a, lvars)))
                try:
                    result = string.join(result, '')
                except TypeError:
                    pass
                return result
            else:
                return self.expand(args, lvars)

    if dict is None:
        dict = subst_dict(target, source)
    if conv is None:
        conv = _strconv[mode]
    if gvars is None:
        gvars = env.Dictionary()

    ss = StringSubber(env, mode, target, source, conv, gvars)
    result = ss.substitute(strSubst, dict)

    if is_String(result):
        # Remove $(-$) pairs and any stuff in between,
        # if that's appropriate.
        remove = _regex_remove[mode]
        if remove:
            result = remove.sub('', result)
        if mode != SUBST_RAW:
            # Compress strings of white space characters into
            # a single space.
            result = string.strip(_space_sep.sub(' ', result))

    return result

def scons_subst_list(strSubst, env, mode=SUBST_RAW, target=None, source=None, dict=None, conv=None, gvars=None):
    """Substitute construction variables in a string (or list or other
    object) and separate the arguments into a command list.

    The companion scons_subst() function (above) handles basic
    substitutions within strings, so see that function instead
    if that's what you're looking for.
    """
    class ListSubber(UserList.UserList):
        """A class to construct the results of a scons_subst_list() call.

        Like StringSubber, this class binds a specific construction
        environment, mode, target and source with two methods
        (substitute() and expand()) that handle the expansion.

        In addition, however, this class is used to track the state of
        the result(s) we're gathering so we can do the appropriate thing
        whenever we have to append another word to the result--start a new
        line, start a new word, append to the current word, etc.  We do
        this by setting the "append" attribute to the right method so
        that our wrapper methods only need ever call ListSubber.append(),
        and the rest of the object takes care of doing the right thing
        internally.
        """
        def __init__(self, env, mode, target, source, conv, gvars):
            UserList.UserList.__init__(self, [])
            self.env = env
            self.mode = mode
            self.target = target
            self.source = source
            self.conv = conv
            self.gvars = gvars

            if self.mode == SUBST_RAW:
                self.add_strip = lambda x, s=self: s.append(x)
            else:
                self.add_strip = lambda x, s=self: None
            self.in_strip = None
            self.next_line()

        def expand(self, s, lvars, within_list):
            """Expand a single "token" as necessary, appending the
            expansion to the current result.

            This handles expanding different types of things (strings,
            lists, callables) appropriately.  It calls the wrapper
            substitute() method to re-expand things as necessary, so that
            the results of expansions of side-by-side strings still get
            re-evaluated separately, not smushed together.
            """

            if is_String(s):
                try:
                    s0, s1 = s[:2]
                except (IndexError, ValueError):
                    self.append(s)
                    return
                if s0 == '$':
                    if s1 == '$':
                        self.append('$')
                    elif s1 == '(':
                        self.open_strip('$(')
                    elif s1 == ')':
                        self.close_strip('$)')
                    else:
                        key = s[1:]
                        if key[0] == '{':
                            key = key[1:-1]
                        try:
                            s = eval(key, self.gvars, lvars)
                        except (IndexError, NameError, TypeError):
                            return
                        except SyntaxError,e:
                            if self.target:
                                raise SCons.Errors.BuildError, (self.target[0], "Syntax error `%s' trying to evaluate `%s'" % (e,s))
                            else:
                                raise SCons.Errors.UserError, "Syntax error `%s' trying to evaluate `%s'" % (e,s)
                        else:
                            # Before re-expanding the result, handle
                            # recursive expansion by copying the local
                            # variable dictionary and overwriting a null
                            # string for the value of the variable name
                            # we just expanded.
                            lv = lvars.copy()
                            var = string.split(key, '.')[0]
                            lv[var] = ''
                            self.substitute(s, lv, 0)
                            self.this_word()
                else:
                    self.append(s)
            elif is_List(s):
                for a in s:
                    self.substitute(a, lvars, 1)
                    self.next_word()
            elif callable(s):
                s = s(target=self.target,
                     source=self.source,
                     env=self.env,
                     for_signature=(self.mode != SUBST_CMD))
                self.substitute(s, lvars, within_list)
            elif s is None:
                self.this_word()
            else:
                self.append(s)

        def substitute(self, args, lvars, within_list):
            """Substitute expansions in an argument or list of arguments.

            This serves as a wrapper for splitting up a string into
            separate tokens.
            """

            if is_String(args) and not isinstance(args, CmdStringHolder):
                args = _separate_args.findall(args)
                for a in args:
                    if a[0] in ' \t\n\r\f\v':
                        if '\n' in a:
                            self.next_line()
                        elif within_list:
                            self.append(a)
                        else:
                            self.next_word()
                    else:
                        self.expand(a, lvars, within_list)
            else:
                self.expand(args, lvars, within_list)

        def next_line(self):
            """Arrange for the next word to start a new line.  This
            is like starting a new word, except that we have to append
            another line to the result."""
            UserList.UserList.append(self, [])
            self.next_word()

        def this_word(self):
            """Arrange for the next word to append to the end of the
            current last word in the result."""
            self.append = self.add_to_current_word

        def next_word(self):
            """Arrange for the next word to start a new word."""
            self.append = self.add_new_word

        def add_to_current_word(self, x):
            """Append the string x to the end of the current last word
            in the result.  If that is not possible, then just add
            it as a new word.  Make sure the entire concatenated string
            inherits the object attributes of x (in particular, the
            escape function) by wrapping it as CmdStringHolder."""

            if not self.in_strip or self.mode != SUBST_SIG:
                try:
                    current_word = self[-1][-1]
                except IndexError:
                    self.add_new_word(x)
                else:
                    # All right, this is a hack and it should probably
                    # be refactored out of existence in the future.
                    # The issue is that we want to smoosh words together
                    # and make one file name that gets escaped if
                    # we're expanding something like foo$EXTENSION,
                    # but we don't want to smoosh them together if
                    # it's something like >$TARGET, because then we'll
                    # treat the '>' like it's part of the file name.
                    # So for now, just hard-code looking for the special
                    # command-line redirection characters...
                    try:
                        last_char = str(current_word)[-1]
                    except IndexError:
                        last_char = '\0'
                    if last_char in '<>|':
                        self.add_new_word(x)
                    else:
                        y = current_word + x
                        literal1 = self.literal(self[-1][-1])
                        literal2 = self.literal(x)
                        y = self.conv(y)
                        if is_String(y):
                            y = CmdStringHolder(y, literal1 or literal2)
                        self[-1][-1] = y

        def add_new_word(self, x):
            if not self.in_strip or self.mode != SUBST_SIG:
                literal = self.literal(x)
                x = self.conv(x)
                if is_String(x):
                    x = CmdStringHolder(x, literal)
                self[-1].append(x)
            self.append = self.add_to_current_word

        def literal(self, x):
            try:
                l = x.is_literal
            except AttributeError:
                return None
            else:
                return l()

        def open_strip(self, x):
            """Handle the "open strip" $( token."""
            self.add_strip(x)
            self.in_strip = 1

        def close_strip(self, x):
            """Handle the "close strip" $) token."""
            self.add_strip(x)
            self.in_strip = None

    if dict is None:
        dict = subst_dict(target, source)
    if conv is None:
        conv = _strconv[mode]
    if gvars is None:
        gvars = env.Dictionary()

    ls = ListSubber(env, mode, target, source, conv, gvars)
    ls.substitute(strSubst, dict, 0)

    return ls.data

def scons_subst_once(strSubst, env, key):
    """Perform single (non-recursive) substitution of a single
    construction variable keyword.

    This is used when setting a variable when copying or overriding values
    in an Environment.  We want to capture (expand) the old value before
    we override it, so people can do things like:

        env2 = env.Copy(CCFLAGS = '$CCFLAGS -g')

    We do this with some straightforward, brute-force code here...
    """
    matchlist = ['$' + key, '${' + key + '}']
    if is_List(strSubst):
        result = []
        for arg in strSubst:
            if is_String(arg):
                if arg in matchlist:
                    arg = env[key]
                    if is_List(arg):
                        result.extend(arg)
                    else:
                        result.append(arg)
                else:
                    r = []
                    for a in _separate_args.findall(arg):
                        if a in matchlist:
                            a = env[key]
                        if is_List(a):
                            r.append(string.join(map(str, a)))
                        else:
                            r.append(str(a))
                    result.append(string.join(r, ''))
            else:
                result.append(arg)
        return result
    elif is_String(strSubst):
        result = []
        for a in _separate_args.findall(strSubst):
            if a in matchlist:
                a = env[key]
            if is_List(a):
                result.append(string.join(map(str, a)))
            else:
                result.append(str(a))
        return string.join(result, '')
    else:
        return strSubst

def render_tree(root, child_func, prune=0, margin=[0], visited={}):
    """
    Render a tree of nodes into an ASCII tree view.
    root - the root node of the tree
    child_func - the function called to get the children of a node
    prune - don't visit the same node twice
    margin - the format of the left margin to use for children of root.
       1 results in a pipe, and 0 results in no pipe.
    visited - a dictionary of visited nodes in the current branch if not prune,
       or in the whole tree if prune.
    """

    if visited.has_key(root):
        return ""

    children = child_func(root)
    retval = ""
    for pipe in margin[:-1]:
        if pipe:
            retval = retval + "| "
        else:
            retval = retval + "  "

    retval = retval + "+-" + str(root) + "\n"
    if not prune:
        visited = copy.copy(visited)
    visited[root] = 1

    for i in range(len(children)):
        margin.append(i<len(children)-1)
        retval = retval + render_tree(children[i], child_func, prune, margin, visited
)
        margin.pop()

    return retval

def is_Dict(e):
    return type(e) is types.DictType or isinstance(e, UserDict.UserDict)

def is_List(e):
    return type(e) is types.ListType or isinstance(e, UserList.UserList)

if hasattr(types, 'UnicodeType'):
    def is_String(e):
        return type(e) is types.StringType \
            or type(e) is types.UnicodeType \
            or isinstance(e, UserString)
else:
    def is_String(e):
        return type(e) is types.StringType or isinstance(e, UserString)

def is_Scalar(e):
    return is_String(e) or not is_List(e)

def flatten(sequence, scalarp=is_Scalar, result=None):
    if result is None:
        result = []
    for item in sequence:
        if scalarp(item):
            result.append(item)
        else:
            flatten(item, scalarp, result)
    return result

class Proxy:
    """A simple generic Proxy class, forwarding all calls to
    subject.  So, for the benefit of the python newbie, what does
    this really mean?  Well, it means that you can take an object, let's
    call it 'objA', and wrap it in this Proxy class, with a statement
    like this

                 proxyObj = Proxy(objA),

    Then, if in the future, you do something like this

                 x = proxyObj.var1,

    since Proxy does not have a 'var1' attribute (but presumably objA does),
    the request actually is equivalent to saying

                 x = objA.var1

    Inherit from this class to create a Proxy."""

    def __init__(self, subject):
        """Wrap an object as a Proxy object"""
        self.__subject = subject

    def __getattr__(self, name):
        """Retrieve an attribute from the wrapped object.  If the named
           attribute doesn't exist, AttributeError is raised"""
        return getattr(self.__subject, name)

    def get(self):
        """Retrieve the entire wrapped object"""
        return self.__subject

# attempt to load the windows registry module:
can_read_reg = 0
try:
    import _winreg

    can_read_reg = 1
    hkey_mod = _winreg

    RegOpenKeyEx = _winreg.OpenKeyEx
    RegEnumKey = _winreg.EnumKey
    RegEnumValue = _winreg.EnumValue
    RegQueryValueEx = _winreg.QueryValueEx
    RegError = _winreg.error

except ImportError:
    try:
        import win32api
        import win32con
        can_read_reg = 1
        hkey_mod = win32con

        RegOpenKeyEx = win32api.RegOpenKeyEx
        RegEnumKey = win32api.RegEnumKey
        RegEnumValue = win32api.RegEnumValue
        RegQueryValueEx = win32api.RegQueryValueEx
        RegError = win32api.error

    except ImportError:
        class _NoError(Exception):
            pass
        RegError = _NoError

if can_read_reg:
    HKEY_CLASSES_ROOT = hkey_mod.HKEY_CLASSES_ROOT
    HKEY_LOCAL_MACHINE = hkey_mod.HKEY_LOCAL_MACHINE
    HKEY_CURRENT_USER = hkey_mod.HKEY_CURRENT_USER
    HKEY_USERS = hkey_mod.HKEY_USERS

    def RegGetValue(root, key):
        """This utility function returns a value in the registry
        without having to open the key first.  Only available on
        Windows platforms with a version of Python that can read the
        registry.  Returns the same thing as
        SCons.Util.RegQueryValueEx, except you just specify the entire
        path to the value, and don't have to bother opening the key
        first.  So:

        Instead of:
          k = SCons.Util.RegOpenKeyEx(SCons.Util.HKEY_LOCAL_MACHINE,
                r'SOFTWARE\Microsoft\Windows\CurrentVersion')
          out = SCons.Util.RegQueryValueEx(k,
                'ProgramFilesDir')

        You can write:
          out = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE,
                r'SOFTWARE\Microsoft\Windows\CurrentVersion\ProgramFilesDir')
        """
        # I would use os.path.split here, but it's not a filesystem
        # path...
        p = key.rfind('\\') + 1
        keyp = key[:p]
        val = key[p:]
        k = SCons.Util.RegOpenKeyEx(root, keyp)
        return SCons.Util.RegQueryValueEx(k,val)

if sys.platform == 'win32':

    def WhereIs(file, path=None, pathext=None, reject=[]):
        if path is None:
            try:
                path = os.environ['PATH']
            except KeyError:
                return None
        if is_String(path):
            path = string.split(path, os.pathsep)
        if pathext is None:
            try:
                pathext = os.environ['PATHEXT']
            except KeyError:
                pathext = '.COM;.EXE;.BAT;.CMD'
        if is_String(pathext):
            pathext = string.split(pathext, os.pathsep)
        for ext in pathext:
            if string.lower(ext) == string.lower(file[-len(ext):]):
                pathext = ['']
                break
        if not is_List(reject):
            reject = [reject]
        for dir in path:
            f = os.path.join(dir, file)
            for ext in pathext:
                fext = f + ext
                if os.path.isfile(fext):
                    try:
                        reject.index(fext)
                    except ValueError:
                        return os.path.normpath(fext)
                    continue
        return None

elif os.name == 'os2':

    def WhereIs(file, path=None, pathext=None, reject=[]):
        if path is None:
            try:
                path = os.environ['PATH']
            except KeyError:
                return None
        if is_String(path):
            path = string.split(path, os.pathsep)
        if pathext is None:
            pathext = ['.exe', '.cmd']
        for ext in pathext:
            if string.lower(ext) == string.lower(file[-len(ext):]):
                pathext = ['']
                break
        if not is_List(reject):
            reject = [reject]
        for dir in path:
            f = os.path.join(dir, file)
            for ext in pathext:
                fext = f + ext
                if os.path.isfile(fext):
                    try:
                        reject.index(fext)
                    except ValueError:
                        return os.path.normpath(fext)
                    continue
        return None

else:

    def WhereIs(file, path=None, pathext=None, reject=[]):
        if path is None:
            try:
                path = os.environ['PATH']
            except KeyError:
                return None
        if is_String(path):
            path = string.split(path, os.pathsep)
        if not is_List(reject):
            reject = [reject]
        for d in path:
            f = os.path.join(d, file)
            if os.path.isfile(f):
                try:
                    st = os.stat(f)
                except OSError:
                    continue
                if stat.S_IMODE(st[stat.ST_MODE]) & 0111:
                    try:
                        reject.index(f)
                    except ValueError:
                        return os.path.normpath(f)
                    continue
        return None

def PrependPath(oldpath, newpath, sep = os.pathsep):
    """This prepends newpath elements to the given oldpath.  Will only
    add any particular path once (leaving the first one it encounters
    and ignoring the rest, to preserve path order), and will
    os.path.normpath and os.path.normcase all paths to help assure
    this.  This can also handle the case where the given old path
    variable is a list instead of a string, in which case a list will
    be returned instead of a string.

    Example:
      Old Path: "/foo/bar:/foo"
      New Path: "/biz/boom:/foo"
      Result:   "/biz/boom:/foo:/foo/bar"
    """

    orig = oldpath
    is_list = 1
    paths = orig
    if not is_List(orig):
        paths = string.split(paths, sep)
        is_list = 0

    if is_List(newpath):
        newpaths = newpath
    else:
        newpaths = string.split(newpath, sep)

    newpaths = newpaths + paths # prepend new paths

    normpaths = []
    paths = []
    # now we add them only if they are unique
    for path in newpaths:
        normpath = os.path.normpath(os.path.normcase(path))
        if path and not normpath in normpaths:
            paths.append(path)
            normpaths.append(normpath)

    if is_list:
        return paths
    else:
        return string.join(paths, sep)

def AppendPath(oldpath, newpath, sep = os.pathsep):
    """This appends new path elements to the given old path.  Will
    only add any particular path once (leaving the last one it
    encounters and ignoring the rest, to preserve path order), and
    will os.path.normpath and os.path.normcase all paths to help
    assure this.  This can also handle the case where the given old
    path variable is a list instead of a string, in which case a list
    will be returned instead of a string.

    Example:
      Old Path: "/foo/bar:/foo"
      New Path: "/biz/boom:/foo"
      Result:   "/foo/bar:/biz/boom:/foo"
    """

    orig = oldpath
    is_list = 1
    paths = orig
    if not is_List(orig):
        paths = string.split(paths, sep)
        is_list = 0

    if is_List(newpath):
        newpaths = newpath
    else:
        newpaths = string.split(newpath, sep)

    newpaths = paths + newpaths # append new paths
    newpaths.reverse()

    normpaths = []
    paths = []
    # now we add them only of they are unique
    for path in newpaths:
        normpath = os.path.normpath(os.path.normcase(path))
        if path and not normpath in normpaths:
            paths.append(path)
            normpaths.append(normpath)

    paths.reverse()

    if is_list:
        return paths
    else:
        return string.join(paths, sep)


def dir_index(directory):
    files = []
    for f in os.listdir(directory):
        fullname = os.path.join(directory, f)
        files.append(fullname)

    # os.listdir() isn't guaranteed to return files in any specific order,
    # but some of the test code expects sorted output.
    files.sort()
    return files

def fs_delete(path, remove=1):
    try:
        if os.path.exists(path):
            if os.path.isfile(path):
                if remove: os.unlink(path)
                display("Removed " + path)
            elif os.path.isdir(path) and not os.path.islink(path):
                # delete everything in the dir
                for p in dir_index(path):
                    if os.path.isfile(p):
                        if remove: os.unlink(p)
                        display("Removed " + p)
                    else:
                        fs_delete(p, remove)
                # then delete dir itself
                if remove: os.rmdir(path)
                display("Removed directory " + path)
    except OSError, e:
        print "scons: Could not remove '%s':" % str(path), e.strerror

if sys.platform == 'cygwin':
    def get_native_path(path):
        """Transforms an absolute path into a native path for the system.  In
        Cygwin, this converts from a Cygwin path to a Win32 one."""
        return string.replace(os.popen('cygpath -w ' + path).read(), '\n', '')
else:
    def get_native_path(path):
        """Transforms an absolute path into a native path for the system.
        Non-Cygwin version, just leave the path alone."""
        return path

display = DisplayEngine()

def Split(arg):
    if is_List(arg):
        return arg
    elif is_String(arg):
        return string.split(arg)
    else:
        return [arg]

class CLVar(UserList.UserList):
    """A class for command-line construction variables.

    This is a list that uses Split() to split an initial string along
    white-space arguments, and similarly to split any strings that get
    added.  This allows us to Do the Right Thing with Append() and
    Prepend() (as well as straight Python foo = env['VAR'] + 'arg1
    arg2') regardless of whether a user adds a list or a string to a
    command-line construction variable.
    """
    def __init__(self, seq = []):
        UserList.UserList.__init__(self, Split(seq))
    def __coerce__(self, other):
        return (self, CLVar(other))
    def __str__(self):
        return string.join(self.data)

class Selector(UserDict.UserDict):
    """A callable dictionary that maps file suffixes to dictionary
    values."""
    def __call__(self, env, source):
        ext = splitext(str(source[0]))[1]
        try:
            return self[ext]
        except KeyError:
            # Try to perform Environment substitution on the keys of
            # emitter_dict before giving up.
            s_dict = {}
            for (k,v) in self.items():
                if not k is None:
                    s_k = env.subst(k)
                    if s_dict.has_key(s_k):
                        # We only raise an error when variables point
                        # to the same suffix.  If one suffix is literal
                        # and a variable suffix contains this literal,
                        # the literal wins and we don't raise an error.
                        raise KeyError, (s_dict[s_k][0], k, s_k)
                    s_dict[s_k] = (k,v)
            try:
                return s_dict[ext][1]
            except KeyError:
                try:
                    return self[None]
                except KeyError:
                    return None


if sys.platform == 'cygwin':
    # On Cygwin, os.path.normcase() lies, so just report back the
    # fact that the underlying Win32 OS is case-insensitive.
    def case_sensitive_suffixes(s1, s2):
        return 0
else:
    def case_sensitive_suffixes(s1, s2):
        return (os.path.normcase(s1) != os.path.normcase(s2))

def adjustixes(fname, pre, suf):
    if pre:
        path, fn = os.path.split(os.path.normpath(fname))
        if fn[:len(pre)] != pre:
            fname = os.path.join(path, pre + fn)
    # Only append a suffix if the file does not have one.
    if suf and not splitext(fname)[1] and fname[-len(suf):] != suf:
            fname = fname + suf
    return fname


def unique(s):
    """Return a list of the elements in s, but without duplicates.

    For example, unique([1,2,3,1,2,3]) is some permutation of [1,2,3],
    unique("abcabc") some permutation of ["a", "b", "c"], and
    unique(([1, 2], [2, 3], [1, 2])) some permutation of
    [[2, 3], [1, 2]].

    For best speed, all sequence elements should be hashable.  Then
    unique() will usually work in linear time.

    If not possible, the sequence elements should enjoy a total
    ordering, and if list(s).sort() doesn't raise TypeError it's
    assumed that they do enjoy a total ordering.  Then unique() will
    usually work in O(N*log2(N)) time.

    If that's not possible either, the sequence elements must support
    equality-testing.  Then unique() will usually work in quadratic
    time.
    """

    n = len(s)
    if n == 0:
        return []

    # Try using a dict first, as that's the fastest and will usually
    # work.  If it doesn't work, it will usually fail quickly, so it
    # usually doesn't cost much to *try* it.  It requires that all the
    # sequence elements be hashable, and support equality comparison.
    u = {}
    try:
        for x in s:
            u[x] = 1
    except TypeError:
        del u  # move on to the next method
    else:
        return u.keys()

    # We can't hash all the elements.  Second fastest is to sort,
    # which brings the equal elements together; then duplicates are
    # easy to weed out in a single pass.
    # NOTE:  Python's list.sort() was designed to be efficient in the
    # presence of many duplicate elements.  This isn't true of all
    # sort functions in all languages or libraries, so this approach
    # is more effective in Python than it may be elsewhere.
    try:
        t = list(s)
        t.sort()
    except TypeError:
        del t  # move on to the next method
    else:
        assert n > 0
        last = t[0]
        lasti = i = 1
        while i < n:
            if t[i] != last:
                t[lasti] = last = t[i]
                lasti = lasti + 1
            i = i + 1
        return t[:lasti]

    # Brute force is all that's left.
    u = []
    for x in s:
        if x not in u:
            u.append(x)
    return u


