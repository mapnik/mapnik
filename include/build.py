#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2013 Artem Pavlenko
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
from glob import glob

Import('env')

base = './mapnik/'
subdirs = [
    '',
    'svg',
    'wkt',
    'cairo',
    'grid',
    'json',
    'util',
    'group',
    'text',
    'text/placements',
    'text/formatting',
    'markers_placements'
    ]

if env['SVG_RENDERER']:
    subdirs.append('svg/output')

if env['GRID_RENDERER']:
    subdirs.append('grid')

if 'install' in COMMAND_LINE_TARGETS:
    for subdir in subdirs:
        pathdir = os.path.join(base,subdir,'*.hpp')
        includes = glob(pathdir)
        inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'+subdir)
        env.Alias(target='install', source=env.Install(inc_target, includes))

env['create_uninstall_target'](env, os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'))
