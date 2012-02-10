#!/usr/bin/env python

# Mapnik uses the build tool SCons.

# This python file is run to compile a plugin
# It must be called from the main 'SConstruct' file like:

# SConscript('path/to/this/file.py')

# see docs at: http://www.scons.org/wiki/SConscript()

import os

# Give this plugin a name
# here this happens to be the same as the directory
PLUGIN_NAME = 'hello'

# Here we pull from the SCons environment exported from the main instance
Import ('plugin_base')
Import ('env')

# the below install details are also pulled from the
# main SConstruct file where configuration happens

# plugins can go anywhere, and be registered in custom locations by Mapnik
# but the standard location is '/usr/local/lib/mapnik/input'
install_dest = env['MAPNIK_INPUT_PLUGINS_DEST']

# clone the environment here
# so that if we modify the env it in this file
# those changes to not pollute other builds later on...
plugin_env = plugin_base.Clone()

# Add the cpp files that need to be compiled
plugin_sources = Split(
  """
  %(PLUGIN_NAME)s_datasource.cpp
  %(PLUGIN_NAME)s_featureset.cpp      
  """ % locals()
        )

# Add any external libraries this plugin should
# directly link to
libraries = [ '' ] # eg 'libfoo'

libraries.append('mapnik')
libraries.append('boost_system%s' % env['BOOST_APPEND'])
# link libicuuc, but ICU_LIB_NAME is used custom builds of icu can
# have different library names like osx which offers /usr/lib/libicucore.dylib
libraries.append(env['ICU_LIB_NAME'])
    
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
              LINKFLAGS=env.get('CUSTOM_LDFLAGS')
              )

# if 'uninstall' is not passed on the command line
# then we actually create the install targets that
# scons will install if 'install' is passed as an arg
if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(install_dest, TARGET)
    env.Alias('install', install_dest)
