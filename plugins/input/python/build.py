#!/usr/bin/env python

# Mapnik uses the build tool SCons.

# This python file is run to compile a plugin
# It must be called from the main 'SConstruct' file like:

# SConscript('path/to/this/file.py')

# see docs at: http://www.scons.org/wiki/SConscript()

import os

# Give this plugin a name
# here this happens to be the same as the directory
PLUGIN_NAME = 'python'

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

libraries.append(env['BOOST_PYTHON_LIB'])

# TODO - do solaris/fedora need direct linking too?
if env['PLATFORM'] == 'Darwin':
    if not env['PYTHON_DYNAMIC_LOOKUP']:
        libraries.append('png')
        if env['JPEG']:
            libraries.append('jpeg')
        libraries.append(env['ICU_LIB_NAME'])
        libraries.append('boost_regex%s' % env['BOOST_APPEND'])
        if env['THREADING'] == 'multi':
            libraries.append('boost_thread%s' % env['BOOST_APPEND'])

    ##### Python linking on OS X is tricky ### 
    # Confounding problems are:
    # 1) likelyhood of multiple python installs of the same major.minor version
    #  because apple supplies python built-in and many users may have installed 
    #  further versions using macports
    # 2) boost python directly links to a python version
    # 3) the below will directly link _mapnik.so to a python version
    # 4) _mapnik.so must link to the same python lib as boost_python.dylib otherwise
    # python will Abort with a Version Mismatch error.
    # See http://trac.mapnik.org/ticket/453 for the seeds of a better approach
    # for now we offer control over method of direct linking...
    # The default below is to link against the python dylib in the form of
    #/path/to/Python.framework/Python instead of -lpython
    
    # http://developer.apple.com/mac/library/DOCUMENTATION/Darwin/Reference/ManPages/man1/ld.1.html
    
    if env['PYTHON_DYNAMIC_LOOKUP']:
        python_link_flag = '-undefined dynamic_lookup'
    elif env['FRAMEWORK_PYTHON']:
        if env['FRAMEWORK_SEARCH_PATH']:
            # if the user has supplied a custom root path to search for 
            # a given Python framework, then use that to direct the linker
            python_link_flag = '-F%s -framework Python -Z' % env['FRAMEWORK_SEARCH_PATH']
        else:
            # otherwise be as explicit as possible for linking to the same Framework
            # as the executable we are building with (or is pointed to by the PYTHON variable)
            # otherwise we may accidentally link against either:
            # /System/Library/Frameworks/Python.framework/Python/Versions/
            # or
            # /Library/Frameworks/Python.framework/Python/Versions/
            # See: http://trac.mapnik.org/ticket/380
            link_prefix = env['PYTHON_SYS_PREFIX']
            if '.framework' in link_prefix:
                python_link_flag = '-F%s -framework Python -Z' % os.path.dirname(link_prefix.split('.')[0])
            elif '/System' in link_prefix:
                python_link_flag = '-F/System/Library/Frameworks/ -framework Python -Z'
            else:
                # should we fall back to -lpython here?
                python_link_flag = '-F/ -framework Python'
            
    # if we are not linking to a framework then use the *nix standard approach
    else:
        # TODO - do we need to pass -L/?
        python_link_flag = '-lpython%s' % env['PYTHON_VERSION']

elif env['PLATFORM'] == 'SunOS':
    # make sure to explicitly link mapnik.so against
    # libmapnik in its installed location
    python_link_flag = '-R%s' % env['MAPNIK_LIB_BASE']
else:
    # all other platforms we don't directly link python
    python_link_flag = ''

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
              LINKFLAGS=env.get('CUSTOM_LDFLAGS')
              )

# if 'uninstall' is not passed on the command line
# then we actually create the install targets that
# scons will install if 'install' is passed as an arg
if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(install_dest, TARGET)
    env.Alias('install', install_dest)
