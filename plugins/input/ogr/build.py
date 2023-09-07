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

Import ('plugin_base')
Import ('env')
from copy import copy

PLUGIN_NAME = 'ogr'

plugin_env = plugin_base.Clone()

plugin_sources = Split(
  """
  %(PLUGIN_NAME)s_converter.cpp
  %(PLUGIN_NAME)s_datasource.cpp
  %(PLUGIN_NAME)s_utils.cpp
  %(PLUGIN_NAME)s_featureset.cpp
  %(PLUGIN_NAME)s_index_featureset.cpp
  """ % locals()
)


cxxflags = []
plugin_env['LIBS'] = []
plugin_env.Append(LIBS=env['PLUGINS']['ogr']['lib'])

if env['RUNTIME_LINK'] == 'static':
    cmd = '%s --dep-libs' % plugin_env['GDAL_CONFIG']
    plugin_env.ParseConfig(cmd)

# Link Library to Dependencies
libraries = copy(plugin_env['LIBS'])

plugin_env.Append(CXXFLAGS=cxxflags)

if env['PLUGIN_LINKING'] == 'shared':
    libraries.insert(0,env['MAPNIK_NAME'])
    libraries.append(env['ICU_LIB_NAME'])

    TARGET = plugin_env.SharedLibrary('../%s' % PLUGIN_NAME,
                                      SHLIBPREFIX='',
                                      SHLIBSUFFIX='.input',
                                      source=plugin_sources,
                                      LIBS=libraries)

    # if the plugin links to libmapnik ensure it is built first
    Depends(TARGET, env.subst('../../../src/%s' % env['MAPNIK_LIB_NAME']))

    if 'uninstall' not in COMMAND_LINE_TARGETS:
        env.Install(env['MAPNIK_INPUT_PLUGINS_DEST'], TARGET)
        env.Alias('install', env['MAPNIK_INPUT_PLUGINS_DEST'])

plugin_obj = {
  'CXXFLAGS': cxxflags,
  'LIBS': libraries,
  'SOURCES': plugin_sources,
}

Return('plugin_obj')
