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
lib_env = env.Clone()
lib_env.Append(CXXFLAGS='-fPIC')

name = "mapnik-json"

source = Split(
    """
    unicode_string_grammar_x3.cpp
    positions_grammar_x3.cpp
    generic_json_grammar_x3.cpp
    feature_grammar_x3.cpp
    geojson_grammar_x3.cpp
    topojson_grammar_x3.cpp
    mapnik_json_generator_grammar.cpp
    parse_feature.cpp
    feature_from_geojson.cpp
    geometry_from_geojson.cpp
    mapnik_feature_to_geojson.cpp
    mapnik_geometry_to_geojson.cpp
    extract_bounding_boxes_x3.cpp
    """
    )

lib = lib_env.StaticLibrary(name, source, LIBS=[])


target = os.path.join(env['MAPNIK_LIB_BASE_DEST'], env.subst('${LIBPREFIX}%s${LIBSUFFIX}' % name))
result = env.InstallAs(target=target, source=lib)
env.Alias(target='install', source=result)
env['create_uninstall_target'](env, target)
