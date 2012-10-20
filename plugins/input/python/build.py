#!/usr/bin/env python

import os

PLUGIN_NAME = 'python'

Import ('plugin_base')
Import ('env')

install_dest = env['MAPNIK_INPUT_PLUGINS_DEST']

plugin_env = plugin_base.Clone()

plugin_sources = Split(
  """
  %(PLUGIN_NAME)s_datasource.cpp
  %(PLUGIN_NAME)s_featureset.cpp
  %(PLUGIN_NAME)s_utils.cpp
  """ % locals()
        )

boost_system = 'boost_system%s' % env['BOOST_APPEND']
libraries = ['mapnik',env['BOOST_PYTHON_LIB'],boost_system,env['ICU_LIB_NAME']]

# NOTE: explicit linking to libpython is uneeded on most linux version if the
# python plugin is used by a app in python using mapnik's python bindings
# we explicitly link to libpython here so that this plugin
# can be used from a pure C++ calling application or a different binding language
python_link_flag = '-lpython%s' % env['PYTHON_VERSION']

if env['PLATFORM'] == 'Darwin':
    if env['PYTHON_DYNAMIC_LOOKUP']:
        python_link_flag = '-undefined dynamic_lookup'
    elif env['FRAMEWORK_PYTHON']:
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

if env['CUSTOM_LDFLAGS']:
    linkflags = '%s %s' % (env['CUSTOM_LDFLAGS'], python_link_flag)
else:
    linkflags = python_link_flag

plugin_env.Append(CPPPATH = env['PYTHON_INCLUDES'])
    
TARGET = plugin_env.SharedLibrary(
              # the name of the target to build, eg 'sqlite.input'
              '../%s' % PLUGIN_NAME,
              # prefix - normally none used
              SHLIBPREFIX='',
              # extension, mapnik expects '.input'
              SHLIBSUFFIX='.input',
              # list of source files to compile
              source=plugin_sources,
              # libraries to link to
              LIBS=libraries,
              # any custom linkflags, eg. LDFLAGS
              # in this case CUSTOM_LDFLAGS comes
              # from Mapnik's main SConstruct file
              # and can be removed here if you do
              # not need it
              LINKFLAGS=linkflags
              )

# if 'uninstall' is not passed on the command line
# then we actually create the install targets that
# scons will install if 'install' is passed as an arg
if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(install_dest, TARGET)
    env.Alias('install', install_dest)
