"""SCons.Platform.win32

Platform-specific initialization for Win32 systems.

There normally shouldn't be any need to import this module directly.  It
will usually be imported through the generic SCons.Platform.Platform()
selection method.
"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 The SCons Foundation
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

__revision__ = "src/engine/SCons/Platform/win32.py issue-2856:2676:d23b7a2f45e8 2012/08/05 15:38:28 garyo"

import os
import os.path
import sys
import tempfile

from SCons.Platform.posix import exitvalmap
from SCons.Platform import TempFileMunge
import SCons.Util

try:
    import msvcrt
    import win32api
    import win32con

    msvcrt.get_osfhandle
    win32api.SetHandleInformation
    win32con.HANDLE_FLAG_INHERIT
except ImportError:
    parallel_msg = \
        "you do not seem to have the pywin32 extensions installed;\n" + \
        "\tparallel (-j) builds may not work reliably with open Python files."
except AttributeError:
    parallel_msg = \
        "your pywin32 extensions do not support file handle operations;\n" + \
        "\tparallel (-j) builds may not work reliably with open Python files."
else:
    parallel_msg = None

    import builtins

    _builtin_file = builtins.file
    _builtin_open = builtins.open

    class _scons_file(_builtin_file):
        def __init__(self, *args, **kw):
            _builtin_file.__init__(self, *args, **kw)
            win32api.SetHandleInformation(msvcrt.get_osfhandle(self.fileno()),
                win32con.HANDLE_FLAG_INHERIT, 0)

    def _scons_open(*args, **kw):
        fp = _builtin_open(*args, **kw)
        win32api.SetHandleInformation(msvcrt.get_osfhandle(fp.fileno()),
                                      win32con.HANDLE_FLAG_INHERIT,
                                      0)
        return fp

    builtins.file = _scons_file
    builtins.open = _scons_open



# The upshot of all this is that, if you are using Python 1.5.2,
# you had better have cmd or command.com in your PATH when you run
# scons.

def piped_spawn(sh, escape, cmd, args, env, stdout, stderr):
    # There is no direct way to do that in python. What we do
    # here should work for most cases:
    #   In case stdout (stderr) is not redirected to a file,
    #   we redirect it into a temporary file tmpFileStdout
    #   (tmpFileStderr) and copy the contents of this file
    #   to stdout (stderr) given in the argument
    if not sh:
        sys.stderr.write("scons: Could not find command interpreter, is it in your PATH?\n")
        return 127
    else:
        # one temporary file for stdout and stderr
        tmpFileStdout = os.path.normpath(tempfile.mktemp())
        tmpFileStderr = os.path.normpath(tempfile.mktemp())

        # check if output is redirected
        stdoutRedirected = 0
        stderrRedirected = 0
        for arg in args:
            # are there more possibilities to redirect stdout ?
            if (arg.find( ">", 0, 1 ) != -1 or
                arg.find( "1>", 0, 2 ) != -1):
                stdoutRedirected = 1
            # are there more possibilities to redirect stderr ?
            if arg.find( "2>", 0, 2 ) != -1:
                stderrRedirected = 1

        # redirect output of non-redirected streams to our tempfiles
        if stdoutRedirected == 0:
            args.append(">" + str(tmpFileStdout))
        if stderrRedirected == 0:
            args.append("2>" + str(tmpFileStderr))

        # actually do the spawn
        try:
            args = [sh, '-c', escape(' '.join(args)) ]
            ret = os.spawnve(os.P_WAIT, sh, args, env)
        except OSError, e:
            # catch any error
            try:
                ret = exitvalmap[e[0]]
            except KeyError:
                sys.stderr.write("scons: unknown OSError exception code %d - %s: %s\n" % (e[0], cmd, e[1]))
            if stderr is not None:
                stderr.write("scons: %s: %s\n" % (cmd, e[1]))
        # copy child output from tempfiles to our streams
        # and do clean up stuff
        if stdout is not None and stdoutRedirected == 0:
            try:
                stdout.write(open( tmpFileStdout, "r" ).read())
                os.remove( tmpFileStdout )
            except (IOError, OSError):
                pass

        if stderr is not None and stderrRedirected == 0:
            try:
                stderr.write(open( tmpFileStderr, "r" ).read())
                os.remove( tmpFileStderr )
            except (IOError, OSError):
                pass
        return ret

def exec_spawn(l, env):
    try:
        result = os.spawnve(os.P_WAIT, l[0], l, env)
    except OSError, e:
        try:
            result = exitvalmap[e[0]]
            sys.stderr.write("scons: %s: %s\n" % (l[0], e[1]))
        except KeyError:
            result = 127
            if len(l) > 2:
                if len(l[2]) < 1000:
                    command = ' '.join(l[0:3])
                else:
                    command = l[0]
            else:
                command = l[0]
            sys.stderr.write("scons: unknown OSError exception code %d - '%s': %s\n" % (e[0], command, e[1]))
    return result

def spawn(sh, escape, cmd, args, env):
    if not sh:
        sys.stderr.write("scons: Could not find command interpreter, is it in your PATH?\n")
        return 127
    return exec_spawn([sh, '-c', escape(' '.join(args))], env)

def escape(arg):
    "escape shell special characters"
    slash = '\\'
    special = '"$()'

    arg = arg.replace(slash, slash+slash)
    for c in special:
        arg = arg.replace(c, slash+c)

    return '"' + arg + '"'

# Determine which windows CPU were running on.
class ArchDefinition(object):
    """
    A class for defining architecture-specific settings and logic.
    """
    def __init__(self, arch, synonyms=[]):
        self.arch = arch
        self.synonyms = synonyms

SupportedArchitectureList = [
    ArchDefinition(
        'x86',
        ['i386', 'i486', 'i586', 'i686'],
    ),

    ArchDefinition(
        'x86_64',
        ['AMD64', 'amd64', 'em64t', 'EM64T', 'x86_64'],
    ),

    ArchDefinition(
        'ia64',
        ['IA64'],
    ),
]

SupportedArchitectureMap = {}
for a in SupportedArchitectureList:
    SupportedArchitectureMap[a.arch] = a
    for s in a.synonyms:
        SupportedArchitectureMap[s] = a

def get_architecture(arch=None):
    """Returns the definition for the specified architecture string.

    If no string is specified, the system default is returned (as defined
    by the PROCESSOR_ARCHITEW6432 or PROCESSOR_ARCHITECTURE environment
    variables).
    """
    if arch is None:
        arch = os.environ.get('PROCESSOR_ARCHITEW6432')
        if not arch:
            arch = os.environ.get('PROCESSOR_ARCHITECTURE')
    return SupportedArchitectureMap.get(arch, ArchDefinition('', ['']))

def generate(env):
    cmd_interp = os.path.realpath('/bin/bash.exe')

    
    if 'ENV' not in env:
        env['ENV'] = {}

    # Import things from the external environment to the construction
    # environment's ENV.  This is a potential slippery slope, because we
    # *don't* want to make builds dependent on the user's environment by
    # default.  We're doing this for SystemRoot, though, because it's
    # needed for anything that uses sockets, and seldom changes, and
    # for SystemDrive because it's related.
    #
    # Weigh the impact carefully before adding other variables to this list.
    import_env = [ 'SystemDrive', 'SystemRoot', 'TEMP', 'TMP' ]
    for var in import_env:
        v = os.environ.get(var)
        if v:
            env['ENV'][var] = v

    if 'COMSPEC' not in env['ENV']:
        v = os.environ.get("COMSPEC")
        if v:
            env['ENV']['COMSPEC'] = v

    env.AppendENVPath('PATH', '/bin')
    env.AppendENVPath('PATH', '/mingw/bin')

    env['ENV']['PATHEXT'] = '.COM;.EXE;.BAT;.CMD'
    env['OBJPREFIX']      = ''
    env['OBJSUFFIX']      = '.obj'
    env['SHOBJPREFIX']    = '$OBJPREFIX'
    env['SHOBJSUFFIX']    = '$OBJSUFFIX'
    env['PROGPREFIX']     = ''
    env['PROGSUFFIX']     = '.exe'
    env['LIBPREFIX']      = ''
    env['LIBSUFFIX']      = '.lib'
    env['SHLIBPREFIX']    = ''
    env['SHLIBSUFFIX']    = '.dll'
    env['LIBPREFIXES']    = [ '$LIBPREFIX' ]
    env['LIBSUFFIXES']    = [ '$LIBSUFFIX' ]
    env['PSPAWN']         = piped_spawn
    env['SPAWN']          = spawn
    env['SHELL']          = cmd_interp
    env['TEMPFILE']       = TempFileMunge
    env['TEMPFILEPREFIX'] = '@'
    env['MAXLINELENGTH']  = 2048
    env['ESCAPE']         = escape
    
    env['HOST_OS']        = 'win32'
    env['HOST_ARCH']      = get_architecture().arch
    

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
