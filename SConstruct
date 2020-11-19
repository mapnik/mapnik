# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2017 Artem Pavlenko
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

from __future__ import print_function # support python2

import os
import sys
import re
import platform
from glob import glob
from copy import copy
from subprocess import Popen, PIPE
import pickle

try:
    import distutils.sysconfig
    HAS_DISTUTILS = True
except:
    HAS_DISTUTILS = False

try:
    # Python 3.3+
    from shlex import quote as shquote
except:
    # Python 2.7
    from pipes import quote as shquote

try:
    # Python 3.3+
    from subprocess import DEVNULL
except:
    # Python 2.7
    DEVNULL = open(os.devnull, 'w')

LIBDIR_SCHEMA_DEFAULT='lib'
severities = ['debug', 'warn', 'error', 'none']

ICU_INCLUDES_DEFAULT='/usr/include'
ICU_LIBS_DEFAULT='/usr/'

DEFAULT_CC = "cc"
DEFAULT_CXX = "c++"
DEFAULT_CXX_STD = "14"
DEFAULT_CXX_CXXFLAGS = " -DU_USING_ICU_NAMESPACE=0"
DEFAULT_CXX_LINKFLAGS = ""
if sys.platform == 'darwin':
    # homebrew default
    ICU_INCLUDES_DEFAULT='/usr/local/opt/icu4c/include/'
    ICU_LIBS_DEFAULT='/usr/local/opt/icu4c/'

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
BOOST_MIN_VERSION = '1.61'
#CAIRO_MIN_VERSION = '1.8.0'

HARFBUZZ_MIN_VERSION = (0, 9, 34)
HARFBUZZ_MIN_VERSION_STRING = "%s.%s.%s" % HARFBUZZ_MIN_VERSION


DEFAULT_LINK_PRIORITY = ['internal','other','frameworks','user','system']


pretty_dep_names = {
    'clntsh':'Oracle database library | configure with OCCI_LIBS & OCCI_INCLUDES | more info: https://github.com/mapnik/mapnik/wiki/OCCI',
    'gdal':'GDAL C++ library | configured using gdal-config program | try setting GDAL_CONFIG SCons option | more info: https://github.com/mapnik/mapnik/wiki/GDAL',
    'ogr':'OGR-enabled GDAL C++ Library | configured using gdal-config program | try setting GDAL_CONFIG SCons option | more info: https://github.com/mapnik/mapnik/wiki/OGR',
    'cairo':'Cairo C library | configured using pkg-config | try setting PKG_CONFIG_PATH SCons option',
    'proj':'Proj.4 C Projections library | configure with PROJ_LIBS & PROJ_INCLUDES | more info: http://trac.osgeo.org/proj/',
    'pg':'Postgres C Library required for PostGIS plugin | configure with pg_config program or configure with PG_LIBS & PG_INCLUDES | more info: https://github.com/mapnik/mapnik/wiki/PostGIS',
    'sqlite3':'SQLite3 C Library | configure with SQLITE_LIBS & SQLITE_INCLUDES | more info: https://github.com/mapnik/mapnik/wiki/SQLite',
    'jpeg':'JPEG C library | configure with JPEG_LIBS & JPEG_INCLUDES',
    'tiff':'TIFF C library | configure with TIFF_LIBS & TIFF_INCLUDES',
    'png':'PNG C library | configure with PNG_LIBS & PNG_INCLUDES',
    'webp':'WEBP C library | configure with WEBP_LIBS & WEBP_INCLUDES',
    'icuuc':'ICU C++ library | configure with ICU_LIBS & ICU_INCLUDES or use ICU_LIB_NAME to specify custom lib name  | more info: http://site.icu-project.org/',
    'harfbuzz':'HarfBuzz text shaping library | configure with HB_LIBS & HB_INCLUDES',
    'harfbuzz-min-version':'HarfBuzz >= %s (required for font-feature-settings support)' % HARFBUZZ_MIN_VERSION_STRING,
    'z':'Z compression library | more info: http://www.zlib.net/',
    'm':'Basic math library, part of C++ stlib',
    'pkg-config':'pkg-config tool | more info: http://pkg-config.freedesktop.org',
    'pg_config':'pg_config program | try setting PG_CONFIG SCons option',
    'pq':'libpq library (postgres client) | try setting PG_CONFIG SCons option or configure with PG_LIBS & PG_INCLUDES',
    'xml2-config':'xml2-config program | try setting XML2_CONFIG SCons option or avoid the need for xml2-config command by configuring with XML2_LIBS & XML2_INCLUDES',
    'libxml2':'libxml2 library | try setting XML2_CONFIG SCons option to point to location of xml2-config program or configure with XML2_LIBS & XML2_INCLUDES',
    'gdal-config':'gdal-config program | try setting GDAL_CONFIG SCons option',
    'freetype-config':'freetype-config program | try setting FREETYPE_CONFIG SCons option or configure with FREETYPE_LIBS & FREETYPE_INCLUDES',
    'freetype':'libfreetype library | try setting FREETYPE_CONFIG SCons option or configure with FREETYPE_LIBS & FREETYPE_INCLUDES',
    'osm':'more info: https://github.com/mapnik/mapnik/wiki/OsmPlugin',
    'boost_regex_icu':'libboost_regex built with optional ICU unicode support is needed for unicode regex support in mapnik.',
    'sqlite_rtree':'The SQLite plugin requires libsqlite3 built with RTREE support (-DSQLITE_ENABLE_RTREE=1)',
    'pgsql2sqlite_rtree':'The pgsql2sqlite program requires libsqlite3 built with RTREE support (-DSQLITE_ENABLE_RTREE=1)',
    'PROJ_LIB':'The directory where proj4 stores its data files. Must exist for proj4 to work correctly',
    'GDAL_DATA':'The directory where GDAL stores its data files. Must exist for GDAL to work correctly',
    'ICU_DATA':'The directory where icu stores its data files. If ICU reports a path, it must exist. ICU can also be built without .dat files and in that case this path is empty'
    }

# Core plugin build configuration
# opts.AddVariables still hardcoded however...
PLUGINS = { # plugins with external dependencies
            # configured by calling project, hence 'path':None
            'postgis': {'default':True,'path':None,'inc':'libpq-fe.h','lib':'pq','lang':'C'},
            'pgraster': {'default':True,'path':None,'inc':'libpq-fe.h','lib':'pq','lang':'C'},
            'gdal':    {'default':True,'path':None,'inc':'gdal_priv.h','lib':'gdal','lang':'C++'},
            'ogr':     {'default':True,'path':None,'inc':'ogrsf_frmts.h','lib':'gdal','lang':'C++'},
            'sqlite':  {'default':True,'path':'SQLITE','inc':'sqlite3.h','lib':'sqlite3','lang':'C'},
            # plugins without external dependencies requiring CheckLibWithHeader...
            'shape':   {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'csv':     {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'raster':  {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'geojson': {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'geobuf':  {'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'},
            'topojson':{'default':True,'path':None,'inc':None,'lib':None,'lang':'C++'}
            }


def init_environment(env):
    env.Decider('MD5-timestamp')
    env['ORIGIN'] = Literal('$ORIGIN')
    env['ENV']['ORIGIN'] = '$ORIGIN'
    if os.environ.get('RANLIB'):
        env['RANLIB'] = os.environ['RANLIB']
    if os.environ.get('AR'):
        env['AR'] = os.environ['AR']

#### SCons build options and initial setup ####
env = Environment(ENV=os.environ)

init_environment(env)

def fix_path(path):
    return str(os.path.abspath(path))

def color_print(color,text,newline=True):
    # 1 - red
    # 2 - green
    # 3 - yellow
    # 4 - blue
    text = "\033[9%sm%s\033[0m" % (color,text)
    if not newline:
        print (text, end='')
    else:
        print (text)

def regular_print(color,text,newline=True):
    if not newline:
        print (text, end = '')
    else:
        print (text)

def shell_command(cmd, *args, **kwargs):
    """ Run command through shell.

    `cmd` should be a valid, properly shell-quoted command.

    Additional positional arguments, if provided, will each
    be individually quoted as necessary and appended to `cmd`,
    separated by spaces.

    `logstream` optional keyword argument should be either:
        - a file-like object, into which the command-line
          and the command's STDERR output will be written; or
        - None, in which case STDERR will go to DEVNULL.

    Additional keyword arguments will be passed to `Popen`.

    Returns a tuple `(result, output)` where:
    `result` = True if the command completed successfully,
               False otherwise
    `output` = captured STDOUT with trailing whitespace removed
    """
    # `cmd` itself is intentionally not wrapped in `shquote` here
    # in order to support passing user-provided commands that may
    # include arguments. For example:
    #
    #   ret, out = shell_command(env['CXX'], '--version')
    #
    # needs to work even if `env['CXX'] == 'ccache c++'`
    #
    if args:
        cmdstr = ' '.join([cmd] + [shquote(a) for a in args])
    else:
        cmdstr = cmd
    # redirect STDERR to `logstream` if provided
    try:
        logstream = kwargs.pop('logstream')
    except KeyError:
        logstream = None
    else:
        if logstream is not None:
            logstream.write(cmdstr + '\n')
            kwargs['stderr'] = logstream
        else:
            kwargs['stderr'] = DEVNULL
    # execute command and capture output
    proc = Popen(cmdstr, shell=True, stdout=PIPE, **kwargs)
    out, err = proc.communicate()
    try:
        outtext = out.decode(sys.stdout.encoding or 'UTF-8').rstrip()
    except UnicodeDecodeError:
        outtext = out.decode('UTF-8', errors='replace').rstrip()
    if logstream is not None and outtext:
        logstream.write('->\t' + outtext.replace('\n', '\n->\t') + '\n')
    return proc.returncode == 0, outtext

def silent_command(cmd, *args):
    return shell_command(cmd, *args, stderr=DEVNULL)

def config_command(cmd, *args):
    return shell_command(cmd, *args, logstream=conf.logstream)

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

def rm_path(item,set,_env):
    for i in _env[set]:
        if i.startswith(item):
            _env[set].remove(i)

def sort_paths(items,priority):
    """Sort paths such that compiling and linking will globally prefer custom or local libs
    over system libraries by fixing up the order libs are passed to the compiler and the linker.

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
        if path in path_types:
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
        return '%s (%s)' % (dep,'more info see: https://github.com/mapnik/mapnik/wiki/Mapnik-Installation & http://www.boost.org')
    return dep

def pretty_deps(indent, deps):
    return indent + indent.join(pretty_dep(dep) for dep in deps)


DEFAULT_PLUGINS = []
for k,v in PLUGINS.items():
   if v['default']:
       DEFAULT_PLUGINS.append(k)

# All of the following options may be modified at the command-line, for example:
# `python scons/scons.py PREFIX=/opt`
opts = Variables()

opts.AddVariables(
    # Compiler options
    ('CXX', 'The C++ compiler to use to compile mapnik', DEFAULT_CXX),
    ('CXX_STD', 'The C++ compiler standard (string).', DEFAULT_CXX_STD),
    ('CC', 'The C compiler used for configure checks of C libs.', DEFAULT_CC),
    ('CUSTOM_CXXFLAGS', 'Custom C++ flags, e.g. -I<include dir> if you have headers in a nonstandard directory <include dir>', ''),
    ('CUSTOM_DEFINES', 'Custom Compiler DEFINES, e.g. -DENABLE_THIS', ''),
    ('CUSTOM_CFLAGS', 'Custom C flags, e.g. -I<include dir> if you have headers in a nonstandard directory <include dir> (only used for configure checks)', ''),
    ('CUSTOM_LDFLAGS', 'Custom linker flags, e.g. -L<lib dir> if you have libraries in a nonstandard directory <lib dir>', ''),
    EnumVariable('LINKING', "Set library format for libmapnik",'shared', ['shared','static']),
    EnumVariable('RUNTIME_LINK', "Set preference for linking dependencies",'shared', ['shared','static']),
    EnumVariable('OPTIMIZATION','Set compiler optimization level','3', ['0','1','2','3','4','s']),
    # Note: setting DEBUG=True will override any custom OPTIMIZATION level
    BoolVariable('DEBUG', 'Compile a debug version of Mapnik', 'False'),
    BoolVariable('COVERAGE', 'Compile a libmapnik and plugins with --coverage', 'False'),
    ListVariable('INPUT_PLUGINS','Input drivers to include',DEFAULT_PLUGINS,PLUGINS.keys()),
    ('WARNING_CXXFLAGS', 'Compiler flags you can set to reduce warning levels which are placed after -Wall.', ''),

    # SCons build behavior options
    ('HOST', 'Set the target host for cross compiling', ''),
    ('CONFIG', "The path to the python file in which to save user configuration options. Currently : '%s'" % SCONS_LOCAL_CONFIG,SCONS_LOCAL_CONFIG),
    BoolVariable('USE_CONFIG', "Use SCons user '%s' file (will also write variables after successful configuration)", 'True'),
    BoolVariable('NO_ATEXIT', 'Will prevent Singletons from being deleted atexit of main thread', 'False'),
    BoolVariable('NO_DLCLOSE', 'Will prevent plugins from being unloaded', 'False'),
    BoolVariable('ENABLE_GLIBC_WORKAROUND', "Workaround known GLIBC symbol exports to allow building against libstdc++-4.8 without binaries needing throw_out_of_range_fmt", 'False'),
    # http://www.scons.org/wiki/GoFastButton
    # http://stackoverflow.com/questions/1318863/how-to-optimize-an-scons-script
    BoolVariable('PRIORITIZE_LINKING', 'Sort list of lib and inc directories to ensure preferential compiling and linking (useful when duplicate libs)', 'True'),
    ('LINK_PRIORITY','Priority list in which to sort library and include paths (default order is internal, other, frameworks, user, then system - see source of `sort_paths` function for more detail)',','.join(DEFAULT_LINK_PRIORITY)),

    # Install Variables
    ('PREFIX', 'The install path "prefix"', '/usr/local'),
    ('LIBDIR_SCHEMA', 'The library sub-directory appended to the "prefix", sometimes lib64 on 64bit linux systems', LIBDIR_SCHEMA_DEFAULT),
    ('DESTDIR', 'The root directory to install into. Useful mainly for binary package building', '/'),
    ('PATH', 'A custom path (or multiple paths divided by ":") to append to the $PATH env to prioritize usage of command line programs (if multiple are present on the system)', ''),
    ('PATH_REMOVE', 'A path prefix to exclude from all known command and compile paths (create multiple excludes separated by :)', ''),
    ('PATH_REPLACE', 'Two path prefixes (divided with a :) to search/replace from all known command and compile paths', ''),
    ('MAPNIK_NAME', 'Name of library', 'mapnik'),

    # Boost variables
    # default is '/usr/include', see FindBoost method below
    ('BOOST_INCLUDES', 'Search path for boost include files', '',False),
    # default is '/usr/' + LIBDIR_SCHEMA, see FindBoost method below
    ('BOOST_LIBS', 'Search path for boost library files', '',False),
    ('BOOST_TOOLKIT','Specify boost toolkit, e.g., gcc41.','',False),
    ('BOOST_ABI', 'Specify boost ABI, e.g., d.','',False),
    ('BOOST_VERSION','Specify boost version, e.g., 1_35.','',False),

    # Variables for required dependencies
    ('FREETYPE_CONFIG', 'The path to the freetype-config executable.', 'freetype-config'),
    ('XML2_CONFIG', 'The path to the xml2-config executable.', 'xml2-config'),
    PathVariable('ICU_INCLUDES', 'Search path for ICU include files', ICU_INCLUDES_DEFAULT, PathVariable.PathAccept),
    PathVariable('ICU_LIBS','Search path for ICU include files',ICU_LIBS_DEFAULT + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    ('ICU_LIB_NAME', 'The library name for icu (such as icuuc, sicuuc, or icucore)', 'icuuc', PathVariable.PathAccept),
    PathVariable('HB_INCLUDES', 'Search path for HarfBuzz include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('HB_LIBS','Search path for HarfBuzz include files','/usr/' + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    BoolVariable('PNG', 'Build Mapnik with PNG read and write support', 'True'),
    PathVariable('PNG_INCLUDES', 'Search path for libpng include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('PNG_LIBS','Search path for libpng library files','/usr/' + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    BoolVariable('JPEG', 'Build Mapnik with JPEG read and write support', 'True'),
    PathVariable('JPEG_INCLUDES', 'Search path for libjpeg include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('JPEG_LIBS', 'Search path for libjpeg library files', '/usr/' + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    BoolVariable('TIFF', 'Build Mapnik with TIFF read and write support', 'True'),
    PathVariable('TIFF_INCLUDES', 'Search path for libtiff include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('TIFF_LIBS', 'Search path for libtiff library files', '/usr/' + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    BoolVariable('WEBP', 'Build Mapnik with WEBP read', 'True'),
    PathVariable('WEBP_INCLUDES', 'Search path for libwebp include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('WEBP_LIBS','Search path for libwebp library files','/usr/' + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    BoolVariable('PROJ', 'Build Mapnik with proj4 support to enable transformations between many different projections', 'True'),
    PathVariable('PROJ_INCLUDES', 'Search path for PROJ.4 include files', '/usr/include', PathVariable.PathAccept),
    PathVariable('PROJ_LIBS', 'Search path for PROJ.4 library files', '/usr/' + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    ('PG_INCLUDES', 'Search path for libpq (postgres client) include files', ''),
    ('PG_LIBS', 'Search path for libpq (postgres client) library files', ''),
    ('FREETYPE_INCLUDES', 'Search path for Freetype include files', ''),
    ('FREETYPE_LIBS', 'Search path for Freetype library files', ''),
    ('XML2_INCLUDES', 'Search path for libxml2 include files', ''),
    ('XML2_LIBS', 'Search path for libxml2 library files', ''),
    ('PKG_CONFIG_PATH', 'Use this path to point pkg-config to .pc files instead of the PKG_CONFIG_PATH environment setting',''),

    # Variables affecting rendering back-ends

    BoolVariable('GRID_RENDERER', 'build support for native grid renderer', 'True'),
    BoolVariable('SVG_RENDERER', 'build support for native svg renderer', 'False'),
    BoolVariable('CPP_TESTS', 'Compile the C++ tests', 'True'),
    BoolVariable('BENCHMARK', 'Compile the C++ benchmark scripts', 'False'),

    # Variables for optional dependencies
    # Note: cairo and and pycairo are optional but configured automatically through pkg-config
    # Therefore, we use a single boolean for whether to attempt to build cairo support.
    BoolVariable('CAIRO', 'Attempt to build with Cairo rendering support', 'True'),
    PathVariable('CAIRO_INCLUDES', 'Search path for cairo include files', '',PathVariable.PathAccept),
    PathVariable('CAIRO_LIBS', 'Search path for cairo library files','',PathVariable.PathAccept),
    ('GDAL_CONFIG', 'The path to the gdal-config executable for finding gdal and ogr details.', 'gdal-config'),
    ('PG_CONFIG', 'The path to the pg_config executable.', 'pg_config'),
    PathVariable('OCCI_INCLUDES', 'Search path for OCCI include files', '/usr/lib/oracle/10.2.0.3/client/include', PathVariable.PathAccept),
    PathVariable('OCCI_LIBS', 'Search path for OCCI library files', '/usr/lib/oracle/10.2.0.3/client/'+ LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    PathVariable('SQLITE_INCLUDES', 'Search path for SQLITE include files', '/usr/include/', PathVariable.PathAccept),
    PathVariable('SQLITE_LIBS', 'Search path for SQLITE library files', '/usr/' + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),
    PathVariable('RASTERLITE_INCLUDES', 'Search path for RASTERLITE include files', '/usr/include/', PathVariable.PathAccept),
    PathVariable('RASTERLITE_LIBS', 'Search path for RASTERLITE library files', '/usr/' + LIBDIR_SCHEMA_DEFAULT, PathVariable.PathAccept),

    # Variables for logging and statistics
    BoolVariable('ENABLE_LOG', 'Enable logging, which is enabled by default when building in *debug*', 'False'),
    BoolVariable('ENABLE_STATS', 'Enable global statistics during map processing', 'False'),
    ('DEFAULT_LOG_SEVERITY', 'The default severity of the logger (eg. ' + ', '.join(severities) + ')', 'error'),

    # Plugin linking
    EnumVariable('PLUGIN_LINKING', "Set plugin linking with libmapnik", 'shared', ['shared','static']),

    # Other variables
    BoolVariable('MEMORY_MAPPED_FILE', 'Utilize memory-mapped files in Shapefile Plugin (higher memory usage, better performance)', 'True'),
    ('SYSTEM_FONTS','Provide location for python bindings to register fonts (if provided then the bundled DejaVu fonts are not installed)',''),
    ('LIB_DIR_NAME','Name to use for the subfolder beside libmapnik where fonts and plugins are installed','mapnik'),
    PathVariable('PYTHON','Full path to Python executable used to build bindings', sys.executable),
    BoolVariable('FULL_LIB_PATH', 'Embed the full and absolute path to libmapnik when linking ("install_name" on OS X/rpath on Linux)', 'True'),
    BoolVariable('ENABLE_SONAME', 'Embed a soname in libmapnik on Linux', 'True'),
    EnumVariable('THREADING','Set threading support','multi', ['multi','single']),
    EnumVariable('XMLPARSER','Set xml parser','ptree', ['libxml2','ptree']),
    BoolVariable('DEMO', 'Compile demo c++ application', 'True'),
    BoolVariable('PGSQL2SQLITE', 'Compile and install a utility to convert postgres tables to sqlite', 'False'),
    BoolVariable('SHAPEINDEX', 'Compile and install a utility to generate shapefile indexes in the custom format (.index) Mapnik supports', 'True'),
    BoolVariable('MAPNIK_INDEX', 'Compile and install a utility to generate spatial indexes for CSV and GeoJSON in the custom format (.index) Mapnik supports', 'True'),
    BoolVariable('SVG2PNG', 'Compile and install a utility to generate render an svg file to a png on the command line', 'False'),
    BoolVariable('MAPNIK_RENDER', 'Compile and install a utility to render a map to an image', 'True'),
    BoolVariable('COLOR_PRINT', 'Print build status information in color', 'True'),
    BoolVariable('BIGINT', 'Compile support for 64-bit integers in mapnik::value', 'True'),
    BoolVariable('QUIET', 'Reduce build verbosity', 'False'),
    )

# variables to pickle after successful configure step
# these include all scons core variables as well as custom
# env variables needed in SConscript files
pickle_store = [# Scons internal variables
        'CC', # compiler user to check if c deps compile during configure
        'CXX', # C++ compiler to compile mapnik
        'CXX_STD', # C++ standard e.g 17 (as in -std=c++17)
        'CFLAGS',
        'CPPDEFINES',
        'CPPFLAGS', # c preprocessor flags
        'CPPPATH',
        'CXXFLAGS', # C++ flags built up during configure
        'LIBPATH',
        'LIBS',
        'LINKFLAGS',
        'RPATH',
        'CUSTOM_LDFLAGS', # user submitted
        'CUSTOM_DEFINES', # user submitted
        'CUSTOM_CXXFLAGS', # user submitted
        'CUSTOM_CFLAGS', # user submitted
        'MAPNIK_LIB_NAME',
        'LINK',
        'RUNTIME_LINK',
        # Mapnik's SConstruct build variables
        'PLUGINS',
        'ABI_VERSION',
        'MAPNIK_VERSION_STRING',
        'MAPNIK_VERSION',
        'PLATFORM',
        'BOOST_ABI',
        'BOOST_APPEND',
        'LIBDIR_SCHEMA',
        'REQUESTED_PLUGINS',
        'COLOR_PRINT',
        'HAS_CAIRO',
        'MAPNIK_HAS_DLFCN',
        'HAS_PYCAIRO',
        'PYCAIRO_PATHS',
        'HAS_LIBXML2',
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
        'MAPNIK_BUNDLED_SHARE_DIRECTORY',
        'MAPNIK_LIB_BASE',
        'MAPNIK_LIB_BASE_DEST',
        'EXTRA_FREETYPE_LIBS',
        'LIBMAPNIK_CPPATHS',
        'LIBMAPNIK_DEFINES',
        'LIBMAPNIK_CXXFLAGS',
        'CAIRO_LIBPATHS',
        'CAIRO_ALL_LIBS',
        'CAIRO_CPPPATHS',
        'GRID_RENDERER',
        'SVG_RENDERER',
        'SQLITE_LINKFLAGS',
        'BOOST_LIB_VERSION_FROM_HEADER',
        'BIGINT',
        'HOST',
        'QUERIED_GDAL_DATA',
        'QUERIED_ICU_DATA',
        'QUERIED_PROJ_LIB',
        'QUIET'
        ]

# Add all other user configurable options to pickle pickle_store
# We add here more options than are needed for the build stage
# but helpful so that scons -h shows the exact cached options
for opt in opts.options:
    if opt.key not in pickle_store:
        pickle_store.append(opt.key)

def rollback_option(env, variable):
    global opts
    for item in opts.options:
        if item.key == variable:
            env[variable] = item.default

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

# need no-op for clean on fresh checkout
# https://github.com/mapnik/mapnik/issues/2112
if not os.path.exists(SCONS_LOCAL_LOG) and not os.path.exists(SCONS_CONFIGURE_CACHE) \
  and ('-c' in command_line_args or '--clean' in command_line_args):
    print ('all good: nothing to clean, but you might want to run "make distclean"')
    Exit(0)

# initially populate environment with defaults and any possible custom arguments
opts.Update(env)

# if we are not configuring overwrite environment with pickled settings
if not force_configure:
    if os.path.exists(SCONS_CONFIGURE_CACHE):
        try:
            pickled_environment = open(SCONS_CONFIGURE_CACHE, 'rb')
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

if 'COLOR_PRINT' in env and env['COLOR_PRINT'] == False:
    color_print = regular_print

if sys.platform == "win32":
    color_print = regular_print

color_print(4,'\nWelcome to Mapnik...\n')

#### Custom Configure Checks ###

def prioritize_paths(context, silent=True):
    env = context.env
    prefs = env['LINK_PRIORITY'].split(',')
    if not silent:
        context.Message( 'Sorting lib and inc compiler paths...')
    env['LIBPATH'] = sort_paths(env['LIBPATH'], prefs)
    env['CPPPATH'] = sort_paths(env['CPPPATH'], prefs)
    if silent:
        context.did_show_result=1
    ret = context.Result( True )
    return ret

def CheckPKGConfig(context, version):
    context.Message( 'Checking for pkg-config... ' )
    context.sconf.cached = False
    ret, _ = config_command('pkg-config --atleast-pkgconfig-version', version)
    context.Result( ret )
    return ret

def CheckPKG(context, name):
    context.Message( 'Checking for %s... ' % name )
    context.sconf.cached = False
    ret, _ = config_command('pkg-config --exists', name)
    context.Result( ret )
    return ret

def CheckPKGVersion(context, name, version):
    context.Message( 'Checking for at least version %s for %s... ' % (version,name) )
    context.sconf.cached = False
    ret, _ = config_command('pkg-config --atleast-version', version, name)
    context.Result( ret )
    return ret

def parse_config(context, config, checks='--libs --cflags'):
    env = context.env
    tool = config.lower().replace('_','-')
    toolname = tool
    if config in ('GDAL_CONFIG'):
        toolname += ' %s' % checks
    context.Message( 'Checking for %s... ' % toolname)
    context.sconf.cached = False
    cmd = '%s %s' % (env[config], checks)
    ret, value = config_command(cmd)
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
                    if value and b'-framework GDAL' in value:
                        env['LIBS'].append('gdal')
                        if os.path.exists('/Library/Frameworks/GDAL.framework/unix/lib'):
                            env['LIBPATH'].insert(0,'/Library/Frameworks/GDAL.framework/unix/lib')
                    if 'GDAL' in env.get('FRAMEWORKS',[]):
                        env["FRAMEWORKS"].remove("GDAL")
            else:
                env.ParseConfig(cmd)
            parsed = True
        except OSError as e:
            ret = False
            print (' (xml2-config not found!)')
    if not parsed:
        if config in ('GDAL_CONFIG'):
            # optional deps...
            if tool not in env['SKIPPED_DEPS']:
                env['SKIPPED_DEPS'].append(tool)
            rollback_option(env, config)
        else: # freetype and libxml2, not optional
            if tool not in env['MISSING_DEPS']:
                env['MISSING_DEPS'].append(tool)
    context.Result( ret )
    return ret

def get_pkg_lib(context, config, lib):
    libpattern = r'-l([^\s]*)'
    libname = None
    env = context.env
    context.Message( 'Checking for name of %s library... ' % lib)
    context.sconf.cached = False
    ret, value = config_command(env[config], '--libs')
    parsed = False
    if ret:
        try:
            if ' ' in value:
                parts = value.split(' ')
                if len(parts) > 1:
                    value = parts[1]
            libnames = re.findall(libpattern, value)
            if libnames:
                libname = libnames[0]
            else:
                # osx 1.8 install gives '-framework GDAL'
                libname = 'gdal'
        except Exception as e:
            ret = False
            print (' unable to determine library name:# {0!s}'.format(e))
            return None
    context.Result( libname )
    return libname

def parse_pg_config(context, config):
    # TODO - leverage `LDFLAGS_SL` if RUNTIME_LINK==static
    env = context.env
    tool = config.lower()
    context.Message( 'Checking for %s... ' % tool)
    context.sconf.cached = False
    ret, lib_path = config_command(env[config], '--libdir')
    ret, inc_path = config_command(env[config], '--includedir')
    if ret:
        env.AppendUnique(CPPPATH = fix_path(inc_path))
        env.AppendUnique(LIBPATH = fix_path(lib_path))
        lpq = env['PLUGINS']['postgis']['lib']
        env.Append(LIBS = lpq)
    else:
        env['SKIPPED_DEPS'].append(tool)
        rollback_option(env, config)
    context.Result( ret )
    return ret

def ogr_enabled(context):
    env = context.env
    context.Message( 'Checking if gdal is ogr enabled... ')
    context.sconf.cached = False
    ret, out = config_command(env['GDAL_CONFIG'], '--ogr-enabled')
    if ret:
        ret = (out == 'yes')
    if not ret:
        if 'ogr' not in env['SKIPPED_DEPS']:
            env['SKIPPED_DEPS'].append('ogr')
    context.Result( ret )
    return ret

def FindBoost(context, prefixes, thread_flag):
    """Routine to auto-find boost header dir, lib dir, and library naming structure.

    """
    context.Message( 'Searching for boost libs and headers... ' )
    env = context.env

    BOOST_LIB_DIR = None
    BOOST_INCLUDE_DIR = None
    BOOST_APPEND = None
    env['BOOST_APPEND'] = str()
    search_lib = 'libboost_filesystem'

    # note: must call normpath to strip trailing slash otherwise dirname
    # does not remove 'lib' and 'include'
    prefixes.insert(0,os.path.dirname(os.path.normpath(env['BOOST_INCLUDES'])))
    prefixes.insert(0,os.path.dirname(os.path.normpath(env['BOOST_LIBS'])))
    for searchDir in prefixes:
        libItems = glob(os.path.join(searchDir, env['LIBDIR_SCHEMA'], '%s*.*' % search_lib))
        if not libItems:
            libItems = glob(os.path.join(searchDir, 'lib/%s*.*' % search_lib))
        incItems = glob(os.path.join(searchDir, 'include/boost*/'))
        if len(libItems) >= 1 and len(incItems) >= 1:
            BOOST_LIB_DIR = os.path.dirname(libItems[0])
            BOOST_INCLUDE_DIR = incItems[0].rstrip('boost/')
            shortest_lib_name = min(libItems, key=len)
            match = re.search(r'%s(.*)\..*' % search_lib, shortest_lib_name)
            if hasattr(match,'groups'):
                BOOST_APPEND = match.groups()[0]
            break

    msg = str()

    if BOOST_LIB_DIR:
        msg += '\nFound boost libs: %s' % BOOST_LIB_DIR
        env['BOOST_LIBS'] = BOOST_LIB_DIR
    elif not env['BOOST_LIBS']:
        env['BOOST_LIBS'] = '/usr/' + env['LIBDIR_SCHEMA']
        msg += '\nUsing default boost lib dir: %s' % env['BOOST_LIBS']
    else:
        msg += '\nUsing boost lib dir: %s' % env['BOOST_LIBS']

    if BOOST_INCLUDE_DIR:
        msg += '\nFound boost headers: %s' % BOOST_INCLUDE_DIR
        env['BOOST_INCLUDES'] = BOOST_INCLUDE_DIR
    elif not env['BOOST_INCLUDES']:
        env['BOOST_INCLUDES'] = '/usr/include'
        msg += '\nUsing default boost include dir: %s' % env['BOOST_INCLUDES']
    else:
        msg += '\nUsing boost include dir: %s' % env['BOOST_INCLUDES']

    if not env['BOOST_TOOLKIT'] and not env['BOOST_ABI'] and not env['BOOST_VERSION']:
        if BOOST_APPEND:
            msg += '\nFound boost lib name extension: %s' % BOOST_APPEND
            env['BOOST_APPEND'] = BOOST_APPEND
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
        msg += '\nFound boost lib name extension: %s' % env['BOOST_APPEND']

    env.AppendUnique(CPPPATH = fix_path(env['BOOST_INCLUDES']))
    env.AppendUnique(LIBPATH = fix_path(env['BOOST_LIBS']))
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

def CheckIcuData(context, silent=False):

    if not silent:
        context.Message('Checking for ICU data directory...')
    ret, out = context.TryRun("""

#include <unicode/putil.h>
#include <iostream>

int main() {
    std::string result = u_getDataDirectory();
    std::cout << result;
    if (result.empty()) {
        return -1;
    }
    return 0;
}

""", '.cpp')
    if silent:
        context.did_show_result=1
    if ret:
        value = out.strip()
        context.Result('u_getDataDirectory returned %s' % value)
        return value
    else:
        ret, value = config_command('icu-config --icudatadir')
        if ret:
            context.Result('icu-config returned %s' % value)
            return value
        else:
            context.Result('Failed to detect (mapnik-config will have null value)')
            return ''


def CheckGdalData(context, silent=False):
    env = context.env
    if not silent:
        context.Message('Checking for GDAL data directory... ')
    context.sconf.cached = False
    ret, out = config_command(env['GDAL_CONFIG'], '--datadir')
    value = out.strip()
    if silent:
        context.did_show_result=1
    if ret:
        context.Result('%s returned %s' % (env['GDAL_CONFIG'], value))
    else:
        context.Result('Failed to detect (mapnik-config will have null value)')
    return value


def CheckProjData(context, silent=False):

    if not silent:
        context.Message('Checking for PROJ_LIB directory...')
    ret, out = context.TryRun("""

// This is narly, could eventually be replaced using https://github.com/OSGeo/proj.4/pull/551]
#include <proj_api.h>
#include <iostream>
#include <cstring>

static void my_proj4_logger(void * user_data, int /*level*/, const char * msg)
{
    std::string* posMsg = static_cast<std::string*>(user_data);
    *posMsg += msg;
}

// https://github.com/OSGeo/gdal/blob/ddbf6d39aa4b005a77ca4f27c2d61a3214f336f8/gdal/alg/gdalapplyverticalshiftgrid.cpp#L616-L633

std::string find_proj_path(const char * pszFilename) {
    std::string osMsg;
    std::string osFilename;
    projCtx ctx = pj_ctx_alloc();
    pj_ctx_set_app_data(ctx, &osMsg);
    pj_ctx_set_debug(ctx, PJ_LOG_DEBUG_MAJOR);
    pj_ctx_set_logger(ctx, my_proj4_logger);
    PAFile f = pj_open_lib(ctx, pszFilename, "rb");
    if( f )
    {
        pj_ctx_fclose(ctx, f);
    }
    size_t nPos = osMsg.find("fopen(");
    if( nPos != std::string::npos )
    {
        osFilename = osMsg.substr(nPos + strlen("fopen("));
        nPos = osFilename.find(")");
        if( nPos != std::string::npos )
            osFilename = osFilename.substr(0, nPos);
    }
    pj_ctx_free(ctx);
    return osFilename;
}


int main() {
    std::string result = find_proj_path(" ");
    std::cout << result;
    if (result.empty()) {
        return -1;
    }
    return 0;
}

""", '.cpp')
    value = out.strip()
    if silent:
        context.did_show_result=1
    if ret:
        context.Result('pj_open_lib returned %s' % value)
    else:
        context.Result('Failed to detect (mapnik-config will have null value)')
    return value

def CheckCairoHasFreetype(context, silent=False):
    if not silent:
        context.Message('Checking for cairo freetype font support ... ')
    context.env.AppendUnique(CPPPATH=copy(env['CAIRO_CPPPATHS']))

    ret, out = context.TryRun("""

#include <cairo-features.h>

int main()
{
    #ifdef CAIRO_HAS_FT_FONT
    return 0;
    #else
    return 1;
    #endif
}

""", '.cpp')
    if silent:
        context.did_show_result=1
    context.Result(ret)
    for item in env['CAIRO_CPPPATHS']:
        rm_path(item,'CPPPATH',context.env)
    return ret

def CheckHasDlfcn(context, silent=False):
    if not silent:
        context.Message('Checking for dlfcn.h support ... ')
    ret = context.TryCompile("""

#include <dlfcn.h>

int main()
{
    return 0;
}

""", '.cpp')
    if silent:
        context.did_show_result=1
    context.Result(ret)
    return ret

def GetBoostLibVersion(context):
    ret, out = context.TryRun("""

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
    context.Result(ret)
    return out.strip()

def CheckBoostScopedEnum(context, silent=False):
    if not silent:
        context.Message('Checking whether Boost was compiled with C++11 scoped enums ... ')
    ret = context.TryLink("""
#include <boost/filesystem.hpp>

int main()
{
    boost::filesystem::path a, b;
    boost::filesystem::copy_file(a, b);
    return 0;
}
""", '.cpp')
    if silent:
        context.did_show_result=1
    context.Result(ret)
    return ret

def icu_at_least(context, min_version_str):
    context.Message('Checking for ICU version >= %s... ' % min_version_str)
    ret, out = context.TryRun("""

#include <unicode/uversion.h>
#include <iostream>

int main()
{
    std::cout << U_ICU_VERSION_MAJOR_NUM << "." << U_ICU_VERSION_MINOR_NUM << std::endl;
    return 0;
}

""", '.cpp')
    try:
        found_version_str = out.strip()
        found_version = tuple(map(int, found_version_str.split('.')))
        min_version = tuple(map(int, min_version_str.split('.')))
    except:
        context.Result('error (could not get version from unicode/uversion.h)')
        return False

    if found_version >= min_version:
        context.Result('yes (found ICU %s)' % found_version_str)
        return True

    context.Result('no (found ICU %s)' % found_version_str)
    return False

def harfbuzz_version(context):
    context.Message('Checking for HarfBuzz version >= %s... ' % HARFBUZZ_MIN_VERSION_STRING)
    ret, out = context.TryRun("""

#include "harfbuzz/hb.h"
#include <iostream>

#ifndef HB_VERSION_ATLEAST
#define HB_VERSION_ATLEAST(...) 0
#endif

int main()
{
    std::cout << HB_VERSION_ATLEAST(%s, %s, %s) << ";" << HB_VERSION_STRING;
    return 0;
}

""" % HARFBUZZ_MIN_VERSION, '.cpp')
    if not ret:
        context.Result('error (could not get version from hb.h)')
    else:
        ok_str, found_version_str = out.strip().split(';', 1)
        ret = int(ok_str)
        if ret:
            context.Result('yes (found HarfBuzz %s)' % found_version_str)
        else:
            context.Result('no (found HarfBuzz %s)' % found_version_str)
    return ret

def harfbuzz_with_freetype_support(context):
    context.Message('Checking for HarfBuzz with freetype support... ')
    ret, out = context.TryRun("""

#include "harfbuzz/hb-ft.h"
#include <iostream>

int main()
{
    return 0;
}

""", '.cpp')
    context.Result(ret)
    return ret

def boost_regex_has_icu(context):
    context.env.Append(LIBS='icui18n')
    if env['RUNTIME_LINK'] == 'static':
        # re-order icu libs to ensure linux linker is happy
        for lib_name in ['icui18n',env['ICU_LIB_NAME'],'icudata']:
            if lib_name in context.env['LIBS']:
                context.env['LIBS'].remove(lib_name)
            context.env.Append(LIBS=lib_name)
    context.Message('Checking if boost_regex was built with ICU unicode support... ')
    ret, out = context.TryRun("""

#include <boost/regex/icu.hpp>
#include <unicode/unistr.h>

int main()
{
    U_NAMESPACE_QUALIFIER UnicodeString ustr;
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
    context.Result(ret)
    return ret

def sqlite_has_rtree(context, silent=False):
    """ check an sqlite3 install has rtree support.

    PRAGMA compile_options;
    http://www.sqlite.org/c3ref/compileoption_get.html
    """

    if not silent:
        context.Message('Checking if SQLite supports RTREE... ')
    ret, out = context.TryRun("""

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
        sqlite3_close(db);
    }
    else
    {
        printf("yes, has rtree!\\n");
        sqlite3_close(db);
        return 0;
    }

    return -1;
}

""", '.c')
    if silent:
        context.did_show_result=1
    context.Result(ret)
    return ret

__cplusplus = {'14':'201402L', '17':'201703L'}

def supports_cxx_std (context, silent=False):
    cplusplus_string = __cplusplus[env['CXX_STD']]
    if not silent:
        context.Message('Checking if compiler (%s) supports -std=c++%s flag... ' % (context.env.get('CXX','CXX'), env['CXX_STD']))
    ret, out = context.TryRun("""

int main()
{
#if __cplusplus >= %s
    return 0;
#else
    return -1;
#endif
}

""" % cplusplus_string ,'.cpp')
    if silent:
        context.did_show_result=1
    context.Result(ret)
    return ret

conf_tests = { 'prioritize_paths'      : prioritize_paths,
               'CheckPKGConfig'        : CheckPKGConfig,
               'CheckPKG'              : CheckPKG,
               'CheckPKGVersion'       : CheckPKGVersion,
               'FindBoost'             : FindBoost,
               'CheckBoost'            : CheckBoost,
               'CheckIcuData'          : CheckIcuData,
               'CheckProjData'         : CheckProjData,
               'CheckGdalData'         : CheckGdalData,
               'CheckCairoHasFreetype' : CheckCairoHasFreetype,
               'CheckHasDlfcn'         : CheckHasDlfcn,
               'GetBoostLibVersion'    : GetBoostLibVersion,
               'parse_config'          : parse_config,
               'parse_pg_config'       : parse_pg_config,
               'ogr_enabled'           : ogr_enabled,
               'get_pkg_lib'           : get_pkg_lib,
               'icu_at_least'          : icu_at_least,
               'harfbuzz_version'      : harfbuzz_version,
               'harfbuzz_with_freetype_support': harfbuzz_with_freetype_support,
               'boost_regex_has_icu'   : boost_regex_has_icu,
               'sqlite_has_rtree'      : sqlite_has_rtree,
               'supports_cxx_std'      : supports_cxx_std,
               'CheckBoostScopedEnum'  : CheckBoostScopedEnum,
               }

def GetMapnikLibVersion():
    ver = []
    for line in open('include/mapnik/version.hpp').readlines():
        if line.startswith('#define MAPNIK_MAJOR_VERSION'):
            ver.append(line.split(' ')[2].strip())
        if line.startswith('#define MAPNIK_MINOR_VERSION'):
            ver.append(line.split(' ')[2].strip())
        if line.startswith('#define MAPNIK_PATCH_VERSION'):
            ver.append(line.split(' ')[2].strip())
    version_string = ".".join(ver)
    return version_string

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
                    optfile = open(conf, 'r')
                    #print optfile.read().replace("\n", " ").replace("'","").replace(" = ","=")
                    optfile.close()

                elif not conf == SCONS_LOCAL_CONFIG:
                    # if default missing, no worries
                    # but if the default is overridden and the file is not found, give warning
                    color_print(1,"SCons CONFIG not found: '%s'" % conf)
            # Recreate the base environment using modified `opts`
            env = Environment(ENV=os.environ,options=opts)
            init_environment(env)
            env['USE_CONFIG'] = True
    else:
        color_print(4,'SCons USE_CONFIG specified as false, will not inherit variables python config file...')


    conf = Configure(env, custom_tests = conf_tests)

    if env['DEBUG']:
        mode = 'debug mode'
    else:
        mode = 'release mode'

    if env['COVERAGE']:
        mode += ' (with coverage)'

    env['PLATFORM'] = platform.uname()[0]
    color_print(4,"Configuring on %s in *%s*..." % (env['PLATFORM'],mode))

    ret, cxx_version = config_command(env['CXX'], '--version')
    if ret:
        color_print(5, "C++ compiler: %s" % cxx_version)
    else:
        color_print(5, "Could not detect C++ compiler")

    env['MISSING_DEPS'] = []
    env['SKIPPED_DEPS'] = []
    env['HAS_CAIRO'] = False
    env['CAIRO_LIBPATHS'] = []
    env['CAIRO_ALL_LIBS'] = []
    env['CAIRO_CPPPATHS'] = []
    env['HAS_PYCAIRO'] = False
    env['PYCAIRO_PATHS'] = []
    env['HAS_LIBXML2'] = False
    env['LIBMAPNIK_LIBS'] = []
    env['LIBMAPNIK_LINKFLAGS'] = []
    env['LIBMAPNIK_CPPATHS'] = []
    env['LIBMAPNIK_DEFINES'] = []
    env['LIBMAPNIK_CXXFLAGS'] = []
    env['PLUGINS'] = PLUGINS
    env['EXTRA_FREETYPE_LIBS'] = []
    env['SQLITE_LINKFLAGS'] = []
    env['QUERIED_PROJ_LIB'] = None
    env['QUERIED_ICU_DATA'] = None
    env['QUERIED_GDAL_DATA'] = None

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
    install_prefix = os.path.normpath(fix_path(env['DESTDIR'])) + fix_path(env['PREFIX'])
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
       env['MAPNIK_LIB_NAME'] = '${LIBPREFIX}${MAPNIK_NAME}${LIBSUFFIX}'
    else:
       env['MAPNIK_LIB_NAME'] = '${SHLIBPREFIX}${MAPNIK_NAME}${SHLIBSUFFIX}'

    if env['PKG_CONFIG_PATH']:
        env['ENV']['PKG_CONFIG_PATH'] = fix_path(env['PKG_CONFIG_PATH'])
        # otherwise this variable == os.environ["PKG_CONFIG_PATH"]

    if env['PATH']:
        env['ENV']['PATH'] = fix_path(env['PATH']) + ':' + env['ENV']['PATH']

    if env['SYSTEM_FONTS']:
        if not os.path.isdir(env['SYSTEM_FONTS']):
            color_print(1,'Warning: Directory specified for SYSTEM_FONTS does not exist!')

    # Set up for libraries and headers dependency checks
    env['CPPPATH'] = ['#include']
    env['LIBPATH'] = ['#src','#src/json','#src/wkt']

    # set any custom cxxflags and ldflags to come first
    if sys.platform == 'darwin' and not env['HOST']:
        DEFAULT_CXX_CXXFLAGS += ' -stdlib=libc++'
        DEFAULT_CXX_LINKFLAGS = ' -stdlib=libc++'
    env.Append(CPPDEFINES = env['CUSTOM_DEFINES'])
    env.Append(CXXFLAGS = "-std=c++%s %s" % (env['CXX_STD'], DEFAULT_CXX_CXXFLAGS))
    env.Append(CXXFLAGS = env['CUSTOM_CXXFLAGS'])
    env.Append(CFLAGS = env['CUSTOM_CFLAGS'])
    env.Append(LINKFLAGS = DEFAULT_CXX_LINKFLAGS)

    custom_ldflags = env.ParseFlags(env['CUSTOM_LDFLAGS'])
    env.Append(LINKFLAGS = custom_ldflags.pop('LINKFLAGS'),
               LIBS = custom_ldflags.pop('LIBS'))
    env.AppendUnique(FRAMEWORKS = custom_ldflags.pop('FRAMEWORKS'),
                     LIBPATH = custom_ldflags.pop('LIBPATH'),
                     RPATH = custom_ldflags.pop('RPATH'))
    # ParseFlags puts everything it does not recognize into CCFLAGS,
    # but let's assume the user knows better: add those to LINKFLAGS.
    # In order to prevent duplication of flags which ParseFlags puts
    # into both CCFLAGS and LINKFLAGS, call AppendUnique.
    env.AppendUnique(LINKFLAGS = custom_ldflags.pop('CCFLAGS'))

    invalid_ldflags = {k:v for k,v in custom_ldflags.items() if v}
    if invalid_ldflags:
        color_print(3, 'Warning: CUSTOM_LDFLAGS contained some flags that SCons recognized as not for linker.')
        color_print(3, 'The following flags will be ignored:')
        for key, value in invalid_ldflags.items():
            color_print(3, '\t%s = %r' % (key, value))

    ### platform specific bits

    thread_suffix = 'mt'
    if env['PLATFORM'] == 'FreeBSD':
        thread_suffix = ''
        env.Append(LIBS = 'pthread')

    if env['MEMORY_MAPPED_FILE']:
        env.Append(CPPDEFINES = '-DMAPNIK_MEMORY_MAPPED_FILE')

    # allow for mac osx /usr/lib/libicucore.dylib compatibility
    # requires custom supplied headers since Apple does not include them
    # details: http://lists.apple.com/archives/xcode-users/2005/Jun/msg00633.html
    # To use system lib download and make && make install one of these:
    # http://www.opensource.apple.com/tarballs/ICU/
    # then copy the headers to a location that mapnik will find
    if 'core' in env['ICU_LIB_NAME']:
        env.Append(CPPDEFINES = '-DU_HIDE_DRAFT_API')
        env.Append(CPPDEFINES = '-DUDISABLE_RENAMING')
        if os.path.exists(env['ICU_LIB_NAME']):
            #-sICU_LINK=" -L/usr/lib -licucore
            env['ICU_LIB_NAME'] = os.path.basename(env['ICU_LIB_NAME']).replace('.dylib','').replace('lib','')

    # Adding the required prerequisite library directories to the include path for
    # compiling and the library path for linking, respectively.
    for required in ('ICU', 'SQLITE', 'HB'):
        inc_path = env['%s_INCLUDES' % required]
        lib_path = env['%s_LIBS' % required]
        env.AppendUnique(CPPPATH = fix_path(inc_path))
        env.AppendUnique(LIBPATH = fix_path(lib_path))

    REQUIRED_LIBSHEADERS = [
        ['z', 'zlib.h', True,'C'],
        [env['ICU_LIB_NAME'],'unicode/unistr.h',True,'C++'],
        ['harfbuzz', 'harfbuzz/hb.h',True,'C++']
    ]
    OPTIONAL_LIBSHEADERS = []

    CHECK_PKG_CONFIG = conf.CheckPKGConfig('0.15.0')
    if env.get('FREETYPE_LIBS') or env.get('FREETYPE_INCLUDES'):
        REQUIRED_LIBSHEADERS.insert(0,['freetype','ft2build.h',True,'C'])
        if env.get('FREETYPE_INCLUDES'):
            inc_path = env['FREETYPE_INCLUDES']
            env.AppendUnique(CPPPATH = fix_path(inc_path))
        if env.get('FREETYPE_LIBS'):
            lib_path = env['FREETYPE_LIBS']
            env.AppendUnique(LIBPATH = fix_path(lib_path))
    elif CHECK_PKG_CONFIG and conf.CheckPKG('freetype2'):
        # Freetype 2.9+ doesn't use freetype-config and uses pkg-config instead
        cmd = 'pkg-config freetype2 --libs --cflags'
        if env['RUNTIME_LINK'] == 'static':
            cmd += ' --static'

        temp_env = Environment(ENV=os.environ)
        try:
            temp_env.ParseConfig(cmd)
            for lib in temp_env['LIBS']:
                env.AppendUnique(LIBPATH = fix_path(lib))
            for inc in temp_env['CPPPATH']:
                env.AppendUnique(CPPPATH = fix_path(inc))
        except OSError as e:
            pass
    elif conf.parse_config('FREETYPE_CONFIG'):
        # check if freetype links to bz2
        if env['RUNTIME_LINK'] == 'static':
            temp_env = env.Clone()
            temp_env['LIBS'] = []
            try:
                # TODO - freetype-config accepts --static as of v2.5.3
                temp_env.ParseConfig('%s --libs' % env['FREETYPE_CONFIG'])
                if 'bz2' in temp_env['LIBS']:
                    env['EXTRA_FREETYPE_LIBS'].append('bz2')
            except OSError as e:
                pass

    if env['XMLPARSER'] == 'libxml2':
        if env.get('XML2_LIBS') or env.get('XML2_INCLUDES'):
            OPTIONAL_LIBSHEADERS.insert(0,['libxml2','libxml/parser.h',True,'C'])
            if env.get('XML2_INCLUDES'):
                inc_path = env['XML2_INCLUDES']
                env.AppendUnique(CPPPATH = fix_path(inc_path))
            if env.get('XML2_LIBS'):
                lib_path = env['XML2_LIBS']
                env.AppendUnique(LIBPATH = fix_path(lib_path))
        elif CHECK_PKG_CONFIG and conf.CheckPKG('libxml-2.0'):
            # libxml2 2.9.10+ doesn't use xml2-config and uses pkg-config instead
            cmd = 'pkg-config libxml-2.0 --libs --cflags'

            temp_env = Environment(ENV=os.environ)
            try:
                temp_env.ParseConfig(cmd)
                for inc in temp_env['CPPPATH']:
                    env.AppendUnique(CPPPATH = fix_path(inc))
                    env['HAS_LIBXML2'] = True
                for lib in temp_env['LIBS']:
                    env.AppendUnique(LIBPATH = fix_path(lib))
                    env['HAS_LIBXML2'] = True
            except OSError as e:
                pass
        elif conf.parse_config('XML2_CONFIG',checks='--cflags'):
            env['HAS_LIBXML2'] = True
        else:
            env['MISSING_DEPS'].append('libxml2')

    if not env['HOST']:
        if conf.CheckHasDlfcn():
            env.Append(CPPDEFINES = '-DMAPNIK_HAS_DLCFN')
        else:
            env['SKIPPED_DEPS'].append('dlfcn')

    if env['JPEG']:
        OPTIONAL_LIBSHEADERS.append(['jpeg', ['stdio.h', 'jpeglib.h'], False,'C','-DHAVE_JPEG'])
        inc_path = env['%s_INCLUDES' % 'JPEG']
        lib_path = env['%s_LIBS' % 'JPEG']
        env.AppendUnique(CPPPATH = fix_path(inc_path))
        env.AppendUnique(LIBPATH = fix_path(lib_path))
    else:
        env['SKIPPED_DEPS'].append('jpeg')

    if env['PROJ']:
        OPTIONAL_LIBSHEADERS.append(['proj', 'proj_api.h', False,'C','-DMAPNIK_USE_PROJ4'])
        inc_path = env['%s_INCLUDES' % 'PROJ']
        lib_path = env['%s_LIBS' % 'PROJ']
        env.AppendUnique(CPPPATH = fix_path(inc_path))
        env.AppendUnique(LIBPATH = fix_path(lib_path))
    else:
        env['SKIPPED_DEPS'].append('proj')

    if env['PNG']:
        OPTIONAL_LIBSHEADERS.append(['png', 'png.h', False,'C','-DHAVE_PNG'])
        inc_path = env['%s_INCLUDES' % 'PNG']
        lib_path = env['%s_LIBS' % 'PNG']
        env.AppendUnique(CPPPATH = fix_path(inc_path))
        env.AppendUnique(LIBPATH = fix_path(lib_path))
    else:
        env['SKIPPED_DEPS'].append('png')

    if env['WEBP']:
        OPTIONAL_LIBSHEADERS.append(['webp', 'webp/decode.h', False,'C','-DHAVE_WEBP'])
        inc_path = env['%s_INCLUDES' % 'WEBP']
        lib_path = env['%s_LIBS' % 'WEBP']
        env.AppendUnique(CPPPATH = fix_path(inc_path))
        env.AppendUnique(LIBPATH = fix_path(lib_path))
    else:
        env['SKIPPED_DEPS'].append('webp')

    if env['TIFF']:
        OPTIONAL_LIBSHEADERS.append(['tiff', 'tiff.h', False,'C','-DHAVE_TIFF'])
        inc_path = env['%s_INCLUDES' % 'TIFF']
        lib_path = env['%s_LIBS' % 'TIFF']
        env.AppendUnique(CPPPATH = fix_path(inc_path))
        env.AppendUnique(LIBPATH = fix_path(lib_path))
    else:
        env['SKIPPED_DEPS'].append('tiff')

    # if requested, sort LIBPATH and CPPPATH before running CheckLibWithHeader tests
    if env['PRIORITIZE_LINKING']:
        conf.prioritize_paths(silent=True)

    # test for CXX_STD support, which is required
    if not env['HOST'] and not conf.supports_cxx_std():
        color_print(1,"C++ compiler does not support C++%s standard (-std=c++%s), which is required."
                      " Please upgrade your compiler" % (env['CXX_STD'], env['CXX_STD']))
        Exit(1)

    if not env['HOST']:
        for libname, headers, required, lang in REQUIRED_LIBSHEADERS:
            if not conf.CheckLibWithHeader(libname, headers, lang):
                if required:
                    color_print(1, 'Could not find required header or shared library for %s' % libname)
                    env['MISSING_DEPS'].append(libname)
                else:
                    color_print(4, 'Could not find optional header or shared library for %s' % libname)
                    env['SKIPPED_DEPS'].append(libname)
            else:
                if libname == env['ICU_LIB_NAME']:
                    if env['ICU_LIB_NAME'] not in env['MISSING_DEPS']:
                        if not conf.icu_at_least("4.0"):
                            # expression_string.cpp and map.cpp use fromUTF* function only available in >= ICU 4.2
                            env['MISSING_DEPS'].append(env['ICU_LIB_NAME'])
                elif libname == 'harfbuzz':
                    if not conf.harfbuzz_version():
                        env['SKIPPED_DEPS'].append('harfbuzz-min-version')
                    if not conf.harfbuzz_with_freetype_support():
                        env['MISSING_DEPS'].append('harfbuzz-with-freetype-support')

    if env['BIGINT']:
        env.Append(CPPDEFINES = '-DBIGINT')

    if env['THREADING'] == 'multi':
        thread_flag = thread_suffix
    else:
        thread_flag = ''

    conf.FindBoost(BOOST_SEARCH_PREFIXES,thread_flag)

    has_boost_devel = True
    if not env['HOST']:
        if not conf.CheckHeader(header='boost/version.hpp',language='C++'):
            env['MISSING_DEPS'].append('boost development headers')
            has_boost_devel = False

    if has_boost_devel:
        if not env['HOST']:
            env['BOOST_LIB_VERSION_FROM_HEADER'] = conf.GetBoostLibVersion()

        # The other required boost headers.
        BOOST_LIBSHEADERS = [
            ['system', 'boost/system/system_error.hpp', True],
            ['filesystem', 'boost/filesystem/operations.hpp', True],
            ['regex', 'boost/regex.hpp', True],
            ['program_options', 'boost/program_options.hpp', False]
        ]

        # if requested, sort LIBPATH and CPPPATH before running CheckLibWithHeader tests
        if env['PRIORITIZE_LINKING']:
            conf.prioritize_paths(silent=True)

        if not env['HOST']:
            # if the user is not setting custom boost configuration
            # enforce boost version greater than or equal to BOOST_MIN_VERSION
            if not conf.CheckBoost(BOOST_MIN_VERSION):
                color_print(4,'Found boost lib version... %s' % env.get('BOOST_LIB_VERSION_FROM_HEADER') )
                color_print(1,'Boost version %s or greater is required' % BOOST_MIN_VERSION)
                if not env['BOOST_VERSION']:
                    env['MISSING_DEPS'].append('boost version >= %s' % BOOST_MIN_VERSION)
            else:
                color_print(4,'Found boost lib version... %s' % env.get('BOOST_LIB_VERSION_FROM_HEADER') )

        if not env['HOST']:
            for count, libinfo in enumerate(BOOST_LIBSHEADERS):
                if not conf.CheckLibWithHeader('boost_%s%s' % (libinfo[0],env['BOOST_APPEND']), libinfo[1], 'C++'):
                    if libinfo[2]:
                        color_print(1,'Could not find required header or shared library for boost %s' % libinfo[0])
                        env['MISSING_DEPS'].append('boost ' + libinfo[0])
                    else:
                        color_print(4,'Could not find optional header or shared library for boost %s' % libinfo[0])
                        env['SKIPPED_DEPS'].append('boost ' + libinfo[0])

        # Boost versions before 1.57 are broken when the system package and
        # Mapnik are compiled against different standards. On Ubuntu 14.04
        # using boost 1.54, it breaks scoped enums. It's a bit of a hack to
        # just turn it off like this, but seems the only available work-
        # around. See https://svn.boost.org/trac/boost/ticket/6779 for more
        # details.
        if not env['HOST']:
            if not conf.CheckBoostScopedEnum():
                boost_version = [int(x) for x in env.get('BOOST_LIB_VERSION_FROM_HEADER').split('_') if x]
                if boost_version < [1, 51]:
                    env.Append(CXXFLAGS = '-DBOOST_NO_SCOPED_ENUMS')
                elif boost_version < [1, 57]:
                    env.Append(CXXFLAGS = '-DBOOST_NO_CXX11_SCOPED_ENUMS')

    if not env['HOST'] and env['ICU_LIB_NAME'] not in env['MISSING_DEPS']:
        # http://lists.boost.org/Archives/boost/2009/03/150076.php
        # we need libicui18n if using static boost libraries, so it is
        # important to try this check with the library linked
        if conf.boost_regex_has_icu():
            # TODO - should avoid having this be globally defined...
            env.Append(CPPDEFINES = '-DBOOST_REGEX_HAS_ICU')
        else:
            env['SKIPPED_DEPS'].append('boost_regex_icu')

        for libname, headers, required, lang, define in OPTIONAL_LIBSHEADERS:
            if not env['HOST']:
                if not conf.CheckLibWithHeader(libname, headers, lang):
                    if required:
                        color_print(1, 'Could not find required header or shared library for %s' % libname)
                        env['MISSING_DEPS'].append(libname)
                    else:
                        color_print(4, 'Could not find optional header or shared library for %s' % libname)
                        env['SKIPPED_DEPS'].append(libname)
                else:
                    env.Append(CPPDEFINES = define)
            else:
                env.Append(CPPDEFINES = define)

    env['REQUESTED_PLUGINS'] = [ driver.strip() for driver in Split(env['INPUT_PLUGINS'])]

    SQLITE_HAS_RTREE = None
    if env['HOST']:
        SQLITE_HAS_RTREE = True

    if not env['HOST']:
        env['QUERIED_PROJ_LIB'] = conf.CheckProjData()
        if os.environ.get('PROJ_LIB'):
            env['QUERIED_PROJ_LIB'] = os.environ['PROJ_LIB']
            color_print(4,'Detected PROJ_LIB in environ, using env value instead: %s' % os.environ['PROJ_LIB'] )
        env['QUERIED_ICU_DATA'] = conf.CheckIcuData()
        if os.environ.get('ICU_DATA'):
            env['QUERIED_ICU_DATA'] = os.environ['ICU_DATA']
            color_print(4,'Detected ICU_DATA in environ, using env value instead: %s' % os.environ['ICU_DATA'] )
        env['QUERIED_GDAL_DATA'] = conf.CheckGdalData()
        if os.environ.get('GDAL_DATA'):
            env['QUERIED_GDAL_DATA'] = os.environ['GDAL_DATA']
            color_print(4,'Detected GDAL_DATA in environ, using env value instead: %s' % os.environ['GDAL_DATA'] )
        # now validate the paths actually exist
        if env['QUERIED_PROJ_LIB'] and not os.path.exists(env['QUERIED_PROJ_LIB']):
            color_print(1,'%s not detected on your system' % env['QUERIED_PROJ_LIB'] )
            env['MISSING_DEPS'].append('PROJ_LIB')
        if env['QUERIED_GDAL_DATA'] and not os.path.exists(env['QUERIED_GDAL_DATA']):
            color_print(1,'%s not detected on your system' % env['QUERIED_GDAL_DATA'] )
            env['MISSING_DEPS'].append('GDAL_DATA')
        if env['QUERIED_ICU_DATA'] and not os.path.exists(env['QUERIED_ICU_DATA']):
            color_print(1,'%s not detected on your system' % env['QUERIED_ICU_DATA'] )
            env['MISSING_DEPS'].append('ICU_DATA')

    if len(env['REQUESTED_PLUGINS']):
        if env['HOST']:
            for plugin in env['REQUESTED_PLUGINS']:
                details = env['PLUGINS'][plugin]
                if details['lib']:
                    env.AppendUnique(LIBS=details['lib'])
        else:
            color_print(4,'Checking for requested plugins dependencies...')
            for plugin in env['REQUESTED_PLUGINS']:
                details = env['PLUGINS'][plugin]
                if plugin == 'gdal':
                    if conf.parse_config('GDAL_CONFIG',checks='--libs'):
                        conf.parse_config('GDAL_CONFIG',checks='--cflags')
                        libname = conf.get_pkg_lib('GDAL_CONFIG','gdal')
                        if libname:
                            if not conf.CheckLibWithHeader(libname, details['inc'], details['lang']):
                                env['SKIPPED_DEPS'].append('gdal')
                                if libname in env['LIBS']:
                                     env['LIBS'].remove(libname)
                            else:
                                details['lib'] = libname
                elif plugin == 'postgis' or plugin == 'pgraster':
                    if env.get('PG_LIBS') or env.get('PG_INCLUDES'):
                        libname = details['lib']
                        if env.get('PG_INCLUDES'):
                            inc_path = env['PG_INCLUDES']
                            env.AppendUnique(CPPPATH = fix_path(inc_path))
                        if env.get('PG_LIBS'):
                            lib_path = env['PG_LIBS']
                            env.AppendUnique(LIBPATH = fix_path(lib_path))
                        if not conf.CheckLibWithHeader(libname, details['inc'], details['lang']):
                            env['SKIPPED_DEPS'].append(libname)
                            if libname in env['LIBS']:
                                 env['LIBS'].remove(libname)
                        else:
                            details['lib'] = libname
                    else:
                        conf.parse_pg_config('PG_CONFIG')
                elif plugin == 'ogr':
                    if conf.ogr_enabled():
                        if conf.parse_config('GDAL_CONFIG',checks='--libs'):
                            conf.parse_config('GDAL_CONFIG',checks='--cflags')
                            libname = conf.get_pkg_lib('GDAL_CONFIG','ogr')
                            if libname:
                                if not conf.CheckLibWithHeader(libname, details['inc'], details['lang']):
                                    if 'gdal' not in env['SKIPPED_DEPS']:
                                        env['SKIPPED_DEPS'].append('gdal')
                                    if libname in env['LIBS']:
                                         env['LIBS'].remove(libname)
                                else:
                                    details['lib'] = libname
                elif details['path'] and details['lib'] and details['inc']:
                    backup = env.Clone().Dictionary()
                    # Note, the 'delete_existing' keyword makes sure that these paths are prepended
                    # to the beginning of the path list even if they already exist
                    incpath = env['%s_INCLUDES' % details['path']]
                    libpath = env['%s_LIBS' % details['path']]
                    env.PrependUnique(CPPPATH = fix_path(incpath),delete_existing=True)
                    env.PrependUnique(LIBPATH = fix_path(libpath),delete_existing=True)
                    if not conf.CheckLibWithHeader(details['lib'], details['inc'], details['lang']):
                        env.Replace(**backup)
                        env['SKIPPED_DEPS'].append(details['lib'])
                    if plugin == 'sqlite':
                        sqlite_backup = env.Clone().Dictionary()
                        # if statically linking, on linux we likely
                        # need to link sqlite to pthreads and dl
                        if env['RUNTIME_LINK'] == 'static' and not env['PLATFORM'] == 'Darwin':
                            if CHECK_PKG_CONFIG and conf.CheckPKG('sqlite3'):
                                sqlite_env = env.Clone()
                                try:
                                    sqlite_env.ParseConfig('pkg-config --static --libs sqlite3')
                                    for lib in sqlite_env['LIBS']:
                                        if not lib in env['LIBS']:
                                            env["SQLITE_LINKFLAGS"].append(lib)
                                            env.Append(LIBS=lib)
                                except OSError as e:
                                    for lib in ["sqlite3","dl","pthread"]:
                                        if not lib in env['LIBS']:
                                            env["SQLITE_LINKFLAGS"].append("lib")
                                            env.Append(LIBS=lib)
                            else:
                                for lib in ["sqlite3","dl","pthread"]:
                                    if not lib in env['LIBS']:
                                        env["SQLITE_LINKFLAGS"].append("lib")
                                        env.Append(LIBS=lib)
                        SQLITE_HAS_RTREE = conf.sqlite_has_rtree()
                        if not SQLITE_HAS_RTREE:
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
            env.PrependUnique(LIBPATH = '#src', delete_existing=True)

    if not env['HOST']:
        if env['PGSQL2SQLITE']:
            if 'sqlite3' not in env['LIBS']:
                env.AppendUnique(LIBS='sqlite3')
                env.AppendUnique(CPPPATH = fix_path(env['SQLITE_INCLUDES']))
                env.AppendUnique(LIBPATH = fix_path(env['SQLITE_LIBS']))
            if 'pq' not in env['LIBS']:
                if not conf.parse_pg_config('PG_CONFIG'):
                    env['PGSQL2SQLITE'] = False
            if not SQLITE_HAS_RTREE:
                env['SKIPPED_DEPS'].append('pgsql2sqlite_rtree')
                env['PGSQL2SQLITE'] = False

    # we rely on an internal, patched copy of agg with critical fixes
    # prepend to make sure we link locally
    env.Prepend(CPPPATH = '#deps/agg/include')
    env.Prepend(LIBPATH = '#deps/agg')
    env.Prepend(CPPPATH = '#deps/mapbox/variant/include')
    env.Prepend(CPPPATH = '#deps/mapbox/geometry/include')
    env.Prepend(CPPPATH = '#deps/mapbox/protozero/include')
    env.Prepend(CPPPATH = '#deps/mapbox/polylabel/include')
    # prepend deps dir for auxillary headers
    env.Prepend(CPPPATH = '#deps')

    if env['CAIRO']:
        if env['CAIRO_LIBS'] or env['CAIRO_INCLUDES']:
            c_inc = env['CAIRO_INCLUDES']
            if env['CAIRO_LIBS']:
                env["CAIRO_LIBPATHS"].append(fix_path(env['CAIRO_LIBS']))
                if not env['CAIRO_INCLUDES']:
                    c_inc = env['CAIRO_LIBS'].replace('lib','',1)
            if c_inc:
                c_inc = os.path.normpath(fix_path(env['CAIRO_INCLUDES']))
                if c_inc.endswith('include'):
                    c_inc = os.path.dirname(c_inc)
                env["CAIRO_CPPPATHS"].extend(
                    [
                      os.path.join(c_inc,'include/cairo'),
                      os.path.join(c_inc,'include/pixman-1'),
                      #os.path.join(c_inc,'include/freetype2'),
                      #os.path.join(c_inc,'include/libpng'),
                    ]
                )
                env["CAIRO_ALL_LIBS"] = ['cairo']
                if env['RUNTIME_LINK'] == 'static':
                    env["CAIRO_ALL_LIBS"].append('pixman-1')
                # todo - run actual checkLib?
                env['HAS_CAIRO'] = True
        else:
            if not CHECK_PKG_CONFIG:
                env['HAS_CAIRO'] = False
                env['SKIPPED_DEPS'].append('pkg-config')
                env['SKIPPED_DEPS'].append('cairo')
            elif not conf.CheckPKG('cairo'):
                env['HAS_CAIRO'] = False
                env['SKIPPED_DEPS'].append('cairo')
            else:
                print ('Checking for cairo lib and include paths... ', end = '')
                cmd = 'pkg-config --libs --cflags cairo'
                if env['RUNTIME_LINK'] == 'static':
                    cmd += ' --static'
                cairo_env = env.Clone()
                try:
                    cairo_env.ParseConfig(cmd)
                    for lib in cairo_env['LIBS']:
                        if not lib in env['LIBS']:
                            env["CAIRO_ALL_LIBS"].append(lib)
                    for lpath in cairo_env['LIBPATH']:
                        if not lpath in env['LIBPATH']:
                            env["CAIRO_LIBPATHS"].append(lpath)
                    for inc in cairo_env['CPPPATH']:
                        if not inc in env['CPPPATH']:
                            env["CAIRO_CPPPATHS"].append(inc)
                    env['HAS_CAIRO'] = True
                    print ('yes')
                except OSError as e:
                    color_print(1,'no')
                    env['SKIPPED_DEPS'].append('cairo')
                    color_print(1,'pkg-config reported: %s' % e)

    else:
        color_print(4,'Not building with cairo support, pass CAIRO=True to enable')

    if not env['HOST'] and env['HAS_CAIRO']:
        if not conf.CheckCairoHasFreetype():
            env['SKIPPED_DEPS'].append('cairo')
            env['HAS_CAIRO'] = False

    #### End Config Stage for Required Dependencies ####

    if env['MISSING_DEPS']:
        # if required dependencies are missing, print warnings and then let SCons finish without building or saving local config
        color_print(1,'\nExiting... the following required dependencies were not found:' + pretty_deps('\n   - ', env['MISSING_DEPS']))
        color_print(1,"\nSee '%s' for details on possible problems." % (fix_path(SCONS_LOCAL_LOG)))
        if env['SKIPPED_DEPS']:
            color_print(4,'\nAlso, these OPTIONAL dependencies were not found:' + pretty_deps('\n   - ', env['SKIPPED_DEPS']))
        color_print(4,"\nSet custom paths to these libraries and header files on the command-line or in a file called '%s'" % SCONS_LOCAL_CONFIG)
        color_print(4,"    ie. $ python scons/scons.py BOOST_INCLUDES=/usr/local/include BOOST_LIBS=/usr/local/lib")
        color_print(4, "\nOnce all required dependencies are found a local '%s' will be saved and then install:" % SCONS_LOCAL_CONFIG)
        color_print(4,"    $ sudo python scons/scons.py install")
        color_print(4,"\nTo view available path variables:\n    $ python scons/scons.py --help or -h")
        color_print(4,'\nTo view overall SCons help options:\n    $ python scons/scons.py --help-options or -H\n')
        color_print(4,'More info: https://github.com/mapnik/mapnik/wiki/Mapnik-Installation')
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
            color_print(4,'\nNote: will build without these OPTIONAL dependencies:' + pretty_deps('\n   - ', env['SKIPPED_DEPS']))
            print

        # fetch the mapnik version header in order to set the
        # ABI version used to build libmapnik.so on linux in src/build.py
        abi = GetMapnikLibVersion()
        abi_split = abi.split('.')
        env['ABI_VERSION'] = abi_split
        env['MAPNIK_VERSION_STRING'] = abi
        env['MAPNIK_VERSION'] = str(int(abi_split[0])*100000+int(abi_split[1])*100+int(abi_split[2]))

        # Common DEFINES.
        env.Append(CPPDEFINES = '-D%s' % env['PLATFORM'].upper())
        if env['THREADING'] == 'multi':
            env.Append(CPPDEFINES = '-DMAPNIK_THREADSAFE')

        if env['NO_ATEXIT']:
            env.Append(CPPDEFINES = '-DMAPNIK_NO_ATEXIT')

        if env['NO_DLCLOSE'] or env['COVERAGE']:
            env.Append(CPPDEFINES = '-DMAPNIK_NO_DLCLOSE')

        if env['ENABLE_GLIBC_WORKAROUND']:
            env.Append(CPPDEFINES = '-DMAPNIK_ENABLE_GLIBC_WORKAROUND')

        # Mac OSX (Darwin) special settings
        if env['PLATFORM'] == 'Darwin':
            pthread = ''
        else:
            pthread = '-pthread'

        # Common debugging flags.
        # http://lists.fedoraproject.org/pipermail/devel/2010-November/144952.html
        debug_flags  = ['-g', '-fno-omit-frame-pointer']
        debug_defines = ['-DDEBUG', '-DMAPNIK_DEBUG']
        ndebug_defines = ['-DNDEBUG']

        # faster compile
        # http://www.boost.org/doc/libs/1_47_0/libs/spirit/doc/html/spirit/what_s_new/spirit_2_5.html#spirit.what_s_new.spirit_2_5.breaking_changes
        env.Append(CPPDEFINES = '-DBOOST_SPIRIT_NO_PREDEFINED_TERMINALS=1')
        env.Append(CPPDEFINES = '-DBOOST_PHOENIX_NO_PREDEFINED_TERMINALS=1')
        # c++11 support / https://github.com/mapnik/mapnik/issues/1683
        #  - upgrade to PHOENIX_V3 since that is needed for c++11 compile
        env.Append(CPPDEFINES = '-DBOOST_SPIRIT_USE_PHOENIX_V3=1')

        # Enable logging in debug mode (always) and release mode (when specified)
        if env['DEFAULT_LOG_SEVERITY']:
            if env['DEFAULT_LOG_SEVERITY'] not in severities:
                severities_list = ', '.join("'%s'" % s for s in severities)
                color_print(1,"Cannot set default logger severity to '%s', available options are %s." % (env['DEFAULT_LOG_SEVERITY'], severities_list))
                Exit(1)
            else:
                log_severity = severities.index(env['DEFAULT_LOG_SEVERITY'])
        else:
            severities_list = ', '.join("'%s'" % s for s in severities)
            color_print(1,"No logger severity specified, available options are %s." % severities_list)
            Exit(1)

        log_enabled = ['-DMAPNIK_LOG', '-DMAPNIK_DEFAULT_LOG_SEVERITY=%d' % log_severity]

        if env['DEBUG']:
            debug_defines += log_enabled
        else:
            if env['ENABLE_LOG']:
                ndebug_defines += log_enabled

        # Enable statistics reporting
        if env['ENABLE_STATS']:
            debug_defines.append('-DMAPNIK_STATS')
            ndebug_defines.append('-DMAPNIK_STATS')

        # Add rdynamic to allow using statics between application and plugins
        # http://stackoverflow.com/questions/8623657/multiple-instances-of-singleton-across-shared-libraries-on-linux
        if env['PLATFORM'] != 'Darwin' and env['CXX'] == 'g++':
            env.MergeFlags('-rdynamic')

        if env['DEBUG']:
            env.Append(CXXFLAGS = debug_flags)
            env.Append(CPPDEFINES = debug_defines)
        else:
            env.Append(CPPDEFINES = ndebug_defines)

        # Common flags for g++/clang++ CXX compiler.
        # TODO: clean up code more to make -Wextra -Wsign-compare -Wsign-conversion -Wconversion viable
        # -Wfloat-equal -Wold-style-cast -Wexit-time-destructors -Wglobal-constructors -Wreserved-id-macro -Wheader-hygiene -Wmissing-noreturn
        common_cxx_flags = '-fvisibility=hidden -fvisibility-inlines-hidden -Wall %s %s -ftemplate-depth-300 -Wsign-compare ' % (env['WARNING_CXXFLAGS'], pthread)

        if 'clang++' in env['CXX']:
            common_cxx_flags += ' -Wno-unsequenced  -Wtautological-compare -Wheader-hygiene '
        if env['DEBUG']:
            env.Append(CXXFLAGS = common_cxx_flags + '-O0')
        else:
            env.Append(CXXFLAGS = common_cxx_flags + '-O%s' % (env['OPTIMIZATION']))


        # if requested, sort LIBPATH and CPPPATH one last time before saving...
        if env['PRIORITIZE_LINKING']:
            conf.prioritize_paths(silent=True)

        # finish config stage and pickle results
        env = conf.Finish()
        env_cache = open(SCONS_CONFIGURE_CACHE, 'wb')
        pickle_dict = {}
        for i in pickle_store:
            pickle_dict[i] = env.get(i)
        pickle.dump(pickle_dict, env_cache)
        env_cache.close()
        # fix up permissions on configure outputs
        # this is hackish but avoids potential problems
        # with a non-root configure following a root install
        # that also triggered a re-configure
        try:
            os.chmod(SCONS_CONFIGURE_CACHE,0o666)
        except: pass
        try:
            os.chmod(SCONS_LOCAL_CONFIG,0o666)
        except: pass
        try:
            os.chmod('.sconsign.dblite',0o666)
        except: pass
        try:
            os.chmod(SCONS_LOCAL_LOG,0o666)
        except: pass
        try:
            for item in glob('%s/*' % SCONF_TEMP_DIR):
                os.chmod(item,0o666)
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
        env['ENV']['PKG_CONFIG_PATH'] = fix_path(env['PKG_CONFIG_PATH'])
        # otherwise this variable == os.environ["PKG_CONFIG_PATH"]

    if env['PATH']:
        env['ENV']['PATH'] = fix_path(env['PATH']) + ':' + env['ENV']['PATH']

    if env['PATH_REMOVE']:
        for p in env['PATH_REMOVE'].split(':'):
            if p in env['ENV']['PATH']:
                env['ENV']['PATH'].replace(p,'')
            rm_path(p,'LIBPATH',env)
            rm_path(p,'CPPPATH',env)
            rm_path(p,'CXXFLAGS',env)
            rm_path(p,'CAIRO_LIBPATHS',env)
            rm_path(p,'CAIRO_CPPPATHS',env)

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
            replace_path('CAIRO_LIBPATHS',search,replace)
            replace_path('CAIRO_CPPPATHS',search,replace)

    # Adjust verbosity
    if env['QUIET']:
        env.Append(CXXCOMSTR = "Compiling $SOURCE")
        env.Append(SHCXXCOMSTR = "Compiling shared $SOURCE")
        env.Append(LINKCOMSTR = "Linking $TARGET")
        env.Append(SHLINKCOMSTR = "Linking shared $TARGET")

    # export env so it is available in build.py files
    Export('env')

    plugin_base = env.Clone()

    if env['COVERAGE']:
        plugin_base.Append(LINKFLAGS='--coverage')
        plugin_base.Append(CXXFLAGS='--coverage')

    Export('plugin_base')

    # Build agg first, doesn't need anything special
    if env['RUNTIME_LINK'] == 'shared':
        SConscript('deps/agg/build.py')

    # Build spirit grammars
    SConscript('src/json/build.py')
    SConscript('src/wkt/build.py')

    # Build the core library
    SConscript('src/build.py')

    # Install headers
    SConscript('include/build.py')

    # Build the requested and able-to-be-compiled input plug-ins
    GDAL_BUILT = False
    OGR_BUILT = False
    POSTGIS_BUILT = False
    PGRASTER_BUILT = False
    for plugin in env['PLUGINS']:
        if env['PLUGIN_LINKING'] == 'static' or plugin not in env['REQUESTED_PLUGINS']:
            if os.path.exists('plugins/input/%s.input' % plugin):
                os.unlink('plugins/input/%s.input' % plugin)
        elif plugin in env['REQUESTED_PLUGINS']:
            details = env['PLUGINS'][plugin]
            if details['lib'] in env['LIBS']:
                if env['PLUGIN_LINKING'] == 'shared':
                    SConscript('plugins/input/%s/build.py' % plugin)
                # hack to avoid breaking on plugins with the same dep
                if plugin == 'ogr': OGR_BUILT = True
                if plugin == 'gdal': GDAL_BUILT = True
                if plugin == 'postgis': POSTGIS_BUILT = True
                if plugin == 'pgraster': PGRASTER_BUILT = True
                if plugin == 'ogr' or plugin == 'gdal':
                    if GDAL_BUILT and OGR_BUILT:
                        env['LIBS'].remove(details['lib'])
                elif plugin == 'postgis' or plugin == 'pgraster':
                    if POSTGIS_BUILT and PGRASTER_BUILT:
                        env['LIBS'].remove(details['lib'])
                else:
                    env['LIBS'].remove(details['lib'])
            elif not details['lib']:
                if env['PLUGIN_LINKING'] == 'shared':
                    # build internal datasource input plugins
                    SConscript('plugins/input/%s/build.py' % plugin)
            else:
                color_print(1,"Notice: dependencies not met for plugin '%s', not building..." % plugin)
                if os.path.exists('plugins/input/%s.input' % plugin):
                    os.unlink('plugins/input/%s.input' % plugin)

    create_uninstall_target(env, env['MAPNIK_LIB_DIR_DEST'], False)
    create_uninstall_target(env, env['MAPNIK_INPUT_PLUGINS_DEST'] , False)

    if 'install' in COMMAND_LINE_TARGETS:
        # if statically linking plugins still make sure
        # to create the dynamic plugins directory
        if env['PLUGIN_LINKING'] == 'static':
            if not os.path.exists(env['MAPNIK_INPUT_PLUGINS_DEST']):
                os.makedirs(env['MAPNIK_INPUT_PLUGINS_DEST'])
        # before installing plugins, wipe out any previously
        # installed plugins that we are no longer building
        for plugin in PLUGINS.keys():
            plugin_path = os.path.join(env['MAPNIK_INPUT_PLUGINS_DEST'],'%s.input' % plugin)
            if os.path.exists(plugin_path):
                if plugin not in env['REQUESTED_PLUGINS'] or env['PLUGIN_LINKING'] == 'static':
                    color_print(4,"Notice: removing out of date plugin: '%s'" % plugin_path)
                    os.unlink(plugin_path)

    # Build the c++ rundemo app if requested
    if not env['HOST']:
        if env['DEMO']:
            SConscript('demo/c++/build.py')

    # Build shapeindex and remove its dependency from the LIBS
    if not env['HOST']:
        if 'boost_program_options%s' % env['BOOST_APPEND'] in env['LIBS']:
            if env['SHAPEINDEX']:
                SConscript('utils/shapeindex/build.py')
            if env['MAPNIK_INDEX']:
                SConscript('utils/mapnik-index/build.py')
            # Build the pgsql2psqlite app if requested
            if env['PGSQL2SQLITE']:
                SConscript('utils/pgsql2sqlite/build.py')
            if env['SVG2PNG']:
                SConscript('utils/svg2png/build.py')
            if env['MAPNIK_RENDER']:
                SConscript('utils/mapnik-render/build.py')
            # devtools not ready for public
            #SConscript('utils/ogrindex/build.py')
            env['LIBS'].remove('boost_program_options%s' % env['BOOST_APPEND'])
        else :
            color_print(1,"WARNING: Cannot find boost_program_options. 'shapeindex' and other command line programs will not be available")

    # Configure fonts and if requested install the bundled DejaVu fonts
    SConscript('fonts/build.py')

    # build C++ tests
    SConscript('test/build.py')

    if env['BENCHMARK']:
        SConscript('benchmark/build.py')

    if os.path.exists('./bindings/python/build.py'):
        SConscript('./bindings/python/build.py')

    # install mapnik-config script
    SConscript('utils/mapnik-config/build.py')

    # write the viewer.ini file
    SConscript('demo/viewer/build.py')
