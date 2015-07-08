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

import os
from glob import glob
from subprocess import Popen, PIPE

Import('env')
lib_env = env.Clone()

def call(cmd, silent=True):
    stdin, stderr = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        return stdin.strip()
    elif not silent:
        print stderr

def ldconfig(*args,**kwargs):
    call('ldconfig')

if env['LINKING'] == 'static':
    lib_env.Append(CXXFLAGS='-fPIC')

mapnik_lib_link_flag = ''

ABI_VERSION = env['ABI_VERSION']

if env['PLATFORM'] == 'Darwin':
    mapnik_libname = env.subst(env['MAPNIK_JSON_LIB_NAME'])
    if env['FULL_LIB_PATH']:
        lib_path = '%s/%s' % (env['MAPNIK_LIB_BASE'],mapnik_libname)
    else:
        lib_path = '@loader_path/'+mapnik_libname
    mapnik_lib_link_flag += ' -Wl,-install_name,%s' % lib_path
    _d = {'version':env['MAPNIK_VERSION_STRING'].replace('-pre','')}
    mapnik_lib_link_flag += ' -current_version %(version)s -compatibility_version %(version)s' % _d
else: # unix, non-macos
    mapnik_libname = env.subst(env['MAPNIK_JSON_LIB_NAME'])
    if env['ENABLE_SONAME']:
        mapnik_libname = env.subst(env['MAPNIK_JSON_LIB_NAME']) + (".%d.%d" % (int(ABI_VERSION[0]),int(ABI_VERSION[1])))
    if env['PLATFORM'] == 'SunOS':
        if env['CXX'].startswith('CC'):
            mapnik_lib_link_flag += ' -R. -h %s' % mapnik_libname
        else:
            mapnik_lib_link_flag += ' -Wl,-h,%s' %  mapnik_libname
    else: # Linux and others
        if env['PLATFORM'] != 'FreeBSD':
            lib_env['LIBS'].append('dl')
        mapnik_lib_link_flag += ' -Wl,-rpath-link,.'
        if env['ENABLE_SONAME']:
            mapnik_lib_link_flag += ' -Wl,-soname,%s' % mapnik_libname
        if env['FULL_LIB_PATH']:
            mapnik_lib_link_flag += ' -Wl,-rpath=%s' % env['MAPNIK_LIB_BASE']
        else:
            mapnik_lib_link_flag += ' -Wl,-z,origin -Wl,-rpath=\$$ORIGIN'

# clone the env one more time to isolate mapnik_lib_link_flag
lib_env_final = lib_env.Clone()
lib_env_final.Prepend(LINKFLAGS=mapnik_lib_link_flag)

name = "mapnik-json"
source = glob('./' + '*.cpp')

if env['PLATFORM'] == 'Darwin' or not env['ENABLE_SONAME']:
    target_path = env['MAPNIK_LIB_BASE_DEST']
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        if env['LINKING'] == 'static':
            lib = lib_env_final.StaticLibrary(name, source, LIBS=[])
        else:
            lib = lib_env_final.SharedLibrary(name, source, LIBS=[])
        result = env.Install(target_path, lib)
        env.Alias(target='install', source=result)

    env['create_uninstall_target'](env, os.path.join(target_path,env.subst(env['MAPNIK_JSON_LIB_NAME'])))
else:
    # Symlink command, only works if both files are in same directory
    def symlink(env, target, source):
        trgt = str(target[0])
        src = str(source[0])

        if os.path.islink(trgt) or os.path.exists(trgt):
            os.remove(trgt)
        os.symlink(os.path.basename(src), trgt)

    major, minor, micro = ABI_VERSION

    soFile = "%s.%d.%d.%d" % (os.path.basename(env.subst(env['MAPNIK_JSON_LIB_NAME'])), int(major), int(minor), int(micro))
    target = os.path.join(env['MAPNIK_LIB_BASE_DEST'], soFile)

    if 'uninstall' not in COMMAND_LINE_TARGETS:
        if env['LINKING'] == 'static':
            lib = lib_env_final.StaticLibrary(name, source, LIBS=[])
        else:
            lib = lib_env_final.SharedLibrary(name, source, LIBS=[])
        result = env.InstallAs(target=target, source=lib)
        env.Alias(target='install', source=result)
        if result:
              env.AddPostAction(result, ldconfig)

    # Install symlinks
    target1 = os.path.join(env['MAPNIK_LIB_BASE_DEST'], "%s.%d.%d" % \
        (os.path.basename(env.subst(env['MAPNIK_JSON_LIB_NAME'])),int(major), int(minor)))
    target2 = os.path.join(env['MAPNIK_LIB_BASE_DEST'], os.path.basename(env.subst(env['MAPNIK_JSON_LIB_NAME'])))
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        link1 = env.Command(target1, target, symlink)
        env.Alias(target='install', source=link1)
        link2 = env.Command(target2, target1, symlink)
        env.Alias(target='install', source=link2)
    # delete in reverse order..
    env['create_uninstall_target'](env, target2)
    env['create_uninstall_target'](env, target1)
    env['create_uninstall_target'](env, target)

    # to enable local testing
    lib_major_minor = "%s.%d.%d" % (os.path.basename(env.subst(env['MAPNIK_JSON_LIB_NAME'])), int(major), int(minor))
    local_lib = os.path.basename(env.subst(env['MAPNIK_JSON_LIB_NAME']))
    if os.path.islink(lib_major_minor) or os.path.exists(lib_major_minor):
        os.remove(lib_major_minor)
    os.symlink(local_lib,lib_major_minor)
    Clean(lib,lib_major_minor);
