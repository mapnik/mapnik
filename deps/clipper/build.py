#  This file is part of Mapnik (c++ mapping toolkit)
#  Copyright (C) 2013 Artem Pavlenko
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
lib_env = env.Clone()
if 'g++' in env['CXX']:
    lib_env.Append(CXXFLAGS='-fPIC')
lib_env.StaticLibrary('clipper', ['./src/clipper.cpp',], LIBS=[])
