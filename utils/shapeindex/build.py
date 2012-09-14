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
    shapeindex.cpp
    """
    )

source += program_env.Object('box2d-static', '#src/box2d.cpp')

headers = ['#plugins/input/shape'] + env['CPPPATH'] 

boost_program_options = 'boost_program_options%s' % env['BOOST_APPEND']
boost_filesystem = 'boost_filesystem%s' % env['BOOST_APPEND']
boost_system = 'boost_system%s' % env['BOOST_APPEND']
libraries =  [boost_program_options, boost_filesystem, boost_system]

if env.get('BOOST_LIB_VERSION_FROM_HEADER'):
    boost_version_from_header = int(env['BOOST_LIB_VERSION_FROM_HEADER'].split('_')[1])
    if boost_version_from_header < 46:
        # avoid ubuntu issue with boost interprocess:
        # https://github.com/mapnik/mapnik/issues/1082
        program_env.Append(CXXFLAGS = '-fpermissive')

shapeindex = program_env.Program('shapeindex', source, CPPPATH=headers, LIBS=libraries, LINKFLAGS=env['CUSTOM_LDFLAGS'])

Depends(shapeindex, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(os.path.join(env['INSTALL_PREFIX'],'bin'), shapeindex)
    env.Alias('install', os.path.join(env['INSTALL_PREFIX'],'bin'))

env['create_uninstall_target'](env, os.path.join(env['INSTALL_PREFIX'],'bin','shapeindex'))