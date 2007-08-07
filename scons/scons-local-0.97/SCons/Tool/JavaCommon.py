"""SCons.Tool.JavaCommon

Stuff for processing Java.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/JavaCommon.py 0.97.D001 2007/05/17 11:35:19 knight"

import os
import os.path
import re
import string

java_parsing = 1

if java_parsing:
    # Parse Java files for class names.
    #
    # This is a really cool parser from Charles Crain
    # that finds appropriate class names in Java source.

    # A regular expression that will find, in a java file:
    #     newlines;
    #     double-backslashes;
    #     a single-line comment "//";
    #     single or double quotes preceeded by a backslash;
    #     single quotes, double quotes, open or close braces, semi-colons;
    #     any alphanumeric token (keyword, class name, specifier);
    #     the multi-line comment begin and end tokens /* and */;
    #     array declarations "[]";
    #     semi-colons;
    #     periods.
    _reToken = re.compile(r'(\n|\\\\|//|\\[\'"]|[\'"\{\}\;\.]|' +
                          r'[A-Za-z_][\w\.]*|/\*|\*/|\[\])')

    class OuterState:
        """The initial state for parsing a Java file for classes,
        interfaces, and anonymous inner classes."""
        def __init__(self):
            self.listClasses = []
            self.listOutputs = []
            self.stackBrackets = []
            self.brackets = 0
            self.nextAnon = 1
            self.package = None

        def trace(self):
            pass

        def __getClassState(self):
            try:
                return self.classState
            except AttributeError:
                ret = ClassState(self)
                self.classState = ret
                return ret

        def __getPackageState(self):
            try:
                return self.packageState
            except AttributeError:
                ret = PackageState(self)
                self.packageState = ret
                return ret

        def __getAnonClassState(self):
            try:
                return self.anonState
            except AttributeError:
                ret = SkipState(1, AnonClassState(self))
                self.anonState = ret
                return ret

        def __getSkipState(self):
            try:
                return self.skipState
            except AttributeError:
                ret = SkipState(1, self)
                self.skipState = ret
                return ret

        def openBracket(self):
            self.brackets = self.brackets + 1

        def closeBracket(self):
            self.brackets = self.brackets - 1
            if len(self.stackBrackets) and \
               self.brackets == self.stackBrackets[-1]:
                self.listOutputs.append(string.join(self.listClasses, '$'))
                self.listClasses.pop()
                self.stackBrackets.pop()

        def parseToken(self, token):
            if token[:2] == '//':
                return IgnoreState('\n', self)
            elif token == '/*':
                return IgnoreState('*/', self)
            elif token == '{':
                self.openBracket()
            elif token == '}':
                self.closeBracket()
            elif token in [ '"', "'" ]:
                return IgnoreState(token, self)
            elif token == "new":
                # anonymous inner class
                if len(self.listClasses) > 0:
                    return self.__getAnonClassState()
                return self.__getSkipState() # Skip the class name
            elif token in ['class', 'interface', 'enum']:
                if len(self.listClasses) == 0:
                    self.nextAnon = 1
                self.stackBrackets.append(self.brackets)
                return self.__getClassState()
            elif token == 'package':
                return self.__getPackageState()
            elif token == '.':
                # Skip the attribute, it might be named "class", in which
                # case we don't want to treat the following token as
                # an inner class name...
                return self.__getSkipState()
            return self

        def addAnonClass(self):
            """Add an anonymous inner class"""
            clazz = self.listClasses[0]
            self.listOutputs.append('%s$%d' % (clazz, self.nextAnon))
            self.nextAnon = self.nextAnon + 1

        def setPackage(self, package):
            self.package = package

    class AnonClassState:
        """A state that looks for anonymous inner classes."""
        def __init__(self, outer_state):
            # outer_state is always an instance of OuterState
            self.outer_state = outer_state
            self.tokens_to_find = 2
        def parseToken(self, token):
            # This is an anonymous class if and only if the next  
            # non-whitespace token is a bracket            
            if token == '\n':
                return self
            if token == '{':
                self.outer_state.openBracket()
                self.outer_state.addAnonClass()
            elif token == '}':
                self.outer_state.closeBracket()
            elif token in ['"', "'"]:
                return IgnoreState(token, self)
            return self.outer_state

    class SkipState:
        """A state that will skip a specified number of tokens before
        reverting to the previous state."""
        def __init__(self, tokens_to_skip, old_state):
            self.tokens_to_skip = tokens_to_skip
            self.old_state = old_state
        def parseToken(self, token):
            self.tokens_to_skip = self.tokens_to_skip - 1
            if self.tokens_to_skip < 1:
                return self.old_state
            return self

    class ClassState:
        """A state we go into when we hit a class or interface keyword."""
        def __init__(self, outer_state):
            # outer_state is always an instance of OuterState
            self.outer_state = outer_state
        def parseToken(self, token):
            # the next non-whitespace token should be the name of the class
            if token == '\n':
                return self
            self.outer_state.listClasses.append(token)
            return self.outer_state

    class IgnoreState:
        """A state that will ignore all tokens until it gets to a
        specified token."""
        def __init__(self, ignore_until, old_state):
            self.ignore_until = ignore_until
            self.old_state = old_state
        def parseToken(self, token):
            if self.ignore_until == token:
                return self.old_state
            return self

    class PackageState:
        """The state we enter when we encounter the package keyword.
        We assume the next token will be the package name."""
        def __init__(self, outer_state):
            # outer_state is always an instance of OuterState
            self.outer_state = outer_state
        def parseToken(self, token):
            self.outer_state.setPackage(token)
            return self.outer_state

    def parse_java_file(fn):
        return parse_java(open(fn, 'r').read())

    def parse_java(contents, trace=None):
        """Parse a .java file and return a double of package directory,
        plus a list of .class files that compiling that .java file will
        produce"""
        package = None
        initial = OuterState()
        currstate = initial
        for token in _reToken.findall(contents):
            # The regex produces a bunch of groups, but only one will
            # have anything in it.
            currstate = currstate.parseToken(token)
            if trace: trace(token, currstate)
        if initial.package:
            package = string.replace(initial.package, '.', os.sep)
        return (package, initial.listOutputs)

else:
    # Don't actually parse Java files for class names.
    #
    # We might make this a configurable option in the future if
    # Java-file parsing takes too long (although it shouldn't relative
    # to how long the Java compiler itself seems to take...).

    def parse_java_file(fn):
        """ "Parse" a .java file.

        This actually just splits the file name, so the assumption here
        is that the file name matches the public class name, and that
        the path to the file is the same as the package name.
        """
        return os.path.split(file)
