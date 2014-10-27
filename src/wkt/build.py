#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2014 Artem Pavlenko
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

Import('env')
lib_env = env.Clone()
if 'g++' in env['CXX']:
    lib_env.Append(CXXFLAGS='-fPIC')

name = "mapnik-wkt"
lib = lib_env.StaticLibrary(name, glob('./' + '*.cpp'), LIBS=[])
target = os.path.join(env['MAPNIK_LIB_BASE_DEST'], env.subst('${LIBPREFIX}%s${LIBSUFFIX}' % name))
result = env.InstallAs(target=target, source=lib)
env.Alias(target='install', source=result)
env['create_uninstall_target'](env, target)
