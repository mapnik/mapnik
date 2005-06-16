#mapnik SConctruct

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

cxx_debug='-Wall -ftemplate-depth-100 -O0 -fno-inline -g -pthread'
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

#datasources
for datasource in Split(env['DATASOURCES']):
    env.BuildDir('build/datasources/'+datasource,'src/datasources/'+datasource,duplicate=0)
    SConscript('datasources/'+datasource+'/SConscript')



