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

Import ('env')

can_build = False

if env.get('BOOST_LIB_VERSION_FROM_HEADER'):
    boost_version_from_header = int(env['BOOST_LIB_VERSION_FROM_HEADER'].split('_')[1])
    if boost_version_from_header >= 56:
        can_build = True

if not can_build:
    print 'WARNING: skipping building the optional topojson datasource plugin which requires boost >= 1.56'
else:
    Import ('plugin_base')

    PLUGIN_NAME = 'topojson'

    plugin_env = plugin_base.Clone()

    plugin_sources = Split(
      """
      %(PLUGIN_NAME)s_datasource.cpp
      %(PLUGIN_NAME)s_featureset.cpp
      """ % locals()
    )

    # Link Library to Dependencies
    libraries = []
    libraries.append(env['ICU_LIB_NAME'])
    libraries.append('boost_system%s' % env['BOOST_APPEND'])
    libraries.append('mapnik-json')

    if env['PLUGIN_LINKING'] == 'shared':
        libraries.append(env['MAPNIK_NAME'])

        TARGET = plugin_env.SharedLibrary('../%s' % PLUGIN_NAME,
                                          SHLIBPREFIX='',
                                          SHLIBSUFFIX='.input',
                                          source=plugin_sources,
                                          LIBS=libraries)

        # if the plugin links to libmapnik ensure it is built first
        Depends(TARGET, env.subst('../../../src/%s' % env['MAPNIK_LIB_NAME']))
        Depends(TARGET, env.subst('../../../src/json/libmapnik-json${LIBSUFFIX}'))

        if 'uninstall' not in COMMAND_LINE_TARGETS:
            env.Install(env['MAPNIK_INPUT_PLUGINS_DEST'], TARGET)
            env.Alias('install', env['MAPNIK_INPUT_PLUGINS_DEST'])

    plugin_obj = {
      'LIBS': libraries,
      'SOURCES': plugin_sources,
    }

    Return('plugin_obj')
