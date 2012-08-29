#  This file is part of Mapnik (c++ mapping toolkit)
#  Copyright (C) 2005 Artem Pavlenko, Jean-Francois Doyon
#
#  Mapnik is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# 

import os
from glob import glob

Import('env')

agg_env = env.Clone()

if env['SUNCC']:
    cxxflags = env['CUSTOM_CXXFLAGS'] + ' -O -KPIC -DNDEBUG'
else:
    cxxflags = env['CUSTOM_CXXFLAGS'] + ' -O%s -fPIC -DNDEBUG' % env['OPTIMIZATION']

agg_env.StaticLibrary('agg', glob('./src/' + '*.cpp'), LIBS=[], CXXFLAGS=cxxflags, LINKFLAGS=env['CUSTOM_LDFLAGS'])

if 'install' in COMMAND_LINE_TARGETS:
    inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/agg')
    # TODO - restrict to just agg headers used in mapnik includes?
    includes = glob('./include/*.h')
    target = env.Install(inc_target, includes)
    env.Alias(target='install', source=target)
