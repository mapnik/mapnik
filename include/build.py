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
from glob import glob

Import('env')

subdirglobs = [
    ('.',                    'mapnik/*.hpp'),
    ('agg',                  '../deps/agg/include/agg*'),
    ('cairo',                'mapnik/cairo/*.hpp'),
    ('css',                  'mapnik/css/*.hpp'),
    ('csv',                  'mapnik/csv/*.hpp'),
    ('deps/mapbox',          '../deps/mapbox/variant/include/mapbox/*.hpp'),
    ('deps/mapbox',          '../deps/mapbox/geometry/include/mapbox/*.hpp'),
    ('deps/mapbox/geometry', '../deps/mapbox/geometry/include/mapbox/geometry/*.hpp'),
    ('geometry',             'mapnik/geometry/*.hpp'),
    ('group',                'mapnik/group/*.hpp'),
    ('json',                 'mapnik/json/*.hpp'),
    ('markers_placements',   'mapnik/markers_placements/*.hpp'),
    ('renderer_common',      'mapnik/renderer_common/*.hpp'),
    ('sparsehash',           '../deps/mapnik/sparsehash/*'),
    ('sparsehash/internal',  '../deps/mapnik/sparsehash/internal/*'),
    ('svg',                  'mapnik/svg/*.hpp'),
    ('text',                 'mapnik/text/*.hpp'),
    ('text/formatting',      'mapnik/text/formatting/*.hpp'),
    ('text/placements',      'mapnik/text/placements/*.hpp'),
    ('transform',            'mapnik/transform/*.hpp'),
    ('util',                 'mapnik/util/*.hpp'),
    ('value',                'mapnik/value/*.hpp'),
    ('wkt',                  'mapnik/wkt/*.hpp'),
]

if env['SVG_RENDERER']:
    subdirglobs.append(('svg/output', 'mapnik/svg/output/*.hpp'))

if env['GRID_RENDERER']:
    subdirglobs.append(('grid', 'mapnik/grid/*.hpp'))

if 'install' in COMMAND_LINE_TARGETS:
    for subdir, filenamepattern in subdirglobs:
        includes = glob(filenamepattern)
        inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'+subdir)
        env.Alias(target='install', source=env.Install(inc_target, includes))

env['create_uninstall_target'](env, os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'))
