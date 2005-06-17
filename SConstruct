#  This file is part of Mapnik (c++ mapping toolkit)
#  Copyright (C) 2005 Artem Pavlenko
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
# $Id$

import os

#edit 'settings.py' to match your system settings
opts = Options('settings.py')

opts.Add('PREFIX', 'Set the install "prefix"', '/opt/mapnik')
opts.Add(PathOption('BOOST_ROOT','boost source root directory','/opt/boost'))
opts.Add(PathOption('AGG_ROOT','agg source root directory','/opt/agg23'))
opts.Add(PathOption('FREETYPE2_ROOT','freetype2 root directory','/opt/freetype2'))
opts.Add(PathOption('PYTHON_ROOT','python root directory','/opt/python'))
opts.Add('PYTHON_VERSION','python version','2.4')
opts.Add(ListOption('DATASOURCES','list of available datasources','postgis',['postgis','shape'])) 
opts.Add('POSTGRESQL_ROOT','path to postgresql prefix','/usr/local')
    
platform = ARGUMENTS.get("OS",Platform())

build_dir = 'build'
build_prefix = build_dir+'/'+str(platform)

cxx = 'g++'

env = Environment(CXX=cxx,ENV=os.environ, options=opts)

cxx_debug='-Wall -ftemplate-depth-100 -O0 -fno-inline -g -pthread -DDEBUG'
cxx_release='-Wall -ftemplate-depth-100 -O2 -finline-functions -Wno-inline -pthread -DNDEBUG'

release_env = env.Copy(CXXFLAGS = cxx_release)
debug_env = env.Copy(CXXFLAGS = cxx_debug)

if ARGUMENTS.get('debug',0):
    env.Append(CXXFLAGS = cxx_debug)
    build_prefix+='/debug'
else:
    env.Append(CXXFLAGS = cxx_release)
    build_prefix+='/release'

Help(opts.GenerateHelpText(env))

conf = Configure(env)

if not conf.CheckLibWithHeader('ltdl','ltdl.h','C'):
    print 'Could not find libltdl/headers , exiting!'
    Exit(1)

if not conf.CheckLib('z'):
    print 'Could not find libz , exiting!'
    Exit(1)

if not conf.CheckLibWithHeader('png','png.h','C'):
    print 'Could not find libpng/headers, exiting!'
    Exit(1)

if not conf.CheckLib('jpeg'):
    print 'Could not find jpeg lib, exiting!'
    Exit(1)
    
env  = conf.Finish()

Export('env')

#build boost libs (filesystem, regex, python)
env.SConscript('boost/SConscript')

#build agg lib
env.SConscript('agg/SConscript')

#main lib
SConscript('src/SConscript')

#python ext
SConscript('python/SConscript')

#shapeindex
SConscript('util/shapeindex/SConscript')

#datasources
for datasource in Split(env['DATASOURCES']):
    env.BuildDir('build/datasources/'+datasource,'src/datasources/'+datasource,duplicate=0)
    SConscript('datasources/'+datasource+'/SConscript')



