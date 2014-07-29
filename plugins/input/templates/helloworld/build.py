#!/usr/bin/env python

# Mapnik uses the build tool SCons.

# This python file is run to compile a plugin
# It must be called from the main 'SConstruct' file like:

# SConscript('path/to/this/file.py')

# see docs at: http://www.scons.org/wiki/SConscript()

import os

# Here we pull from the SCons environment exported from the main instance
Import ('plugin_base')
Import ('env')

# Give this plugin a name
# here this happens to be the same as the directory
PLUGIN_NAME = 'hello'

# the below install details are also pulled from the
# main SConstruct file where configuration happens

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

libraries.append('boost_system%s' % env['BOOST_APPEND'])
# link libicuuc, but ICU_LIB_NAME is used custom builds of icu can
# have different library names like osx which offers /usr/lib/libicucore.dylib
libraries.append(env['ICU_LIB_NAME'])

# this is valid if we are building an external plugin as shared library
if env['PLUGIN_LINKING'] == 'shared':
    # plugins can go anywhere, and be registered in custom locations by Mapnik
    # but the standard location is '/usr/local/lib/mapnik/input'
    install_dest = env['MAPNIK_INPUT_PLUGINS_DEST']

    # only link mapnik if we are build an external shared object
    libraries.append(env['MAPNIK_NAME'])

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
                  LIBS=libraries
                  )

    # if the plugin links to libmapnik ensure it is built first
    Depends(TARGET, env.subst('../../../../src/%s' % env['MAPNIK_LIB_NAME']))

    # if 'uninstall' is not passed on the command line
    # then we actually create the install targets that
    # scons will install if 'install' is passed as an arg
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        env.Install(install_dest, TARGET)
        env.Alias('install', install_dest)

# Return the plugin building options to scons
# This is used when statically linking the plugin with mapnik)
plugin_obj = {
  'LIBS': libraries,
  'SOURCES': plugin_sources,
}

Return('plugin_obj')
