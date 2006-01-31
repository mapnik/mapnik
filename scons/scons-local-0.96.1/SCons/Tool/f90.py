"""engine.SCons.Tool.f90

Tool-specific initialization for the generic Posix f90 Fortran compiler.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/f90.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import SCons.Defaults
import SCons.Scanner.Fortran
import SCons.Tool
import SCons.Util
import fortran

compilers = ['f90']

#
F90Suffixes = ['.f90']
F90PPSuffixes = []
if SCons.Util.case_sensitive_suffixes('.f90', '.F90'):
    F90PPSuffixes.append('.F90')
else:
    F90Suffixes.append('.F90')

#
F90Scan = SCons.Scanner.Fortran.FortranScan("F90PATH")

for suffix in F90Suffixes + F90PPSuffixes:
    SCons.Defaults.ObjSourceScan.add_scanner(suffix, F90Scan)

#
fVLG = fortran.VariableListGenerator

F90Generator = fVLG('F90', 'FORTRAN', '_FORTRAND')
F90FlagsGenerator = fVLG('F90FLAGS', 'FORTRANFLAGS')
F90CommandGenerator = fVLG('F90COM', 'FORTRANCOM', '_F90COMD')
F90PPCommandGenerator = fVLG('F90PPCOM', 'FORTRANPPCOM', '_F90PPCOMD')
ShF90Generator = fVLG('SHF90', 'SHFORTRAN', 'F90', 'FORTRAN', '_FORTRAND')
ShF90FlagsGenerator = fVLG('SHF90FLAGS', 'SHFORTRANFLAGS')
ShF90CommandGenerator = fVLG('SHF90COM', 'SHFORTRANCOM', '_SHF90COMD')
ShF90PPCommandGenerator = fVLG('SHF90PPCOM', 'SHFORTRANPPCOM', '_SHF90PPCOMD')

del fVLG

#
F90Action = SCons.Action.Action('$_F90COMG ')
F90PPAction = SCons.Action.Action('$_F90PPCOMG ')
ShF90Action = SCons.Action.Action('$_SHF90COMG ')
ShF90PPAction = SCons.Action.Action('$_SHF90PPCOMG ')

def add_to_env(env):
    """Add Builders and construction variables for f90 to an Environment."""
    env.AppendUnique(FORTRANSUFFIXES = F90Suffixes + F90PPSuffixes)

    static_obj, shared_obj = SCons.Tool.createObjBuilders(env)

    for suffix in F90Suffixes:
        static_obj.add_action(suffix, F90Action)
        shared_obj.add_action(suffix, ShF90Action)
        static_obj.add_emitter(suffix, fortran.FortranEmitter)
        shared_obj.add_emitter(suffix, fortran.ShFortranEmitter)

    for suffix in F90PPSuffixes:
        static_obj.add_action(suffix, F90PPAction)
        shared_obj.add_action(suffix, ShF90PPAction)
        static_obj.add_emitter(suffix, fortran.FortranEmitter)
        shared_obj.add_emitter(suffix, fortran.ShFortranEmitter)
  
    env['_F90G']        = F90Generator
    env['_F90FLAGSG']   = F90FlagsGenerator
    env['_F90COMG']     = F90CommandGenerator
    env['_F90PPCOMG']   = F90PPCommandGenerator

    env['_SHF90G']      = ShF90Generator
    env['_SHF90FLAGSG'] = ShF90FlagsGenerator
    env['_SHF90COMG']   = ShF90CommandGenerator
    env['_SHF90PPCOMG'] = ShF90PPCommandGenerator

    env['_F90INCFLAGS'] = '$( ${_concat(INCPREFIX, F90PATH, INCSUFFIX, __env__, RDirs)} $)'
    env['_F90COMD']     = '$_F90G $_F90FLAGSG $_F90INCFLAGS $_FORTRANMODFLAG -c -o $TARGET $SOURCES'
    env['_F90PPCOMD']   = '$_F90G $_F90FLAGSG $CPPFLAGS $_CPPDEFFLAGS $_F90INCFLAGS $_FORTRANMODFLAG -c -o $TARGET $SOURCES'
    env['_SHF90COMD']   = '$_SHF90G $_SHF90FLAGSG $_F90INCFLAGS $_FORTRANMODFLAG -c -o $TARGET $SOURCES'
    env['_SHF90PPCOMD'] = '$_SHF90G $_SHF90FLAGSG $CPPFLAGS $_CPPDEFFLAGS $_F90INCFLAGS $_FORTRANMODFLAG -c -o $TARGET $SOURCES'

def generate(env):
    fortran.add_to_env(env)
    add_to_env(env)

    env['_FORTRAND']        = env.Detect(compilers) or 'f90'

def exists(env):
    return env.Detect(compilers)
