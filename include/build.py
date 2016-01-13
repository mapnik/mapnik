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

base = './mapnik/'
subdirs = [
    '',
    'csv',
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

# Mark all `*_grammar.hpp` and `*_grammar_impl.hpp` headers as memory hogs.
# Sources that #include (directly or indirectly) any of those are restricted
# to at most --hogs=M parallel compilation tasks. They still count towards
# the total limit given by --jobs=N, but don't prevent parallel compilation
# of regular sources. There may M memory-intensive plus N-M memory-savvy
# sources being compiled in parallel.
env.MemoryHog(Split(
    """
    mapnik/css_color_grammar.hpp
    mapnik/css_color_grammar_impl.hpp
    mapnik/csv/csv_grammar.hpp
    mapnik/expression_grammar.hpp
    mapnik/expression_grammar_impl.hpp
    mapnik/image_filter_grammar.hpp
    mapnik/image_filter_grammar_impl.hpp
    mapnik/json/extract_bounding_box_grammar.hpp
    mapnik/json/extract_bounding_box_grammar_impl.hpp
    mapnik/json/feature_collection_grammar.hpp
    mapnik/json/feature_collection_grammar_impl.hpp
    mapnik/json/feature_generator_grammar.hpp
    mapnik/json/feature_generator_grammar_impl.hpp
    mapnik/json/feature_grammar.hpp
    mapnik/json/feature_grammar_impl.hpp
    mapnik/json/geometry_generator_grammar.hpp
    mapnik/json/geometry_generator_grammar_impl.hpp
    mapnik/json/geometry_grammar.hpp
    mapnik/json/geometry_grammar_impl.hpp
    mapnik/json/positions_grammar.hpp
    mapnik/json/positions_grammar_impl.hpp
    mapnik/json/properties_generator_grammar.hpp
    mapnik/json/properties_generator_grammar_impl.hpp
    mapnik/json/symbolizer_grammar.hpp
    mapnik/json/topojson_grammar.hpp
    mapnik/json/topojson_grammar_impl.hpp
    mapnik/path_expression_grammar.hpp
    mapnik/path_expression_grammar_impl.hpp
    mapnik/svg/output/svg_output_grammars.hpp
    mapnik/svg/output/svg_output_grammars_impl.hpp
    mapnik/svg/svg_path_grammar.hpp
    mapnik/svg/svg_points_grammar.hpp
    mapnik/svg/svg_transform_grammar.hpp
    mapnik/transform_expression_grammar.hpp
    mapnik/transform_expression_grammar_impl.hpp
    mapnik/wkt/wkt_generator_grammar.hpp
    mapnik/wkt/wkt_generator_grammar_impl.hpp
    mapnik/wkt/wkt_grammar.hpp
    mapnik/wkt/wkt_grammar_impl.hpp
    """
    ))
