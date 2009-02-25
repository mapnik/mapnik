# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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
# $Id$


import os, sys, platform
from subprocess import Popen, PIPE

def color_print(color,text,newline=True):
    # 1 - red
    # 2 - green
    # 3 - yellow
    # 4 - blue
    text = "\033[9%sm%s\033[0m" % (color,text)
    if not newline:
        print text,
    else:
        print text

def call(cmd):
    stdin, stderr = Popen(cmd,shell=True,stdout=PIPE,stderr=PIPE).communicate()
    if not stderr:
      return stdin.strip()
    else:
      color_print(1,'Problem encounted with SCons scripts, please post bug report to: http://trac.mapnik.org')

# Helper function for uniquely appending paths to a SCons path listing.
def uniq_add(env, key, val):
    if not val in env[key]: env[key].append(val)

# Helper function for removing paths, if they exist, from the SCons path listing.
def remove_path(env, key, val):
    if val in env[key]: env[key].remove(val)

        
# Helper function for removing paths for a plugin lib
# We don't currently have control over whether the plugin
# path we remove is also shared by another plugin or 
# required lib/header, so this may need improvement
def remove_plugin_path(plugin_lib):
    plugin = {}
    # find the plugin details by 'lib' name
    for k,v in PLUGINS.items():
        if v['lib'] == plugin_lib and plugin_lib == 'gdal':
            if v['path'] == 'OGR':
                plugin = PLUGINS['ogr']
            elif v['path'] == 'GDAL':
                plugin = PLUGINS['gdal']
        elif v['lib'] == plugin_lib:
            plugin = PLUGINS[k]
    # if the plugin details are found remove its paths
    if plugin:
        lib_path = '%s_LIBS' % plugin['path']
        inc_path = '%s_INCLUDES' % plugin['path']
        remove_path(env, 'CPPPATH', env[inc_path])
        remove_path(env, 'LIBPATH', env[lib_path])

# Helper function for building up paths to add for a lib (plugin or required)
def add_paths(prereq):
    inc_path = env['%s_INCLUDES' % prereq]
    lib_path = env['%s_LIBS' % prereq]
    uniq_add(env, 'CPPPATH', inc_path)
    uniq_add(env, 'LIBPATH', lib_path)

if platform.uname()[4] == 'x86_64':
    LIBDIR_SCHEMA='lib64' 
elif platform.uname()[4] == 'ppc64':
    LIBDIR_SCHEMA='lib64'
else:
    LIBDIR_SCHEMA='lib'

#### SCons build options and initial setup ####

SCONS_LOCAL_CONFIG = 'config.py'

# Warn user of current set of build options.
if os.path.exists(SCONS_LOCAL_CONFIG):
    optfile = file(SCONS_LOCAL_CONFIG)
    print "Saved options:", optfile.read().replace("\n", ", ")[:-2]
    optfile.close()

# Core plugin build configuration
# opts.Add still hardcoded however...
PLUGINS = { # plugins with external dependencies
            'postgis': {'default':True,'path':'PGSQL','inc':'libpq-fe.h','lib':'pq','cxx':False},
            'gdal':    {'default':False,'path':'GDAL','inc':'gdal_priv.h','lib':'gdal','cxx':True},
            'ogr':     {'default':False,'path':'OGR','inc':'ogrsf_frmts.h','lib':'gdal','cxx':True},
            'occi':    {'default':False,'path':'OCCI','inc':'occi.h','lib':'ociei','cxx':True},
            'sqlite':  {'default':False,'path':'SQLITE','inc':'sqlite3.h','lib':'sqlite3','cxx':False},
            
            # plugins without external dependencies requiring CheckLibWithHeader...
            # note: osm plugin does depend on libxml2
            'osm':     {'default':False,'path':None,'inc':None,'lib':None,'cxx':True},
            'shape':   {'default':True,'path':None,'inc':None,'lib':None,'cxx':True},
            'raster':  {'default':True,'path':None,'inc':None,'lib':None,'cxx':True},
            }

DEFAULT_PLUGINS = []
for k,v in PLUGINS.items():
   if v['default']:
     DEFAULT_PLUGINS.append(k)

# All of the following options may be modified at the command-line, for example:
# `python scons/scons.py PREFIX=/opt`
opts = Variables()

# Compiler options
opts.Add('CXX', 'The C++ compiler to use (defaults to g++).', 'g++')
opts.Add(EnumVariable('OPTIMIZATION','Set g++ optimization level','2', ['0','1','2','3']))
# Note: setting DEBUG=True will override any custom OPTIMIZATION level
opts.Add(BoolVariable('DEBUG', 'Compile a debug version of Mapnik', 'False'))
opts.Add(ListVariable('INPUT_PLUGINS','Input drivers to include',DEFAULT_PLUGINS,PLUGINS.keys()))

# SCons build behavior options
opts.Add('CONFIG', "The path to the python file in which to save user configuration options. Currently : '%s'" % SCONS_LOCAL_CONFIG,SCONS_LOCAL_CONFIG)
opts.Add(BoolVariable('USE_CONFIG', "Use SCons user '%s' file (will also write variables after successful configuration)", 'True'))
opts.Add(BoolVariable('SCONS_CACHE', 'Use SCons dependency caching to speed build process', 'False'))
opts.Add(BoolVariable('USE_USER_ENV', 'Allow the SCons build environment to inherit from the current user environment', 'True'))

# Install Variables
opts.Add('PREFIX', 'The install path "prefix"', '/usr/local')
opts.Add('PYTHON_PREFIX','Custom install path "prefix" for python bindings (default of no prefix)','')
opts.Add('DESTDIR', 'The root directory to install into. Useful mainly for binary package building', '/')

# Boost variables
opts.Add(PathVariable('BOOST_INCLUDES', 'Search path for boost include files', '/usr/include', PathVariable.PathAccept))
opts.Add(PathVariable('BOOST_LIBS', 'Search path for boost library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add('BOOST_TOOLKIT','Specify boost toolkit, e.g., gcc41.','',False)
opts.Add('BOOST_ABI', 'Specify boost ABI, e.g., d.','',False)
opts.Add('BOOST_VERSION','Specify boost version, e.g., 1_35.','',False)

# Variables for required dependencies
opts.Add(('FREETYPE_CONFIG', 'The path to the freetype-config executable.', 'freetype-config'))
opts.Add(('XML2_CONFIG', 'The path to the xml2-config executable.', 'xml2-config'))
opts.Add(PathVariable('ICU_INCLUDES', 'Search path for ICU include files', '/usr/include', PathVariable.PathAccept))
opts.Add(PathVariable('ICU_LIBS','Search path for ICU include files','/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add(PathVariable('PNG_INCLUDES', 'Search path for libpng include files', '/usr/include', PathVariable.PathAccept))
opts.Add(PathVariable('PNG_LIBS','Search path for libpng include files','/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add(PathVariable('JPEG_INCLUDES', 'Search path for libjpeg include files', '/usr/include', PathVariable.PathAccept))
opts.Add(PathVariable('JPEG_LIBS', 'Search path for libjpeg library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add(PathVariable('TIFF_INCLUDES', 'Search path for libtiff include files', '/usr/include', PathVariable.PathAccept))
opts.Add(PathVariable('TIFF_LIBS', 'Search path for libtiff library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add(PathVariable('PROJ_INCLUDES', 'Search path for PROJ.4 include files', '/usr/local/include', PathVariable.PathAccept))
opts.Add(PathVariable('PROJ_LIBS', 'Search path for PROJ.4 library files', '/usr/local/' + LIBDIR_SCHEMA, PathVariable.PathAccept))

# Variables affecting rendering back-ends
opts.Add(BoolVariable('INTERNAL_LIBAGG', 'Use provided libagg', 'True'))

# Variables for optional dependencies
# Note: cairo, cairomm, and pycairo all optional but configured automatically through pkg-config
# Therefore, we use a single boolean for whether to attempt to build cairo support.
opts.Add(BoolVariable('CAIRO', 'Attempt to build with Cairo rendering support', 'True'))
opts.Add(PathVariable('PGSQL_INCLUDES', 'Search path for PostgreSQL include files', '/usr/include/postgresql', PathVariable.PathAccept))
opts.Add(PathVariable('PGSQL_LIBS', 'Search path for PostgreSQL library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add(PathVariable('GDAL_INCLUDES', 'Search path for GDAL include files', '/usr/local/include', PathVariable.PathAccept))
opts.Add(PathVariable('GDAL_LIBS', 'Search path for GDAL library files', '/usr/local/' + LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add(PathVariable('OGR_INCLUDES', 'Search path for OGR include files', '/usr/local/include', PathVariable.PathAccept))
opts.Add(PathVariable('OGR_LIBS', 'Search path for OGR library files', '/usr/local/' + LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add(PathVariable('OCCI_INCLUDES', 'Search path for OCCI include files', '/usr/lib/oracle/10.2.0.3/client/include', PathVariable.PathAccept))
opts.Add(PathVariable('OCCI_LIBS', 'Search path for OCCI library files', '/usr/lib/oracle/10.2.0.3/client/'+ LIBDIR_SCHEMA, PathVariable.PathAccept))
opts.Add(PathVariable('SQLITE_INCLUDES', 'Search path for SQLITE include files', '/usr/include/', PathVariable.PathAccept))
opts.Add(PathVariable('SQLITE_LIBS', 'Search path for SQLITE library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept))

# Other variables
opts.Add('SYSTEM_FONTS','Provide location for python bindings to register fonts (if given aborts installation of bundled DejaVu fonts)','')
opts.Add('LIB_DIR_NAME','Name to use for lib folder where fonts and plugins are installed', '/mapnik/', PathVariable.PathAccept)
opts.Add(PathVariable('PYTHON','Full path to Python executable used to build bindings', sys.executable))
opts.Add(BoolVariable('FRAMEWORK_PYTHON', 'Link against Framework Python on Mac OSX', 'True'))
opts.Add(ListVariable('BINDINGS','Language bindings to build','all',['python']))
opts.Add(EnumVariable('THREADING','Set threading support','multi', ['multi','single']))
opts.Add(EnumVariable('XMLPARSER','Set xml parser ','libxml2', ['tinyxml','spirit','libxml2']))

# Construct the SCons build environment as a union of the users environment and the `opts`


# Build up base environment, then reinitiate based on user customizations.

# This method seems un-pythonic and a bit dodgy, but it works.
# It seems that creating an alternative environment that loads user options
# and then updating the main env using those options is the more logical route
# But my testing indicate that something like:
# >>>user_opts = Variables([env['CONFIG']])
# >>> user_opts.Update(env)
# does not seem to work as expected.

# Create clean environment with only the options within this SConstruct file
color_print(4,'Constructing build environment...')
env = Environment(options=opts)

# Unless USE_USER_ENV=False, recreate the base environment
# using all variables (for example) in the shell users .profile or /etc/profile
if env['USE_USER_ENV']:
    env = Environment(ENV=os.environ,options=opts)
else:
    # Default has been overridden, so print a warning
    color_print(4,'USER_USER_ENV specified as false, will not inherit variables from user environment...')

# Unless USE_USER_ENV=False, recreate the base environment
# using all variables (for example) in the shell users .profile or /etc/profile
user_conf = env['CONFIG']

if env['USE_CONFIG']:
    if not user_conf.endswith('.py'):
        color_print(1,'SCons CONFIG file specified is not a python file, will not be read...')
    else:
        # Accept more than one file as comma-delimited list
        user_confs = user_conf.split(',')
        # If they exist add the files to the existing `opts`
        for conf in user_confs:
            if os.path.exists(conf):
                opts.files.append(conf)
                color_print(4,"SCons CONFIG found: '%s', variables will be inherited..." % conf)
            elif not conf == SCONS_LOCAL_CONFIG:
                # if default missing, no worries
                # but if the default is overridden and the file is not found, give warning
                color_print(1,"SCons CONFIG not found: '%s'" % conf)
        # Recreate the base environment using modified `opts`
        env = Environment(ENV=os.environ,options=opts)
        env['USE_CONFIG'] = True
else:
    color_print(4,'SCons USE_CONFIG specified as false, will not inherit variables python config file...')


env['MISSING_DEPS'] = []
env['SKIPPED_DEPS'] = []

env['LIBDIR_SCHEMA'] = LIBDIR_SCHEMA
env['PLATFORM'] = platform.uname()[0]

if env['DEBUG']:
  mode = 'debug mode'
else:
  mode = 'release mode'

color_print (4,"Building on %s in *%s*..." % (env['PLATFORM'],mode))
Help(opts.GenerateHelpText(env))

if env['SCONS_CACHE']:
    # caching is 'auto' by default in SCons
    # But let's also cache implicit deps...
    SetOption('implicit_cache', 1)
    # uncomment for more speed improvements
    #env.Decider('MD5-timestamp')
    #SetOption('max_drift', 1)
    
else:
    # Set the cache mode to 'force' unless requested, avoiding hidden caching of Scons 'opts' in '.sconsign.dblite'
    # This allows for a SCONS_LOCAL_CONFIG, if present, to be used as the primary means of storing paths to successful build dependencies
    from SCons.SConf import SetCacheMode
    SetCacheMode('force')
    

thread_suffix = 'mt'
if env['PLATFORM'] == 'FreeBSD':
    thread_suffix = ''
    env.Append(LIBS = 'pthread')

def CheckPKGConfig(context, version):
   context.Message( 'Checking for pkg-config... ' )
   ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
   context.Result( ret )
   return ret

def CheckPKG(context, name):
    context.Message( 'Checking for %s... ' % name )
    ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
    context.Result( ret )
    return ret

def CheckBoost(context, version, silent=False):
    # Boost versions are in format major.minor.subminor
    v_arr = version.split(".")
    version_n = 0
    if len(v_arr) > 0:
        version_n += int(v_arr[0])*100000
    if len(v_arr) > 1:
        version_n += int(v_arr[1])*100
    if len(v_arr) > 2:
        version_n += int(v_arr[2])
        
    if not silent:
        context.Message('Checking for Boost version >= %s... ' % (version))
    ret = context.TryRun("""

#include <boost/version.hpp>

int main() 
{
    return BOOST_VERSION >= %d ? 0 : 1;
}

""" % version_n, '.cpp')[0]
    if silent:
        context.did_show_result=1
    context.Result(ret)
    return ret

def GetBoostLibVersion(context):
    ret = context.TryRun("""

#include <boost/version.hpp>
#include <iostream>

int main() 
{

std::cout << BOOST_LIB_VERSION << std::endl;
return 0;
}

""", '.cpp')
    # hack to avoid printed output
    context.did_show_result=1
    context.Result(ret[0])
    return ret[1].strip()

def GetMapnikLibVersion(context):
    ret = context.TryRun("""

#include <mapnik/version.hpp>
#include <iostream>

int main() 
{
    std::cout << MAPNIK_VERSION << std::endl;
    return 0;
}

""", '.cpp')
    # hack to avoid printed output
    context.did_show_result=1
    context.Result(ret[0])
    if not ret[1]:
        return []
    version = int(ret[1].strip())    
    patch_level = version % 100
    minor_version = version / 100 % 1000
    major_version = version / 100000
    return [major_version,minor_version,patch_level]
  
conf = Configure(env, custom_tests = { 'CheckPKGConfig' : CheckPKGConfig,
                                       'CheckPKG' : CheckPKG,
                                       'CheckBoost' : CheckBoost,
                                       'GetBoostLibVersion' : GetBoostLibVersion,
                                       'GetMapnikLibVersion' : GetMapnikLibVersion })

    
#### Libraries and headers dependency checks ####

# Libraries and headers dependency checks
env['CPPPATH'] = ['#include', '#']
env['LIBPATH'] = ['#src']

# Solaris & Sun Studio settings (the `SUNCC` flag will only be
# set if the `CXX` option begins with `CC`)
SOLARIS = env['PLATFORM'] == 'SunOS'
SUNCC = SOLARIS and env['CXX'].startswith('CC')

# For Solaris include paths (e.g., for freetype2, ltdl, etc.).
if SOLARIS:
    blastwave_dir = '/opt/csw/%s'
    uniq_add(env, 'CPPPATH', blastwave_dir % 'include')
    uniq_add(env, 'LIBPATH', blastwave_dir % LIBDIR_SCHEMA)

# If the Sun Studio C++ compiler (`CC`) is used instead of GCC.
if SUNCC:
    env['CC'] = 'cc'
    # To be compatible w/Boost everything needs to be compiled
    # with the `-library=stlport4` flag (which needs to come
    # before the `-o` flag).
    env['CXX'] = 'CC -library=stlport4'
    if env['THREADING'] == 'multi':
        env['CXXFLAGS'] = ['-mt']

# Decide which libagg to use
if env['INTERNAL_LIBAGG']:
    env.Prepend(CPPPATH = '#agg/include')
    env.Prepend(LIBPATH = '#agg')
else:
    env.ParseConfig('pkg-config --libs --cflags libagg')
         
# Adding the required prerequisite library directories to the include path for
# compiling and the library path for linking, respectively.
for required in ('BOOST', 'PNG', 'JPEG', 'TIFF','PROJ'):
    add_paths(required)

requested_plugins = [ driver.strip() for driver in Split(env['INPUT_PLUGINS'])]

# Adding the required prerequisite library directories for the plugins...
for plugin in requested_plugins:
    extra_paths = PLUGINS[plugin]['path']
    if extra_paths:
      # Note, these are now removed below if the CheckLibWithHeader fails...
      add_paths(extra_paths)

try:
    env.ParseConfig(env['FREETYPE_CONFIG'] + ' --libs --cflags')
except OSError:
    env['MISSING_DEPS'].append(env['FREETYPE_CONFIG'])

if env['XMLPARSER'] == 'tinyxml':
    env['CPPPATH'].append('#tinyxml')
    env.Append(CXXFLAGS = '-DBOOST_PROPERTY_TREE_XML_PARSER_TINYXML -DTIXML_USE_STL')
elif env['XMLPARSER'] == 'libxml2':
    try:
        env.ParseConfig(env['XML2_CONFIG'] + ' --libs --cflags')
        env.Append(CXXFLAGS = '-DHAVE_LIBXML2')
    except OSError:
        env['MISSING_DEPS'].append(env['XML2_CONFIG'])

C_LIBSHEADERS = [
    ['m', 'math.h', True],
    ['ltdl', 'ltdl.h', True],
    ['png', 'png.h', True],
    ['tiff', 'tiff.h', True],
    ['z', 'zlib.h', True],
    ['jpeg', ['stdio.h', 'jpeglib.h'], True],
    ['proj', 'proj_api.h', True],
]

if env['CAIRO'] and conf.CheckPKGConfig('0.15.0') and conf.CheckPKG('cairomm-1.0'):
        env.ParseConfig('pkg-config --libs --cflags cairomm-1.0')
        env.Append(CXXFLAGS = '-DHAVE_CAIRO')
else:
   env['SKIPPED_DEPS'].extend(['cairo','cairomm','pycairo'])
        
CXX_LIBSHEADERS = [
    ['icuuc','unicode/unistr.h',True],
    ['icudata','unicode/utypes.h' , True],
]

# append plugin details to the 'LIBSHEADERS' lists
for plugin in requested_plugins:
    details = PLUGINS[plugin]
    if details['lib'] and details['inc']:
      check = [details['lib'],details['inc'],False]
      if details['cxx']:
        CXX_LIBSHEADERS.append(check)
      else:
        C_LIBSHEADERS.append(check)


# get boost version from boost headers rather than previous approach
# of fetching from the user provided INCLUDE path
boost_system_required = False
boost_lib_version_from_header = conf.GetBoostLibVersion()
if boost_lib_version_from_header:
    boost_version_from_header = int(boost_lib_version_from_header.split('_')[1])
    if boost_version_from_header >= 35 and env['PLATFORM'] == 'Darwin':
        boost_system_required = True
    else:
        boost_system_required = False

# The other required boost headers.
BOOST_LIBSHEADERS = [
    ['system', 'boost/system/system_error.hpp', boost_system_required],
    ['filesystem', 'boost/filesystem/operations.hpp', True],
    ['regex', 'boost/regex.hpp', True],
    ['iostreams','boost/iostreams/device/mapped_file.hpp',True],
    ['program_options', 'boost/program_options.hpp', False]
]
    
if env['THREADING'] == 'multi':
    BOOST_LIBSHEADERS.append(['thread', 'boost/thread/mutex.hpp', True])
    thread_flag = thread_suffix
else:
    thread_flag = ''

for libinfo in C_LIBSHEADERS:
    if not conf.CheckLibWithHeader(libinfo[0], libinfo[1], 'C'):
    
        # if we cannot build a plugin remove its paths
        remove_plugin_path(libinfo[0])

        if libinfo[0] == 'pq':
            libinfo[0] = 'pq (postgres/postgis)'
        if libinfo[2]:
            color_print (1,'Could not find required header or shared library for %s' % libinfo[0])
            env['MISSING_DEPS'].append(libinfo[0])
        else:
            color_print(4,'Could not find optional header or shared library for %s' % libinfo[0])
            env['SKIPPED_DEPS'].append(libinfo[0])

for libinfo in CXX_LIBSHEADERS:
    if not conf.CheckLibWithHeader(libinfo[0], libinfo[1], 'C++'):
        
        # if we cannot build a plugin remove its paths
        remove_plugin_path(libinfo[0])
            
        if libinfo[0] == 'gdal' and libinfo[1] == 'ogrsf_frmts.h':
            libinfo[0] = 'ogr'
        if libinfo[2]:
            color_print(1,'Could not find required header or shared library for %s' % libinfo[0])
            env['MISSING_DEPS'].append(libinfo[0])
        else:
            color_print(4,'Could not find optional header or shared library for %s' % libinfo[0])
            env['SKIPPED_DEPS'].append(libinfo[0])
    
    # touch up the user output so they can see whether both gdal and ogr support was enabled 
    elif libinfo[0] == 'gdal':
        if libinfo[1] == 'ogrsf_frmts.h':
            print 'ogr vector support... enabled'
        else:
            print 'gdal raster support... enabled'
        

# Creating BOOST_APPEND according to the Boost library naming order,
# which goes <toolset>-<threading>-<abi>-<version>. See:
#  http://www.boost.org/doc/libs/1_35_0/more/getting_started/unix-variants.html#library-naming
append_params = ['']
if env['BOOST_TOOLKIT']: append_params.append(env['BOOST_TOOLKIT'])
if thread_flag: append_params.append(thread_flag)
if env['BOOST_ABI']: append_params.append(env['BOOST_ABI'])
if env['BOOST_VERSION']: append_params.append(env['BOOST_VERSION'])

# if the user is not setting custom boost configuration
# enforce boost version greater than or equal to 1.33
if not conf.CheckBoost('1.33'):
    color_print (1,'Boost version 1.33 or greater is requred') 
    if not env['BOOST_VERSION']:
        env['MISSING_DEPS'].append('boost version >=1.33')
else:
    color_print (4,'Found boost lib version... %s' % boost_lib_version_from_header )

# Constructing the BOOST_APPEND setting that will be used to find the
# Boost libraries.
if len(append_params) > 1: 
    env['BOOST_APPEND'] = '-'.join(append_params)
else: 
    env['BOOST_APPEND'] = ''

for count, libinfo in enumerate(BOOST_LIBSHEADERS):
    if thread_flag:
        if not conf.CheckLibWithHeader('boost_%s%s' % (libinfo[0],env['BOOST_APPEND']), libinfo[1], 'C++'):
            if libinfo[2]:
              color_print(1,'Could not find required header or shared library for boost %s' % libinfo[0])
              env['MISSING_DEPS'].append('boost ' + libinfo[0])
            else:
              color_print(4,'Could not find optional header or shared library for boost %s' % libinfo[0])
              env['SKIPPED_DEPS'].append('boost ' + libinfo[0])
              
    elif not conf.CheckLibWithHeader('boost_%s%s' % (libinfo[0], env['BOOST_APPEND']), libinfo[1], 'C++') :
        color_print(1,'Could not find header or shared library for boost %s' % libinfo[0])
        env['MISSING_DEPS'].append('boost ' + libinfo[0])

if 'python' in env['BINDINGS']:
    # checklibwithheader does not work for boost_python since we can't feed it
    # multiple header files, so we fall back on a simple check for boost_python headers
    if not conf.CheckHeader(header='boost/python/detail/config.hpp',language='C++'):
        color_print(1,'Could not find required header files for boost python')
        env['MISSING_DEPS'].append('boost python')

# fetch the mapnik version header in order to set the
# ABI version used to build libmapnik.so on linux in src/SConscript
abi = conf.GetMapnikLibVersion()
abi_fallback = [0,6,0]
if not abi:
    color_print(1,'Problem encountered parsing mapnik version, falling back to %s' % abi_fallback)
    env['ABI_VERSION'] = abi_fallback
else:
    env['ABI_VERSION'] = abi
         
#### End Config Stage ####

if env['MISSING_DEPS']:
    # if required dependencies are missing, print warnings and then let SCons finish without building or saving local config
    color_print(1,'\nExiting... the following required dependencies were not found:\n   - %s' % '\n   - '.join(env['MISSING_DEPS']))
    color_print(1,"\nSee the 'config.log' for details on possible problems.")
   
    if env['SKIPPED_DEPS']:
        color_print(4,'\nAlso, these optional dependencies were not found:\n   - %s' % '\n   - '.join(env['SKIPPED_DEPS']))
    
    color_print(4,"\nSet custom paths to these libraries and header files on the command-line or in a file called '%s'" % SCONS_LOCAL_CONFIG)
    
    color_print(4,"    ie. $ python scons/scons.py BOOST_INCLUDES=/usr/local/include/boost-1_37 BOOST_LIBS=/usr/local/lib")
    
    color_print(4, "\nOnce all required dependencies are found a local '%s' will be saved and then install:" % SCONS_LOCAL_CONFIG)

    color_print(4,"    $ sudo python scons/scons.py install")
    
    color_print(4,"\nTo view available path variables:\n    $ python scons/scons.py --help or -h")
    
    color_print(4,'\nTo view overall SCons help options:\n    $ python scons/scons.py --help-options or -H\n')
    
    color_print(4,'More info: http://trac.mapnik.org/wiki/MapnikInstallation')
    
    # Need some way to exit cleanly here...
    # calling Exit() does not work because that will abort the users ability to get help
    env = conf.Finish()
    #Exit()
else:
    # Save the custom variables in a SCONS_LOCAL_CONFIG that will be reloaded to allow for `install` without re-specifying custom variables
    color_print(4,"All Required dependencies found!")
    if env['USE_CONFIG']:
        if os.path.exists(SCONS_LOCAL_CONFIG):
          action = 'Overwriting and re-saving'
          os.unlink(SCONS_LOCAL_CONFIG)    
        # Serialize all user customizations into local config file
        else:
          action = 'Saving new'
        color_print(4,"%s file '%s', to hold successful path variables from commandline and python config file(s)..." % (action,SCONS_LOCAL_CONFIG))
        opts.Save(SCONS_LOCAL_CONFIG,env)
    else:
      color_print(4,"Did not use user config file(s), therefore no local configurations will be saved...")

    Export('env')
    Export('conf')
    
    bindings = [ binding.strip() for binding in Split(env['BINDINGS'])]
    
    #### Build instructions & settings ####
    
    # Build agg first, doesn't need anything special
    if env['INTERNAL_LIBAGG']:
        SConscript('agg/SConscript')
    
    # Build the core library
    SConscript('src/SConscript')
    
    # Build shapeindex and remove its dependency from the LIBS
    if 'boost_program_options%s' % env['BOOST_APPEND'] in env['LIBS']:
        SConscript('utils/shapeindex/SConscript')
        env['LIBS'].remove('boost_program_options%s' % env['BOOST_APPEND'])
    else :
        color_print(1,"WARNING: Cannot find boost_program_options. 'shapeindex' won't be available")

    # Build the requested and able-to-be-compiled input plug-ins
    if requested_plugins:
        color_print(4,'Requested plugins to be built...',newline=False)
    for plugin in requested_plugins:
        details = PLUGINS[plugin]
        if details['lib'] in env['LIBS']:
            SConscript('plugins/input/%s/SConscript' % plugin)
            env['LIBS'].remove(details['lib'])
            color_print(4,'%s' % plugin,newline=False)
        elif not details['lib']:
            # build internal shape and raster plugins
            SConscript('plugins/input/%s/SConscript' % plugin)
            color_print(4,'%s' % plugin,newline=False)
    if requested_plugins:
        print
        
    # Build the Python bindings.
    if 'python' in env['BINDINGS']:
        if not os.access(env['PYTHON'], os.X_OK):
            color_print(1,"Cannot run python interpreter at '%s', make sure that you have the permissions to execute it." % env['PYTHON'])
            Exit(1)
    
        sys_prefix = "%s -c 'import sys; print sys.prefix'" % env['PYTHON']
        env['PYTHON_SYS_PREFIX'] = call(sys_prefix)
        
        # Note: we use the plat_specific argument here to make sure to respect the arch-specific site-packages location
        site_packages = "%s -c 'from distutils.sysconfig import get_python_lib; print get_python_lib(plat_specific=True)'" % env['PYTHON']
        env['PYTHON_SITE_PACKAGES'] = call(site_packages)
        
        sys_version = "%s -c 'from distutils.sysconfig import get_python_version; print get_python_version()'" % env['PYTHON']
        env['PYTHON_VERSION'] = call(sys_version)
        
        py_includes = "%s -c 'from distutils.sysconfig import get_python_inc; print get_python_inc()'" % env['PYTHON']
        env['PYTHON_INCLUDES'] =  call(py_includes)

        # if user-requested custom prefix fall back to manual concatenation for building subdirectories
        py_relative_install = env['LIBDIR_SCHEMA'] + '/python' + env['PYTHON_VERSION'] + '/site-packages/'        
        if env['PYTHON_PREFIX']:
            env['PYTHON_INSTALL_LOCATION'] = env['DESTDIR'] + '/' + env['PYTHON_PREFIX'] + '/' +  py_relative_install
        
        # TODO...
        # When the env['PREFIX'] is changed should that
        # also affect/override where to install python?
        # Perhaps env['PREFIX'] should be None by default
        # while 'usr/local' would be overridden in the code if
        # env['PREFIX'] is set?
        # This way we could more easily check for a 'custom' PREFIX
        
        # code here but disabled...
        #elif not env['PREFIX'] == '/usr/local':
        #    env['PYTHON_INSTALL_LOCATION'] = env['DESTDIR'] + '/' + env['PREFIX'] + '/' +  py_relative_install        
        
        else:
            env['PYTHON_INSTALL_LOCATION'] = env['DESTDIR'] + '/' + env['PYTHON_SITE_PACKAGES']
           
        majver, minver = env['PYTHON_VERSION'].split('.')
    
        if (int(majver), int(minver)) < (2, 2):
            color_print(1,"Python version 2.2 or greater required")
            Exit(1)
    
        color_print(4,'Bindings Python version... %s' % env['PYTHON_VERSION'])

        color_print(4,'Python %s prefix... %s' % (env['PYTHON_VERSION'], env['PYTHON_SYS_PREFIX']))
        
        color_print(4,'Python bindings will install in... %s' % os.path.normpath(env['PYTHON_INSTALL_LOCATION']))
    
        SConscript('bindings/python/SConscript')
        
    env = conf.Finish()
    
    # Common C++ flags.
    if env['THREADING'] == 'multi' :
        common_cxx_flags = '-D%s -DBOOST_SPIRIT_THREADSAFE -DMAPNIK_THREADSAFE ' % env['PLATFORM'].upper()
    else :
        common_cxx_flags = '-D%s ' % env['PLATFORM'].upper()
        
    # Mac OSX (Darwin) special settings
    if env['PLATFORM'] == 'Darwin':
        pthread = ''
        # Getting the macintosh version number, sticking as a compiler macro
        # for Leopard -- needed because different workarounds are needed than
        # for Tiger.
        if platform.mac_ver()[0].startswith('10.5'):
            common_cxx_flags += '-DOSX_LEOPARD '
    else:
        pthread = '-pthread'
    
    # Common debugging flags.
    debug_flags  = '-g -DDEBUG -DMAPNIK_DEBUG'
    ndebug_flags = '-DNDEBUG'
    
    # Customizing the C++ compiler flags depending on: 
    #  (1) the C++ compiler used; and
    #  (2) whether debug binaries are requested.
    if SUNCC:
        if env['DEBUG']:
            env.Append(CXXFLAGS = common_cxx_flags + debug_flags)
        else:
            env.Append(CXXFLAGS = common_cxx_flags + '-O %s' % ndebug_flags)
    else:
        # Common flags for GCC.
        gcc_cxx_flags = '-ansi -Wall %s -ftemplate-depth-100 %s' % (pthread, common_cxx_flags)
    
        if env['DEBUG']:
            env.Append(CXXFLAGS = gcc_cxx_flags + '-O0 -fno-inline %s' % debug_flags)
        else: 
	    env.Append(CXXFLAGS = gcc_cxx_flags + '-O%s -finline-functions -Wno-inline %s' % (env['OPTIMIZATION'],ndebug_flags))

    
    SConscript('fonts/SConscript')
