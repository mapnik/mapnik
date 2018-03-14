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
program_env['LINKFLAGS'] = '-l%s plugins/input/csv/csv_utils.os'  % plugin_base['MAPNIK_NAME']  + program_env['LINKFLAGS']
source = Split(
    """
    mapnik-index.cpp
    process_csv_file.cpp
    process_geojson_file_x3.cpp
    """
    )

headers = env['CPPPATH']

boost_program_options = 'boost_program_options%s' % env['BOOST_APPEND']
boost_system = 'boost_system%s' % env['BOOST_APPEND']
libraries =  [boost_program_options, boost_system]
# need on linux: https://github.com/mapnik/mapnik/issues/3145
libraries.append('mapnik-json')
libraries.append('mapnik-wkt')
libraries.append(env['ICU_LIB_NAME'])

if env['RUNTIME_LINK'] == 'static':
    libraries.extend(copy(env['LIBMAPNIK_LIBS']))
    if env['PLATFORM'] == 'Linux':
        libraries.append('dl')

mapnik_index = program_env.Program('mapnik-index', source, CPPPATH=headers, LIBS=libraries)

Depends(mapnik_index, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(os.path.join(env['INSTALL_PREFIX'],'bin'), mapnik_index)
    env.Alias('install', os.path.join(env['INSTALL_PREFIX'],'bin'))

env['create_uninstall_target'](env, os.path.join(env['INSTALL_PREFIX'],'bin','mapnik-index'))
