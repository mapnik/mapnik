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


import os
import sys
import glob
from copy import copy
from subprocess import Popen, PIPE

Import('env')

lib_env = env.Clone()

def call(cmd, silent=True):
    stdin, stderr = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        return stdin.strip()
    elif not silent:
        print stderr

def ldconfig(*args,**kwargs):
    call('ldconfig')

if env['LINKING'] == 'static':
    lib_env.Append(CXXFLAGS="-fPIC")

mapnik_lib_link_flag = ''

# note: .data gets the actual list to allow a true copy
# and avoids unintended pollution of other environments
libmapnik_cxxflags = copy(lib_env['CXXFLAGS'].data)

ABI_VERSION = env['ABI_VERSION']

filesystem = 'boost_filesystem%s' % env['BOOST_APPEND']
regex = 'boost_regex%s' % env['BOOST_APPEND']

# clear out and re-set libs for this env
lib_env['LIBS'] = ['freetype','ltdl','png','tiff','z','jpeg','proj',env['ICU_LIB_NAME'],filesystem,regex]

if len(env['EXTRA_FREETYPE_LIBS']):
    lib_env['LIBS'].extend(copy(env['EXTRA_FREETYPE_LIBS']))

if env['XMLPARSER'] == 'libxml2':
    lib_env['LIBS'].append('xml2')

if env['THREADING'] == 'multi':
    lib_env['LIBS'].append('boost_thread%s' % env['BOOST_APPEND'])
        
if env['HAS_BOOST_SYSTEM']:
    lib_env['LIBS'].append('boost_system%s' % env['BOOST_APPEND'])
    

if not env['RUNTIME_LINK'] == 'static':
    if env['INTERNAL_LIBAGG']:
          lib_env['LIBS'].insert(0, 'agg')
    else:
        lib_env['LIBS'].append([lib for lib in env['LIBS'] if lib.startswith('agg')])
    

if env['PLATFORM'] == 'Darwin':
    mapnik_libname = 'libmapnik2.dylib'
else:
    mapnik_libname = 'libmapnik2.so.' + ("%d.%d" % (ABI_VERSION[0],ABI_VERSION[1])) 

if env['PLATFORM'] == 'Darwin':
    if env['FULL_LIB_PATH']:
        lib_path = '%s/%s' % (env['MAPNIK_LIB_BASE'],mapnik_libname)
    else:
        lib_path = mapnik_libname
    mapnik_lib_link_flag += ' -Wl,-install_name,%s' % lib_path
    _d = {'version':env['MAPNIK_VERSION_STRING']}
    mapnik_lib_link_flag += ' -current_version %(version)s -compatibility_version %(version)s' % _d
elif env['PLATFORM'] == 'SunOS':
    if env['CXX'].startswith('CC'):
        mapnik_lib_link_flag += ' -R. -h %s' % mapnik_libname
    else:
        mapnik_lib_link_flag += ' -Wl,-h,%s' %  mapnik_libname
else: # Linux and others
    mapnik_lib_link_flag += ' -Wl,-rpath-link,. -Wl,-soname,%s' % mapnik_libname

source = Split(
    """
    color.cpp
    box2d.cpp
    expression_string.cpp
    filter_factory.cpp
    feature_type_style.cpp
    font_engine_freetype.cpp
    font_set.cpp
    gradient.cpp
    graphics.cpp
    image_reader.cpp
    image_util.cpp
    layer.cpp
    line_pattern_symbolizer.cpp
    map.cpp
    load_map.cpp
    memory.cpp
    parse_path.cpp
    palette.cpp
    placement_finder.cpp
    plugin.cpp
    png_reader.cpp
    point_symbolizer.cpp
    polygon_pattern_symbolizer.cpp
    save_map.cpp
    shield_symbolizer.cpp
    text_symbolizer.cpp
    tiff_reader.cpp
    wkb.cpp
    projection.cpp
    proj_transform.cpp
    distance.cpp
    scale_denominator.cpp
    memory_datasource.cpp
    stroke.cpp
    symbolizer.cpp
    arrow.cpp
    unicode.cpp
    glyph_symbolizer.cpp
    markers_symbolizer.cpp
    metawriter.cpp
    raster_colorizer.cpp
    text_placements.cpp
    wkt/wkt_factory.cpp
    metawriter_inmem.cpp
    metawriter_factory.cpp
    mapped_memory_cache.cpp
    marker_cache.cpp
    svg_parser.cpp
    svg_path_parser.cpp
    svg_points_parser.cpp 
    svg_transform_parser.cpp
    """   
    )

processor_cpp = 'feature_style_processor.cpp'

if env['RENDERING_STATS']:
    env3 = lib_env.Clone()
    env3.Append(CXXFLAGS='-DRENDERING_STATS')
    if env['LINKING'] == 'static':
        source.insert(0,env3.StaticObject(processor_cpp))
    else:
        source.insert(0,env3.SharedObject(processor_cpp))
else:
    source.insert(0,processor_cpp);

    

# add the datasource_cache.cpp with custom LIBTOOL flag if needed
if env['LIBTOOL_SUPPORTS_ADVISE']:
    env3 = lib_env.Clone()
    env3.Append(CXXFLAGS='-DLIBTOOL_SUPPORTS_ADVISE')
    libmapnik_cxxflags.append('-DLIBTOOL_SUPPORTS_ADVISE')
    cpp = 'datasource_cache.cpp'
    if env['LINKING'] == 'static':
        source.insert(0,env3.StaticObject(cpp))
    else:
        source.insert(0,env3.SharedObject(cpp))
else:
    source.insert(0,'datasource_cache.cpp')

if env['JPEG']:
    source += Split(
        """
        jpeg_reader.cpp
        """)
        
# agg backend
source += Split(
    """
    agg/agg_renderer.cpp
    agg/process_building_symbolizer.cpp
    agg/process_glyph_symbolizer.cpp
    agg/process_line_symbolizer.cpp
    agg/process_line_pattern_symbolizer.cpp
    agg/process_text_symbolizer.cpp
    agg/process_point_symbolizer.cpp
    agg/process_polygon_symbolizer.cpp
    agg/process_polygon_pattern_symbolizer.cpp
    agg/process_raster_symbolizer.cpp
    agg/process_shield_symbolizer.cpp
    agg/process_markers_symbolizer.cpp
    """ 
    )

if env['RUNTIME_LINK'] == "static":
    source += glob.glob('../agg/src/' + '*.cpp')

# grid backend
source += Split(
    """
    grid/grid_renderer.cpp
    grid/process_building_symbolizer.cpp
    grid/process_glyph_symbolizer.cpp
    grid/process_line_pattern_symbolizer.cpp
    grid/process_line_symbolizer.cpp
    grid/process_markers_symbolizer.cpp
    grid/process_point_symbolizer.cpp
    grid/process_polygon_pattern_symbolizer.cpp
    grid/process_polygon_symbolizer.cpp
    grid/process_raster_symbolizer.cpp
    grid/process_shield_symbolizer.cpp
    grid/process_text_symbolizer.cpp	
    """)

if env['SVG_RENDERER']: # svg backend
    source += Split(
              """
      	svg/svg_renderer.cpp
      	svg/svg_generator.cpp	
      	svg/svg_output_attributes.cpp
      	svg/process_symbolizers.cpp
      	svg/process_building_symbolizer.cpp
      	svg/process_glyph_symbolizer.cpp
      	svg/process_line_pattern_symbolizer.cpp
      	svg/process_line_symbolizer.cpp
      	svg/process_markers_symbolizer.cpp
      	svg/process_point_symbolizer.cpp
      	svg/process_polygon_pattern_symbolizer.cpp
      	svg/process_polygon_symbolizer.cpp
      	svg/process_raster_symbolizer.cpp
      	svg/process_shield_symbolizer.cpp
      	svg/process_text_symbolizer.cpp	
      	""")
    lib_env.Append(CXXFLAGS = '-DSVG_RENDERER')
    libmapnik_cxxflags.append('-DSVG_RENDERER')

if env['HAS_CAIRO']:
    lib_env.PrependUnique(LIBPATH=env['CAIROMM_LIBPATHS'])
    lib_env.Append(LIBS=env['CAIROMM_LINKFLAGS'])
    lib_env.Append(CXXFLAGS = '-DHAVE_CAIRO')
    libmapnik_cxxflags.append('-DHAVE_CAIRO')
    lib_env.PrependUnique(CPPPATH=copy(env['CAIROMM_CPPPATHS']))
    source.insert(0,'cairo_renderer.cpp')
    #cairo_env.PrependUnique(CPPPATH=env['CAIROMM_CPPPATHS'])
    # not safe, to much depends on graphics.hpp
    #cairo_env = lib_env.Clone()
    #cairo_env.Append(CXXFLAGS = '-DHAVE_CAIRO')
    #fixup = ['feature_type_style.cpp','load_map.cpp','cairo_renderer.cpp','graphics.cpp','image_util.cpp']
    #for cpp in fixup:
    #    if cpp in source:
    #        source.remove(cpp)
    #    if env['LINKING'] == 'static':
    #        source.insert(0,cairo_env.StaticObject(cpp))
    #    else:
    #        source.insert(0,cairo_env.SharedObject(cpp))


if env['XMLPARSER'] == 'tinyxml':
    source += Split(
        """
        ../tinyxml/tinystr.cpp
        ../tinyxml/tinyxml.cpp
        ../tinyxml/tinyxmlerror.cpp
        ../tinyxml/tinyxmlparser.cpp
        """)
elif env['XMLPARSER'] == 'libxml2' and env['HAS_LIBXML2']:
    source += Split(
        """
        libxml2_loader.cpp
        """)
    env2 = lib_env.Clone()
    env2.Append(CXXFLAGS = '-DHAVE_LIBXML2')
    libmapnik_cxxflags.append('-DHAVE_LIBXML2')
    fixup = ['load_map.cpp','libxml2_loader.cpp']
    for cpp in fixup:
        if cpp in source:
            source.remove(cpp)
        if env['LINKING'] == 'static':
            source.insert(0,env2.StaticObject(cpp))
        else:
            source.insert(0,env2.SharedObject(cpp))

if env['CUSTOM_LDFLAGS']:
    linkflags = '%s %s' % (env['CUSTOM_LDFLAGS'], mapnik_lib_link_flag)
else:
    linkflags = mapnik_lib_link_flag

if env['LINKING'] == 'static':
    mapnik = lib_env.StaticLibrary('mapnik2', source, LINKFLAGS=linkflags)
else:
    mapnik = lib_env.SharedLibrary('mapnik2', source, LINKFLAGS=linkflags)

# cache library values for other builds to use
env['LIBMAPNIK_LIBS'] = copy(lib_env['LIBS'])
env['LIBMAPNIK_CXXFLAGS'] = libmapnik_cxxflags

if env['PLATFORM'] != 'Darwin':
    # Symlink command, only works if both files are in same directory
    def symlink(env, target, source):
        trgt = str(target[0])
        src = str(source[0])

        if os.path.islink(trgt) or os.path.exists(trgt):
            os.remove(trgt)
        os.symlink(os.path.basename(src), trgt)

    major, minor, micro = ABI_VERSION
    
    soFile = "%s.%d.%d.%d" % (os.path.basename(str(mapnik[0])), major, minor, micro)
    target = os.path.join(env['MAPNIK_LIB_BASE_DEST'], soFile)
    
    if 'uninstall' not in COMMAND_LINE_TARGETS:
      result = env.InstallAs(target=target, source=mapnik)
      env.Alias(target='install', source=result)
      if result:
            env.AddPostAction(result, ldconfig)

    
    # Install symlinks
    target1 = os.path.join(env['MAPNIK_LIB_BASE_DEST'], "%s.%d.%d" % (os.path.basename(str(mapnik[0])),major, minor))
    target2 = os.path.join(env['MAPNIK_LIB_BASE_DEST'], os.path.basename(str(mapnik[0])))
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        if 'install' in COMMAND_LINE_TARGETS:
            link1 = env.Command(target1, target, symlink)
            env.Alias(target='install', source=link1)
            link2 = env.Command(target2, target1, symlink)
            env.Alias(target='install', source=link2)
    # delete in reverse order..
    env['create_uninstall_target'](env, target2)
    env['create_uninstall_target'](env, target1)
    env['create_uninstall_target'](env, target)

else:
    target_path = env['MAPNIK_LIB_BASE_DEST']
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        result = env.Install(target_path, mapnik)
        env.Alias(target='install', source=result)

    env['create_uninstall_target'](env, os.path.join(target_path,mapnik_libname))

includes = glob.glob('../include/mapnik/*.hpp')
svg_includes = glob.glob('../include/mapnik/svg/*.hpp')
wkt_includes = glob.glob('../include/mapnik/wkt/*.hpp')
grid_includes = glob.glob('../include/mapnik/grid/*.hpp')

inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik')
svg_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/svg')
wkt_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/wkt')
grid_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/grid')

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Alias(target='install', source=env.Install(inc_target, includes))
    env.Alias(target='install', source=env.Install(svg_inc_target, svg_includes))
    env.Alias(target='install', source=env.Install(wkt_inc_target, wkt_includes))
    env.Alias(target='install', source=env.Install(grid_inc_target, grid_includes))

env['create_uninstall_target'](env, inc_target)
env['create_uninstall_target'](env, svg_inc_target)
env['create_uninstall_target'](env, wkt_inc_target)
env['create_uninstall_target'](env, grid_inc_target)
