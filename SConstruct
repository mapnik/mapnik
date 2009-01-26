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

def color_print(color,text):
    # 1 - red
    # 2 - green
    # 3 - yellow
    # 4 - blue
    print "\033[9%sm%s\033[0m" % (color,text)

# Helper function for uniquely appending paths to a SCons path listing.
def uniq_add(env, key, val):
    if not val in env[key]: env[key].append(val)

if platform.uname()[4] == 'x86_64':
    LIBDIR_SCHEMA='lib64' 
elif platform.uname()[4] == 'ppc64':
    LIBDIR_SCHEMA='lib64'
else:
    LIBDIR_SCHEMA='lib'

#### SCons build options and initial setup ####

OVERWRITE_CONFIG = False

SCONS_LOCAL_CONFIG = 'config.py'

# All of the following options may be modified at the command-line, for example:
# `python scons/scons.py PREFIX=/opt`
opts = Variables()

# Compiler options
opts.Add('CXX', 'The C++ compiler to use (defaults to g++).', 'g++')
opts.Add(EnumVariable('OPTIMIZATION','Set g++ optimization level','2', ['0','1','2','3']))
# Note: setting DEBUG=True will override any custom OPTIMIZATION level
opts.Add(BoolVariable('DEBUG', 'Compile a debug version of mapnik', 'False'))

# Install Variables
opts.Add('PREFIX', 'The install path "prefix"', '/usr/local')
opts.Add('DESTDIR', 'The root directory to install into. Useful mainly for binary package building', '/')


# SCons build behavior options
opts.Add('CONFIG', 'The file and path of a SCons user config .py file', SCONS_LOCAL_CONFIG)
opts.Add(BoolVariable('SCONS_CACHE', 'Use SCons dependency caching to speed build process', 'False'))
opts.Add(BoolVariable('USE_USER_ENV', 'Allow the SCons build env to inherit from the current user environment', 'True'))

# Boost variables
opts.Add(PathVariable('BOOST_INCLUDES', 'Search path for boost include files', '/usr/include'))
opts.Add(PathVariable('BOOST_LIBS', 'Search path for boost library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add('BOOST_TOOLKIT','Specify boost toolkit, e.g., gcc41.','',False)
opts.Add('BOOST_ABI', 'Specify boost ABI, e.g., d.','',False)
opts.Add('BOOST_VERSION','Specify boost version, e.g., 1_35.','',False)

# Variables for required dependencies
opts.Add(('FREETYPE_CONFIG', 'The path to the freetype-config executable.', 'freetype-config'))
opts.Add(('XML2_CONFIG', 'The path to the xml2-config executable.', 'xml2-config'))
opts.Add(PathVariable('ICU_INCLUDES', 'Search path for ICU include files', '/usr/include'))
opts.Add(PathVariable('ICU_LIBS','Search path for ICU include files','/usr/' + LIBDIR_SCHEMA))
opts.Add(PathVariable('PNG_INCLUDES', 'Search path for libpng include files', '/usr/include'))
opts.Add(PathVariable('PNG_LIBS','Search path for libpng include files','/usr/' + LIBDIR_SCHEMA))
opts.Add(PathVariable('JPEG_INCLUDES', 'Search path for libjpeg include files', '/usr/include'))
opts.Add(PathVariable('JPEG_LIBS', 'Search path for libjpeg library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add(PathVariable('TIFF_INCLUDES', 'Search path for libtiff include files', '/usr/include'))
opts.Add(PathVariable('TIFF_LIBS', 'Search path for libtiff library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add(PathVariable('PROJ_INCLUDES', 'Search path for PROJ.4 include files', '/usr/local/include'))
opts.Add(PathVariable('PROJ_LIBS', 'Search path for PROJ.4 library files', '/usr/local/' + LIBDIR_SCHEMA))

# Variables affecting rendering backends
opts.Add(BoolVariable('INTERNAL_LIBAGG', 'Use provided libagg', 'True'))


# Variables for optional dependencies
# Note: Cairo, Cairomm, and PyCairo all optional but configured automatically through pkg-config
# Therefore, we use a single boolean for whether to attempt to build cairo support.
opts.Add(BoolVariable('CAIRO', 'Attempt to build with Cairo rendering support', 'True'))
opts.Add(ListVariable('INPUT_PLUGINS','Input drivers to include','all',['postgis','shape','raster','gdal']))
opts.Add(PathVariable('PGSQL_INCLUDES', 'Search path for PostgreSQL include files', '/usr/include/postgresql', PathVariable.PathAccept))
opts.Add(PathVariable('PGSQL_LIBS', 'Search path for PostgreSQL library files', '/usr/' + LIBDIR_SCHEMA))
opts.Add(PathVariable('GDAL_INCLUDES', 'Search path for GDAL include files', '/usr/include/gdal', PathVariable.PathAccept))
opts.Add(PathVariable('GDAL_LIBS', 'Search path for GDAL library files', '/usr/' + LIBDIR_SCHEMA))

# Other variables
opts.Add(PathVariable('PYTHON','Python executable', sys.executable))
opts.Add(ListVariable('BINDINGS','Language bindings to build','all',['python']))
opts.Add(EnumVariable('THREADING','Set threading support','multi', ['multi','single']))
opts.Add(EnumVariable('XMLPARSER','Set xml parser ','libxml2', ['tinyxml','spirit','libxml2']))

# Construct the SCons build environment as a union of the users environment and the `opts`


# Build up base environment, then reinitate based on user constomizations.

# This method seems unpythonic and a bit dodgy, but it works.
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
if not user_conf == '' and not user_conf == 'n' and user_conf:
    if not user_conf.endswith('.py'):
        color_print(1,'SCONS_USER_CONFIG file specified is not a python file, will not be read...')
    else:
        # Accept more than one file as comma-delimited list
        user_confs = user_conf.split(',')
        # If they exist add the files to the existing `opts`
        for conf in user_confs:
            if os.path.exists(conf):
                opts.files.append(conf)
                color_print(4,"SCons CONFIG found: '%s', settings with be included..." % conf)
            elif not conf == SCONS_LOCAL_CONFIG:
                # if default missing, no worries
                # but if default is overriden and file not found, give warning
                color_print(1,"SCONS_USER_CONFIG file: '%s' could not be found..." % conf)
        # Recreate the base environment using modified `opts`
        env = Environment(ENV=os.environ,options=opts)
else:
    color_print(4,'SCONS_USER_CONFIG specified as false, will not inherit variables python config file')

#Progress(['/\r', '|\r', '\\\r', '-\r'], interval=5, file=open('/dev/tty', 'w'))

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
    pass # caching is 'auto' by default in SCons
else:
    # Set the cache mode to 'force' unless requested, avoiding hidden caching of Scons 'opts' in '.sconsign.dblite'
    # This allows for a 'user_config.py', if present, to be used as the primary means of storing paths to succesful build depedencies
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


conf = Configure(env, custom_tests = { 'CheckPKGConfig' : CheckPKGConfig,
                                       'CheckPKG' : CheckPKG })

#### Libraries and headers dependency checks ####

# Libraries and headers dependency checks
env['CPPPATH'] = ['#tinyxml', '#include', '#']
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

        

# Adding the prerequisite library directories to the include path for
# compiling and the library path for linking, respectively.
for prereq in ('BOOST', 'PNG', 'JPEG', 'TIFF', 'PGSQL', 'PROJ', 'GDAL',):
    inc_path = env['%s_INCLUDES' % prereq]
    lib_path = env['%s_LIBS' % prereq]
    uniq_add(env, 'CPPPATH', inc_path)
    uniq_add(env, 'LIBPATH', lib_path)
    
try:
    env.ParseConfig(env['FREETYPE_CONFIG'] + ' --libs --cflags')
except OSError:
    env['MISSING_DEPS'].append(env['FREETYPE_CONFIG'])

if env['XMLPARSER'] == 'tinyxml':
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
    ['pq', 'libpq-fe.h', False]
]

if env['CAIRO'] and conf.CheckPKGConfig('0.15.0') and conf.CheckPKG('cairomm-1.0'):
        env.ParseConfig('pkg-config --libs --cflags cairomm-1.0')
        env.Append(CXXFLAGS = '-DHAVE_CAIRO')
else:
   env['SKIPPED_DEPS'].extend(['cairo','cairomm','pycairo'])
        
CXX_LIBSHEADERS = [
    ['icuuc','unicode/unistr.h',True],
    ['icudata','unicode/utypes.h' , True],
    ['gdal', 'gdal_priv.h',False]
]


# Test function for a particular Boost Version.
def test_boost_ver(ver):
    return ver in env['BOOST_INCLUDES'] or ver in env['BOOST_VERSION']

if ((test_boost_ver('1_35') or test_boost_ver('1_36')) and 
    env['PLATFORM'] == 'Darwin'):
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
        if libinfo[2]:
            color_print (1,'Could not find required header or shared library for %s' % libinfo[0])
            env['MISSING_DEPS'].append(libinfo[0])
        else:
            color_print(4,'Could not find optional header or shared library for %s' % libinfo[0])
            if libinfo[0] == 'pq':
                # Make `pq` lib more understandable
                env['SKIPPED_DEPS'].append('pq (postgres/postgis)')
            else:
                env['SKIPPED_DEPS'].append(libinfo[0])
        

for libinfo in CXX_LIBSHEADERS:
    if not conf.CheckLibWithHeader(libinfo[0], libinfo[1], 'C++'):
        if libinfo[2]:
            color_print(1,'Could not find required header or shared library for %s' % libinfo[0])
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
              
#### End Config Stage ####

if env['MISSING_DEPS']:
    # if required dependencies are missing, print warnings and then let SCons finish without building or saving local config
    color_print(1,'\nExiting... the following required dependencies were not found:\n   - %s' % '\n   - '.join(env['MISSING_DEPS']))
   
    if env['SKIPPED_DEPS']:
        color_print(4,'\nAlso the these optional dependencies were skipped:\n   - %s' % '\n   - '.join(env['SKIPPED_DEPS']))
    
    color_print(4,"\n\nSet custom paths to these libraries and header files on the commandline or in a file called '%s'\n\nTo view available path variables:\n    $ python scons/scons.py --help or -h" % SCONS_LOCAL_CONFIG)
    
    color_print(4,'\n\nTo see overall SCons help options:\n    $ python scons/scons.py --help-options or -H\n')
    
    color_print(4,'More info: http://trac.mapnik.org/wiki/MapnikInstallation')
    
    # Need some way to exit cleanly here...
    # calling Exit() does not work because that will abort the users ability to get help
    env = conf.Finish()
    #Exit()
else:
    # Save the custom variables in a config.py that will be reloaded to allow for `install` without re-specifying custom variables
    color_print(4,"All Required dependencies found!")
    if OVERWRITE_CONFIG:
      color_print(4,"Saving '%s' file to hold successful path variables." % SCONS_LOCAL_CONFIG)
      if os.path.exists(SCONS_LOCAL_CONFIG):
        os.unlink(SCONS_LOCAL_CONFIG)
    # Serialize all user customizations into local config file
    opts.Save(SCONS_LOCAL_CONFIG,env)

    Export('env')
    Export('conf')
    
    inputplugins = [ driver.strip() for driver in Split(env['INPUT_PLUGINS'])]
    
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