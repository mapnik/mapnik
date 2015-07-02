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

PLUGIN_NAME = 'occi'

plugin_env = plugin_base.Clone()

plugin_sources = Split(
  """
  %(PLUGIN_NAME)s_types.cpp
  %(PLUGIN_NAME)s_datasource.cpp
  %(PLUGIN_NAME)s_featureset.cpp
  spatial_classesm.cpp
  spatial_classeso.cpp
  """ % locals()
)

libraries = [ 'clntsh', 'occi' ]
libraries.append('boost_system%s' % env['BOOST_APPEND'])
libraries.append(env['ICU_LIB_NAME'])

if env['PLUGIN_LINKING'] == 'shared':
    libraries.append(env['MAPNIK_NAME'])

    # libocci.dylib, at least for 11.2 links to libstdc++
    # so we defer symbol resolution to runtime in order to
    # dodge linking errors like
    # Undefined symbols for architecture x86_64:
    #   "std::string::_Rep::_M_destroy(std::allocator<char> const&)", referenced from:
    #      RegisterClasses(oracle::occi::Environment*) in spatial_classesm.os

    if env['PLATFORM'] == 'Darwin':
        plugin_env.Append(LINKFLAGS='-undefined dynamic_lookup')

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
  'LIBS': libraries,
  'SOURCES': plugin_sources,
}

Return('plugin_obj')
