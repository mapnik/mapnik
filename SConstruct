#
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

if platform.uname()[4] == 'x86_64':
    LIBDIR_SCHEMA='lib64' 
else:
    LIBDIR_SCHEMA='lib'

#### SCons build options and initial setup ####

# All of the following options may be modified at the command-line, for example:
# `python scons/scons PREFIX=/opt`
opts = Options('config.py')
opts.Add('CXX', 'The C++ compiler to use (defaults to g++).', 'g++')
opts.Add('PREFIX', 'The install path "prefix"', '/usr/local')
opts.Add(PathOption('BOOST_INCLUDES', 'Search path for boost include files', '/usr/include'))
opts.Add(PathOption('BOOST_LIBS', 'Search path for boost library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add('BOOST_TOOLKIT','Specify boost toolkit e.g. gcc41.','',False)
opts.Add(('FREETYPE_CONFIG', 'The path to the freetype-config executable.', 'freetype-config'))
opts.Add(('XML2_CONFIG', 'The path to the xml2-config executable.', 'xml2-config'))
opts.Add(PathOption('FRIBIDI_INCLUDES', 'Search path for fribidi include files', '/usr/include'))
opts.Add(PathOption('FRIBIDI_LIBS','Search path for fribidi include files','/usr/' + LIBDIR_SCHEMA))
opts.Add(PathOption('PNG_INCLUDES', 'Search path for libpng include files', '/usr/include'))
opts.Add(PathOption('PNG_LIBS','Search path for libpng include files','/usr/' + LIBDIR_SCHEMA))
opts.Add(PathOption('JPEG_INCLUDES', 'Search path for libjpeg include files', '/usr/include'))
opts.Add(PathOption('JPEG_LIBS', 'Search path for libjpeg library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add(PathOption('TIFF_INCLUDES', 'Search path for libtiff include files', '/usr/include'))
opts.Add(PathOption('TIFF_LIBS', 'Search path for libtiff library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add(PathOption('PGSQL_INCLUDES', 'Search path for PostgreSQL include files', '/usr/include'))
opts.Add(PathOption('PGSQL_LIBS', 'Search path for PostgreSQL library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add(PathOption('PROJ_INCLUDES', 'Search path for PROJ.4 include files', '/usr/local/include'))
opts.Add(PathOption('PROJ_LIBS', 'Search path for PROJ.4 library files', '/usr/local/' + LIBDIR_SCHEMA))
opts.Add(PathOption('GDAL_INCLUDES', 'Search path for GDAL include files', '/usr/include'))
opts.Add(PathOption('GDAL_LIBS', 'Search path for GDAL library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add(PathOption('PYTHON','Python executable', sys.executable))
opts.Add(ListOption('INPUT_PLUGINS','Input drivers to include','all',['postgis','shape','raster','gdal']))
opts.Add(ListOption('BINDINGS','Language bindings to build','all',['python']))
opts.Add(BoolOption('DEBUG', 'Compile a debug version of mapnik', 'False'))
opts.Add('DESTDIR', 'The root directory to install into. Useful mainly for binary package building', '/')
opts.Add(BoolOption('BIDI', 'BIDI support', 'False'))
opts.Add(EnumOption('THREADING','Set threading support','multi', ['multi','single']))
opts.Add(EnumOption('XMLPARSER','Set xml parser ','tinyxml', ['tinyxml','spirit','libxml2']))

env = Environment(ENV=os.environ, options=opts)

def color_print(color,text):
    # 1 - red
    # 2 - green
    # 3 - yellow
    # 4 - blue
    print "\033[9%sm%s\033[0m" % (color,text)

env['LIBDIR_SCHEMA'] = LIBDIR_SCHEMA
env['PLATFORM'] = platform.uname()[0]
color_print (4,"Building on %s ..." % env['PLATFORM'])
Help(opts.GenerateHelpText(env))

conf = Configure(env)

#### Libraries and headers dependency checks ####

# Helper function for uniquely appending paths to a SCons path listing.
def uniq_add(env, key, val):
    if not val in env[key]: env[key].append(val)

# Libraries and headers dependency checks
env['CPPPATH'] = ['#agg/include', '#tinyxml', '#include', '#']
env['LIBPATH'] = ['#agg', '#src']

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

# Adding the prerequisite library directories to the include path for
# compiling and the library path for linking, respectively.
for prereq in ('BOOST', 'PNG', 'JPEG', 'TIFF', 'PGSQL', 'PROJ', 'GDAL',):
    inc_path = env['%s_INCLUDES' % prereq]
    lib_path = env['%s_LIBS' % prereq]
    uniq_add(env, 'CPPPATH', inc_path)
    uniq_add(env, 'LIBPATH', lib_path)
    
env.ParseConfig(env['FREETYPE_CONFIG'] + ' --libs --cflags')

if env['BIDI']:
    env.Append(CXXFLAGS = '-DUSE_FRIBIDI')
    if env['FRIBIDI_INCLUDES'] not in env['CPPPATH']:
        env['CPPPATH'].append(env['FRIBIDI_INCLUDES'])
    if env['FRIBIDI_LIBS'] not in env['LIBPATH']:
        env['LIBPATH'].append(env['FRIBIDI_LIBS'])  
    env['LIBS'].append('fribidi')

if env['XMLPARSER'] == 'tinyxml':
    env.Append(CXXFLAGS = '-DBOOST_PROPERTY_TREE_XML_PARSER_TINYXML -DTIXML_USE_STL')
elif env['XMLPARSER'] == 'libxml2':
    env.ParseConfig(env['XML2_CONFIG'] + ' --libs --cflags')
    env.Append(CXXFLAGS = '-DHAVE_LIBXML2');
    
C_LIBSHEADERS = [
    ['m', 'math.h', True],
    ['ltdl', 'ltdl.h', True],
    ['png', 'png.h', True],
    ['tiff', 'tiff.h', True],
    ['z', 'zlib.h', True],
    ['jpeg', ['stdio.h', 'jpeglib.h'], True],
    ['proj', 'proj_api.h', True],
    ['iconv', 'iconv.h', False],
    ['pq', 'libpq-fe.h', False]
]

CXX_LIBSHEADERS = [
    ['gdal', 'gdal_priv.h',False]
]

if env['BIDI'] : C_LIBSHEADERS.append(['fribidi','fribidi/fribidi.h',True])

BOOST_LIBSHEADERS = [
    ['thread', 'boost/thread/mutex.hpp', True],
    # ['system', 'boost/system/system_error.hpp', True], # uncomment this on Darwin + boost_1_35
    ['filesystem', 'boost/filesystem/operations.hpp', True],
    ['regex', 'boost/regex.hpp', True],
    ['program_options', 'boost/program_options.hpp', False]
]

for libinfo in C_LIBSHEADERS:
    if not conf.CheckLibWithHeader(libinfo[0], libinfo[1], 'C') and libinfo[2]:
        color_print (1,'Could not find header or shared library for %s, exiting!' % libinfo[0])
        Exit(1)

for libinfo in CXX_LIBSHEADERS:
    if not conf.CheckLibWithHeader(libinfo[0], libinfo[1], 'C++') and libinfo[2]:
        color_print(1,'Could not find header or shared library for %s, exiting!' % libinfo[0])
        Exit(1)

if len(env['BOOST_TOOLKIT']):
    env['BOOST_APPEND'] = '-%s' % env['BOOST_TOOLKIT']
else:
    env['BOOST_APPEND']=''
    
for count, libinfo in enumerate(BOOST_LIBSHEADERS):
    if  env['THREADING'] == 'multi' :
        if not conf.CheckLibWithHeader('boost_%s%s-mt' % (libinfo[0],env['BOOST_APPEND']), libinfo[1], 'C++') and libinfo[2] :
            color_print(1,'Could not find header or shared library for boost %s, exiting!' % libinfo[0])
            Exit(1)
    elif not conf.CheckLibWithHeader('boost_%s%s' % (libinfo[0], env['BOOST_APPEND']), libinfo[1], 'C++') :
        color_print(1,'Could not find header or shared library for boost %s, exiting!' % libinfo[0])
        Exit(1)    
  
Export('env')

inputplugins = [ driver.strip() for driver in Split(env['INPUT_PLUGINS'])]

bindings = [ binding.strip() for binding in Split(env['BINDINGS'])]

#### Build instructions & settings ####

# Build agg first, doesn't need anything special
SConscript('agg/SConscript')

# Build the core library
SConscript('src/SConscript')

# Build shapeindex and remove its dependency from the LIBS
if 'boost_program_options%s-mt' % env['BOOST_APPEND'] in env['LIBS']:
    SConscript('utils/shapeindex/SConscript')
    env['LIBS'].remove('boost_program_options%s-mt' % env['BOOST_APPEND'])

# Build the input plug-ins
if 'postgis' in inputplugins and 'pq' in env['LIBS']:
    SConscript('plugins/input/postgis/SConscript')
    env['LIBS'].remove('pq')

if 'shape' in inputplugins:
    SConscript('plugins/input/shape/SConscript')

if 'raster' in inputplugins:
    SConscript('plugins/input/raster/SConscript')

if 'gdal' in inputplugins and 'gdal' in env['LIBS']:
    SConscript('plugins/input/gdal/SConscript')

if 'gigabase' in inputplugins and 'gigabase_r' in env['LIBS']:
    SConscript('plugins/input/gigabase/SConscript')

# Build the Python bindings.
if 'python' in env['BINDINGS']:
    if not os.access(env['PYTHON'], os.X_OK):
        color_print(1,"Cannot run python interpreter at '%s', make sure that you have the permissions to execute it." % env['PYTHON'])
        Exit(1)

    env['PYTHON_PREFIX'] = os.popen("%s -c 'import sys; print sys.prefix'" % env['PYTHON']).read().strip()
    env['PYTHON_VERSION'] = os.popen("%s -c 'import sys; print sys.version'" % env['PYTHON']).read()[0:3]

    color_print(4,'Bindings Python version... %s' % env['PYTHON_VERSION'])

    majver, minver = env['PYTHON_VERSION'].split('.')

    if (int(majver), int(minver)) < (2, 2):
        color_print(1,"Python version 2.2 or greater required")
        Exit(1)

    color_print(4,'Python %s prefix... %s' % (env['PYTHON_VERSION'], env['PYTHON_PREFIX']))

    SConscript('bindings/python/SConscript')
    
env = conf.Finish()

# Common C++ flags.
common_cxx_flags = '-D%s -DBOOST_SPIRIT_THREADSAFE ' % env['PLATFORM'].upper()

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
        env.Append(CXXFLAGS = gcc_cxx_flags + '-O2 -finline-functions -Wno-inline %s' % ndebug_flags)


SConscript('fonts/SConscript')

