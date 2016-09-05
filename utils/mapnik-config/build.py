#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2015 Artem Pavlenko
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

import re
import os
import sys
from copy import copy
from subprocess import Popen, PIPE

Import('env')

config_env = env.Clone()


def GetMapnikLibVersion():
    ver = []
    for line in open('../../include/mapnik/version.hpp').readlines():
        if line.startswith('#define MAPNIK_MAJOR_VERSION'):
            ver.append(line.split(' ')[2].strip())
        if line.startswith('#define MAPNIK_MINOR_VERSION'):
            ver.append(line.split(' ')[2].strip())
        if line.startswith('#define MAPNIK_PATCH_VERSION'):
            ver.append(line.split(' ')[2].strip())
    version_string = ".".join(ver)
    return version_string

if (GetMapnikLibVersion() != config_env['MAPNIK_VERSION_STRING']):
    print 'Error: version.hpp mismatch (%s) to cached value (%s): please reconfigure mapnik' % (GetMapnikLibVersion(),config_env['MAPNIK_VERSION_STRING'])
    Exit(1)

config_variables = '''#!/usr/bin/env bash

## variables

CONFIG_PREFIX="$( cd "$( dirname $( dirname "$0" ))" && pwd )"

CONFIG_MAPNIK_VERSION_STRING='%(version_string)s'
CONFIG_MAPNIK_VERSION='%(version)s'
CONFIG_GIT_REVISION='%(git_revision)s'
CONFIG_GIT_DESCRIBE='%(git_describe)s'
CONFIG_FONTS="%(fonts)s"
CONFIG_INPUT_PLUGINS="%(input_plugins)s"
CONFIG_MAPNIK_DEFINES='%(defines)s'
CONFIG_MAPNIK_LIBNAME='%(mapnik_libname)s'
CONFIG_MAPNIK_LIBPATH="%(mapnik_libpath)s"
CONFIG_DEP_LIBS='%(dep_libs)s'
CONFIG_MAPNIK_LDFLAGS="%(ldflags)s"
CONFIG_MAPNIK_INCLUDE="${CONFIG_PREFIX}/include -I${CONFIG_PREFIX}/include/mapnik/agg -I${CONFIG_PREFIX}/include/mapnik"
CONFIG_DEP_INCLUDES="%(dep_includes)s"
CONFIG_CXXFLAGS="%(cxxflags)s"
CONFIG_CXX='%(cxx)s'
CONFIG_MAPNIK_GDAL_DATA='%(mapnik_bundled_gdal_data)s'
CONFIG_MAPNIK_PROJ_LIB='%(mapnik_bundled_proj_data)s'
CONFIG_MAPNIK_ICU_DATA='%(mapnik_bundled_icu_data)s'

'''

def write_config(configuration,template,config_file):
    template = open(template,'r').read()
    open(config_file,'w').write(config_variables  % configuration + template)
    try:
        os.chmod(config_file,0755)
    except: pass


cxxflags_raw = config_env['LIBMAPNIK_CXXFLAGS'];
# strip clang specific flags to avoid breaking gcc
# while it is not recommended to mix compilers, this nevertheless
# makes it easier to compile apps with gcc and mapnik-config against mapnik built with clang
to_remove = ['-Wno-unsequenced','-Wno-unknown-warning-option','-Wtautological-compare','-Wheader-hygiene']
cxxflags_cleaned = [item for item in config_env['LIBMAPNIK_CXXFLAGS'] if item not in to_remove];

cxxflags = ' '.join(cxxflags_cleaned)

defines = ' '.join(config_env['LIBMAPNIK_DEFINES'])

dep_includes = ''.join([' -I${NODE_CONFIG_PREFIX:-""}%s' % i for i in config_env['CPPPATH'] if not i.startswith('#')])

dep_includes += ' '

if config_env['HAS_CAIRO']:
    dep_includes += ''.join([' -I${NODE_CONFIG_PREFIX:-""}%s' % i for i in env['CAIRO_CPPPATHS'] if not i.startswith('#')])

ldflags = ''.join([' -L%s' % i for i in config_env['LIBPATH'] if not i.startswith('#')])
ldflags += config_env['LIBMAPNIK_LINKFLAGS']

dep_libs = ''.join([' -l%s' % i for i in env['LIBMAPNIK_LIBS']])

# remove local agg from public linking
dep_libs = dep_libs.replace('-lagg','')

git_revision = 'N/A'
git_describe = config_env['MAPNIK_VERSION_STRING']

try:
    git_cmd = "git rev-list --max-count=1 HEAD"
    stdin, stderr = Popen(git_cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        git_revision = stdin.strip()

    git_cmd = "git describe"
    stdin, stderr = Popen(git_cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        git_describe = stdin.strip()
except:
    pass

# for fonts and input plugins we should try
# to store the relative path, if feasible
fontspath = config_env['MAPNIK_FONTS']
lib_root = os.path.join(config_env['PREFIX'], config_env['LIBDIR_SCHEMA'])
if lib_root in fontspath:
    fontspath = "${CONFIG_PREFIX}/" + os.path.relpath(fontspath,config_env['PREFIX'])
inputpluginspath = config_env['MAPNIK_INPUT_PLUGINS']
if lib_root in inputpluginspath:
    inputpluginspath = "${CONFIG_PREFIX}/" + os.path.relpath(inputpluginspath,config_env['PREFIX'])

lib_path = "${CONFIG_PREFIX}/" + config_env['LIBDIR_SCHEMA']

mapnik_bundled_gdal_data = ''
mapnik_bundled_proj_data = ''
mapnik_bundled_icu_data = ''

configuration = {
    "git_revision": git_revision,
    "git_describe": git_describe,
    "version_string": config_env['MAPNIK_VERSION_STRING'],
    "version": config_env['MAPNIK_VERSION'],
    "mapnik_libname": env['MAPNIK_NAME'],
    "mapnik_libpath": lib_path,
    "ldflags": ldflags,
    "dep_libs": dep_libs,
    "dep_includes": dep_includes,
    "fonts": fontspath,
    "input_plugins": inputpluginspath,
    "defines":defines,
    "cxxflags":cxxflags,
    "cxx":env['CXX'],
    "mapnik_bundled_gdal_data":mapnik_bundled_gdal_data,
    "mapnik_bundled_proj_data":mapnik_bundled_proj_data,
    "mapnik_bundled_icu_data":mapnik_bundled_icu_data,
}

## if we are statically linking depedencies
## then they do not need to be reported in ldflags
#if env['RUNTIME_LINK'] == 'static':
#    configuration['ldflags'] = ''
#    configuration['dep_libs'] = ''

template = 'mapnik-config.template.sh'
config_file = 'mapnik-config'
source = config_file
write_config(configuration,template,config_file)
target_path = os.path.normpath(os.path.join(config_env['INSTALL_PREFIX'],'bin'))
full_target = os.path.join(target_path,config_file)

Depends(full_target, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))
Depends(full_target, '../../include/mapnik/version.hpp')

if 'install' in COMMAND_LINE_TARGETS:
    # we must add 'install' catch here because otherwise
    # custom command will be run when not installing
    env.Alias('install',full_target)

    env.Command(full_target, config_file,
       [
       Copy("$TARGET","$SOURCE"),
       Chmod("$TARGET", 0755),
       ])

config_env['create_uninstall_target'](env,os.path.join(target_path,config_file))
