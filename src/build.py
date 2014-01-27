#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2013 Artem Pavlenko
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
#


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
libmapnik_defines = copy(lib_env['CPPDEFINES'])

ABI_VERSION = env['ABI_VERSION']

enabled_imaging_libraries = []
filesystem = 'boost_filesystem%s' % env['BOOST_APPEND']
regex = 'boost_regex%s' % env['BOOST_APPEND']
system = 'boost_system%s' % env['BOOST_APPEND']

# clear out and re-set libs for this env
# note: order matters on linux: see lorder | tsort
lib_env['LIBS'] = [filesystem,regex]

if env['HAS_CAIRO']:
    lib_env.Append(LIBS=env['CAIRO_ALL_LIBS'])

# maybe bz2
if len(env['EXTRA_FREETYPE_LIBS']):
    lib_env['LIBS'].extend(copy(env['EXTRA_FREETYPE_LIBS']))

lib_env['LIBS'].append('harfbuzz-icu')

if '-DHAVE_PNG' in env['CPPDEFINES']:
   lib_env['LIBS'].append('png')
   enabled_imaging_libraries.append('png_reader.cpp')

if '-DMAPNIK_USE_PROJ4' in env['CPPDEFINES']:
   lib_env['LIBS'].append('proj')

if '-DHAVE_TIFF' in env['CPPDEFINES']:
   lib_env['LIBS'].append('tiff')
   enabled_imaging_libraries.append('tiff_reader.cpp')

if '-DHAVE_WEBP' in env['CPPDEFINES']:
   lib_env['LIBS'].append('webp')
   enabled_imaging_libraries.append('webp_reader.cpp')

lib_env['LIBS'].append('xml2')

if '-DBOOST_REGEX_HAS_ICU' in env['CPPDEFINES']:
    lib_env['LIBS'].append('icui18n')

lib_env['LIBS'].append(system)

lib_env['LIBS'].append('harfbuzz')

if '-DHAVE_JPEG' in env['CPPDEFINES']:
   lib_env['LIBS'].append('jpeg')
   enabled_imaging_libraries.append('jpeg_reader.cpp')

lib_env['LIBS'].append(env['ICU_LIB_NAME'])

lib_env['LIBS'].append('freetype')

if env['RUNTIME_LINK'] == 'static':
    if env['PLATFORM'] == 'Linux':
        lib_env['LINKFLAGS'].append('-pthread')
    if 'icuuc' in env['ICU_LIB_NAME']:
        lib_env['LIBS'].append('icudata')

if env['RUNTIME_LINK'] != 'static':
    lib_env['LIBS'].insert(0, 'agg')

lib_env['LIBS'].append('z')

if env['PLATFORM'] == 'Darwin':
    mapnik_libname = env.subst(env['MAPNIK_LIB_NAME'])
    if env['FULL_LIB_PATH']:
        lib_path = '%s/%s' % (env['MAPNIK_LIB_BASE'],mapnik_libname)
    else:
        lib_path = '@loader_path/'+mapnik_libname
    mapnik_lib_link_flag += ' -Wl,-install_name,%s' % lib_path
    _d = {'version':env['MAPNIK_VERSION_STRING'].replace('-pre','')}
    mapnik_lib_link_flag += ' -current_version %(version)s -compatibility_version %(version)s' % _d
else: # unix, non-macos
    mapnik_libname = env.subst(env['MAPNIK_LIB_NAME'])
    if env['ENABLE_SONAME']:
        mapnik_libname = env.subst(env['MAPNIK_LIB_NAME']) + (".%d.%d" % (int(ABI_VERSION[0]),int(ABI_VERSION[1])))
    if env['PLATFORM'] == 'SunOS':
        if env['CXX'].startswith('CC'):
            mapnik_lib_link_flag += ' -R. -h %s' % mapnik_libname
        else:
            mapnik_lib_link_flag += ' -Wl,-h,%s' %  mapnik_libname
    else: # Linux and others
        lib_env['LIBS'].append('dl')
        mapnik_lib_link_flag += ' -Wl,-rpath-link,.'
        if env['ENABLE_SONAME']:
            mapnik_lib_link_flag += ' -Wl,-soname,%s' % mapnik_libname
        if env['FULL_LIB_PATH']:
            mapnik_lib_link_flag += ' -Wl,-rpath=%s' % env['MAPNIK_LIB_BASE']
        else:
            mapnik_lib_link_flag += ' -Wl,-z,origin -Wl,-rpath=\$$ORIGIN'

source = Split(
    """
    fs.cpp
    debug_symbolizer.cpp
    request.cpp
    well_known_srs.cpp
    params.cpp
    image_filter_types.cpp
    miniz_png.cpp
    color.cpp
    css_color_grammar.cpp
    conversions.cpp
    image_compositing.cpp
    image_filter_grammar.cpp
    image_scaling.cpp
    box2d.cpp
    building_symbolizer.cpp
    datasource_cache.cpp
    datasource_cache_static.cpp
    debug.cpp
    expression_node.cpp
    expression_grammar.cpp
    expression_string.cpp
    expression.cpp
    transform_expression_grammar.cpp
    transform_expression.cpp
    feature_kv_iterator.cpp
    feature_style_processor.cpp
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
    path_expression_grammar.cpp
    plugin.cpp
    point_symbolizer.cpp
    polygon_pattern_symbolizer.cpp
    polygon_symbolizer.cpp
    rule.cpp
    save_map.cpp
    shield_symbolizer.cpp
    text_symbolizer.cpp
    wkb.cpp
    projection.cpp
    proj_transform.cpp
    distance.cpp
    scale_denominator.cpp
    simplify.cpp
    memory_datasource.cpp
    stroke.cpp
    symbolizer.cpp
    unicode.cpp
    markers_symbolizer.cpp
    raster_colorizer.cpp
    raster_symbolizer.cpp
    wkt/wkt_factory.cpp
    wkt/wkt_generator.cpp
    mapped_memory_cache.cpp
    marker_cache.cpp
    svg/svg_parser.cpp
    svg/svg_path_parser.cpp
    svg/svg_points_parser.cpp
    svg/svg_transform_parser.cpp
    warp.cpp
    json/geometry_grammar.cpp
    json/geometry_parser.cpp
    json/feature_grammar.cpp
    json/feature_parser.cpp
    json/feature_collection_parser.cpp
    json/geojson_generator.cpp
    text/vertex_cache.cpp
    text/layout.cpp
    text/text_line.cpp
    text/itemizer.cpp
    text/scrptrun.cpp
    text/face.cpp
    text/placement_finder.cpp
    text/renderer.cpp
    text/symbolizer_helpers.cpp
    text/text_properties.cpp
    text/formatting/base.cpp
    text/formatting/expression.cpp
    text/formatting/list.cpp
    text/formatting/text.cpp
    text/formatting/format.cpp
    text/formatting/registry.cpp
    text/placements/registry.cpp
    text/placements/base.cpp
    text/placements/dummy.cpp
    text/placements/list.cpp
    text/placements/simple.cpp
    xml_tree.cpp
    config_error.cpp
    color_factory.cpp
    """
    )

if env['PLUGIN_LINKING'] == 'static':
    hit = False
    for plugin in env['REQUESTED_PLUGINS']:
        details = env['PLUGINS'][plugin]
        if details['lib'] in env['LIBS'] or not details['lib']:
            plugin_env = SConscript('../plugins/input/%s/build.py' % plugin)
            if not plugin_env:
                print("Notice: no 'plugin_env' variable found for plugin: '%s'" % plugin)
            else:
                hit = True
                DEF = '-DMAPNIK_STATIC_PLUGIN_%s' % plugin.upper()
                lib_env.Append(CPPDEFINES = DEF)
                if DEF not in libmapnik_defines:
                    libmapnik_defines.append(DEF)
                if plugin_env.has_key('SOURCES') and plugin_env['SOURCES']:
                    source += ['../plugins/input/%s/%s' % (plugin, src) for src in plugin_env['SOURCES']]
                if plugin_env.has_key('CPPDEFINES') and plugin_env['CPPDEFINES']:
                    lib_env.AppendUnique(CPPDEFINES=plugin_env['CPPDEFINES'])
                if plugin_env.has_key('CXXFLAGS') and plugin_env['CXXFLAGS']:
                    lib_env.AppendUnique(CXXFLAGS=plugin_env['CXXFLAGS'])
                if plugin_env.has_key('LINKFLAGS') and plugin_env['LINKFLAGS']:
                    lib_env.AppendUnique(LINKFLAGS=plugin_env['LINKFLAGS'])
                if plugin_env.has_key('CPPPATH') and plugin_env['CPPPATH']:
                    lib_env.AppendUnique(CPPPATH=copy(plugin_env['CPPPATH']))
                if plugin_env.has_key('LIBS') and plugin_env['LIBS']:
                    lib_env.AppendUnique(LIBS=plugin_env['LIBS'])
        else:
            print("Notice: dependencies not met for plugin '%s', not building..." % plugin)
    if hit:
        lib_env.Append(CPPDEFINES = '-DMAPNIK_STATIC_PLUGINS')
        libmapnik_defines.append('-DMAPNIK_STATIC_PLUGINS')

if env['HAS_CAIRO']:
    lib_env.AppendUnique(LIBPATH=env['CAIRO_LIBPATHS'])
    lib_env.Append(CPPDEFINES = '-DHAVE_CAIRO')
    libmapnik_defines.append('-DHAVE_CAIRO')
    lib_env.AppendUnique(CPPPATH=copy(env['CAIRO_CPPPATHS']))
    source.insert(0,'cairo_renderer.cpp')
    source.insert(0,'cairo_context.cpp')

for cpp in enabled_imaging_libraries:
    source.append(cpp)

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
    agg/process_debug_symbolizer.cpp
    """
    )

# clipper
source += Split(
    """
     ../deps/clipper/src/clipper.cpp
    """)

if env['RUNTIME_LINK'] == "static":
    source += glob.glob('../deps/agg/src/' + '*.cpp')

# grid backend
if env['GRID_RENDERER']:
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
    lib_env.Append(CPPDEFINES = '-DGRID_RENDERER')
    libmapnik_defines.append('-DGRID_RENDERER')

# https://github.com/mapnik/mapnik/issues/1438
if env['SVG_RENDERER']: # svg backend
    source += Split(
    """
    svg/output/svg_renderer.cpp
    svg/output/svg_generator.cpp
    svg/output/svg_output_attributes.cpp
    svg/output/process_symbolizers.cpp
    svg/output/process_building_symbolizer.cpp
    svg/output/process_line_pattern_symbolizer.cpp
    svg/output/process_line_symbolizer.cpp
    svg/output/process_markers_symbolizer.cpp
    svg/output/process_point_symbolizer.cpp
    svg/output/process_polygon_pattern_symbolizer.cpp
    svg/output/process_polygon_symbolizer.cpp
    svg/output/process_raster_symbolizer.cpp
    svg/output/process_shield_symbolizer.cpp
    svg/output/process_text_symbolizer.cpp
    """)
    lib_env.Append(CPPDEFINES = '-DSVG_RENDERER')
    libmapnik_defines.append('-DSVG_RENDERER')

if env['XMLPARSER'] == 'libxml2' and env['HAS_LIBXML2']:
    source += Split(
        """
        libxml2_loader.cpp
        """)
    lib_env.Append(CPPDEFINES = '-DHAVE_LIBXML2')
    libmapnik_defines.append('-DHAVE_LIBXML2')
else:
    source += Split(
        """
        rapidxml_loader.cpp
        """
    )

# clone the env one more time to isolate mapnik_lib_link_flag
lib_env_final = lib_env.Clone()
lib_env_final.Prepend(LINKFLAGS=mapnik_lib_link_flag)

# cache library values for other builds to use
env['LIBMAPNIK_LIBS'] = copy(lib_env['LIBS'])
env['LIBMAPNIK_LINKFLAGS'] = copy(lib_env['LINKFLAGS'])
env.Append(LIBMAPNIK_LINKFLAGS=env['CUSTOM_LDFLAGS'])
env['LIBMAPNIK_CXXFLAGS'] = libmapnik_cxxflags
env['LIBMAPNIK_DEFINES'] = libmapnik_defines

mapnik = None

if env['PLATFORM'] == 'Darwin' or not env['ENABLE_SONAME']:
    target_path = env['MAPNIK_LIB_BASE_DEST']
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        if env['LINKING'] == 'static':
            mapnik = lib_env_final.StaticLibrary('mapnik', source)
        else:
            mapnik = lib_env_final.SharedLibrary('mapnik', source)
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
            mapnik = lib_env_final.StaticLibrary('mapnik', source)
        else:
            mapnik = lib_env_final.SharedLibrary('mapnik', source)
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

    # to enable local testing
    lib_major_minor = "%s.%d.%d" % (os.path.basename(env.subst(env['MAPNIK_LIB_NAME'])), int(major), int(minor))
    local_lib = os.path.basename(env.subst(env['MAPNIK_LIB_NAME']))
    if os.path.islink(lib_major_minor) or os.path.exists(lib_major_minor):
        os.remove(lib_major_minor)
    os.symlink(local_lib,lib_major_minor)
    Clean(mapnik,lib_major_minor);

if not env['RUNTIME_LINK'] == 'static':
    Depends(mapnik, env.subst('../deps/agg/libagg.a'))
