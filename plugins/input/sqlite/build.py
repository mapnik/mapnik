#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2007 Artem Pavlenko, Jean-Francois Doyon
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

prefix = env['PREFIX']

plugin_env = plugin_base.Clone()

sqlite_src = Split(
  """
  sqlite_datasource.cpp
  sqlite_featureset.cpp      
  """
        )

libraries = [ 'sqlite3' ]

# Link Library to Dependencies
libraries.append('mapnik')
libraries.append(env['ICU_LIB_NAME'])
libraries.append('boost_system%s' % env['BOOST_APPEND'])
libraries.append('boost_filesystem%s' % env['BOOST_APPEND'])

linkflags = env['CUSTOM_LDFLAGS']
if env['SQLITE_LINKFLAGS']:
    linkflags.append(env['SQLITE_LINKFLAGS'])

input_plugin = plugin_env.SharedLibrary('../sqlite', source=sqlite_src, SHLIBPREFIX='', SHLIBSUFFIX='.input', LIBS=libraries, LINKFLAGS=linkflags)

# if the plugin links to libmapnik ensure it is built first
Depends(input_plugin, env.subst('../../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(env['MAPNIK_INPUT_PLUGINS_DEST'], input_plugin)
    env.Alias('install', env['MAPNIK_INPUT_PLUGINS_DEST'])
