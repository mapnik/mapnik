# MIT License
#
# Copyright The SCons Foundation
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

"""Adds user-friendly customizable variables to an SCons build."""

import os.path
import sys
from functools import cmp_to_key
from typing import Callable, Dict, List, Optional, Sequence, Union

import SCons.Errors
import SCons.Util
import SCons.Warnings

# Note: imports are for the benefit of SCons.Main (and tests); since they
#   are not used here, the "as Foo" form is for checkers.
from .BoolVariable import BoolVariable
from .EnumVariable import EnumVariable
from .ListVariable import ListVariable
from .PackageVariable import PackageVariable
from .PathVariable import PathVariable

__all__ = [
    "Variable",
    "Variables",
    "BoolVariable",
    "EnumVariable",
    "ListVariable",
    "PackageVariable",
    "PathVariable",
]

class Variable:
    """A Build Variable."""

    __slots__ = ('key', 'aliases', 'help', 'default', 'validator', 'converter', 'do_subst')

    def __lt__(self, other):
        """Comparison fuction so Variable instances sort."""
        return self.key < other.key

    def __str__(self) -> str:
        """Provide a way to "print" a Variable object."""
        return (
            f"({self.key!r}, {self.aliases}, {self.help!r}, {self.default!r}, "
            f"validator={self.validator}, converter={self.converter})"
        )


class Variables:
    """A container for multiple Build Variables.

    Includes methods to updates the environment with the variables,
    and to render the help text.

    Arguments:
      files: string or list of strings naming variable config scripts
         (default ``None``)
      args: dictionary to override values set from *files*.  (default ``None``)
      is_global: if true, return a global singleton :class:`Variables` object
         instead of a fresh instance. Currently inoperable (default ``False``)

    .. versionchanged:: 4.8.0
       The default for *is_global* changed to ``False`` (previously
       ``True`` but it had no effect due to an implementation error).

    .. deprecated:: 4.8.0
       *is_global* is deprecated.
    """

    def __init__(
        self,
        files: Optional[Union[str, Sequence[str]]] = None,
        args: Optional[dict] = None,
        is_global: bool = False,
    ) -> None:
        self.options: List[Variable] = []
        self.args = args if args is not None else {}
        if not SCons.Util.is_Sequence(files):
            files = [files] if files else []
        self.files: Sequence[str] = files
        self.unknown: Dict[str, str] = {}

    def __str__(self) -> str:
        """Provide a way to "print" a Variables object."""
        s = "Variables(\n  options=[\n"
        for option in self.options:
            s += f"    {str(option)},\n"
        s += "  ],\n"
        s += f"  args={self.args},\n  files={self.files},\n  unknown={self.unknown},\n)"
        return s

    # lint: W0622: Redefining built-in 'help'
    def _do_add(
        self,
        key: Union[str, List[str]],
        help: str = "",
        default=None,
        validator: Optional[Callable] = None,
        converter: Optional[Callable] = None,
        **kwargs,
    ) -> None:
        """Create a Variable and add it to the list.

        This is the internal implementation for :meth:`Add` and
        :meth:`AddVariables`. Not part of the public API.

        .. versionadded:: 4.8.0
              *subst* keyword argument is now recognized.
        """
        option = Variable()

        # If we get a list or a tuple, we take the first element as the
        # option key and store the remaining in aliases.
        if SCons.Util.is_Sequence(key):
            option.key = key[0]
            option.aliases = list(key[1:])
        else:
            option.key = key
            # TODO: normalize to not include key in aliases. Currently breaks tests.
            option.aliases = [key,]
        if not option.key.isidentifier():
            raise SCons.Errors.UserError(f"Illegal Variables key {option.key!r}")
        option.help = help
        option.default = default
        option.validator = validator
        option.converter = converter
        option.do_subst = kwargs.pop("subst", True)
        # TODO should any remaining kwargs be saved in the Variable?

        self.options.append(option)

        # options might be added after the 'unknown' dict has been set up,
        # so we remove the key and all its aliases from that dict
        for alias in option.aliases + [option.key,]:
            if alias in self.unknown:
                del self.unknown[alias]

    def keys(self) -> list:
        """Return the variable names."""
        for option in self.options:
            yield option.key

    def Add(
        self, key: Union[str, Sequence], *args, **kwargs,
    ) -> None:
        """Add a Build Variable.

        Arguments:
          key: the name of the variable, or a 5-tuple (or other sequence).
            If *key* is a tuple, and there are no additional arguments
            except the *help*, *default*, *validator* and *converter*
            keyword arguments, *key* is unpacked into the variable name
            plus the *help*, *default*, *validator* and *converter*
            arguments; if there are additional arguments, the first
            elements of *key* is taken as the variable name, and the
            remainder as aliases.
          args: optional positional arguments, corresponding to the
            *help*, *default*, *validator* and *converter* keyword args.
          kwargs: arbitrary keyword arguments used by the variable itself.

        Keyword Args:
          help: help text for the variable (default: empty string)
          default: default value for variable (default: ``None``)
          validator: function called to validate the value (default: ``None``)
          converter: function to be called to convert the variable's
            value before putting it in the environment. (default: ``None``)
          subst: perform substitution on the value before the converter
            and validator functions (if any) are called (default: ``True``)

        .. versionadded:: 4.8.0
              The *subst* keyword argument is now specially recognized.
        """
        if SCons.Util.is_Sequence(key):
            # If no other positional args (and no fundamental kwargs),
            # unpack key, and pass the kwargs on:
            known_kw = {'help', 'default', 'validator', 'converter'}
            if not args and not known_kw.intersection(kwargs.keys()):
                return self._do_add(*key, **kwargs)

        return self._do_add(key, *args, **kwargs)

    def AddVariables(self, *optlist) -> None:
        """Add a list of Build Variables.

        Each list element is a tuple/list of arguments to be passed on
        to the underlying method for adding variables.

        Example::

            opt = Variables()
            opt.AddVariables(
                ('debug', '', 0),
                ('CC', 'The C compiler'),
                ('VALIDATE', 'An option for testing validation', 'notset', validator, None),
            )
        """
        for opt in optlist:
            self._do_add(*opt)

    def Update(self, env, args: Optional[dict] = None) -> None:
        """Update an environment with the Build Variables.

        Args:
            env: the environment to update.
            args: a dictionary of keys and values to update in *env*.
               If omitted, uses the saved :attr:`args`
        """
        # first pull in the defaults
        values = {opt.key: opt.default for opt in self.options if opt.default is not None}

        # next set the values specified in any options script(s)
        for filename in self.files:
            # TODO: issue #816 use Node to access saved-variables file?
            if os.path.exists(filename):
                # lint: W0622: Redefining built-in 'dir'
                dir = os.path.split(os.path.abspath(filename))[0]
                if dir:
                    sys.path.insert(0, dir)
                try:
                    values['__name__'] = filename
                    with open(filename) as f:
                        contents = f.read()
                    exec(contents, {}, values)
                finally:
                    if dir:
                        del sys.path[0]
                    del values['__name__']

        # set the values specified on the command line
        if args is None:
            args = self.args

        for arg, value in args.items():
            added = False
            for option in self.options:
                if arg in option.aliases + [option.key,]:
                    values[option.key] = value
                    added = True
            if not added:
                self.unknown[arg] = value

        # put the variables in the environment:
        # (don't copy over variables that are not declared as options)
        for option in self.options:
            try:
                env[option.key] = values[option.key]
            except KeyError:
                pass

        # apply converters
        for option in self.options:
            if option.converter and option.key in values:
                if option.do_subst:
                    value = env.subst('${%s}' % option.key)
                else:
                    value = env[option.key]
                try:
                    try:
                        env[option.key] = option.converter(value)
                    except TypeError:
                        env[option.key] = option.converter(value, env)
                except ValueError as exc:
                    # We usually want the converter not to fail and leave
                    # that to the validator, but in case, handle it.
                    msg = f'Error converting option: {option.key!r}\n{exc}'
                    raise SCons.Errors.UserError(msg) from exc

        # apply validators
        for option in self.options:
            if option.validator and option.key in values:
                if option.do_subst:
                    val = env[option.key]
                    if not SCons.Util.is_String(val):
                        # issue #4585: a _ListVariable should not be further
                        #    substituted, breaks on values with spaces.
                        value = val
                    else:
                        value = env.subst('${%s}' % option.key)
                else:
                    value = env[option.key]
                option.validator(option.key, value, env)

    def UnknownVariables(self) -> dict:
        """Return dict of unknown variables.

        Identifies variables that were not recognized in this object.
        """
        return self.unknown

    def Save(self, filename, env) -> None:
        """Save the variables to a script.

        Saves all the variables which have non-default settings
        to the given file as Python expressions.  This script can
        then be used to load the variables for a subsequent run.
        This can be used to create a build variable "cache" or
        capture different configurations for selection.

        Args:
            filename: Name of the file to save into
            env: the environment to get the option values from
        """
        # Create the file and write out the header
        try:
            # TODO: issue #816 use Node to access saved-variables file?
            with open(filename, 'w') as fh:
                # Make an assignment in the file for each option
                # within the environment that was assigned a value
                # other than the default. We don't want to save the
                # ones set to default: in case the SConscript settings
                # change you would then pick up old defaults.
                for option in self.options:
                    try:
                        value = env[option.key]
                        try:
                            prepare = value.prepare_to_store
                        except AttributeError:
                            try:
                                eval(repr(value))
                            except KeyboardInterrupt:
                                raise
                            except Exception:
                                # Convert stuff that has a repr() that
                                # cannot be evaluated into a string
                                value = SCons.Util.to_String(value)
                        else:
                            value = prepare()

                        defaultVal = env.subst(SCons.Util.to_String(option.default))
                        if option.converter:
                            try:
                                defaultVal = option.converter(defaultVal)
                            except TypeError:
                                defaultVal = option.converter(defaultVal, env)

                        if str(env.subst(f'${option.key}')) != str(defaultVal):
                            fh.write(f'{option.key} = {value!r}\n')
                    except KeyError:
                        pass
        except OSError as exc:
            msg = f'Error writing options to file: {filename}\n{exc}'
            raise SCons.Errors.UserError(msg) from exc

    def GenerateHelpText(self, env, sort: Union[bool, Callable] = False) -> str:
        """Generate the help text for the Variables object.

        Args:
            env: an environment that is used to get the current values
                of the variables.
            sort: Either a comparison function used for sorting
                (must take two arguments and return ``-1``, ``0`` or ``1``)
                or a boolean to indicate if it should be sorted.
        """
        # TODO the 'sort' argument matched the old way Python's sorted()
        #   worked, taking a comparison function argument. That has been
        #   removed so now we have to convert to a key.
        if callable(sort):
            options = sorted(self.options, key=cmp_to_key(lambda x, y: sort(x.key, y.key)))
        elif sort is True:
            options = sorted(self.options)
        else:
            options = self.options

        def format_opt(opt, self=self, env=env) -> str:
            if opt.key in env:
                actual = env.subst(f'${opt.key}')
            else:
                actual = None
            return self.FormatVariableHelpText(
                env, opt.key, opt.help, opt.default, actual, opt.aliases
            )
        return ''.join(_f for _f in map(format_opt, options) if _f)

    fmt = '\n%s: %s\n    default: %s\n    actual: %s\n'
    aliasfmt = '\n%s: %s\n    default: %s\n    actual: %s\n    aliases: %s\n'

    # lint: W0622: Redefining built-in 'help'
    def FormatVariableHelpText(
        self,
        env,
        key: str,
        help: str,
        default,
        actual,
        aliases: Optional[List[str]] = None,
    ) -> str:
        """Format the help text for a single variable.

        The caller is responsible for obtaining all the values,
        although now the :class:`Variable` class is more publicly exposed,
        this method could easily do most of that work - however
        that would change the existing published API.
        """
        if aliases is None:
            aliases = []
        # Don't display the key name itself as an alias.
        aliases = [a for a in aliases if a != key]
        if aliases:
            return self.aliasfmt % (key, help, default, actual, aliases)
        return self.fmt % (key, help, default, actual)

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
