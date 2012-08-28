#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2009 Artem Pavlenko, Dane Springmeyer
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
from copy import copy

Import ('env')

prefix = env['PREFIX']

program_env = env.Clone()

source = Split(
    """
    main.cpp
    sqlite.cpp
    """
    )

program_env['CXXFLAGS'] = copy(env['LIBMAPNIK_CXXFLAGS'])

if env['HAS_CAIRO']:
    program_env.PrependUnique(CPPPATH=env['CAIROMM_CPPPATHS'])
    program_env.Append(CXXFLAGS = '-DHAVE_CAIRO')

program_env.PrependUnique(CPPPATH=['#plugins/input/postgis'])

libraries = []
boost_program_options = 'boost_program_options%s' % env['BOOST_APPEND']
libraries.extend([boost_program_options,'sqlite3','pq','mapnik','icuuc'])

if env.get('BOOST_LIB_VERSION_FROM_HEADER'):
    boost_version_from_header = int(env['BOOST_LIB_VERSION_FROM_HEADER'].split('_')[1])
    if boost_version_from_header >= 50:
        boost_system = 'boost_system%s' % env['BOOST_APPEND']
        libraries.extend([boost_system])

linkflags = env['CUSTOM_LDFLAGS']
if env['SQLITE_LINKFLAGS']:
    linkflags.append(env['SQLITE_LINKFLAGS'])

if env['RUNTIME_LINK'] == 'static':
    libraries.extend(['ldap','pam','ssl','crypto','krb5'])

pgsql2sqlite = program_env.Program('pgsql2sqlite', source, LIBS=libraries, LINKFLAGS=linkflags)
Depends(pgsql2sqlite, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(os.path.join(env['INSTALL_PREFIX'],'bin'), pgsql2sqlite)
    env.Alias('install', os.path.join(env['INSTALL_PREFIX'],'bin'))

env['create_uninstall_target'](env, os.path.join(env['INSTALL_PREFIX'],'bin','pgsql2sqlite'))
