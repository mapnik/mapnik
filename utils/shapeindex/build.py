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

Import ('plugin_base')

program_env = plugin_base.Clone()
program_env['LINKFLAGS'] = '-lmapnik -lmapnik-json' + program_env['LINKFLAGS']

source = Split(
    """
    shapeindex.cpp
    """
    )

headers = ['#plugins/input/shape'] + env['CPPPATH']

boost_program_options = 'boost_program_options%s' % env['BOOST_APPEND']
boost_system = 'boost_system%s' % env['BOOST_APPEND']
libraries =  [env['MAPNIK_NAME'], boost_program_options, boost_system]
libraries.append(env['ICU_LIB_NAME'])
if env['RUNTIME_LINK'] == 'static':
    libraries.extend(copy(env['LIBMAPNIK_LIBS']))
    if env['PLATFORM'] == 'Linux':
        libraries.append('dl')

shapeindex = program_env.Program('shapeindex', source, CPPPATH=headers, LIBS=libraries)

Depends(shapeindex, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(os.path.join(env['INSTALL_PREFIX'],'bin'), shapeindex)
    env.Alias('install', os.path.join(env['INSTALL_PREFIX'],'bin'))

env['create_uninstall_target'](env, os.path.join(env['INSTALL_PREFIX'],'bin','shapeindex'))
