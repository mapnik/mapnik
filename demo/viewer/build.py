#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2010 Artem Pavlenko, Jean-Francois Doyon
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

Import ('env')
import os
import platform

lib_dir = os.path.normpath(env['DESTDIR'] + '/' + env['PREFIX'] + '/' + env['LIBDIR_SCHEMA'] + '/mapnik') 

fonts = 1
ini_template = '''
[mapnik]
plugins_dir=%(lib_dir)s/input
fonts/1/dir=%(lib_dir)s/fonts
'''

if platform.uname()[0] == 'Darwin':
    ini_template += 'fonts/2/dir=/Library/Fonts\n'
    fonts += 1

ini_template += 'fonts/size=%d\n' % fonts

ini = ini_template % locals()

open('viewer.ini','w').write(ini)

try:
    os.chmod('viewer.ini',0666)
except: pass
