#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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

Import ('env')

program_env = env.Clone()

source = Split(
    """
    ogrindex.cpp
    """
    )

headers = ['#plugins/input/ogr'] + env['CPPPATH'] 

program_env['LIBS'] = [env['PLUGINS']['ogr']['lib']]

# Link Library to Dependencies
program_env['LIBS'].append('mapnik')
program_env['LIBS'].append(env['ICU_LIB_NAME'])
program_env['LIBS'].append('boost_system%s' % env['BOOST_APPEND'])
program_env['LIBS'].append('boost_filesystem%s' % env['BOOST_APPEND'])
program_env['LIBS'].append('boost_program_options%s' % env['BOOST_APPEND'])

if env['RUNTIME_LINK'] == 'static':
    cmd = 'gdal-config --dep-libs'
    program_env.ParseConfig(cmd)

ogrindex = program_env.Program('ogrindex', source, CPPPATH=headers, LINKFLAGS=env['CUSTOM_LDFLAGS'])

Depends(ogrindex, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(os.path.join(env['INSTALL_PREFIX'],'bin'), ogrindex)
    env.Alias('install', os.path.join(env['INSTALL_PREFIX'],'bin'))
