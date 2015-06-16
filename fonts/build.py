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
import glob

Import('env')

if not env['SYSTEM_FONTS']:
    # grab all the deja vu fonts
    includes = glob.glob('*/*/*.ttf')
    # grab single unifont ttf (available at http://unifoundry.com/unifont.html)
    includes.extend(glob.glob('unifont*.ttf'))
    target_path = env['MAPNIK_FONTS_DEST']
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        env.Alias(target='install', source=env.Install(target_path, includes))
    env['create_uninstall_target'](env, target_path)
