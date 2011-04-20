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
Import ('env')

# the below install details are also pulled from the
# main SConstruct file where configuration happens

# DESTDIR is default None, PREFIX is default '/usr/local/'
install_prefix = os.path.join(env['DESTDIR'],env['PREFIX'])

# LIBDIR_SCHEMA is likely either 'lib' or 'lib64', LIB_DIR_NAME is default None
lib_name = '%s%s' % (env['LIBDIR_SCHEMA'],env['LIB_DIR_NAME'])

# plugins can go anywhere, and be registered in custom locations by Mapnik
# but the standard location is '/usr/local/lib/mapnik2/input'
install_dest = os.path.join(install_prefix,lib_name,'input')

# clone the environment here
# so that if we modify the env it in this file
# those changes to not pollute other builds later on...
plugin_env = env.Clone()

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

# on mac os x we need to directly link to mapnik and libicu*
if env['PLATFORM'] == 'Darwin':
    libraries.append('mapnik2')
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

if env['PLATFORM'] == 'Darwin':
    # if the plugin links to libmapnik2 ensure it is built first
    # this is optional, but helps to ensure proper compilation
    # if scons is building with multiple threads (e.g. with -jN)
    Depends(TARGET,'../../../../src/libmapnik2.dylib')

# if 'uninstall' is not passed on the command line
# then we actually create the install targets that
# scons will install if 'install' is passed as an arg
if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(install_dest, TARGET)
    env.Alias('install', install_dest)
