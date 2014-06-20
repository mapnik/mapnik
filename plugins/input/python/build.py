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
import copy
Import ('plugin_base')
Import ('env')

PLUGIN_NAME = 'python'

plugin_env = plugin_base.Clone()

plugin_sources = Split(
  """
  %(PLUGIN_NAME)s_datasource.cpp
  %(PLUGIN_NAME)s_featureset.cpp
  %(PLUGIN_NAME)s_utils.cpp
  """ % locals()
)

# Link Library to Dependencies
libraries = []
libraries.append('boost_system%s' % env['BOOST_APPEND'])
libraries.append(env['BOOST_PYTHON_LIB'])
libraries.append(env['ICU_LIB_NAME'])

python_cpppath = env['PYTHON_INCLUDES']
allcpp_paths = copy.copy(env['CPPPATH'])
allcpp_paths.extend(python_cpppath)
# NOTE: explicit linking to libpython is uneeded on most linux version if the
# python plugin is used by a app in python using mapnik's python bindings
# we explicitly link to libpython here so that this plugin
# can be used from a pure C++ calling application or a different binding language
if env['PLATFORM'] == 'Darwin' and env['FRAMEWORK_PYTHON']:
    if env['FRAMEWORK_SEARCH_PATH']:
        python_link_flag = '-F%s -framework Python -Z' % env['FRAMEWORK_SEARCH_PATH']
    else:
        link_prefix = env['PYTHON_SYS_PREFIX']
        if '.framework' in link_prefix:
            python_link_flag = '-F%s -framework Python -Z' % os.path.dirname(link_prefix.split('.')[0])
        elif '/System' in link_prefix:
            python_link_flag = '-F/System/Library/Frameworks/ -framework Python -Z'
        else:
            python_link_flag = '-F/ -framework Python'
else:
    # on linux the linkflags end up to early in the compile flags to work correctly
    python_link_flag = '-L%s' % env['PYTHON_SYS_PREFIX'] + os.path.sep + env['LIBDIR_SCHEMA']
    # so instead add to libraries
    libraries.append('python%s' % env['PYTHON_VERSION'])

plugin_env.Append(LINKFLAGS=python_link_flag)

if env['PLUGIN_LINKING'] == 'shared':
    libraries.append(env['MAPNIK_NAME'])
    TARGET = plugin_env.SharedLibrary('../%s' % PLUGIN_NAME,
                                      SHLIBPREFIX='',
                                      SHLIBSUFFIX='.input',
                                      source=plugin_sources,
                                      CPPPATH=allcpp_paths,
                                      LIBS=libraries)

    # if the plugin links to libmapnik ensure it is built first
    Depends(TARGET, env.subst('../../../src/%s' % env['MAPNIK_LIB_NAME']))

    # if 'uninstall' is not passed on the command line
    # then we actually create the install targets that
    # scons will install if 'install' is passed as an arg
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        env.Install(env['MAPNIK_INPUT_PLUGINS_DEST'], TARGET)
        env.Alias('install', env['MAPNIK_INPUT_PLUGINS_DEST'])

plugin_obj = {
  'LIBS': libraries,
  'SOURCES': plugin_sources,
  'CPPPATH': python_cpppath,
  'LINKFLAGS': python_link_flag.replace('-Z','').split(' '),
}

Return('plugin_obj')
