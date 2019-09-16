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
from SCons.Environment import OverrideEnvironment

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
    print ('Error: version.hpp mismatch (%s) to cached value (%s): please reconfigure mapnik' % (GetMapnikLibVersion(),config_env['MAPNIK_VERSION_STRING']))
    Exit(1)

def write_config(env, template_filename, config_filename):
    """
    Load shell script `template_filename`, replace values in variable
    assignments of the form `CONFIG_key='default'` with corresponding
    value `env['key']`, and save the result as `config_filename`.
    """
    with open(template_filename, 'r') as template_file:
        template = template_file.read()
    with open(config_filename, 'w') as config_file:
        escape = env['ESCAPE']
        def subst(matchobj):
            key = matchobj.group(1)
            val = env.get(key)
            if val is None:
                return matchobj.group(0)
            else:
                return 'CONFIG_%s=%s' % (key, escape(str(val)))
        config = re.sub(r'^CONFIG_(\w+)=.*', subst, template, flags=re.M)
        config_file.write(config)
    try:
        os.chmod(config_filename, 0o755)
    except: pass


template_env = OverrideEnvironment(config_env, {})

# strip any -Warning flags
cxxflags_cleaned = [x for x in config_env['LIBMAPNIK_CXXFLAGS']
                      if x.startswith('-Wp,') or not x.startswith('-W')]
# strip clang specific flags to avoid breaking gcc
# while it is not recommended to mix compilers, this nevertheless
# makes it easier to compile apps with gcc and mapnik-config against mapnik built with clang
cxxflags_cleaned = [x for x in cxxflags_cleaned if x != '-Qunused-arguments']
cxx_cleaned = config_env['CXX'].replace(' -Qunused-arguments', '')

template_env['CXXFLAGS'] = ' '.join(cxxflags_cleaned)
template_env['CXX'] = re.sub(r'^ccache +', '', cxx_cleaned)

template_env['DEFINES'] = ' '.join(config_env['LIBMAPNIK_DEFINES'])

dep_includes = ''.join([' -I%s' % i for i in config_env['CPPPATH'] if not i.startswith('#')])

dep_includes += ' '

if config_env['HAS_CAIRO']:
    dep_includes += ''.join([' -I%s' % i for i in env['CAIRO_CPPPATHS'] if not i.startswith('#')])
template_env['DEP_INCLUDES'] = dep_includes

ldflags = ''.join([' -L%s' % i for i in config_env['LIBPATH'] if not i.startswith('#')])
ldflags += config_env['LIBMAPNIK_LINKFLAGS']
template_env['LDFLAGS'] = ldflags

dep_libs = ''.join([' -l%s' % i for i in env['LIBMAPNIK_LIBS']])

# remove local agg from public linking
dep_libs = dep_libs.replace('-lagg','')
template_env['DEP_LIBS'] = dep_libs

try:
    stdin, stderr = Popen("git rev-list --max-count=1 HEAD",
                        shell=True, universal_newlines=True,
                        stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        template_env["GIT_REVISION"] = stdin.strip()

    stdin, stderr = Popen("git describe",
                        shell=True, universal_newlines=True,
                        stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        template_env["GIT_DESCRIBE"] = stdin.strip()
except:
    pass

## if we are statically linking dependencies
## then they do not need to be reported in ldflags
#if env['RUNTIME_LINK'] == 'static':
#    template_env['LDFLAGS'] = ''
#    template_env['DEP_LIBS'] = ''

template = 'mapnik-config.template.sh'
config_file = 'mapnik-config'
write_config(template_env, template, config_file)
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
       Chmod("$TARGET", 0o755),
       ])

config_env['create_uninstall_target'](env,os.path.join(target_path,config_file))
