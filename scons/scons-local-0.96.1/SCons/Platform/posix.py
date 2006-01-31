"""SCons.Platform.posix

Platform-specific initialization for POSIX (Linux, UNIX, etc.) systems.

There normally shouldn't be any need to import this module directly.  It
will usually be imported through the generic SCons.Platform.Platform()
selection method.
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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Platform/posix.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import os
import os.path
import popen2
import string
import sys
import select

import SCons.Util

exitvalmap = {
    2 : 127,
    13 : 126,
}

def escape(arg):
    "escape shell special characters"
    slash = '\\'
    special = '"$'

    arg = string.replace(arg, slash, slash+slash)
    for c in special:
        arg = string.replace(arg, c, slash+c)

    return '"' + arg + '"'

def _get_env_command(sh, escape, cmd, args, env):
    if env:
        s = 'env - '
        for key in env.keys():
            s = s + '%s=%s '%(key, escape(env[key]))
        s = s + sh + ' -c '
        s = s + escape(string.join(args))
    else:
        s = string.join(args)
    return s

def env_spawn(sh, escape, cmd, args, env):
    s = _get_env_command( sh, escape, cmd, args, env)
    stat = os.system(s)
    if stat & 0xff:
        return stat | 0x80
    return stat >> 8

def spawn_spawn(sh, escape, cmd, args, env):
    args = [sh, '-c', string.join(args)]
    stat = os.spawnvpe(os.P_WAIT, sh, args, env)
    # os.spawnvpe() returns the actual exit code, not the encoding
    # returned by os.waitpid() or os.system().
    return stat

def fork_spawn(sh, escape, cmd, args, env):
    pid = os.fork()
    if not pid:
        # Child process.
        exitval = 127
        args = [sh, '-c', string.join(args)]
        try:
            os.execvpe(sh, args, env)
        except OSError, e:
            exitval = exitvalmap[e[0]]
            sys.stderr.write("scons: %s: %s\n" % (cmd, e[1]))
        os._exit(exitval)
    else:
        # Parent process.
        pid, stat = os.waitpid(pid, 0)
        if stat & 0xff:
            return stat | 0x80
        return stat >> 8

def process_cmd_output(cmd_stdout, cmd_stderr, stdout, stderr):
    stdout_eof = stderr_eof = 0
    while not (stdout_eof and stderr_eof):
        (i,o,e) = select.select([cmd_stdout, cmd_stderr], [], [])
        if cmd_stdout in i:
            str = cmd_stdout.read()
            if len(str) == 0:
                stdout_eof = 1
            elif stdout != None:
                stdout.write(str)
        if cmd_stderr in i:
            str = cmd_stderr.read()
            if len(str) == 0:
                #sys.__stderr__.write( "stderr_eof=1\n" )
                stderr_eof = 1
            else:
                #sys.__stderr__.write( "str(stderr) = %s\n" % str )
                stderr.write(str)
    

def piped_env_spawn(sh, escape, cmd, args, env, stdout, stderr):
    # spawn using Popen3 combined with the env command
    # the command name and the command's stdout is written to stdout
    # the command's stderr is written to stderr
    s = _get_env_command( sh, escape, cmd, args, env)
    proc = popen2.Popen3(s, 1)
    process_cmd_output(proc.fromchild, proc.childerr, stdout, stderr)
    stat = proc.wait()
    if stat & 0xff:
        return stat | 0x80
    return stat >> 8

def piped_fork_spawn(sh, escape, cmd, args, env, stdout, stderr):
    # spawn using fork / exec and providing a pipe for the command's
    # stdout / stderr stream
    if stdout != stderr:
        (rFdOut, wFdOut) = os.pipe()
        (rFdErr, wFdErr) = os.pipe()
    else:
        (rFdOut, wFdOut) = os.pipe()
        rFdErr = rFdOut
        wFdErr = wFdOut
    # do the fork
    pid = os.fork()
    if not pid:
        # Child process
        os.close( rFdOut )
        if rFdOut != rFdErr:
            os.close( rFdErr )
        os.dup2( wFdOut, 1 ) # is there some symbolic way to do that ?
        os.dup2( wFdErr, 2 )
        os.close( wFdOut )
        if stdout != stderr:
            os.close( wFdErr )
        exitval = 127
        args = [sh, '-c', string.join(args)]
        try:
            os.execvpe(sh, args, env)
        except OSError, e:
            exitval = exitvalmap[e[0]]
            stderr.write("scons: %s: %s\n" % (cmd, e[1]))
        os._exit(exitval)
    else:
        # Parent process
        pid, stat = os.waitpid(pid, 0)
        os.close( wFdOut )
        if stdout != stderr:
            os.close( wFdErr )
        childOut = os.fdopen( rFdOut )
        if stdout != stderr:
            childErr = os.fdopen( rFdErr )
        else:
            childErr = childOut
        process_cmd_output(childOut, childErr, stdout, stderr)
        os.close( rFdOut )
        if stdout != stderr:
            os.close( rFdErr )
        if stat & 0xff:
            return stat | 0x80
        return stat >> 8



def generate(env):
    # If os.spawnvpe() exists, we use it to spawn commands.  Otherwise
    # if the env utility exists, we use os.system() to spawn commands,
    # finally we fall back on os.fork()/os.exec().  
    #
    # os.spawnvpe() is prefered because it is the most efficient.  But
    # for Python versions without it, os.system() is prefered because it
    # is claimed that it works better with threads (i.e. -j) and is more
    # efficient than forking Python.
    #
    # NB: Other people on the scons-users mailing list have claimed that
    # os.fork()/os.exec() works better than os.system().  There may just
    # not be a default that works best for all users.

    if os.__dict__.has_key('spawnvpe'):
        spawn = spawn_spawn
    elif env.Detect('env'):
        spawn = env_spawn
    else:
        spawn = fork_spawn

    if env.Detect('env'):
        pspawn = piped_env_spawn
    else:
        pspawn = piped_fork_spawn

    if not env.has_key('ENV'):
        env['ENV']        = {}
    env['ENV']['PATH']    = '/usr/local/bin:/bin:/usr/bin'
    env['OBJPREFIX']      = ''
    env['OBJSUFFIX']      = '.o'
    env['SHOBJPREFIX']    = '$OBJPREFIX'
    env['SHOBJSUFFIX']    = '$OBJSUFFIX'
    env['PROGPREFIX']     = ''
    env['PROGSUFFIX']     = ''
    env['LIBPREFIX']      = 'lib'
    env['LIBSUFFIX']      = '.a'
    env['SHLIBPREFIX']    = '$LIBPREFIX'
    env['SHLIBSUFFIX']    = '.so'
    env['LIBPREFIXES']    = '$LIBPREFIX'
    env['LIBSUFFIXES']    = [ '$LIBSUFFIX', '$SHLIBSUFFIX' ]
    env['PSPAWN']         = pspawn
    env['SPAWN']          = spawn
    env['SHELL']          = 'sh'
    env['ESCAPE']         = escape

    # This platform supports RPATH specifications.
    env['__RPATH'] = '$_RPATH'
