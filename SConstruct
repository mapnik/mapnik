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

if platform.uname()[4] == 'ppc64':
    LIBDIR_SCHEMA='lib64'
else:
    LIBDIR_SCHEMA='lib'

py3 = None

# local file to hold custom user configuration variables
# Todo check timestamp, reload if changed?
SCONS_LOCAL_CONFIG = 'config.py'
# build log
SCONS_LOCAL_LOG = 'config.log'
# local pickled file to cache configured environment
SCONS_CONFIGURE_CACHE = 'config.cache'
# directory SCons uses to stash build tests
SCONF_TEMP_DIR = '.sconf_temp'
# auto-search directories for boost libs/headers
BOOST_SEARCH_PREFIXES = ['/usr/local','/opt/local','/sw','/usr',]
BOOST_MIN_VERSION = '1.47'
CAIROMM_MIN_VERSION = '1.8.0'

DEFAULT_LINK_PRIORITY = ['internal','other','frameworks','user','system']


pretty_dep_names = {
    'ociei':'Oracle database library | configure with OCCI_LIBS & OCCI_INCLUDES | more info: http://trac.mapnik.org/wiki/OCCI',
    'gdal':'GDAL C++ library | configured using gdal-config program | try setting GDAL_CONFIG SCons option | more info: http://trac.mapnik.org/wiki/GDAL',
    'ogr':'OGR-enabled GDAL C++ Library | configured using gdal-config program | try setting GDAL_CONFIG SCons option | more info: http://trac.mapnik.org/wiki/OGR',
    'geos_c':'GEOS Simple Geometry Specification C Library | configured with GEOS_LIB & GEOS_INCLUDE | more info: http://trac.mapnik.org/wiki/GEOS',
    'cairo':'Cairo C library | configured using pkg-config | try setting PKG_CONFIG_PATH SCons option',
    'cairomm':'Cairomm C++ bindings to Cairo library | configured using pkg-config | try setting PKG_CONFIG_PATH SCons option',
    'cairomm-version':'Cairomm version is too old (so cairo renderer will not be built), you need at least %s' % CAIROMM_MIN_VERSION,
    'pycairo':'Python bindings to Cairo library | configured using pkg-config | try setting PKG_CONFIG_PATH SCons option',
    'proj':'Proj.4 C Projections library | configure with PROJ_LIBS & PROJ_INCLUDES | more info: http://trac.osgeo.org/proj/',
    'pg':'Postgres C Library requiered for PostGIS plugin | configure with pg_config program | more info: http://trac.mapnik.org/wiki/PostGIS',
    'sqlite3':'SQLite3 C Library | configure with SQLITE_LIBS & SQLITE_INCLUDES | more info: http://trac.mapnik.org/wiki/SQLite',
    'jpeg':'JPEG C library | configure with JPEG_LIBS & JPEG_INCLUDES',
    'tiff':'TIFF C library | configure with TIFF_LIBS & TIFF_INCLUDES',
    'png':'PNG C library | configure with PNG_LIBS & PNG_INCLUDES',
    'icuuc':'ICU C++ library | configure with ICU_LIBS & ICU_INCLUDES or use ICU_LIB_NAME to specify custom lib name  | more info: http://site.icu-project.org/',
    'ltdl':'GNU Libtool | more info: http://www.gnu.org/software/libtool',
    'z':'Z compression library | more info: http://www.zlib.net/',
    'm':'Basic math library, part of C++ stlib',
    'pkg-config':'pkg-config tool | more info: http://pkg-config.freedesktop.org',
    'pg_config':'pg_config program | try setting PG_CONFIG SCons option',
    'xml2-config':'xml2-config program | try setting XML2_CONFIG SCons option',
    'gdal-config':'gdal-config program | try setting GDAL_CONFIG SCons option',
    'geos-config':'geos-config program | try setting GEOS_CONFIG SCons option',
    'freetype-config':'freetype-config program | try setting FREETYPE_CONFIG SCons option',
    'osm':'more info: http://trac.mapnik.org/wiki/OsmPlugin',
    'curl':'libcurl is required for the "osm" plugin - more info: http://trac.mapnik.org/wiki/OsmPlugin',
    'boost_regex_icu':'libboost_regex built with optional ICU unicode support is needed for unicode regex support in mapnik.',
    'sqlite_rtree':'The SQLite plugin requires libsqlite3 built with RTREE support (-DSQLITE_ENABLE_RTREE=1)',
    'pgsql2sqlite_rtree':'The pgsql2sqlite program requires libsqlite3 built with RTREE support (-DSQLITE_ENABLE_RTREE=1)'
    }

# Core plugin build configuration
# opts.AddVariables still hardcoded however...
PLUGINS = { # plugins with external dependencies
            # configured by calling project, hence 'path':None
            'postgis': {'default':True,'path':None,'inc':'libpq-fe.h','lib':'pq','lang':'C'},
            'gdal':    {'default':True,'path':None,'inc':'gdal_priv.h','lib':'gdal','lang':'C++'},
            'ogr':     {'default':True,'path':None,'inc':'ogrsf_frmts.h','lib':'gdal','lang':'C++'},
            'geos':    {'default':False,'path':None,'inc':'geos_c.h','lib':'geos_c','lang':'C'},
            # configured with custom paths, hence 'path': PREFIX/INCLUDES/LIBS
            'occi':    {'default':False,'path':'OCCI','inc':'occi.h','lib':'ociei','lang':'C++'},
            'sqlite':  {'default':True,'path':'SQLITE','inc':'sqlite3.h','lib':'sqlite3','lang':'C'},
            'rasterlite':  {'default':False,'path':'RASTERLITE','inc':['sqlite3.h','rasterlite.h'],'lib':'rasterlite','lang':'C'},

            # todo: osm plugin does also depend on libxml2 (but there is a separate check for that)
            'osm':     {'default':True,'path':None,'inc':'curl/curl.h','lib':'curl','lang':'C'},

            # plugins without external dependencies requiring CheckLibWithHeader...
            'shape':   {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'csv':     {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'raster':  {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'geojson': {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'kismet':  {'default':False,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'python':  {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            }


#### SCons build options and initial setup ####
env = Environment(ENV=os.environ)
env.Decider('MD5-timestamp')
env.SourceCode(".", None)

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

def regular_print(color,text,newline=True):
    if not newline:
        print text,
    else:
        print text

def call(cmd, silent=False):
    stdin, stderr = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        return stdin.strip()
    elif not silent:
        color_print(1,'Problem encounted with SCons scripts, please post bug report to: http://trac.mapnik.org\nError was: %s' % stderr)

def strip_first(string,find,replace=''):
    if string.startswith(find):
        return string.replace(find,replace,1)
    return string

# http://www.scons.org/wiki/InstallTargets
def create_uninstall_target(env, path, is_glob=False):
    if 'uninstall' in COMMAND_LINE_TARGETS:
        if is_glob:
            all_files = Glob(path,strings=True)
            for filei in all_files:
                env.Command( "uninstall-"+filei, filei,
                [
                Delete("$SOURCE"),
                ])
                env.Alias("uninstall", "uninstall-"+filei)
        else:
            if os.path.exists(path):
                env.Command( "uninstall-"+path, path,
                [
                Delete("$SOURCE"),
                ])
                env.Alias("uninstall", "uninstall-"+path)

def shortest_name(libs):
    name = '-'*200
    for lib in libs:
        if len(name) > len(lib):
            name = lib
    return name


def sort_paths(items,priority):
    """Sort paths such that compiling and linking will globally prefer custom or local libs
    over system libraries by fixing up the order libs are passed to gcc and the linker.

    Ideally preference could be by-target instead of global, but our SCons implementation
    is not currently utilizing different SCons build env()'s as we should.

    Overally the current approach within these scripts is to prepend paths of preference
    and append all others, but this does not give enough control (particularly due to the
    approach of assuming /usr/LIBSCHEMA and letting paths be parsed and added by pkg-config).

    In effect /usr/lib is likely to come before /usr/local/lib which makes linking against
    custom built icu or boost impossible when those libraries are available in both places.

    Sorting using a priority list allows this to be controlled, and fine tuned.
    """

    new = []
    path_types = {'internal':[],'other':[],'frameworks':[],'user':[],'system':[]}
    # parse types of paths into logical/meaningful groups
    # based on commonly encountered lib directories on linux and osx
    for i in items:
        # internal paths for code kept inside
        # the mapnik sources
        if i.startswith('#'):
            path_types['internal'].append(i)
        # Mac OS X user installed frameworks
        elif '/Library/Frameworks' in i:
            path_types['frameworks'].append(i)
        # various 'local' installs like /usr/local or /opt/local
        elif 'local' in i or '/sw' in i:
            if '/usr/local' in i:
                path_types['user'].insert(0,i)
            else:
                path_types['user'].append(i)
        # key system libs (likely others will fall into 'other')
        elif '/usr/' in i or '/System' in i or i.startswith('/lib'):
            path_types['system'].append(i)
        # anything not yet matched...
        # likely a combo of rare system lib paths and
        # very custom user paths that should ideally be
        # in 'user'
        else:
            path_types['other'].append(i)
    # build up new list based on priority list
    for path in priority:
        if path_types.has_key(path):
            dirs = path_types[path]
            new.extend(dirs)
            path_types.pop(path)
        else:
            color_print(1,'\nSorry, "%s" is NOT a valid value for option "LINK_PRIORITY": values include: %s' % (path,','.join(path_types.keys())))
            color_print(1,'\tinternal: the local directory of the Mapnik sources (prefix #) (eg. used to link internal agg)')
            color_print(1,'\tframeworks: on osx the /Library/Frameworks directory')
            color_print(1,'\tuser: any path with "local" or "/sw" inside it')
            color_print(1,'\tsystem: any path not yet matched with "/usr/","/lib", or "/System" (osx) inside it')
            color_print(1,'\tother: any paths you specified not matched by criteria used to parse the others')
            color_print(1,'\tother: any paths you specified not matched by criteria used to parse the others')
            color_print(1,'The Default priority is: %s' % ','.join(DEFAULT_LINK_PRIORITY))
            color_print(1,'Any priority groups not listed will be appended to the list at the end')
            Exit(1)
    # append remaining paths potentially not requested
    # by any custom priority list defined by user
    for k,v in path_types.items():
        new.extend(v)
    return new

def pretty_dep(dep):
    pretty = pretty_dep_names.get(dep)
    if pretty:
        return '%s (%s)' % (dep,pretty)
    elif 'boost' in dep:
        return '%s (%s)' % (dep,'more info see: http://trac.mapnik.org/wiki/MapnikInstallation & http://www.boost.org')
    return dep


DEFAULT_PLUGINS = []
for k,v in PLUGINS.items():
   if v['default']:
       DEFAULT_PLUGINS.append(k)

# All of the following options may be modified at the command-line, for example:
# `python scons/scons.py PREFIX=/opt`
opts = Variables()

opts.AddVariables(
    # Compiler options
    ('CXX', 'The C++ compiler to use to compile mapnik (defaults to g++).', 'g++'),
    ('CC', 'The C compiler used for configure checks of C libs (defaults to gcc).', 'gcc'),
    ('CUSTOM_CXXFLAGS', 'Custom C++ flags, e.g. -I<include dir> if you have headers in a nonstandard directory <include dir>', ''),
    ('CUSTOM_CFLAGS', 'Custom C flags, e.g. -I<include dir> if you have headers in a nonstandard directory <include dir> (only used for configure checks)', ''),
    ('CUSTOM_LDFLAGS', 'Custom linker flags, e.g. -L<lib dir> if you have libraries in a nonstandard directory <lib dir>', ''),
    EnumVariable('LINKING', "Set library format for libmapnik",'shared', ['shared','static']),
    EnumVariable('RUNTIME_LINK', "Set preference for linking dependencies",'shared', ['shared','static']),
    EnumVariable('OPTIMIZATION','Set g++ optimization level','3', ['0','1','2','3','4','s']),
    # Note: setting DEBUG=True will override any custom OPTIMIZATION level
    BoolVariable('DEBUG', 'Compile a debug version of Mapnik', 'False'),
    BoolVariable('DEBUG_UNDEFINED', 'Compile a version of Mapnik using clang/llvm undefined behavior asserts', 'False'),
    ListVariable('INPUT_PLUGINS','Input drivers to include',DEFAULT_PLUGINS,PLUGINS.keys()),
    ('WARNING_CXXFLAGS', 'Compiler flags you can set to reduce warning levels which are placed after -Wall.', ''),

    # SCons build behavior options
    ('CONFIG', "The path to the python file in which to save user configuration options. Currently : '%s'" % SCONS_LOCAL_CONFIG,SCONS_LOCAL_CONFIG),
    BoolVariable('USE_CONFIG', "Use SCons user '%s' file (will also write variables after successful configuration)", 'True'),
    # http://www.scons.org/wiki/GoFastButton
    # http://stackoverflow.com/questions/1318863/how-to-optimize-an-scons-script
    BoolVariable('FAST', "Make SCons faster at the cost of less precise dependency tracking", 'False'),
    BoolVariable('PRIORITIZE_LINKING', 'Sort list of lib and inc directories to ensure preferential compiling and linking (useful when duplicate libs)', 'True'),
    ('LINK_PRIORITY','Priority list in which to sort library and include paths (default order is internal, other, frameworks, user, then system - see source of `sort_paths` function for more detail)',','.join(DEFAULT_LINK_PRIORITY)),

    # Install Variables
    ('PREFIX', 'The install path "prefix"', '/usr/local'),
    ('PYTHON_PREFIX','Custom install path "prefix" for python bindings (default of no prefix)',''),
    ('DESTDIR', 'The root directory to install into. Useful mainly for binary package building', '/'),
    ('PATH', 'A custom path (or multiple paths divided by ":") to append to the $PATH env to prioritize usage of command line programs (if multiple are present on the system)', ''),
    ('PATH_REMOVE', 'A path prefix to exclude from all known command and compile paths', ''),
    ('PATH_REPLACE', 'Two path prefixes (divided with a :) to search/replace from all known command and compile paths', ''),

    # Boost variables
    # default is '/usr/include', see FindBoost method below
    ('BOOST_INCLUDES', 'Search path for boost include files', '',False),
    # default is '/usr/' + LIBDIR_SCHEMA, see FindBoost method below
    ('BOOST_LIBS', 'Search path for boost library files', '',False),
    ('BOOST_TOOLKIT','Specify boost toolkit, e.g., gcc41.','',False),
    ('BOOST_ABI', 'Specify boost ABI, e.g., d.','',False),
    ('BOOST_VERSION','Specify boost version, e.g., 1_35.','',False),
    ('BOOST_PYTHON_LIB','Specify library name to specific Boost Python lib (e.g. "boost_python-py26")','boost_python'),

    # Variables for required dependencies
    ('FREETYPE_CONFIG', 'The path to the freetype-config executable.', 'freetype-config'),
    ('XML2_CONFIG', 'The path to the xml2-config executable.', 'xml2-config'),
    PathVariable('ICU_INCLUDES', 'Search path for ICU include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('ICU_LIBS','Search path for ICU include files','/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    ('ICU_LIB_NAME', 'The library name for icu (such as icuuc, sicuuc, or icucore)', 'icuuc'),
    PathVariable('PNG_INCLUDES', 'Search path for libpng include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('PNG_LIBS','Search path for libpng include files','/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    BoolVariable('JPEG', 'Build Mapnik with JPEG read and write support', 'True'),
    PathVariable('JPEG_INCLUDES', 'Search path for libjpeg include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('JPEG_LIBS', 'Search path for libjpeg library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('TIFF_INCLUDES', 'Search path for libtiff include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('TIFF_LIBS', 'Search path for libtiff library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('PROJ_INCLUDES', 'Search path for PROJ.4 include files', '/usr/local/include', PathVariable.PathAccept),
    PathVariable('PROJ_LIBS', 'Search path for PROJ.4 library files', '/usr/local/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    ('PKG_CONFIG_PATH', 'Use this path to point pkg-config to .pc files instead of the PKG_CONFIG_PATH environment setting',''),

    # Variables affecting rendering back-ends

    BoolVariable('RENDERING_STATS', 'Output rendering statistics during style processing', 'False'),

    BoolVariable('SVG_RENDERER', 'build support for native svg renderer', 'False'),

    # Variables for optional dependencies
    ('GEOS_CONFIG', 'The path to the geos-config executable.', 'geos-config'),
    # Note: cairo, cairomm, and pycairo all optional but configured automatically through pkg-config
    # Therefore, we use a single boolean for whether to attempt to build cairo support.
    BoolVariable('CAIRO', 'Attempt to build with Cairo rendering support', 'True'),
    PathVariable('CAIRO_INCLUDES', 'Search path for cairo/cairomm include files', '',PathVariable.PathAccept),
    PathVariable('CAIRO_LIBS', 'Search path for cairo/cairomm library files','',PathVariable.PathAccept),
    ('GDAL_CONFIG', 'The path to the gdal-config executable for finding gdal and ogr details.', 'gdal-config'),
    ('PG_CONFIG', 'The path to the pg_config executable.', 'pg_config'),
    PathVariable('OCCI_INCLUDES', 'Search path for OCCI include files', '/usr/lib/oracle/10.2.0.3/client/include', PathVariable.PathAccept),
    PathVariable('OCCI_LIBS', 'Search path for OCCI library files', '/usr/lib/oracle/10.2.0.3/client/'+ LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('SQLITE_INCLUDES', 'Search path for SQLITE include files', '/usr/include/', PathVariable.PathAccept),
    PathVariable('SQLITE_LIBS', 'Search path for SQLITE library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),
    PathVariable('RASTERLITE_INCLUDES', 'Search path for RASTERLITE include files', '/usr/include/', PathVariable.PathAccept),
    PathVariable('RASTERLITE_LIBS', 'Search path for RASTERLITE library files', '/usr/' + LIBDIR_SCHEMA, PathVariable.PathAccept),

    # Variables for logging and statistics
    BoolVariable('ENABLE_LOG', 'Enable logging, which is enabled by default when building in *debug*', 'False'),
    BoolVariable('ENABLE_STATS', 'Enable global statistics during map processing', 'False'),
    ('DEFAULT_LOG_SEVERITY', 'The default severity of the logger (eg. "info", "debug", "warn", "error", "fatal", "none")', 'error'),

    # Other variables
    BoolVariable('SHAPE_MEMORY_MAPPED_FILE', 'Utilize memory-mapped files in Shapefile Plugin (higher memory usage, better performance)', 'True'),
    ('SYSTEM_FONTS','Provide location for python bindings to register fonts (if given aborts installation of bundled DejaVu fonts)',''),
    ('LIB_DIR_NAME','Name to use for the subfolder beside libmapnik where fonts and plugins are installed','mapnik'),
    PathVariable('PYTHON','Full path to Python executable used to build bindings', sys.executable),
    BoolVariable('FRAMEWORK_PYTHON', 'Link against Framework Python on Mac OS X', 'True'),
    BoolVariable('PYTHON_DYNAMIC_LOOKUP', 'On OSX, do not directly link python lib, but rather dynamically lookup symbols', 'True'),
    ('FRAMEWORK_SEARCH_PATH','Custom framework search path on Mac OS X', ''),
    BoolVariable('FULL_LIB_PATH', 'Use the full path for the libmapnik.dylib "install_name" when linking on Mac OS X', 'True'),
    ListVariable('BINDINGS','Language bindings to build','all',['python']),
    EnumVariable('THREADING','Set threading support','multi', ['multi','single']),
    EnumVariable('XMLPARSER','Set xml parser','libxml2', ['libxml2','ptree']),
    ('JOBS', 'Set the number of parallel compilations', "1", lambda key, value, env: int(value), int),
    BoolVariable('DEMO', 'Compile demo c++ application', 'True'),
    BoolVariable('PGSQL2SQLITE', 'Compile and install a utility to convert postgres tables to sqlite', 'False'),
    BoolVariable('COLOR_PRINT', 'Print build status information in color', 'True'),
    BoolVariable('SAMPLE_INPUT_PLUGINS', 'Compile and install sample plugins', 'False'),
    )

# variables to pickle after successful configure step
# these include all scons core variables as well as custom
# env variables needed in SConscript files
pickle_store = [# Scons internal variables
        'CC', # compiler user to check if c deps compile during configure
        'CXX', # C++ compiler to compile mapnik
        'CFLAGS',
        'CPPDEFINES',
        'CPPFLAGS', # c preprocessor flags
        'CPPPATH',
        'CXXFLAGS', # C++ flags built up during configure
        'LIBPATH',
        'LIBS',
        'LINKFLAGS',
        'CUSTOM_LDFLAGS', # user submitted
        'CUSTOM_CXXFLAGS', # user submitted
        'CUSTOM_CFLAGS', # user submitted
        'MAPNIK_LIB_NAME',
        'LINK',
        'RUNTIME_LINK',
        # Mapnik's SConstruct build variables
        'PLUGINS',
        'ABI_VERSION',
        'MAPNIK_VERSION_STRING',
        'PLATFORM',
        'BOOST_ABI',
        'BOOST_APPEND',
        'LIBDIR_SCHEMA',
        'REQUESTED_PLUGINS',
        'SUNCC',
        'PYTHON_VERSION',
        'PYTHON_INCLUDES',
        'PYTHON_INSTALL_LOCATION',
        'PYTHON_SYS_PREFIX',
        'COLOR_PRINT',
        'HAS_CAIRO',
        'HAS_PYCAIRO',
        'HAS_LIBXML2',
        'PYTHON_IS_64BIT',
        'SAMPLE_INPUT_PLUGINS',
        'PKG_CONFIG_PATH',
        'PATH',
        'PATH_REMOVE',
        'PATH_REPLACE',
        'MAPNIK_LIB_DIR',
        'MAPNIK_LIB_DIR_DEST',
        'INSTALL_PREFIX',
        'MAPNIK_INPUT_PLUGINS',
        'MAPNIK_INPUT_PLUGINS_DEST',
        'MAPNIK_FONTS',
        'MAPNIK_FONTS_DEST',
        'MAPNIK_LIB_BASE',
        'MAPNIK_LIB_BASE_DEST',
        'EXTRA_FREETYPE_LIBS',
        'LIBMAPNIK_CPPATHS',
        'LIBMAPNIK_CXXFLAGS',
        'CAIROMM_LIBPATHS',
        'CAIROMM_LINKFLAGS',
        'CAIROMM_CPPPATHS',
        'SVG_RENDERER',
        'SQLITE_LINKFLAGS',
        'BOOST_LIB_VERSION_FROM_HEADER'
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

HELP_REQUESTED = False

if ('-h' in command_line_args) or ('--help' in command_line_args):
    HELP_REQUESTED = True


if 'configure' in command_line_args and not HELP_REQUESTED:
    force_configure = True
elif HELP_REQUESTED:
    # to ensure config gets skipped when help is requested
    preconfigured = True

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
            preconfigured = False
    else:
        preconfigured = False

# check for missing keys in pickled settings
# which can occur when keys are added or changed between
# rebuilds, e.g. for folks following trunk
for opt in pickle_store:
    if not opt in env:
        #print 'missing opt', opt
        preconfigured = False

# if custom arguments are supplied make sure to accept them
if opts.args:
    # since we have custom arguments update environment with all opts to
    # make sure to absorb the custom ones
    opts.Update(env)
    # now since we've got custom arguments we'll disregard any
    # pickled environment and force another configuration
    preconfigured = False

elif preconfigured:
    if not HELP_REQUESTED:
        color_print(4,'Using previous successful configuration...')
        color_print(4,'Re-configure by running "python scons/scons.py configure".')

if env.has_key('COLOR_PRINT') and env['COLOR_PRINT'] == False:
    color_print = regular_print

if sys.platform == "win32":
    color_print = regular_print

color_print(4,'\nWelcome to Mapnik...\n')

#### Custom Configure Checks ###

def prioritize_paths(context,silent=True):
    env = context.env
    prefs = env['LINK_PRIORITY'].split(',')
    if not silent:
        context.Message( 'Sorting lib and inc compiler paths...')
    env['LIBPATH'] = sort_paths(env['LIBPATH'],prefs)
    env['CPPPATH'] = sort_paths(env['CPPPATH'],prefs)
    if silent:
        context.did_show_result=1
    ret = context.Result( True )
    return ret

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

def CheckPKGVersion(context, name, version):
    context.Message( 'Checking for at least version %s for %s... ' % (version,name) )
    ret = context.TryAction('pkg-config --atleast-version=%s \'%s\'' % (version,name))[0]
    context.Result( ret )
    return ret

def parse_config(context, config, checks='--libs --cflags'):
    env = context.env
    tool = config.lower().replace('_','-')
    toolname = tool
    if config in ('GDAL_CONFIG','GEOS_CONFIG'):
        toolname += ' %s' % checks
    context.Message( 'Checking for %s... ' % toolname)
    cmd = '%s %s' % (env[config],checks)
    ret = context.TryAction(cmd)[0]
    parsed = False
    if ret:
        try:
            if 'gdal-config' in cmd:
                env.ParseConfig(cmd)
                # hack for potential -framework GDAL syntax
                # which will not end up being added to env['LIBS']
                # and thus breaks knowledge below that gdal worked
                # TODO - upgrade our scons logic to support Framework linking
                if env['PLATFORM'] == 'Darwin':
                    value = call(cmd,silent=True)
                    if value and '-framework GDAL' in value:
                        env['LIBS'].append('gdal')
                        if os.path.exists('/Library/Frameworks/GDAL.framework/unix/lib'):
                            env['LIBPATH'].insert(0,'/Library/Frameworks/GDAL.framework/unix/lib')
                    if 'GDAL' in env.get('FRAMEWORKS',[]):
                        env["FRAMEWORKS"].remove("GDAL")
            else:
                env.ParseConfig(cmd)
            parsed = True
        except OSError, e:
            ret = False
            print ' (xml2-config not found!)'
    if not parsed:
        if config in ('GDAL_CONFIG','GEOS_CONFIG'):
            # optional deps...
            env['SKIPPED_DEPS'].append(tool)
            conf.rollback_option(config)
        else: # freetype and libxml2, not optional
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
    parsed = False
    if ret:
        try:
            value = call(cmd,silent=True)
            libnames = re.findall(libpattern,value)
            if libnames:
                libname = libnames[0]
            else:
                # osx 1.8 install gives '-framework GDAL'
                libname = 'gdal'
        except Exception, e:
            ret = False
            print ' unable to determine library name:'# %s' % str(e)
            return None
    context.Result( libname )
    return libname

def parse_pg_config(context, config):
    # TODO - leverage `LDFLAGS_SL` if RUNTIME_LINK==static
    env = context.env
    tool = config.lower()
    context.Message( 'Checking for %s... ' % tool)
    ret = context.TryAction(env[config])[0]
    if ret:
        lib_path = call('%s --libdir' % env[config])
        inc_path = call('%s --includedir' % env[config])
        env.AppendUnique(CPPPATH = os.path.realpath(inc_path))
        env.AppendUnique(LIBPATH = os.path.realpath(lib_path))
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

def FindBoost(context, prefixes, thread_flag):
    """Routine to auto-find boost header dir, lib dir, and library naming structure.

    """
    context.Message( 'Searching for boost libs and headers... ' )
    env = context.env

    BOOST_LIB_DIR = None
    BOOST_INCLUDE_DIR = None
    BOOST_APPEND = None
    env['BOOST_APPEND'] = str()

    if env['THREADING'] == 'multi':
        search_lib = 'libboost_thread'
    else:
        search_lib = 'libboost_filesystem'

    # note: must call normpath to strip trailing slash otherwise dirname
    # does not remove 'lib' and 'include'
    prefixes.insert(0,os.path.dirname(os.path.normpath(env['BOOST_INCLUDES'])))
    prefixes.insert(0,os.path.dirname(os.path.normpath(env['BOOST_LIBS'])))
    for searchDir in prefixes:
        libItems = glob(os.path.join(searchDir, LIBDIR_SCHEMA, '%s*.*' % search_lib))
        if not libItems:
            libItems = glob(os.path.join(searchDir, 'lib/%s*.*' % search_lib))
        incItems = glob(os.path.join(searchDir, 'include/boost*/'))
        if len(libItems) >= 1 and len(incItems) >= 1:
            BOOST_LIB_DIR = os.path.dirname(libItems[0])
            BOOST_INCLUDE_DIR = incItems[0].rstrip('boost/')
            shortest_lib_name = shortest_name(libItems)
            match = re.search(r'%s(.*)\..*' % search_lib, shortest_lib_name)
            if hasattr(match,'groups'):
                BOOST_APPEND = match.groups()[0]
            break

    msg = str()

    if BOOST_LIB_DIR:
        msg += '\n  *libs found: %s' % BOOST_LIB_DIR
        env['BOOST_LIBS'] = BOOST_LIB_DIR
    else:
        env['BOOST_LIBS'] = '/usr/' + LIBDIR_SCHEMA
        msg += '\n  *using default boost lib dir: %s' % env['BOOST_LIBS']

    if BOOST_INCLUDE_DIR:
        msg += '\n  *headers found: %s' % BOOST_INCLUDE_DIR
        env['BOOST_INCLUDES'] = BOOST_INCLUDE_DIR
    else:
        env['BOOST_INCLUDES'] = '/usr/include'
        msg += '\n  *using default boost include dir: %s' % env['BOOST_INCLUDES']

    if not env['BOOST_TOOLKIT'] and not env['BOOST_ABI'] and not env['BOOST_VERSION']:
        if BOOST_APPEND:
            msg += '\n  *lib naming extension found: %s' % BOOST_APPEND
            env['BOOST_APPEND'] = BOOST_APPEND
        else:
            msg += '\n  *no lib naming extension found'
    else:
        # Creating BOOST_APPEND according to the Boost library naming order,
        # which goes <toolset>-<threading>-<abi>-<version>. See:
        #  http://www.boost.org/doc/libs/1_35_0/more/getting_started/unix-variants.html#library-naming
        append_params = ['']
        if env['BOOST_TOOLKIT']: append_params.append(env['BOOST_TOOLKIT'])
        if thread_flag: append_params.append(thread_flag)
        if env['BOOST_ABI']: append_params.append(env['BOOST_ABI'])
        if env['BOOST_VERSION']: append_params.append(env['BOOST_VERSION'])

        # Constructing the BOOST_APPEND setting that will be used to find the
        # Boost libraries.
        if len(append_params) > 1:
            env['BOOST_APPEND'] = '-'.join(append_params)
        msg += '\n  *using boost lib naming: %s' % env['BOOST_APPEND']

    env.AppendUnique(CPPPATH = os.path.realpath(env['BOOST_INCLUDES']))
    env.AppendUnique(LIBPATH = os.path.realpath(env['BOOST_LIBS']))
    if env['COLOR_PRINT']:
        msg = "\033[94m%s\033[0m" % (msg)
    ret = context.Result(msg)
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
    std::cout << MAPNIK_VERSION_STRING << std::endl;
    return 0;
}

""", '.cpp')
    # hack to avoid printed output
    context.did_show_result=1
    context.Result(ret[0])
    if not ret[1]:
        return []
    return ret[1].strip()

def icu_at_least_four_two(context):
    ret = context.TryRun("""

#include <unicode/uversion.h>
#include <iostream>

int main()
{
    std::cout << U_ICU_VERSION_MAJOR_NUM << "." << U_ICU_VERSION_MINOR_NUM << std::endl;
    return 0;
}

""", '.cpp')
    # hack to avoid printed output
    context.Message('Checking for ICU version >= 4.2... ')
    context.did_show_result=1
    result = ret[1].strip()
    if not result:
        context.Result('error, could not get major and minor version from unicode/uversion.h')
        return False

    major, minor = map(int,result.split('.'))
    if major >= 4 and minor >= 0:
        color_print(4,'found: icu %s' % result)
        return True

    color_print(1,'\nFound insufficient icu version... %s' % result)
    return False

def boost_regex_has_icu(context):
    if env['RUNTIME_LINK'] == 'static':
        context.env.Append(LIBS='icui18n')
        context.env.Append(LIBS='icudata')
    ret = context.TryRun("""

#include <boost/regex/icu.hpp>
#include <unicode/unistr.h>

int main()
{
    UnicodeString ustr;
    try {
        boost::u32regex pattern = boost::make_u32regex(ustr);
    }
    // an exception is fine, still indicates support is
    // likely compiled into regex
    catch (...) {
        return 0;
    }
    return 0;
}

""", '.cpp')
    context.Message('Checking if boost_regex was built with ICU unicode support... ')
    context.Result(ret[0])
    if ret[0]:
        return True
    return False

def sqlite_has_rtree(context):
    """ check an sqlite3 install has rtree support.

    PRAGMA compile_options;
    http://www.sqlite.org/c3ref/compileoption_get.html
    """

    ret = context.TryRun("""

#include <sqlite3.h>
#include <stdio.h>

int main()
{
    sqlite3* db;
    int rc;
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK)
    {
        printf("error 1: %s\\n", sqlite3_errmsg(db));
    }
    const char * sql = "create virtual table foo using rtree(pkid, xmin, xmax, ymin, ymax)";
    rc = sqlite3_exec(db, sql, 0, 0, 0);
    if (rc != SQLITE_OK)
    {
        printf("error 2: %s\\n", sqlite3_errmsg(db));
    }
    else
    {
        printf("yes, has rtree!\\n");
        return 0;
    }

    return -1;
}

""", '.c')
    context.Message('Checking if SQLite supports RTREE... ')
    context.Result(ret[0])
    if ret[0]:
        return True
    return False

conf_tests = { 'prioritize_paths'      : prioritize_paths,
               'CheckPKGConfig'        : CheckPKGConfig,
               'CheckPKG'              : CheckPKG,
               'CheckPKGVersion'       : CheckPKGVersion,
               'FindBoost'             : FindBoost,
               'CheckBoost'            : CheckBoost,
               'GetBoostLibVersion'    : GetBoostLibVersion,
               'GetMapnikLibVersion'   : GetMapnikLibVersion,
               'parse_config'          : parse_config,
               'parse_pg_config'       : parse_pg_config,
               'ogr_enabled'           : ogr_enabled,
               'get_pkg_lib'           : get_pkg_lib,
               'rollback_option'       : rollback_option,
               'icu_at_least_four_two' : icu_at_least_four_two,
               'boost_regex_has_icu'   : boost_regex_has_icu,
               'sqlite_has_rtree'      : sqlite_has_rtree,
               }


if not preconfigured:

    color_print(4,'Configuring build environment...')

    if not env['FAST']:
        SetCacheMode('force')

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
                    #print optfile.read().replace("\n", " ").replace("'","").replace(" = ","=")
                    optfile.close()

                elif not conf == SCONS_LOCAL_CONFIG:
                    # if default missing, no worries
                    # but if the default is overridden and the file is not found, give warning
                    color_print(1,"SCons CONFIG not found: '%s'" % conf)
            # Recreate the base environment using modified `opts`
            env = Environment(ENV=os.environ,options=opts)
            env.Decider('MD5-timestamp')
            env.SourceCode(".", None)
            env['USE_CONFIG'] = True
    else:
        color_print(4,'SCons USE_CONFIG specified as false, will not inherit variables python config file...')

    conf = Configure(env, custom_tests = conf_tests)

    if env['DEBUG']:
        mode = 'debug mode'
    else:
        mode = 'release mode'

    env['PLATFORM'] = platform.uname()[0]
    color_print(4,"Configuring on %s in *%s*..." % (env['PLATFORM'],mode))

    env['MISSING_DEPS'] = []
    env['SKIPPED_DEPS'] = []
    env['HAS_CAIRO'] = False
    env['CAIROMM_LIBPATHS'] = []
    env['CAIROMM_LINKFLAGS'] = []
    env['CAIROMM_CPPPATHS'] = []
    env['HAS_PYCAIRO'] = False
    env['HAS_LIBXML2'] = False
    env['LIBMAPNIK_LIBS'] = []
    env['LIBMAPNIK_CPPATHS'] = []
    env['LIBMAPNIK_CXXFLAGS'] = []
    env['LIBDIR_SCHEMA'] = LIBDIR_SCHEMA
    env['PLUGINS'] = PLUGINS
    env['EXTRA_FREETYPE_LIBS'] = []
    env['SQLITE_LINKFLAGS'] = []
    # previously a leading / was expected for LIB_DIR_NAME
    # now strip it to ensure expected behavior
    if env['LIB_DIR_NAME'].startswith(os.path.sep):
        env['LIB_DIR_NAME'] = strip_first(env['LIB_DIR_NAME'],os.path.sep)

    # base install location
    env['MAPNIK_LIB_BASE'] = os.path.join(env['PREFIX'],env['LIBDIR_SCHEMA'])
    # directory for plugins and fonts
    env['MAPNIK_LIB_DIR'] = os.path.join(env['MAPNIK_LIB_BASE'],env['LIB_DIR_NAME'])
    # input plugins sub directory
    env['MAPNIK_INPUT_PLUGINS'] = os.path.join(env['MAPNIK_LIB_DIR'],'input')
    # fonts sub directory
    if env['SYSTEM_FONTS']:
        env['MAPNIK_FONTS'] = os.path.normpath(env['SYSTEM_FONTS'])
    else:
        env['MAPNIK_FONTS'] = os.path.join(env['MAPNIK_LIB_DIR'],'fonts')

    # install prefix is a pre-pended base location to
    # re-route the install and only intended for package building
    # we normalize to ensure no trailing slash and proper pre-pending to the absolute prefix
    install_prefix = os.path.normpath(os.path.realpath(env['DESTDIR'])) + os.path.realpath(env['PREFIX'])
    env['INSTALL_PREFIX'] = strip_first(install_prefix,'//','/')
    # all values from above based on install_prefix
    # if env['DESTDIR'] == '/' these should be unchanged
    env['MAPNIK_LIB_BASE_DEST'] = os.path.join(env['INSTALL_PREFIX'],env['LIBDIR_SCHEMA'])
    env['MAPNIK_LIB_DIR_DEST'] =  os.path.join(env['MAPNIK_LIB_BASE_DEST'],env['LIB_DIR_NAME'])
    env['MAPNIK_INPUT_PLUGINS_DEST'] = os.path.join(env['MAPNIK_LIB_DIR_DEST'],'input')
    if env['SYSTEM_FONTS']:
        env['MAPNIK_FONTS_DEST'] = os.path.normpath(env['SYSTEM_FONTS'])
    else:
        env['MAPNIK_FONTS_DEST'] = os.path.join(env['MAPNIK_LIB_DIR_DEST'],'fonts')

    if env['LINKING'] == 'static':
       env['MAPNIK_LIB_NAME'] = '${LIBPREFIX}mapnik${LIBSUFFIX}'
    else:
       env['MAPNIK_LIB_NAME'] = '${SHLIBPREFIX}mapnik${SHLIBSUFFIX}'

    if env['PKG_CONFIG_PATH']:
        env['ENV']['PKG_CONFIG_PATH'] = os.path.realpath(env['PKG_CONFIG_PATH'])
        # otherwise this variable == os.environ["PKG_CONFIG_PATH"]

    if env['PATH']:
        env['ENV']['PATH'] = os.path.realpath(env['PATH']) + ':' + env['ENV']['PATH']

    if env['SYSTEM_FONTS']:
        if not os.path.isdir(env['SYSTEM_FONTS']):
            color_print(1,'Warning: Directory specified for SYSTEM_FONTS does not exist!')
    #### Libraries and headers dependency checks ####

    # Set up for libraries and headers dependency checks
    env['CPPPATH'] = ['#include', '#']
    env['LIBPATH'] = ['#src']

    # set any custom cxxflags and ldflags to come first
    env.Append(CXXFLAGS = env['CUSTOM_CXXFLAGS'])
    env.Append(CFLAGS = env['CUSTOM_CFLAGS'])
    env.Append(LINKFLAGS = env['CUSTOM_LDFLAGS'])

    ### platform specific bits

    thread_suffix = 'mt'
    if env['PLATFORM'] == 'FreeBSD':
        thread_suffix = ''
        env.Append(LIBS = 'pthread')

    # Solaris & Sun Studio settings (the `SUNCC` flag will only be
    # set if the `CXX` option begins with `CC`)
    SOLARIS = env['PLATFORM'] == 'SunOS'
    env['SUNCC'] = SOLARIS and env['CXX'].startswith('CC')

    # If the Sun Studio C++ compiler (`CC`) is used instead of GCC.
    if env['SUNCC']:
        env['CC'] = 'cc'
        # To be compatible w/Boost everything needs to be compiled
        # with the `-library=stlport4` flag (which needs to come
        # before the `-o` flag).
        env['CXX'] = 'CC -library=stlport4'
        if env['THREADING'] == 'multi':
            env.Append(CXXFLAGS = '-mt')

    # allow for mac osx /usr/lib/libicucore.dylib compatibility
    # requires custom supplied headers since Apple does not include them
    # details: http://lists.apple.com/archives/xcode-users/2005/Jun/msg00633.html
    # To use system lib download and make && make install one of these:
    # http://www.opensource.apple.com/tarballs/ICU/
    # then copy the headers to a location that mapnik will find
    if 'core' in env['ICU_LIB_NAME']:
        env.Append(CXXFLAGS = '-DU_HIDE_DRAFT_API')
        env.Append(CXXFLAGS = '-DUDISABLE_RENAMING')
        if os.path.exists(env['ICU_LIB_NAME']):
            #-sICU_LINK=" -L/usr/lib -licucore
            env['ICU_LIB_NAME'] = os.path.basename(env['ICU_LIB_NAME']).replace('.dylib','').replace('lib','')

    # Adding the required prerequisite library directories to the include path for
    # compiling and the library path for linking, respectively.
    for required in ('PNG', 'JPEG', 'TIFF','PROJ','ICU', 'SQLITE'):
        inc_path = env['%s_INCLUDES' % required]
        lib_path = env['%s_LIBS' % required]
        env.AppendUnique(CPPPATH = os.path.realpath(inc_path))
        env.AppendUnique(LIBPATH = os.path.realpath(lib_path))

    if conf.parse_config('FREETYPE_CONFIG'):
        # check if freetype links to bz2
        if env['RUNTIME_LINK'] == 'static':
            temp_env = env.Clone()
            temp_env['LIBS'] = []
            try:
                temp_env.ParseConfig('%s --libs' % env['FREETYPE_CONFIG'])
                if 'bz2' in temp_env['LIBS']:
                    env['EXTRA_FREETYPE_LIBS'].append('bz2')
            except OSError,e:
                pass

    # libxml2 should be optional but is currently not
    # https://github.com/mapnik/mapnik/issues/913
    if conf.parse_config('XML2_CONFIG',checks='--cflags'):
        env['HAS_LIBXML2'] = True

    LIBSHEADERS = [
        ['m', 'math.h', True,'C'],
        ['ltdl', 'ltdl.h', True,'C'],
        ['png', 'png.h', True,'C'],
        ['tiff', 'tiff.h', True,'C'],
        ['z', 'zlib.h', True,'C'],
        ['proj', 'proj_api.h', True,'C'],
        [env['ICU_LIB_NAME'],'unicode/unistr.h',True,'C++'],
    ]

    if env['JPEG']:
        env.Append(CXXFLAGS = '-DHAVE_JPEG')
        LIBSHEADERS.append(['jpeg', ['stdio.h', 'jpeglib.h'], True,'C'])
    else:
        env['SKIPPED_DEPS'].extend(['jpeg'])

    # if requested, sort LIBPATH and CPPPATH before running CheckLibWithHeader tests
    if env['PRIORITIZE_LINKING']:
        conf.prioritize_paths(silent=False)

    for libname, headers, required, lang in LIBSHEADERS:
        if not conf.CheckLibWithHeader(libname, headers, lang):
            if required:
                color_print(1, 'Could not find required header or shared library for %s' % libname)
                env['MISSING_DEPS'].append(libname)
            else:
                color_print(4, 'Could not find optional header or shared library for %s' % libname)
                env['SKIPPED_DEPS'].append(libname)

    if env['ICU_LIB_NAME'] not in env['MISSING_DEPS']:
        if not conf.icu_at_least_four_two():
            # expression_string.cpp and map.cpp use fromUTF* function only available in >= ICU 4.2
            env['MISSING_DEPS'].append(env['ICU_LIB_NAME'])

    if env['THREADING'] == 'multi':
        thread_flag = thread_suffix
    else:
        thread_flag = ''

    conf.FindBoost(BOOST_SEARCH_PREFIXES,thread_flag)

    env['BOOST_LIB_VERSION_FROM_HEADER'] = conf.GetBoostLibVersion()

    # The other required boost headers.
    BOOST_LIBSHEADERS = [
        ['system', 'boost/system/system_error.hpp', True],
        ['filesystem', 'boost/filesystem/operations.hpp', True],
        ['regex', 'boost/regex.hpp', True],
        ['program_options', 'boost/program_options.hpp', False]
    ]

    if env['THREADING'] == 'multi':
        BOOST_LIBSHEADERS.append(['thread', 'boost/thread/mutex.hpp', True])
        # on solaris the configure checks for boost_thread
        # require the -pthreads flag to be able to check for
        # threading support, so we add as a global library instead
        # of attaching to cxxflags after configure
        if env['PLATFORM'] == 'SunOS':
            env.Append(CXXFLAGS = '-pthreads')

    # if requested, sort LIBPATH and CPPPATH before running CheckLibWithHeader tests
    if env['PRIORITIZE_LINKING']:
        conf.prioritize_paths()

    # if the user is not setting custom boost configuration
    # enforce boost version greater than or equal to BOOST_MIN_VERSION
    if not conf.CheckBoost(BOOST_MIN_VERSION):
        color_print(4,'Found boost lib version... %s' % env.get('BOOST_LIB_VERSION_FROM_HEADER') )
        color_print(1,'Boost version %s or greater is required' % BOOST_MIN_VERSION)
        if not env['BOOST_VERSION']:
            env['MISSING_DEPS'].append('boost version >=%s' % BOOST_MIN_VERSION)
    else:
        color_print(4,'Found boost lib version... %s' % env.get('BOOST_LIB_VERSION_FROM_HEADER') )

    for count, libinfo in enumerate(BOOST_LIBSHEADERS):
        if not conf.CheckLibWithHeader('boost_%s%s' % (libinfo[0],env['BOOST_APPEND']), libinfo[1], 'C++'):
            if libinfo[2]:
                color_print(1,'Could not find required header or shared library for boost %s' % libinfo[0])
                env['MISSING_DEPS'].append('boost ' + libinfo[0])
            else:
                color_print(4,'Could not find optional header or shared library for boost %s' % libinfo[0])
                env['SKIPPED_DEPS'].append('boost ' + libinfo[0])

    if env['ICU_LIB_NAME'] not in env['MISSING_DEPS']:
        # http://lists.boost.org/Archives/boost/2009/03/150076.php
        if conf.boost_regex_has_icu():
            # TODO - should avoid having this be globally defined...
            env.Append(CXXFLAGS = '-DBOOST_REGEX_HAS_ICU')
        else:
            env['SKIPPED_DEPS'].append('boost_regex_icu')

    env['REQUESTED_PLUGINS'] = [ driver.strip() for driver in Split(env['INPUT_PLUGINS'])]

    if len(env['REQUESTED_PLUGINS']):
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
            elif plugin == 'geos':
                if conf.parse_config('GEOS_CONFIG',checks='--ldflags --cflags'):
                    lgeos_c = env['PLUGINS']['geos']['lib']
                    env.Append(LIBS = lgeos_c)

            elif details['path'] and details['lib'] and details['inc']:
                backup = env.Clone().Dictionary()
                # Note, the 'delete_existing' keyword makes sure that these paths are prepended
                # to the beginning of the path list even if they already exist
                incpath = env['%s_INCLUDES' % details['path']]
                libpath = env['%s_LIBS' % details['path']]
                env.PrependUnique(CPPPATH = os.path.realpath(incpath),delete_existing=True)
                env.PrependUnique(LIBPATH = os.path.realpath(libpath),delete_existing=True)
                if not conf.CheckLibWithHeader(details['lib'], details['inc'], details['lang']):
                    env.Replace(**backup)
                    env['SKIPPED_DEPS'].append(details['lib'])
                if plugin == 'sqlite':
                    sqlite_backup = env.Clone().Dictionary()

                    # if statically linking, on linux we likely
                    # need to link sqlite to pthreads and dl
                    if env['RUNTIME_LINK'] == 'static':
                        if conf.CheckPKGConfig('0.15.0') and conf.CheckPKG('sqlite3'):
                            sqlite_env = env.Clone()
                            try:
                                sqlite_env.ParseConfig('pkg-config --static --libs sqlite3')
                                for lib in sqlite_env['LIBS']:
                                    if not lib in env['LIBS']:
                                        env["SQLITE_LINKFLAGS"].append(lib)
                                        env.Append(LIBS=lib)
                            except OSError,e:
                                pass

                    if not conf.sqlite_has_rtree():
                        env.Replace(**sqlite_backup)
                        if details['lib'] in env['LIBS']:
                            env['LIBS'].remove(details['lib'])
                        env['SKIPPED_DEPS'].append('sqlite_rtree')
                    else:
                        env.Replace(**sqlite_backup)

            elif details['lib'] and details['inc']:
                if not conf.CheckLibWithHeader(details['lib'], details['inc'], details['lang']):
                    env['SKIPPED_DEPS'].append(details['lib'])

        # re-append the local paths for mapnik sources to the beginning of the list
        # to make sure they come before any plugins that were 'prepended'
        env.PrependUnique(CPPPATH = '#include', delete_existing=True)
        env.PrependUnique(CPPPATH = '#', delete_existing=True)
        env.PrependUnique(LIBPATH = '#src', delete_existing=True)

    if env['PGSQL2SQLITE']:
        if not conf.sqlite_has_rtree():
            env['SKIPPED_DEPS'].append('pgsql2sqlite_rtree')
            env['PGSQL2SQLITE'] = False

    # we rely on an internal, patched copy of agg with critical fixes
    # prepend to make sure we link locally
    env.Prepend(CPPPATH = '#deps/agg/include')
    env.Prepend(LIBPATH = '#deps/agg')

    if env['CAIRO']:
        if env['CAIRO_LIBS'] or env['CAIRO_INCLUDES']:
            c_inc = env['CAIRO_INCLUDES']
            if env['CAIRO_LIBS']:
                env["CAIROMM_LIBPATHS"].append(os.path.realpath(env['CAIRO_LIBS']))
                if not env['CAIRO_INCLUDES']:
                    c_inc = env['CAIRO_LIBS'].replace('lib','',1)
            if c_inc:
                c_inc = os.path.normpath(os.path.realpath(env['CAIRO_INCLUDES']))
                if c_inc.endswith('include'):
                    c_inc = os.path.dirname(c_inc)
                env["CAIROMM_CPPPATHS"].extend(
                    [
                      os.path.join(c_inc,'include/cairomm-1.0'),
                      os.path.join(c_inc,'lib/cairomm-1.0/include'),
                      os.path.join(c_inc,'include/cairo'),
                      os.path.join(c_inc,'include/sigc++-2.0'),
                      os.path.join(c_inc,'lib/sigc++-2.0/include'),
                      os.path.join(c_inc,'include/pixman-1'),
                      #os.path.join(c_inc,'include/freetype2'),
                      #os.path.join(c_inc,'include/libpng'),
                    ]
                )
                env["CAIROMM_LINKFLAGS"] = ['cairo','cairomm-1.0']
                if env['RUNTIME_LINK'] == 'static':
                    env["CAIROMM_LINKFLAGS"].extend(
                        ['sigc-2.0','pixman-1','expat','fontconfig','iconv']
                    )
                # todo - run actual checkLib?
                env['HAS_CAIRO'] = True
        else:
            if not conf.CheckPKGConfig('0.15.0'):
                env['HAS_CAIRO'] = False
                env['SKIPPED_DEPS'].append('pkg-config')
                env['SKIPPED_DEPS'].append('cairo')
                env['SKIPPED_DEPS'].append('cairomm')
            elif not conf.CheckPKG('cairo'):
                env['HAS_CAIRO'] = False
                env['SKIPPED_DEPS'].append('cairo')
            elif not conf.CheckPKG('cairomm-1.0'):
                env['HAS_CAIRO'] = False
                env['SKIPPED_DEPS'].append('cairomm')
            elif not conf.CheckPKGVersion('cairomm-1.0',CAIROMM_MIN_VERSION):
                env['HAS_CAIRO'] = False
                env['SKIPPED_DEPS'].append('cairomm-version')
            else:
                print 'Checking for cairo/cairomm lib and include paths... ',
                cmd = 'pkg-config --libs --cflags cairomm-1.0'
                if env['RUNTIME_LINK'] == 'static':
                    cmd += ' --static'
                cairo_env = env.Clone()
                try:
                    cairo_env.ParseConfig(cmd)
                    for lib in cairo_env['LIBS']:
                        if not lib in env['LIBS']:
                            env["CAIROMM_LINKFLAGS"].append(lib)
                    for lpath in cairo_env['LIBPATH']:
                        if not lpath in env['LIBPATH']:
                            env["CAIROMM_LIBPATHS"].append(lpath)
                    for inc in cairo_env['CPPPATH']:
                        if not inc in env['CPPPATH']:
                            env["CAIROMM_CPPPATHS"].append(inc)
                    env['HAS_CAIRO'] = True
                    print 'yes'
                except OSError,e:
                    color_print(1,'no')
                    env['SKIPPED_DEPS'].append('cairo')
                    env['SKIPPED_DEPS'].append('cairomm')
                    color_print(1,'pkg-config reported: %s' % e)

    else:
        color_print(4,'Not building with cairo support, pass CAIRO=True to enable')

    if 'python' in env['BINDINGS']:
        if not os.access(env['PYTHON'], os.X_OK):
            color_print(1,"Cannot run python interpreter at '%s', make sure that you have the permissions to execute it." % env['PYTHON'])
            Exit(1)

        py3 = 'True' in os.popen('''%s -c "import sys as s;s.stdout.write(str(s.version_info[0] == 3))"''' % env['PYTHON']).read().strip()

        if py3:
            sys_prefix = '''%s -c "import sys; print(sys.prefix)"''' % env['PYTHON']
        else:
            sys_prefix = '''%s -c "import sys; print sys.prefix"''' % env['PYTHON']
        env['PYTHON_SYS_PREFIX'] = call(sys_prefix)

        if HAS_DISTUTILS:
            if py3:
                sys_version = '''%s -c "from distutils.sysconfig import get_python_version; print(get_python_version())"''' % env['PYTHON']
            else:
                sys_version = '''%s -c "from distutils.sysconfig import get_python_version; print get_python_version()"''' % env['PYTHON']
            env['PYTHON_VERSION'] = call(sys_version)

            if py3:
                py_includes = '''%s -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())"''' % env['PYTHON']
            else:
                py_includes = '''%s -c "from distutils.sysconfig import get_python_inc; print get_python_inc()"''' % env['PYTHON']
            env['PYTHON_INCLUDES'] = call(py_includes)

            # Note: we use the plat_specific argument here to make sure to respect the arch-specific site-packages location
            if py3:
                site_packages = '''%s -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(plat_specific=True))"''' % env['PYTHON']
            else:
                site_packages = '''%s -c "from distutils.sysconfig import get_python_lib; print get_python_lib(plat_specific=True)"''' % env['PYTHON']
            env['PYTHON_SITE_PACKAGES'] = call(site_packages)
        else:
            env['PYTHON_SYS_PREFIX'] = os.popen('''%s -c "import sys; print sys.prefix"''' % env['PYTHON']).read().strip()
            env['PYTHON_VERSION'] = os.popen('''%s -c "import sys; print sys.version"''' % env['PYTHON']).read()[0:3]
            env['PYTHON_INCLUDES'] = env['PYTHON_SYS_PREFIX'] + '/include/python' + env['PYTHON_VERSION']
            env['PYTHON_SITE_PACKAGES'] = env['DESTDIR'] + os.path.sep + env['PYTHON_SYS_PREFIX'] + os.path.sep + env['LIBDIR_SCHEMA'] + '/python' + env['PYTHON_VERSION'] + '/site-packages/'

        # if user-requested custom prefix fall back to manual concatenation for building subdirectories
        if env['PYTHON_PREFIX']:
            py_relative_install = env['LIBDIR_SCHEMA'] + '/python' + env['PYTHON_VERSION'] + '/site-packages/'
            env['PYTHON_INSTALL_LOCATION'] = env['DESTDIR'] + os.path.sep + env['PYTHON_PREFIX'] + os.path.sep +  py_relative_install
        else:
            env['PYTHON_INSTALL_LOCATION'] = env['DESTDIR'] + os.path.sep + env['PYTHON_SITE_PACKAGES']

        if py3:
            is_64_bit = '''%s -c "import sys; print(sys.maxsize == 9223372036854775807)"''' % env['PYTHON']
        else:
            is_64_bit = '''%s -c "import sys; print sys.maxint == 9223372036854775807"''' % env['PYTHON']

        if is_64_bit:
            env['PYTHON_IS_64BIT'] = True
        else:
            env['PYTHON_IS_64BIT'] = False

        if py3 and env['BOOST_PYTHON_LIB'] == 'boost_python':
            env['BOOST_PYTHON_LIB'] = 'boost_python3%s' % env['BOOST_APPEND']
        elif env['BOOST_PYTHON_LIB'] == 'boost_python':
            env['BOOST_PYTHON_LIB'] = 'boost_python%s' % env['BOOST_APPEND']

        if not conf.CheckHeader(header='boost/python/detail/config.hpp',language='C++'):
            color_print(1,'Could not find required header files for boost python')
            env['MISSING_DEPS'].append('boost python')

        if not conf.CheckLibWithHeader(libs=[env['BOOST_PYTHON_LIB'],'python%s' % env['PYTHON_VERSION']], header='boost/python/detail/config.hpp', language='C++'):
            color_print(1, 'Could not find library "%s" for boost python bindings' % env['BOOST_PYTHON_LIB'])
            # failing on launchpad, so let's make it a warning for now
            #env['MISSING_DEPS'].append('boost python')

        if env['CAIRO']:
            if conf.CheckPKGConfig('0.15.0') and conf.CheckPKG('pycairo'):
                env['HAS_PYCAIRO'] = True
            else:
                env['SKIPPED_DEPS'].extend(['pycairo'])
        else:
            color_print(4,'Not building with pycairo support, pass CAIRO=True to enable')


    #### End Config Stage for Required Dependencies ####

    if env['MISSING_DEPS']:
        # if required dependencies are missing, print warnings and then let SCons finish without building or saving local config
        color_print(1,'\nExiting... the following required dependencies were not found:\n   - %s' % '\n   - '.join([pretty_dep(dep) for dep in env['MISSING_DEPS']]))
        color_print(1,"\nSee '%s' for details on possible problems." % (os.path.realpath(SCONS_LOCAL_LOG)))
        if env['SKIPPED_DEPS']:
            color_print(4,'\nAlso, these OPTIONAL dependencies were not found:\n   - %s' % '\n   - '.join([pretty_dep(dep) for dep in env['SKIPPED_DEPS']]))
        color_print(4,"\nSet custom paths to these libraries and header files on the command-line or in a file called '%s'" % SCONS_LOCAL_CONFIG)
        color_print(4,"    ie. $ python scons/scons.py BOOST_INCLUDES=/usr/local/include BOOST_LIBS=/usr/local/lib")
        color_print(4, "\nOnce all required dependencies are found a local '%s' will be saved and then install:" % SCONS_LOCAL_CONFIG)
        color_print(4,"    $ sudo python scons/scons.py install")
        color_print(4,"\nTo view available path variables:\n    $ python scons/scons.py --help or -h")
        color_print(4,'\nTo view overall SCons help options:\n    $ python scons/scons.py --help-options or -H\n')
        color_print(4,'More info: http://trac.mapnik.org/wiki/MapnikInstallation')
        if not HELP_REQUESTED:
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
            color_print(3,'\nNote: will build without these OPTIONAL dependencies:\n   - %s' % '\n   - '.join([pretty_dep(dep) for dep in env['SKIPPED_DEPS']]))
            print

        # fetch the mapnik version header in order to set the
        # ABI version used to build libmapnik.so on linux in src/build.py
        abi = conf.GetMapnikLibVersion()
        abi_fallback = "2.0.1-pre"
        if not abi:
            color_print(1,'Problem encountered parsing mapnik version, falling back to %s' % abi_fallback)
            abi = abi_fallback

        env['ABI_VERSION'] = abi.replace('-pre','').split('.')
        env['MAPNIK_VERSION_STRING'] = abi

        # Common C++ flags.
        if env['THREADING'] == 'multi':
            common_cxx_flags = '-D%s -DBOOST_SPIRIT_THREADSAFE -DMAPNIK_THREADSAFE ' % env['PLATFORM'].upper()
        else :
            common_cxx_flags = '-D%s ' % env['PLATFORM'].upper()

        # Mac OSX (Darwin) special settings
        if env['PLATFORM'] == 'Darwin':
            pthread = ''
            # Getting the macintosh version number, sticking as a compiler macro
            # for Leopard -- needed because different workarounds are needed than
            # for Tiger.
            # this was used for fribidi - not longer needed
            # but will retain logic for future use
            #if platform.mac_ver()[0].startswith('10.5'):
            #    common_cxx_flags += '-DOSX_LEOPARD '
        else:
            pthread = '-pthread'

        # Common debugging flags.
        # http://lists.fedoraproject.org/pipermail/devel/2010-November/144952.html
        debug_flags  = '-g -fno-omit-frame-pointer -DDEBUG -DMAPNIK_DEBUG'
        ndebug_flags = '-DNDEBUG'

        # Enable logging in debug mode (always) and release mode (when specified)
        severities = ['info', 'debug', 'warn', 'error', 'fatal', 'none']
        if env['DEFAULT_LOG_SEVERITY']:
            if env['DEFAULT_LOG_SEVERITY'] not in severities:
                severities_list = ', '.join(["'%s'" % s for s in severities])
                color_print(1,"Cannot set default logger severity to '%s', available options are %s." % (env['DEFAULT_LOG_SEVERITY'], severities_list))
                Exit(1)
            else:
                log_severity = severities.index(env['DEFAULT_LOG_SEVERITY'])
        else:
            severities_list = ', '.join(["'%s'" % s for s in severities])
            color_print(1,"No logger severity specified, available options are %s." % severities_list)
            Exit(1)

        log_enabled = ' -DMAPNIK_LOG -DMAPNIK_DEFAULT_LOG_SEVERITY=%d' % log_severity

        if env['DEBUG']:
            debug_flags += log_enabled
        else:
            if env['ENABLE_LOG']:
                ndebug_flags += log_enabled

        # Enable statistics reporting
        if env['ENABLE_STATS']:
            debug_flags += ' -DMAPNIK_STATS'
            ndebug_flags += ' -DMAPNIK_STATS'

        # Add rdynamic to allow using statics between application and plugins
        # http://stackoverflow.com/questions/8623657/multiple-instances-of-singleton-across-shared-libraries-on-linux
        if env['PLATFORM'] != 'Darwin' and env['CXX'] == 'g++':
            env.MergeFlags('-rdynamic')

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
            gcc_cxx_flags = '-ansi -Wall %s %s -ftemplate-depth-300 %s' % (env['WARNING_CXXFLAGS'], pthread, common_cxx_flags)
            if env['DEBUG']:
                env.Append(CXXFLAGS = gcc_cxx_flags + '-O0 -fno-inline %s' % debug_flags)
            else:
                env.Append(CXXFLAGS = gcc_cxx_flags + '-O%s -fno-strict-aliasing -finline-functions -Wno-inline -Wno-parentheses -Wno-char-subscripts %s' % (env['OPTIMIZATION'],ndebug_flags))

            if env['DEBUG_UNDEFINED']:
                env.Append(CXXFLAGS = '-fcatch-undefined-behavior -ftrapv -fwrapv')

        if 'python' in env['BINDINGS']:
            majver, minver = env['PYTHON_VERSION'].split('.')
            # we don't want the includes it in the main environment...
            # as they are later set in the python build.py
            # ugly hack needed until we have env specific conf
            backup = env.Clone().Dictionary()
            env.AppendUnique(CPPPATH = os.path.realpath(env['PYTHON_INCLUDES']))

            if not conf.CheckHeader(header='Python.h',language='C'):
                color_print(1,'Could not find required header files for the Python language (version %s)' % env['PYTHON_VERSION'])
                Exit(1)

            if (int(majver), int(minver)) < (2, 2):
                color_print(1,"Python version 2.2 or greater required")
                Exit(1)

            color_print(4,'Bindings Python version... %s' % env['PYTHON_VERSION'])
            color_print(4,'Python %s prefix... %s' % (env['PYTHON_VERSION'], env['PYTHON_SYS_PREFIX']))
            color_print(4,'Python bindings will install in... %s' % os.path.normpath(env['PYTHON_INSTALL_LOCATION']))
            env.Replace(**backup)

        # if requested, sort LIBPATH and CPPPATH one last time before saving...
        if env['PRIORITIZE_LINKING']:
            conf.prioritize_paths()

        # finish config stage and pickle results
        env = conf.Finish()
        env_cache = open(SCONS_CONFIGURE_CACHE, 'w')
        pickle_dict = {}
        for i in pickle_store:
            pickle_dict[i] = env.get(i)
        pickle.dump(pickle_dict,env_cache)
        env_cache.close()
        # fix up permissions on configure outputs
        # this is hackish but avoids potential problems
        # with a non-root configure following a root install
        # that also triggered a re-configure
        try:
            os.chmod(SCONS_CONFIGURE_CACHE,0666)
        except: pass
        try:
            os.chmod(SCONS_LOCAL_CONFIG,0666)
        except: pass
        try:
            os.chmod('.sconsign.dblite',0666)
        except: pass
        try:
            os.chmod(SCONS_LOCAL_LOG,0666)
        except: pass
        try:
            for item in glob('%s/*' % SCONF_TEMP_DIR):
                os.chmod(item,0666)
        except: pass

        if 'configure' in command_line_args:
            color_print(4,'\nConfigure completed: run `make` to build or `make install`')
            if not HELP_REQUESTED:
                Exit(0)

# autogenerate help on default/current SCons options
Help(opts.GenerateHelpText(env))

#### Builds ####
if not HELP_REQUESTED:

    if 'uninstall' in COMMAND_LINE_TARGETS:
        # dummy action in case there is nothing to uninstall, to avoid phony error..
        env.Alias("uninstall", "")
    env['create_uninstall_target'] = create_uninstall_target

    if env['PKG_CONFIG_PATH']:
        env['ENV']['PKG_CONFIG_PATH'] = os.path.realpath(env['PKG_CONFIG_PATH'])
        # otherwise this variable == os.environ["PKG_CONFIG_PATH"]

    if env['PATH']:
        env['ENV']['PATH'] = os.path.realpath(env['PATH']) + ':' + env['ENV']['PATH']

    if env['PATH_REMOVE']:
        p = env['PATH_REMOVE']
        if p in env['ENV']['PATH']:
            env['ENV']['PATH'].replace(p,'')
        def rm_path(set):
            for i in env[set]:
                if p in i:
                    env[set].remove(i)
        rm_path('LIBPATH')
        rm_path('CPPPATH')
        rm_path('CXXFLAGS')
        rm_path('CAIROMM_LIBPATHS')
        rm_path('CAIROMM_CPPPATHS')

    if env['PATH_REPLACE']:
        searches,replace = env['PATH_REPLACE'].split(':')
        for search in searches.split(','):
            if search in env['ENV']['PATH']:
                env['ENV']['PATH'] = os.path.abspath(env['ENV']['PATH'].replace(search,replace))
            def replace_path(set,s,r):
                idx = 0
                for i in env[set]:
                    if s in i:
                        env[set][idx] = os.path.abspath(env[set][idx].replace(s,r))
                    idx +=1
            replace_path('LIBPATH',search,replace)
            replace_path('CPPPATH',search,replace)
            replace_path('CXXFLAGS',search,replace)
            replace_path('CAIROMM_LIBPATHS',search,replace)
            replace_path('CAIROMM_CPPPATHS',search,replace)

    # export env so it is available in build.py files
    Export('env')

    plugin_base = env.Clone()
    # for this to work you need:
    # if __GNUC__ >= 4
    # define MAPNIK_EXP __attribute__ ((visibility ("default")))
    #plugin_base.Append(CXXFLAGS='-fvisibility=hidden')
    #plugin_base.Append(CXXFLAGS='-fvisibility-inlines-hidden')

    Export('plugin_base')

    # clear the '_CPPDEFFLAGS' variable
    # for unknown reasons this variable puts -DNone
    # in the g++ args prompting unnecessary recompiles
    env['_CPPDEFFLAGS'] = None
    plugin_base['_CPPDEFFLAGS'] = None


    if env['FAST']:
        # caching is 'auto' by default in SCons
        # But let's also cache implicit deps...
        EnsureSConsVersion(0,98)
        SetOption('implicit_cache', 1)
        SetOption('max_drift', 1)

    else:
        # Set the cache mode to 'force' unless requested, avoiding hidden caching of Scons 'opts' in '.sconsign.dblite'
        # This allows for a SCONS_LOCAL_CONFIG, if present, to be used as the primary means of storing paths to successful build dependencies
        SetCacheMode('force')

    if env['JOBS'] > 1:
        SetOption("num_jobs", env['JOBS'])

    # Build agg first, doesn't need anything special
    if env['RUNTIME_LINK'] == 'shared':
        SConscript('deps/agg/build.py')

    # Build the core library
    SConscript('src/build.py')

    # Build the requested and able-to-be-compiled input plug-ins
    GDAL_BUILT = False
    OGR_BUILT = False
    for plugin in env['REQUESTED_PLUGINS']:
        details = env['PLUGINS'][plugin]
        if details['lib'] in env['LIBS']:
            SConscript('plugins/input/%s/build.py' % plugin)
            if plugin == 'ogr': OGR_BUILT = True
            if plugin == 'gdal': GDAL_BUILT = True
            if plugin == 'ogr' or plugin == 'gdal':
                if GDAL_BUILT and OGR_BUILT:
                    env['LIBS'].remove(details['lib'])
            else:
                env['LIBS'].remove(details['lib'])
        elif not details['lib']:
            # build internal shape and raster plugins
            SConscript('plugins/input/%s/build.py' % plugin)
        else:
            color_print(1,"Notice: dependencies not met for plugin '%s', not building..." % plugin)

    create_uninstall_target(env, env['MAPNIK_LIB_DIR_DEST'], False)
    create_uninstall_target(env, env['MAPNIK_INPUT_PLUGINS_DEST'] , False)
    create_uninstall_target(env, env['MAPNIK_INPUT_PLUGINS_DEST'] , False)

    # before installing plugins, wipe out any previously
    # installed plugins that we are no longer building
    if 'install' in COMMAND_LINE_TARGETS:
        for plugin in PLUGINS.keys():
            if plugin not in env['REQUESTED_PLUGINS']:
                plugin_path = os.path.join(env['MAPNIK_INPUT_PLUGINS_DEST'],'%s.input' % plugin)
                if os.path.exists(plugin_path):
                    color_print(1,"Notice: removing out of date plugin: '%s'" % plugin_path)
                    os.unlink(plugin_path)

    # Build the c++ rundemo app if requested
    if env['DEMO']:
        SConscript('demo/c++/build.py')

    # Build the pgsql2psqlite app if requested
    if env['PGSQL2SQLITE']:
        SConscript('utils/pgsql2sqlite/build.py')

    # Build shapeindex and remove its dependency from the LIBS
    if 'boost_program_options%s' % env['BOOST_APPEND'] in env['LIBS']:
        SConscript('utils/shapeindex/build.py')

        # devtools not ready for public
        #SConscript('utils/ogrindex/build.py')
        SConscript('utils/svg2png/build.py')
        env['LIBS'].remove('boost_program_options%s' % env['BOOST_APPEND'])
    else :
        color_print(1,"WARNING: Cannot find boost_program_options. 'shapeindex' won't be available")

    # Build the Python bindings
    if 'python' in env['BINDINGS']:
        SConscript('bindings/python/build.py')

        # Install the python speed testing scripts if python bindings will be available
        SConscript('utils/performance/build.py')

    # Install the mapnik upgrade script
    SConscript('utils/upgrade_map_xml/build.py')

    # Configure fonts and if requested install the bundled DejaVu fonts
    SConscript('fonts/build.py')

    # build C++ tests
    # not ready for release
    SConscript('tests/cpp_tests/build.py')

    # not ready for release
    #if env['SVG_RENDERER']:
    #    SConscript('tests/cpp_tests/svg_renderer_tests/build.py')

    # install pkg-config script and mapnik-config script
    SConscript('utils/mapnik-config/build.py')

    # write the viewer.ini file
    SConscript('demo/viewer/build.py')

    # if requested, build the sample input plugins
    if env['SAMPLE_INPUT_PLUGINS']:
        SConscript('plugins/input/templates/helloworld/build.py')
