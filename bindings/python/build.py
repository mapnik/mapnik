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

import os, re, sys, glob
from subprocess import Popen, PIPE


Import('env')

def call(cmd, silent=True):
    stdin, stderr = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        return stdin.strip()
    elif not silent:
        print stderr

def run_2to3(*args,**kwargs):
    call('2to3 -w %s' % os.path.dirname(kwargs['target'][0].path))

def is_py3():
    return 'True' in os.popen('''%s -c "import sys as s;s.stdout.write(str(s.version_info[0] == 3))"''' % env['PYTHON']).read().strip()


prefix = env['PREFIX']
target_path = os.path.normpath(env['PYTHON_INSTALL_LOCATION'] + os.path.sep + 'mapnik')
target_path_deprecated = os.path.normpath(env['PYTHON_INSTALL_LOCATION'] + os.path.sep + 'mapnik2')

py_env = env.Clone()
py_env.Append(CPPPATH = env['PYTHON_INCLUDES'])

py_env.Append(CPPDEFINES = env['LIBMAPNIK_DEFINES'])

py_env['LIBS'] = ['mapnik',env['BOOST_PYTHON_LIB']]

link_all_libs = env['LINKING'] == 'static' or env['RUNTIME_LINK'] == 'static' or (env['PLATFORM'] == 'Darwin' and not env['PYTHON_DYNAMIC_LOOKUP'])

if link_all_libs:
    py_env.AppendUnique(LIBS=env['LIBMAPNIK_LIBS'])

# even though boost_thread is no longer used in mapnik core
# we need to link in for boost_python to avoid missing symbol: _ZN5boost6detail12get_tss_dataEPKv / boost::detail::get_tss_data
py_env.AppendUnique(LIBS = 'boost_thread%s' % env['BOOST_APPEND'])

# note: on linux -lrt must be linked after thread to avoid: undefined symbol: clock_gettime
if env['RUNTIME_LINK'] == 'static' and env['PLATFORM'] == 'Linux':
    py_env.AppendUnique(LIBS='rt')

# TODO - do solaris/fedora need direct linking too?
if env['PLATFORM'] == 'Darwin':
    ##### Python linking on OS X is tricky ### 
    # Confounding problems are:
    # 1) likelyhood of multiple python installs of the same major.minor version
    #  because apple supplies python built-in and many users may have installed 
    #  further versions using macports
    # 2) boost python directly links to a python version
    # 3) the below will directly link _mapnik.so to a python version
    # 4) _mapnik.so must link to the same python lib as boost_python.dylib otherwise
    # python will Abort with a Version Mismatch error.
    # See https://github.com/mapnik/mapnik/issues/453 for the seeds of a better approach
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
            # See: https://github.com/mapnik/mapnik/issues/380
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

paths = '''
"""Configuration paths of Mapnik fonts and input plugins (auto-generated by SCons)."""

from os.path import normpath,join,dirname

mapniklibpath = '%s'
mapniklibpath = normpath(join(dirname(__file__),mapniklibpath))
'''

paths += "inputpluginspath = join(mapniklibpath,'input')\n"

if env['SYSTEM_FONTS']:
    paths += "fontscollectionpath = normpath('%s')\n" % env['SYSTEM_FONTS']
else:
    paths += "fontscollectionpath = join(mapniklibpath,'fonts')\n"

paths += "__all__ = [mapniklibpath,inputpluginspath,fontscollectionpath]\n"

if not os.path.exists('mapnik'):
    os.mkdir('mapnik')

file('mapnik/paths.py','w').write(paths % (env['MAPNIK_LIB_DIR']))

# force open perms temporarily so that `sudo scons install`
# does not later break simple non-install non-sudo rebuild
try:
    os.chmod('mapnik/paths.py',0666)
except: pass

# install the shared object beside the module directory
sources = glob.glob('*.cpp')

if 'install' in COMMAND_LINE_TARGETS:
    # install the core mapnik python files, including '__init__.py'
    init_files = glob.glob('mapnik/*.py')
    if 'mapnik/paths.py' in init_files:
        init_files.remove('mapnik/paths.py') 
    init_module = env.Install(target_path, init_files)
    env.Alias(target='install', source=init_module)
    # install mapnik2 module which redirects to mapnik and issues DeprecatedWarning
    init_mapnik2 = env.Install(target_path_deprecated, 'mapnik2/__init__.py')
    env.Alias(target='install', source=init_mapnik2)
      
    # fix perms and install the custom generated 'paths.py'
    targetp = os.path.join(target_path,'paths.py')
    env.Alias("install", targetp)
    # use env.Command rather than env.Install
    # to enable setting proper perms on `paths.py`
    env.Command( targetp, 'mapnik/paths.py',
        [
        Copy("$TARGET","$SOURCE"),
        Chmod("$TARGET", 0644),
        ])

if 'uninstall' not in COMMAND_LINE_TARGETS:
    if env['HAS_CAIRO']:
        py_env.Append(CPPPATH = env['CAIRO_CPPPATHS'])
        py_env.Append(CPPDEFINES = '-DHAVE_CAIRO')
        if link_all_libs:
            py_env.Append(LIBS=env['CAIRO_ALL_LIBS'])
    
    if env['HAS_PYCAIRO']:
        py_env.ParseConfig('pkg-config --cflags pycairo')
        py_env.Append(CPPDEFINES = '-DHAVE_PYCAIRO')

py_env.Append(LINKFLAGS=python_link_flag)
_mapnik = py_env.LoadableModule('mapnik/_mapnik', sources, LDMODULEPREFIX='', LDMODULESUFFIX='.so')

Depends(_mapnik, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if env['PLATFORM'] == 'SunOS' and env['PYTHON_IS_64BIT']:
    # http://mail.python.org/pipermail/python-dev/2006-August/068528.html
    cxx_module_path = os.path.join(target_path,'64')
else:
    cxx_module_path = target_path

if 'uninstall' not in COMMAND_LINE_TARGETS:
    pymapniklib = env.Install(cxx_module_path,_mapnik)
    py_env.Alias(target='install',source=pymapniklib)
    if 'install' in COMMAND_LINE_TARGETS:
        if is_py3():
            env.AddPostAction(pymapniklib, run_2to3)


env['create_uninstall_target'](env, target_path)
env['create_uninstall_target'](env, target_path_deprecated)
    
