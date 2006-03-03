#  This file is part of Mapnik (c++ mapping toolkit)
#  Copyright (C) 2005 Artem Pavlenko, Jean-Francois Doyon
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

import os, sys

opts = Options()

opts.Add('PREFIX', 'The install path "prefix"', '/usr/local')
opts.Add(PathOption('BOOST_INCLUDES', 'Search path for boost include files', '/usr/include'))
opts.Add(PathOption('BOOST_LIBS', 'Search path for boost library files', '/usr/lib'))
opts.Add(PathOption('FREETYPE_CONFIG', 'The path to the freetype-config executable.', '/usr/bin/freetype-config'))
opts.Add(PathOption('PNG_INCLUDES', 'Search path for libpng include files', '/usr/include'))
opts.Add(PathOption('PNG_LIBS', 'Search path for libpng include files', '/usr/lib'))
opts.Add(PathOption('JPEG_INCLUDES', 'Search path for libjpeg include files', '/usr/include'))
opts.Add(PathOption('JPEG_LIBS', 'Search path for libjpeg library files', '/usr/lib'))
opts.Add(PathOption('TIFF_INCLUDES', 'Search path for libtiff include files', '/usr/include'))
opts.Add(PathOption('TIFF_LIBS', 'Search path for libtiff library files', '/usr/lib'))
opts.Add(PathOption('PGSQL_INCLUDES', 'Search path for PostgreSQL include files', '/usr/include'))
opts.Add(PathOption('PGSQL_LIBS', 'Search path for PostgreSQL library files', '/usr/lib'))
opts.Add(PathOption('PYTHON','Python executable', sys.executable))
opts.Add(ListOption('INPUT_PLUGINS','Input drivers to include','all',['postgis','shape','raster']))
opts.Add(ListOption('BINDINGS','Language bindings to build','all',['python']))
opts.Add('DEBUG', 'Compile a debug version of mapnik', '')

env = Environment(ENV=os.environ, options=opts)

Help(opts.GenerateHelpText(env))

conf = Configure(env)

# Libraries and headers dependency checks

env['CPPPATH'] = ['#agg/include', '#include']

for path in [env['BOOST_INCLUDES'], env['PNG_INCLUDES'], env['JPEG_INCLUDES'], env['TIFF_INCLUDES'], env['PGSQL_INCLUDES']]:
    if path not in env['CPPPATH']: env['CPPPATH'].append(path)

env['LIBPATH'] = ['#agg', '#src']

for path in [env['BOOST_LIBS'], env['PNG_LIBS'], env['JPEG_LIBS'], env['TIFF_LIBS'], env['PGSQL_LIBS']]:
    if path not in env['LIBPATH']: env['LIBPATH'].append(path)

env.ParseConfig(env['FREETYPE_CONFIG'] + ' --libs --cflags')

C_LIBSHEADERS = [
    ['ltdl', 'ltdl.h', True],
    ['png', 'png.h', True],
    ['tiff', 'tiff.h', True],
    ['z', 'zlib.h', True],
    ['jpeg', ['stdio.h', 'jpeglib.h'], True],
    ['pq', 'libpq-fe.h', False]
]

BOOST_LIBSHEADERS = [
    ['thread', 'boost/thread/mutex.hpp', True],
    ['filesystem', 'boost/filesystem/operations.hpp', True],
    ['wserialization', ['boost/archive/text_oarchive.hpp',
                        'boost/archive/text_iarchive.hpp',
                        'boost/archive/xml_oarchive.hpp',
                        'boost/archive/xml_iarchive.hpp'], True
    ],
    ['regex', 'boost/regex.hpp', True],
    ['program_options', 'boost/program_options.hpp', False]
]

for libinfo in C_LIBSHEADERS:
    if not conf.CheckLibWithHeader(libinfo[0], libinfo[1], 'C') and libinfo[2]:
        print 'Could not find header or shared library for %s, exiting!' % libinfo[0]
        Exit(1)

env['BOOST_APPEND'] = ''

for count, libinfo in enumerate(BOOST_LIBSHEADERS):
    if not conf.CheckLibWithHeader('boost_%s%s' % (libinfo[0], env['BOOST_APPEND']), libinfo[1], 'C++'):
        if not conf.CheckLibWithHeader('boost_%s-%s-mt' % (libinfo[0], env['CC']), libinfo[1], 'C++') and libinfo[2] and count == 0:
            print 'Could not find header or shared library for boost %s, exiting!' % libinfo[0]
            Exit(1)
        else:
            env['BOOST_APPEND'] = '-%s-mt' % env['CC']

Export('env')

inputplugins = [ driver.strip() for driver in Split(env['INPUT_PLUGINS'])]

bindings = [ binding.strip() for binding in Split(env['BINDINGS'])]

# Check out the Python situation

if 'python' in env['BINDINGS']:
    if not os.access(env['PYTHON'], os.X_OK):
        print "Cannot run python interpreter at '%s', make sure that you have the permissions to execute it." % env['PYTHON']
        Exit(1)

    env['PYTHON_PREFIX'] = os.popen("%s -c 'import sys; print sys.prefix'" % env['PYTHON']).read().strip()
    env['PYTHON_VERSION'] = os.popen("%s -c 'import sys; print sys.version'" % env['PYTHON']).read()[0:3]

    print 'Bindings Python version... %s' % env['PYTHON_VERSION']

    majver, minver = env['PYTHON_VERSION'].split('.')

    if (int(majver), int(minver)) < (2, 2):
        print "Python version 2.2 or greater required"
        Exit(1)

    print 'Python %s prefix... %s' % (env['PYTHON_VERSION'], env['PYTHON_PREFIX'])

    SConscript('bindings/python/SConscript')

env = conf.Finish()

# Setup the c++ args for our own codebase

if env['DEBUG']:
    env.Append(CXXFLAGS = '-Wall -ftemplate-depth-100 -O0 -fno-inline -g -pthread -DDEBUG')
else:
    env.Append(CXXFLAGS = '-Wall -ftemplate-depth-100 -O3 -finline-functions -Wno-inline -pthread -DNDEBUG')

# Build agg first, doesn't need anything special

SConscript('agg/SConscript')

# Build shapeindex and remove its dependency from the LIBS

if 'boost_program_options%s' % env['BOOST_APPEND'] in env['LIBS']:
    SConscript('utils/shapeindex/SConscript')
    env['LIBS'].remove('boost_program_options%s' % env['BOOST_APPEND'])

# Build the input plug-ins

if 'postgis' in inputplugins and 'pq' in env['LIBS']:
    SConscript('plugins/input/postgis/SConscript')
    env['LIBS'].remove('pq')

if 'shape' in inputplugins:
    SConscript('plugins/input/shape/SConscript')

if 'raster' in inputplugins:
    SConscript('plugins/input/raster/SConscript')

# Build the core library

SConscript('src/SConscript')

# Install some free default fonts

SConscript('fonts/SConscript')
