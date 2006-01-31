"""optik

A powerful, extensible, and easy-to-use command-line parser for Python.

By Greg Ward <gward@python.net>

See http://optik.sourceforge.net/
"""

# Copyright (c) 2001 Gregory P. Ward.  All rights reserved.
# See the README.txt distributed with Optik for licensing terms.

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Optik/__init__.py 0.96.1.D001 2004/08/23 09:55:29 knight"

# Original Optik revision this is based on:
__Optik_revision__ = "__init__.py,v 1.11 2002/04/11 19:17:34 gward Exp"

__version__ = "1.3"


# Re-import these for convenience
from SCons.Optik.option import Option
from SCons.Optik.option_parser import \
     OptionParser, SUPPRESS_HELP, SUPPRESS_USAGE
from SCons.Optik.errors import OptionValueError


# Some day, there might be many Option classes.  As of Optik 1.3, the
# preferred way to instantiate Options is indirectly, via make_option(),
# which will become a factory function when there are many Option
# classes.
make_option = Option
