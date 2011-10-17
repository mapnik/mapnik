#!/usr/bin/env python

import os
Import ('env')

PLUGIN_NAME = 'csv'

install_dest = env['MAPNIK_INPUT_PLUGINS_DEST']
plugin_env = env.Clone()

plugin_sources = Split(
  """
  %(PLUGIN_NAME)s_datasource.cpp
  """ % locals()
  )

libraries = []
libraries.append('mapnik2')
libraries.append(env['ICU_LIB_NAME'])
    
TARGET = plugin_env.SharedLibrary(
              '../%s' % PLUGIN_NAME,
              SHLIBPREFIX='',
              SHLIBSUFFIX='.input',
              source=plugin_sources,
              LIBS=libraries,
              LINKFLAGS=env.get('CUSTOM_LDFLAGS')
              )

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(install_dest, TARGET)
    env.Alias('install', install_dest)
