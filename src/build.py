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
system = 'boost_system%s' % env['BOOST_APPEND']

# clear out and re-set libs for this env
lib_env['LIBS'] = ['freetype','ltdl','png','tiff','z','proj',env['ICU_LIB_NAME'],filesystem,system,regex]

if env['JPEG']:
   lib_env['LIBS'].append('jpeg')

if len(env['EXTRA_FREETYPE_LIBS']):
    lib_env['LIBS'].extend(copy(env['EXTRA_FREETYPE_LIBS']))

# libxml2 should be optional but is currently not
# https://github.com/mapnik/mapnik/issues/913
lib_env['LIBS'].append('xml2')

if env['THREADING'] == 'multi':
    lib_env['LIBS'].append('boost_thread%s' % env['BOOST_APPEND'])


if env['RUNTIME_LINK'] == 'static':
    if 'icuuc' in env['ICU_LIB_NAME']:
        lib_env['LIBS'].append('icudata')
        lib_env['LIBS'].append('icui18n')
else:
    lib_env['LIBS'].insert(0, 'agg')

if env['PLATFORM'] == 'Darwin':
    mapnik_libname = env.subst(env['MAPNIK_LIB_NAME'])
    if env['FULL_LIB_PATH']:
        lib_path = '%s/%s' % (env['MAPNIK_LIB_BASE'],mapnik_libname)
    else:
        lib_path = mapnik_libname
    mapnik_lib_link_flag += ' -Wl,-install_name,%s' % lib_path
    _d = {'version':env['MAPNIK_VERSION_STRING'].replace('-pre','')}
    mapnik_lib_link_flag += ' -current_version %(version)s -compatibility_version %(version)s' % _d
else: # unix, non-macos
    mapnik_libname = env.subst(env['MAPNIK_LIB_NAME']) + (".%d.%d" % (int(ABI_VERSION[0]),int(ABI_VERSION[1])))
    if env['PLATFORM'] == 'SunOS':
        if env['CXX'].startswith('CC'):
            mapnik_lib_link_flag += ' -R. -h %s' % mapnik_libname
        else:
            mapnik_lib_link_flag += ' -Wl,-h,%s' %  mapnik_libname
    else: # Linux and others
        mapnik_lib_link_flag += ' -Wl,-rpath-link,. -Wl,-soname,%s' % mapnik_libname

source = Split(
    """
    color.cpp
    conversions.cpp
    image_compositing.cpp
    box2d.cpp
    building_symbolizer.cpp
    datasource_cache.cpp
    debug.cpp
    deepcopy.cpp
    expression_string.cpp
    expression.cpp
    transform_expression.cpp
    feature_kv_iterator.cpp
    feature_type_style.cpp
    font_engine_freetype.cpp
    font_set.cpp
    gamma_method.cpp
    gradient.cpp
    graphics.cpp
    image_reader.cpp
    image_util.cpp
    layer.cpp
    line_symbolizer.cpp
    line_pattern_symbolizer.cpp
    map.cpp
    load_map.cpp
    memory.cpp
    parse_path.cpp
    parse_transform.cpp
    palette.cpp
    placement_finder.cpp
    plugin.cpp
    png_reader.cpp
    point_symbolizer.cpp
    polygon_pattern_symbolizer.cpp
    polygon_symbolizer.cpp
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
    symbolizer_helpers.cpp
    arrow.cpp
    unicode.cpp
    markers_symbolizer.cpp
    metawriter.cpp
    raster_colorizer.cpp
    wkt/wkt_factory.cpp
    metawriter_inmem.cpp
    metawriter_factory.cpp
    mapped_memory_cache.cpp
    marker_cache.cpp
    svg_parser.cpp
    svg_path_parser.cpp
    svg_points_parser.cpp
    svg_transform_parser.cpp
    warp.cpp
    json/feature_collection_parser.cpp
    json/geojson_generator.cpp
    markers_placement.cpp
    processed_text.cpp
    formatting/base.cpp
    formatting/expression.cpp
    formatting/list.cpp
    formatting/text.cpp
    formatting/format.cpp
    formatting/registry.cpp
    text_placements/registry.cpp
    text_placements/base.cpp
    text_placements/dummy.cpp
    text_placements/list.cpp
    text_placements/simple.cpp
    text_properties.cpp
    xml_tree.cpp
    config_error.cpp
    """
    )

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

if env.get('BOOST_LIB_VERSION_FROM_HEADER'):
    boost_version_from_header = int(env['BOOST_LIB_VERSION_FROM_HEADER'].split('_')[1])
    if boost_version_from_header < 46:
        # avoid ubuntu issue with boost interprocess:
        # https://github.com/mapnik/mapnik/issues/1001
        env4 = lib_env.Clone()
        env4.Append(CXXFLAGS = '-fpermissive')
        cpp ='mapped_memory_cache.cpp'
        source.remove(cpp)
        if env['LINKING'] == 'static':
            source.insert(0,env4.StaticObject(cpp))
        else:
            source.insert(0,env4.SharedObject(cpp))

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
    source += glob.glob('../deps/agg/src/' + '*.cpp')

# grid backend
source += Split(
    """
    grid/grid.cpp
    grid/grid_renderer.cpp
    grid/process_building_symbolizer.cpp
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

if env['XMLPARSER'] == 'libxml2' and env['HAS_LIBXML2']:
    source += Split(
        """
        libxml2_loader.cpp
        """)
    env2 = lib_env.Clone()
    env2.Append(CXXFLAGS = '-DHAVE_LIBXML2')
    libmapnik_cxxflags.append('-DHAVE_LIBXML2')
    fixup = ['libxml2_loader.cpp']
    for cpp in fixup:
        if cpp in source:
            source.remove(cpp)
        if env['LINKING'] == 'static':
            source.insert(0,env2.StaticObject(cpp))
        else:
            source.insert(0,env2.SharedObject(cpp))
else:
    source += Split(
        """
        rapidxml_loader.cpp
        """
    )

if env['CUSTOM_LDFLAGS']:
    linkflags = '%s %s' % (env['CUSTOM_LDFLAGS'], mapnik_lib_link_flag)
else:
    linkflags = mapnik_lib_link_flag

# cache library values for other builds to use
env['LIBMAPNIK_LIBS'] = copy(lib_env['LIBS'])
env['LIBMAPNIK_CXXFLAGS'] = libmapnik_cxxflags

if env['PLATFORM'] == 'Darwin':
    target_path = env['MAPNIK_LIB_BASE_DEST']
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        if env['LINKING'] == 'static':
            mapnik = lib_env.StaticLibrary('mapnik', source, LINKFLAGS=linkflags)
        else:
            mapnik = lib_env.SharedLibrary('mapnik', source, LINKFLAGS=linkflags)
        result = env.Install(target_path, mapnik)
        env.Alias(target='install', source=result)

    env['create_uninstall_target'](env, os.path.join(target_path,env.subst(env['MAPNIK_LIB_NAME'])))
else:
    # Symlink command, only works if both files are in same directory
    def symlink(env, target, source):
        trgt = str(target[0])
        src = str(source[0])

        if os.path.islink(trgt) or os.path.exists(trgt):
            os.remove(trgt)
        os.symlink(os.path.basename(src), trgt)

    major, minor, micro = ABI_VERSION

    soFile = "%s.%d.%d.%d" % (os.path.basename(env.subst(env['MAPNIK_LIB_NAME'])), int(major), int(minor), int(micro))
    target = os.path.join(env['MAPNIK_LIB_BASE_DEST'], soFile)

    if 'uninstall' not in COMMAND_LINE_TARGETS:
        if env['LINKING'] == 'static':
            mapnik = lib_env.StaticLibrary('mapnik', source, LINKFLAGS=linkflags)
        else:
            mapnik = lib_env.SharedLibrary('mapnik', source, LINKFLAGS=linkflags)
        result = env.InstallAs(target=target, source=mapnik)
        env.Alias(target='install', source=result)
        if result:
              env.AddPostAction(result, ldconfig)


    # Install symlinks
    target1 = os.path.join(env['MAPNIK_LIB_BASE_DEST'], "%s.%d.%d" % \
        (os.path.basename(env.subst(env['MAPNIK_LIB_NAME'])),int(major), int(minor)))
    target2 = os.path.join(env['MAPNIK_LIB_BASE_DEST'], os.path.basename(env.subst(env['MAPNIK_LIB_NAME'])))
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        link1 = env.Command(target1, target, symlink)
        env.Alias(target='install', source=link1)
        link2 = env.Command(target2, target1, symlink)
        env.Alias(target='install', source=link2)
    # delete in reverse order..
    env['create_uninstall_target'](env, target2)
    env['create_uninstall_target'](env, target1)
    env['create_uninstall_target'](env, target)

includes = glob.glob('../include/mapnik/*.hpp')
svg_includes = glob.glob('../include/mapnik/svg/*.hpp')
wkt_includes = glob.glob('../include/mapnik/wkt/*.hpp')
grid_includes = glob.glob('../include/mapnik/grid/*.hpp')
json_includes = glob.glob('../include/mapnik/json/*.hpp')
util_includes = glob.glob('../include/mapnik/util/*.hpp')
text_placements_includes = glob.glob('../include/mapnik/text_placements/*.hpp')
formatting_includes = glob.glob('../include/mapnik/formatting/*.hpp')

inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik')
svg_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/svg')
wkt_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/wkt')
grid_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/grid')
json_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/json')
util_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/util')
text_placements_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/text_placements')
formatting_inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/formatting')

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Alias(target='install', source=env.Install(inc_target, includes))
    env.Alias(target='install', source=env.Install(svg_inc_target, svg_includes))
    env.Alias(target='install', source=env.Install(wkt_inc_target, wkt_includes))
    env.Alias(target='install', source=env.Install(grid_inc_target, grid_includes))
    env.Alias(target='install', source=env.Install(json_inc_target, json_includes))
    env.Alias(target='install', source=env.Install(util_inc_target, util_includes))
    env.Alias(target='install', source=env.Install(text_placements_inc_target, text_placements_includes))
    env.Alias(target='install', source=env.Install(formatting_inc_target, formatting_includes))

env['create_uninstall_target'](env, inc_target)
env['create_uninstall_target'](env, svg_inc_target)
env['create_uninstall_target'](env, wkt_inc_target)
env['create_uninstall_target'](env, grid_inc_target)
env['create_uninstall_target'](env, json_inc_target)
env['create_uninstall_target'](env, util_inc_target)
env['create_uninstall_target'](env, text_placements_inc_target)
env['create_uninstall_target'](env, formatting_inc_target)
