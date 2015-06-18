#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2015 Artem Pavlenko
#
# Mapnik is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#

import os
import glob
from copy import copy

Import ('env')

program_env = env.Clone()

source = Split(
    """
    svg2png.cpp
    """
    )

program_env['CXXFLAGS'] = copy(env['LIBMAPNIK_CXXFLAGS'])
program_env.Append(CPPDEFINES = env['LIBMAPNIK_DEFINES'])

if env['HAS_CAIRO']:
    program_env.PrependUnique(CPPPATH=env['CAIRO_CPPPATHS'])
    program_env.Append(CPPDEFINES = '-DHAVE_CAIRO')

boost_program_options = 'boost_program_options%s' % env['BOOST_APPEND']
libraries = [env['MAPNIK_NAME'],boost_program_options]
libraries.extend(copy(env['LIBMAPNIK_LIBS']))
if env['RUNTIME_LINK'] == 'static' and env['PLATFORM'] == 'Linux':
    libraries.append('dl')

svg2png = program_env.Program('svg2png', source, LIBS=libraries)

Depends(svg2png, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(os.path.join(env['INSTALL_PREFIX'],'bin'), svg2png)
    env.Alias('install', os.path.join(env['INSTALL_PREFIX'],'bin'))

env['create_uninstall_target'](env, os.path.join(env['INSTALL_PREFIX'],'bin','svg2png'))
