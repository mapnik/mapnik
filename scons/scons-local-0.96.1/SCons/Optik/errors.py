"""optik.errors

Exception classes used by Optik.
"""

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Optik/errors.py 0.96.1.D001 2004/08/23 09:55:29 knight"

# Original Optik revision this is based on:
__Optik_revision__ = "errors.py,v 1.5 2002/02/13 23:29:47 gward Exp"

# Copyright (c) 2001 Gregory P. Ward.  All rights reserved.
# See the README.txt distributed with Optik for licensing terms.

# created 2001/10/17 GPW (from optik.py)


class OptikError (Exception):
    def __init__ (self, msg):
        self.msg = msg

    def __str__ (self):
        return self.msg


class OptionError (OptikError):
    """
    Raised if an Option instance is created with invalid or
    inconsistent arguments.
    """

    def __init__ (self, msg, option):
        self.msg = msg
        self.option_id = str(option)

    def __str__ (self):
        if self.option_id:
            return "option %s: %s" % (self.option_id, self.msg)
        else:
            return self.msg

class OptionConflictError (OptionError):
    """
    Raised if conflicting options are added to an OptionParser.
    """

class OptionValueError (OptikError):
    """
    Raised if an invalid option value is encountered on the command
    line.
    """

class BadOptionError (OptikError):
    """
    Raised if an invalid or ambiguous option is seen on the command-line.
    """
