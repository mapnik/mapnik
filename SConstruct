# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2009 Artem Pavlenko, Jean-Francois Doyon, Dane Springmeyer
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


import os
import sys
import re
import platform
from glob import glob
from subprocess import Popen, PIPE
from SCons.SConf import SetCacheMode
import pickle

try:
    import distutils.sysconfig
    HAS_DISTUTILS = True
except:
    HAS_DISTUTILS = False
                  
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
        color_print(1,'Problem encounted with SCons scripts, please post bug report to: http://trac.mapnik.org\nError was: %s' % stderr)

if platform.uname()[4] == 'x86_64':
    LIBDIR_SCHEMA='lib64' 
elif platform.uname()[4] == 'ppc64':
    LIBDIR_SCHEMA='lib64'
else:
    LIBDIR_SCHEMA='lib'

# local file to hold custom user configuration variables
SCONS_LOCAL_CONFIG = 'config.py'
# local pickled file to cache configured environment
SCONS_CONFIGURE_CACHE = 'config.cache'
# directory SCons uses to stash build tests
SCONF_TEMP_DIR = '.sconf_temp'

# Core plugin build configuration
# opts.AddVariables still hardcoded however...
PLUGINS = { # plugins with external dependencies
            'postgis': {'default':True,'path':'PGSQL','inc':'libpq-fe.h','lib':'pq','lang':'C'},
            'gdal':    {'default':False,'path':None,'inc':'gdal_priv.h','lib':'gdal','lang':'C++'},
            'ogr':     {'default':False,'path':None,'inc':'ogrsf_frmts.h','lib':'gdal','lang':'C++'},
            'occi':    {'default':False,'path':'OCCI','inc':'occi.h','lib':'ociei','lang':'C++'},
            'sqlite':  {'default':False,'path':'SQLITE','inc':'sqlite3.h','lib':'sqlite3','lang':'C'},
            
            # plugins without external dependencies requiring CheckLibWithHeader...
            # note: osm plugin does depend on libxml2
            'osm':     {'default':False,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'shape':   {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'raster':  {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            }

DEFAULT_PLUGINS = []
for k,v in PLUGINS.items():
   if v['default']:
       DEFAULT_PLUGINS.append(k)

#### SCons build options and initial setup ####
color_print(4,'\nWelcome to Mapnik...\n')
env = Environment(ENV=os.environ)

# All of the following options may be modified at the command-line, for example:
# `python scons/scons.py PREFIX=/opt`
opts = Variables()

opts.AddVariables(
    # Compiler options
    ('CXX', 'The C++ compiler to use (defaults to g++).', 'g++'),
    EnumVariable('OPTIMIZATION','Set g++ optimization level','2', ['0','1','2','3']),
    # Note: setting DEBUG=True will override any custom OPTIMIZATION level
    BoolVariable('DEBUG', 'Compile a debug version of Mapnik', 'False'),
    BoolVariable('XML_DEBUG', 'Compile a XML verbose debug version of mapnik', 'False'),
    ListVariable('INPUT_PLUGINS','Input drivers to include',DEFAULT_PLUGINS,PLUGINS.keys()),
    
    # SCons build behavior options
    ('CONFIG', "The path to the python file in which to save user configuration options. Currently : '%s'" % SCONS_LOCAL_CONFIG,SCONS_LOCAL_CONFIG),
    BoolVariable('USE_CONFIG', "Use SCons user '%s' file (will also write variables after successful configuration)", 'True'),
    BoolVariable('FAST', "Make scons faster at the cost of less precise dependency tracking", 'False'),
    
    # Install Variables
    ('PREFIX', 'The install path "prefix"', '/usr/local'),
    ('PYTHON_PREFIX','Custom install path "prefix" for python bindings (default of no prefix)',''),
    ('DESTDIR', 'The root directory to install into. Useful mainly for binary package building', '/'),
    
    # Boost variables
    PathVariable('BOOST_INCLUDES', 'Search path for boost include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('BOOST_LIBS', 'Search path for boost library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    ('BOOST_TOOLKIT','Specify boost toolkit, e.g., gcc41.','',False),
    ('BOOST_ABI', 'Specify boost ABI, e.g., d.','',False),
    ('BOOST_VERSION','Specify boost version, e.g., 1_35.','',False),
    
    # Variables for required dependencies
    ('FREETYPE_CONFIG', 'The path to the freetype-config executable.', 'freetype-config'),
    ('XML2_CONFIG', 'The path to the xml2-config executable.', 'xml2-config'),
    PathVariable('ICU_INCLUDES', 'Search path for ICU include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('ICU_LIBS','Search path for ICU include files','/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('PNG_INCLUDES', 'Search path for libpng include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('PNG_LIBS','Search path for libpng include files','/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('JPEG_INCLUDES', 'Search path for libjpeg include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('JPEG_LIBS', 'Search path for libjpeg library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('TIFF_INCLUDES', 'Search path for libtiff include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('TIFF_LIBS', 'Search path for libtiff library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('PROJ_INCLUDES', 'Search path for PROJ.4 include files', '/usr/local/include', PathVariable.PathAccept),
    PathVariable('PROJ_LIBS', 'Search path for PROJ.4 library files', '/usr/local/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    
    # Variables affecting rendering back-ends
    BoolVariable('INTERNAL_LIBAGG', 'Use provided libagg', 'True'),
    
    # Variables for optional dependencies
    # Note: cairo, cairomm, and pycairo all optional but configured automatically through pkg-config
    # Therefore, we use a single boolean for whether to attempt to build cairo support.
    BoolVariable('CAIRO', 'Attempt to build with Cairo rendering support', 'True'),
    ('GDAL_CONFIG', 'The path to the gdal-config executable for finding gdal and ogr details.', 'gdal-config'),
    ('PG_CONFIG', 'The path to the pg_config executable.', 'pg_config'),
    PathVariable('OCCI_INCLUDES', 'Search path for OCCI include files', '/usr/lib/oracle/10.2.0.3/client/include', PathVariable.PathAccept),
    PathVariable('OCCI_LIBS', 'Search path for OCCI library files', '/usr/lib/oracle/10.2.0.3/client/'+ LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('SQLITE_INCLUDES', 'Search path for SQLITE include files', '/usr/include/', PathVariable.PathAccept),
    PathVariable('SQLITE_LIBS', 'Search path for SQLITE library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    
    # Other variables
    ('SYSTEM_FONTS','Provide location for python bindings to register fonts (if given aborts installation of bundled DejaVu fonts)',''),
    ('LIB_DIR_NAME','Name to use for lib folder where fonts and plugins are installed','/mapnik/'),
    PathVariable('PYTHON','Full path to Python executable used to build bindings', sys.executable),
    BoolVariable('FRAMEWORK_PYTHON', 'Link against Framework Python on Mac OSX', 'True'),
    ListVariable('BINDINGS','Language bindings to build','all',['python']),
    EnumVariable('THREADING','Set threading support','multi', ['multi','single']),
    EnumVariable('XMLPARSER','Set xml parser ','libxml2', ['tinyxml','spirit','libxml2']),
    ('JOBS', 'Set the number of parallel compilations', "1", lambda key, value, env: int(value), int),
    BoolVariable('DEMO', 'Compile demo c++ application', 'False'),
    BoolVariable('PGSQL2SQLITE', 'Compile and install a utility to convert postgres tables to sqlite', 'False'),
    )
# variables to pickle after successful configure step
# these include all scons core variables as well as custom
# env variables needed in Sconscript files
pickle_store = [# Scons internal variables
        'CC',
        'CXX',
        'CCFLAGS',
        'CPPDEFINES',
        'CPPFLAGS',
        'CPPPATH',
        'CXXFLAGS',
        'LIBPATH',
        'LIBS',
        'LINKFLAGS',
        # Mapnik's SConstruct build variables
        'PLUGINS',
        'ABI_VERSION',
        'PLATFORM',
        'BOOST_ABI',
        'BOOST_APPEND',
        'LIBDIR_SCHEMA',
        'REQUESTED_PLUGINS',
        'SUNCC',
        'PYTHON_VERSION',
        'PYTHON_INCLUDES',
        'PYTHON_INSTALL_LOCATION',
        ]

# Add all other user configurable options to pickle pickle_store
# We add here more options than are needed for the build stage
# but helpful so that scons -h shows the exact cached options
for opt in opts.options:
    if opt.key not in pickle_store:
        pickle_store.append(opt.key)

# Method of adding configure behavior to Scons adapted from:
# http://freeorion.svn.sourceforge.net/svnroot/freeorion/trunk/FreeOrion/SConstruct
preconfigured = False
force_configure = False
command_line_args = sys.argv[1:]

if 'configure' in command_line_args:
    force_configure = True
elif ('-h' in command_line_args) or ('--help' in command_line_args):
    preconfigured = True # this is just to ensure config gets skipped when help is requested

# initially populate environment with defaults and any possible custom arguments
opts.Update(env)

# if we are not configuring overwrite environment with pickled settings
if not force_configure:
    if os.path.exists(SCONS_CONFIGURE_CACHE):
        try:
            pickled_environment = open(SCONS_CONFIGURE_CACHE, 'r')
            pickled_values = pickle.load(pickled_environment)
            for key, value in pickled_values.items():
                env[key] = value
            preconfigured = True
        except:
            # unpickling failed, so reconfigure as fallback
            preconfigured = False
    else:
        preconfigured = False

# if custom arguments are supplied make sure to accept them
if opts.args:
    # since we have custom arguments update environment with all opts to 
    # make sure to absorb the custom ones
    opts.Update(env)
    # now since we've got custom arguments we'll disregard any 
    # pickled environment and force another configuration
    preconfigured = False
    if opts.args.get('FAST'):
        # because we are clearing the 'sconf_temp' files each configure when FAST=False
        # we now need to flush the dblite otherwise SCons will skip checks
        # of fail because .sconsign.dblite could be out of sync with cacheing from using
        # or moving to using FAST=True
        try:
            os.unlink('.sconsign.dblite')
        except: pass

elif preconfigured:
    if ('-h' not in command_line_args) and ('--help' not in command_line_args):
        color_print(4,'Using previous successful configuration...')
        color_print(4,'Re-configure by running "python scons/scons.py configure".')




#### Custom Configure Checks ###

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

def parse_config(context, config, checks='--libs --cflags'):
    env = context.env
    tool = config.lower().replace('_','-')
    if config == 'GDAL_CONFIG':
        tool += ' %s' % checks
    context.Message( 'Checking for %s... ' % tool)
    cmd = '%s %s' % (env[config],checks)
    ret = context.TryAction(cmd)[0]
    if ret:
        env.ParseConfig(cmd)
    else:
        if config == 'GDAL_CONFIG':
            env['SKIPPED_DEPS'].append(tool)
            conf.rollback_option('GDAL_CONFIG')
        else:
            env['MISSING_DEPS'].append(tool)        
    context.Result( ret )
    return ret

def get_pkg_lib(context, config, lib):
    libpattern = r'-l([^\s]*)'
    libname = None
    env = context.env
    context.Message( 'Checking for name of %s library... ' % lib)
    cmd = '%s --libs' % env[config]
    ret = context.TryAction(cmd)[0]
    if ret:
        libnames = re.findall(libpattern,call(cmd))
        if libnames:
          libname = libnames[0]
    context.Result( libname )
    return libname

def parse_pg_config(context, config):
    env = context.env
    tool = config.lower()
    context.Message( 'Checking for %s... ' % tool)
    ret = context.TryAction(env[config])[0]
    if ret:
        lib_path = call('%s --libdir' % env[config])
        inc_path = call('%s --includedir' % env[config])
        env.AppendUnique(CPPPATH = inc_path)
        env.AppendUnique(LIBPATH = lib_path)
        lpq = env['PLUGINS']['postgis']['lib']
        env.Append(LIBS = lpq)
    else:
        env['SKIPPED_DEPS'].append(tool)
        conf.rollback_option(config)
    context.Result( ret )
    return ret

def ogr_enabled(context):
    env = context.env
    context.Message( 'Checking if gdal is ogr enabled... ')
    ret = context.TryAction('%s --ogr-enabled' % env['GDAL_CONFIG'])[0]
    if not ret:
        env['SKIPPED_DEPS'].append('ogr')
    context.Result( ret )
    return ret

def rollback_option(context,variable):
    global opts
    env = context.env
    for item in opts.options:
        if item.key == variable:
            env[variable] = item.default

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

conf_tests = { 'CheckPKGConfig' : CheckPKGConfig,
               'CheckPKG' : CheckPKG,
               'CheckBoost' : CheckBoost,
               'GetBoostLibVersion' : GetBoostLibVersion,
               'GetMapnikLibVersion' : GetMapnikLibVersion,
               'parse_config' : parse_config,
               'parse_pg_config' : parse_pg_config,
               'ogr_enabled': ogr_enabled,
               'get_pkg_lib': get_pkg_lib,
               'rollback_option': rollback_option
               }


if not preconfigured:

    color_print(4,'Configuring build environment...')

    if env['USE_CONFIG']:
        if not env['CONFIG'].endswith('.py'):
            color_print(1,'SCons CONFIG file specified is not a python file, will not be read...')
        else:
            # Accept more than one file as comma-delimited list
            user_confs = env['CONFIG'].split(',')
            # If they exist add the files to the existing `opts`
            for conf in user_confs:
                if os.path.exists(conf):
                    opts.files.append(conf)
                    color_print(4,"SCons CONFIG found: '%s', variables will be inherited..." % conf)
                    optfile = file(conf)
                    print optfile.read().replace("\n", " ").replace("'","").replace(" = ","=")
                    optfile.close()
                    
                elif not conf == SCONS_LOCAL_CONFIG:
                    # if default missing, no worries
                    # but if the default is overridden and the file is not found, give warning
                    color_print(1,"SCons CONFIG not found: '%s'" % conf)
            # Recreate the base environment using modified `opts`
            env = Environment(ENV=os.environ,options=opts)
            env['USE_CONFIG'] = True
    else:
        color_print(4,'SCons USE_CONFIG specified as false, will not inherit variables python config file...')        

    conf = Configure(env, custom_tests = conf_tests)
    
    if env['DEBUG']:
        mode = 'debug mode'
    else:
        mode = 'release mode'

    if env['XML_DEBUG']:
        mode += ' (with XML debug on)'
        
    env['PLATFORM'] = platform.uname()[0]
    color_print (4,"Configuring on %s in *%s*..." % (env['PLATFORM'],mode))

    env['MISSING_DEPS'] = []
    env['SKIPPED_DEPS'] = []
    
    env['LIBDIR_SCHEMA'] = LIBDIR_SCHEMA
    env['PLUGINS'] = PLUGINS
    
    if env['FAST']:
        # caching is 'auto' by default in SCons
        # But let's also cache implicit deps...
        EnsureSConsVersion(0,98)
        SetOption('implicit_cache', 1)
        env.Decider('MD5-timestamp')
        SetOption('max_drift', 1)
        
    else:
        # Set the cache mode to 'force' unless requested, avoiding hidden caching of Scons 'opts' in '.sconsign.dblite'
        # This allows for a SCONS_LOCAL_CONFIG, if present, to be used as the primary means of storing paths to successful build dependencies
        SetCacheMode('force')
    
    if env['JOBS'] > 1:
        SetOption("num_jobs", env['JOBS'])  
    
    thread_suffix = 'mt'
    if env['PLATFORM'] == 'FreeBSD':
        thread_suffix = ''
        env.Append(LIBS = 'pthread')
    
    if env['SYSTEM_FONTS']:
        if not os.path.isdir(env['SYSTEM_FONTS']):
            color_print(1,'Warning: Directory specified for SYSTEM_FONTS does not exist!')
    #### Libraries and headers dependency checks ####
    
    # Set up for libraries and headers dependency checks
    env['CPPPATH'] = ['#include', '#']
    env['LIBPATH'] = ['#src']
    
    # Solaris & Sun Studio settings (the `SUNCC` flag will only be
    # set if the `CXX` option begins with `CC`)
    SOLARIS = env['PLATFORM'] == 'SunOS'
    env['SUNCC'] = SOLARIS and env['CXX'].startswith('CC')
    
    # For Solaris include paths (e.g., for freetype2, ltdl, etc.).
    if SOLARIS:
        blastwave_dir = '/opt/csw/%s'
        env.AppendUnique(CPPPATH = blastwave_dir % 'include')
        env.AppendUnique(LIBPATH = blastwave_dir % LIBDIR_SCHEMA)
    
    # If the Sun Studio C++ compiler (`CC`) is used instead of GCC.
    if env['SUNCC']:
        env['CC'] = 'cc'
        # To be compatible w/Boost everything needs to be compiled
        # with the `-library=stlport4` flag (which needs to come
        # before the `-o` flag).
        env['CXX'] = 'CC -library=stlport4'
        if env['THREADING'] == 'multi':
            env['CXXFLAGS'] = ['-mt']
    
    # Adding the required prerequisite library directories to the include path for
    # compiling and the library path for linking, respectively.
    for required in ('BOOST', 'PNG', 'JPEG', 'TIFF','PROJ','ICU'):
        inc_path = env['%s_INCLUDES' % required]
        lib_path = env['%s_LIBS' % required]
        env.AppendUnique(CPPPATH = inc_path)
        env.AppendUnique(LIBPATH = lib_path)

    conf.parse_config('FREETYPE_CONFIG')

    if env['XMLPARSER'] == 'tinyxml':
        env['CPPPATH'].append('#tinyxml')
        env.Append(CXXFLAGS = '-DBOOST_PROPERTY_TREE_XML_PARSER_TINYXML -DTIXML_USE_STL')
    elif env['XMLPARSER'] == 'libxml2':
        if conf.parse_config('XML2_CONFIG'):
            env.Append(CXXFLAGS = '-DHAVE_LIBXML2')
            
    if env['CAIRO'] and conf.CheckPKGConfig('0.15.0') and conf.CheckPKG('cairomm-1.0'):
        env.ParseConfig('pkg-config --libs --cflags cairomm-1.0')
        env.Append(CXXFLAGS = '-DHAVE_CAIRO')
    else:
        env['SKIPPED_DEPS'].extend(['cairo','cairomm'])

    LIBSHEADERS = [
        ['m', 'math.h', True,'C'],
        ['ltdl', 'ltdl.h', True,'C'],
        ['png', 'png.h', True,'C'],
        ['tiff', 'tiff.h', True,'C'],
        ['z', 'zlib.h', True,'C'],
        ['jpeg', ['stdio.h', 'jpeglib.h'], True,'C'],
        ['proj', 'proj_api.h', True,'C'],
        ['icuuc','unicode/unistr.h',True,'C++'],
        ['icudata','unicode/utypes.h' , True,'C++'],
    ]
    
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
    
    for libinfo in LIBSHEADERS:
        if not conf.CheckLibWithHeader(libinfo[0], libinfo[1], libinfo[3]):
            if libinfo[2]:
                color_print (1,'Could not find required header or shared library for %s' % libinfo[0])
                env['MISSING_DEPS'].append(libinfo[0])
            else:
                color_print(4,'Could not find optional header or shared library for %s' % libinfo[0])
                env['SKIPPED_DEPS'].append(libinfo[0])            
    
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
        if not conf.CheckLibWithHeader('boost_%s%s' % (libinfo[0],env['BOOST_APPEND']), libinfo[1], 'C++'):
            if libinfo[2]:
                color_print(1,'Could not find required header or shared library for boost %s' % libinfo[0])
                env['MISSING_DEPS'].append('boost ' + libinfo[0])
            else:
                color_print(4,'Could not find optional header or shared library for boost %s' % libinfo[0])
                env['SKIPPED_DEPS'].append('boost ' + libinfo[0])
    
    env['REQUESTED_PLUGINS'] = [ driver.strip() for driver in Split(env['INPUT_PLUGINS'])]
    
    color_print(4,'Checking for requested plugins dependencies...')
    for plugin in env['REQUESTED_PLUGINS']:
        details = env['PLUGINS'][plugin]
        if plugin == 'gdal':
            if conf.parse_config('GDAL_CONFIG',checks='--libs'):
                conf.parse_config('GDAL_CONFIG',checks='--cflags')
                libname = conf.get_pkg_lib('GDAL_CONFIG','gdal')
                if libname:
                    details['lib'] = libname
        elif plugin == 'postgis':
            conf.parse_pg_config('PG_CONFIG')
        elif plugin == 'ogr':
            if conf.ogr_enabled():
                if not 'gdal' in env['REQUESTED_PLUGINS']:
                    conf.parse_config('GDAL_CONFIG',checks='--libs')
                    conf.parse_config('GDAL_CONFIG',checks='--cflags')
                libname = conf.get_pkg_lib('GDAL_CONFIG','ogr')
                if libname:
                    details['lib'] = libname
                
        elif details['path'] and details['lib'] and details['inc']:
            backup = env.Clone().Dictionary()
            # Note, the 'delete_existing' keyword makes sure that these paths are prepended
            # to the beginning of the path list even if they already exist
            env.PrependUnique(CPPPATH = env['%s_INCLUDES' % details['path']],delete_existing=True)
            env.PrependUnique(LIBPATH = env['%s_LIBS' % details['path']],delete_existing=True)
            if not conf.CheckLibWithHeader(details['lib'], details['inc'], details['lang']):
                env.Replace(**backup)
                env['SKIPPED_DEPS'].append(details['lib'])

    # re-append the local paths for mapnik sources to the beginning of the list
    # to make sure they come before any plugins that were 'prepended'
    env.PrependUnique(CPPPATH = ['#include', '#'], delete_existing=True)
    env.PrependUnique(LIBPATH = '#src', delete_existing=True)
    
    # Decide which libagg to use
    # if we are using internal agg, then prepend to make sure
    # we link locally
    if env['INTERNAL_LIBAGG']:
        env.Prepend(CPPPATH = '#agg/include')
        env.Prepend(LIBPATH = '#agg')
    else:
        env.ParseConfig('pkg-config --libs --cflags libagg')
    
    if 'python' in env['BINDINGS']:
        # checklibwithheader does not work for boost_python since we can't feed it
        # multiple header files, so we fall back on a simple check for boost_python headers
        if not conf.CheckHeader(header='boost/python/detail/config.hpp',language='C++'):
            color_print(1,'Could not find required header files for boost python')
            env['MISSING_DEPS'].append('boost python')

        if env['CAIRO'] and conf.CheckPKGConfig('0.15.0') and conf.CheckPKG('pycairo'):
            env.ParseConfig('pkg-config --cflags pycairo')
            env.Append(CXXFLAGS = '-DHAVE_PYCAIRO')
        else:
            env['SKIPPED_DEPS'].extend(['pycairo'])
             
    #### End Config Stage for Required Dependencies ####
    
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
        Exit(1)
    else:
        # Save the custom variables in a SCONS_LOCAL_CONFIG
        # that will be reloaded to allow for `install` without re-specifying custom variables
        color_print(4,"\nAll Required dependencies found!\n")
        if env['USE_CONFIG']:
            if os.path.exists(SCONS_LOCAL_CONFIG):
                action = 'Overwriting and re-saving'
                os.unlink(SCONS_LOCAL_CONFIG)    
            else:
                action = 'Saving new'
            color_print(4,"%s file '%s'..." % (action,SCONS_LOCAL_CONFIG))
            color_print(4,"Will hold custom path variables from commandline and python config file(s)...")
            opts.Save(SCONS_LOCAL_CONFIG,env)
        else:
          color_print(4,"Did not use user config file, no custom path variables will be saved...")

        if env['SKIPPED_DEPS']:
            color_print(1,'\nHowever, these optional dependencies were not found:\n   - %s' % '\n   - '.join(env['SKIPPED_DEPS']))
            print

        # fetch the mapnik version header in order to set the
        # ABI version used to build libmapnik.so on linux in src/SConscript
        abi = conf.GetMapnikLibVersion()
        abi_fallback = [0,6,0]
        if not abi:
            color_print(1,'Problem encountered parsing mapnik version, falling back to %s' % abi_fallback)
            env['ABI_VERSION'] = abi_fallback
        else:
            env['ABI_VERSION'] = abi

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
        
        if env['XML_DEBUG']: 
            common_cxx_flags += '-DMAPNIK_XML_DEBUG '
        
        # Customizing the C++ compiler flags depending on: 
        #  (1) the C++ compiler used; and
        #  (2) whether debug binaries are requested.
        if env['SUNCC']:
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

        if 'python' in env['BINDINGS']:
            if not os.access(env['PYTHON'], os.X_OK):
                color_print(1,"Cannot run python interpreter at '%s', make sure that you have the permissions to execute it." % env['PYTHON'])
                Exit(1)
        
            sys_prefix = "%s -c 'import sys; print sys.prefix'" % env['PYTHON']
            env['PYTHON_SYS_PREFIX'] = call(sys_prefix)
            
            if HAS_DISTUTILS:                        
                sys_version = "%s -c 'from distutils.sysconfig import get_python_version; print get_python_version()'" % env['PYTHON']
                env['PYTHON_VERSION'] = call(sys_version)
                py_includes = "%s -c 'from distutils.sysconfig import get_python_inc; print get_python_inc()'" % env['PYTHON']
                env['PYTHON_INCLUDES'] = call(py_includes)
                # Note: we use the plat_specific argument here to make sure to respect the arch-specific site-packages location
                site_packages = "%s -c 'from distutils.sysconfig import get_python_lib; print get_python_lib(plat_specific=True)'" % env['PYTHON']
                env['PYTHON_SITE_PACKAGES'] = call(site_packages)
            else:
                env['PYTHON_SYS_PREFIX'] = os.popen("%s -c 'import sys; print sys.prefix'" % env['PYTHON']).read().strip()
                env['PYTHON_VERSION'] = os.popen("%s -c 'import sys; print sys.version'" % env['PYTHON']).read()[0:3]
                env['PYTHON_INCLUDES'] = env['PYTHON_SYS_PREFIX'] + '/include/python' + env['PYTHON_VERSION']
                env['PYTHON_SITE_PACKAGES'] = env['DESTDIR'] + '/' + env['PYTHON_SYS_PREFIX'] + '/' + env['LIBDIR_SCHEMA'] + '/python' + env['PYTHON_VERSION'] + '/site-packages/'
        
            # if user-requested custom prefix fall back to manual concatenation for building subdirectories       
            if env['PYTHON_PREFIX']:
                py_relative_install = env['LIBDIR_SCHEMA'] + '/python' + env['PYTHON_VERSION'] + '/site-packages/' 
                env['PYTHON_INSTALL_LOCATION'] = env['DESTDIR'] + '/' + env['PYTHON_PREFIX'] + '/' +  py_relative_install            
            else:
                env['PYTHON_INSTALL_LOCATION'] = env['DESTDIR'] + '/' + env['PYTHON_SITE_PACKAGES']
               
            majver, minver = env['PYTHON_VERSION'].split('.')
        
            if (int(majver), int(minver)) < (2, 2):
                color_print(1,"Python version 2.2 or greater required")
                Exit(1)
        
            color_print(4,'Bindings Python version... %s' % env['PYTHON_VERSION'])
            color_print(4,'Python %s prefix... %s' % (env['PYTHON_VERSION'], env['PYTHON_SYS_PREFIX']))
            color_print(4,'Python bindings will install in... %s' % os.path.normpath(env['PYTHON_INSTALL_LOCATION']))

        # finish config stage and pickle results
        env = conf.Finish()
        env_cache = open(SCONS_CONFIGURE_CACHE, 'w')
        pickle_dict = {}
        for i in pickle_store:
            pickle_dict[i] = env.get(i)
        pickle.dump(pickle_dict,env_cache)
        env_cache.close()
        # fix up permissions on configure outputs
        try:
            os.chmod(SCONS_CONFIGURE_CACHE,0666)
        except: pass
        try:
            os.chmod(SCONS_LOCAL_CONFIG,0666)
        except: pass
        try:
            os.chmod('.sconsign.dblite',0666)
        except: pass

        # clean up test build targets
        if not env['FAST']:
            try:
                for test in glob('%s/*' % SCONF_TEMP_DIR):
                    os.unlink(test)
            except: pass
        if 'configure' in command_line_args:
            color_print(4,'\n*Configure complete*\nNow run "python scons/scons.py" to build or "python scons/scons.py install" to install')
            Exit(0)

# autogenerate help on default/current SCons options
Help(opts.GenerateHelpText(env))

#### Builds ####

# export env so it is available in Sconscript files
Export('env')

# Build agg first, doesn't need anything special
if env['INTERNAL_LIBAGG']:
    SConscript('agg/SConscript')

# Build the core library
SConscript('src/SConscript')

# Build the c++ rundemo app if requested
if env['DEMO']:
    SConscript('demo/c++/SConscript')

# Build the pgsql2psqlite app if requested
if env['PGSQL2SQLITE']:
    SConscript('utils/pgsql2sqlite/SConscript')

# Build shapeindex and remove its dependency from the LIBS
if 'boost_program_options%s' % env['BOOST_APPEND'] in env['LIBS']:
    SConscript('utils/shapeindex/SConscript')
    env['LIBS'].remove('boost_program_options%s' % env['BOOST_APPEND'])
else :
    color_print(1,"WARNING: Cannot find boost_program_options. 'shapeindex' won't be available")

GDAL_BUILT = False
OGR_BUILT = False
# Build the requested and able-to-be-compiled input plug-ins
for plugin in env['REQUESTED_PLUGINS']:
    details = env['PLUGINS'][plugin]
    if details['lib'] in env['LIBS']:
        SConscript('plugins/input/%s/SConscript' % plugin)
        if plugin == 'ogr': OGR_BUILT = True
        if plugin == 'gdal': GDAL_BUILT = True
        if plugin == 'ogr' or plugin == 'gdal':
            if GDAL_BUILT and OGR_BUILT:
                env['LIBS'].remove(details['lib'])
        else:
            env['LIBS'].remove(details['lib'])
    elif not details['lib']:
        # build internal shape and raster plugins
        SConscript('plugins/input/%s/SConscript' % plugin)
    
# Build the Python bindings
if 'python' in env['BINDINGS']:
    SConscript('bindings/python/SConscript')

# Configure fonts and if requested install the bundled DejaVu fonts
SConscript('fonts/SConscript')