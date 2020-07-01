#
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
#
#

import os
import sys
import glob
import platform
from copy import copy
from subprocess import Popen, PIPE

Import('env')

lib_env = env.Clone()

def call(cmd, silent=True):
    stdin, stderr = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        return stdin.strip()
    elif not silent:
        print (stderr)

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
lib_env['LIBS'] = [filesystem,
                   regex]

if env['COVERAGE']:
    lib_env.Append(LINKFLAGS='--coverage')
    lib_env.Append(CXXFLAGS='--coverage')

if env['HAS_CAIRO']:
    lib_env.Append(LIBS=env['CAIRO_ALL_LIBS'])

# maybe bz2
if len(env['EXTRA_FREETYPE_LIBS']):
    lib_env['LIBS'].extend(copy(env['EXTRA_FREETYPE_LIBS']))

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

if env['XMLPARSER'] == 'libxml2' and env['HAS_LIBXML2']:
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
    if 'icuuc' in env['ICU_LIB_NAME']:
        lib_env['LIBS'].append('icudata')

if env['PLATFORM'] == 'Linux':
    lib_env['LINKFLAGS'].append('-pthread')

if env['RUNTIME_LINK'] != 'static':
    lib_env['LIBS'].insert(0, 'agg')

lib_env['LIBS'].append('z')

if env['PLATFORM'] == 'FreeBSD':
    lib_env['LIBS'].append('pthread')

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
        if env['PLATFORM'] != 'FreeBSD':
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
    expression_grammar_x3.cpp
    fs.cpp
    request.cpp
    well_known_srs.cpp
    params.cpp
    parse_image_filters.cpp
    generate_image_filters.cpp
    image_filter_grammar_x3.cpp
    color.cpp
    conversions_numeric.cpp
    conversions_string.cpp
    image_copy.cpp
    image_compositing.cpp
    image_scaling.cpp
    datasource_cache.cpp
    datasource_cache_static.cpp
    debug.cpp
    geometry/box2d.cpp
    geometry/closest_point.cpp
    geometry/reprojection.cpp
    geometry/envelope.cpp
    geometry/interior.cpp
    geometry/polylabel.cpp
    expression_node.cpp
    expression_string.cpp
    expression.cpp
    transform_expression.cpp
    transform_expression_grammar_x3.cpp
    feature_kv_iterator.cpp
    feature_style_processor.cpp
    feature_type_style.cpp
    dasharray_parser.cpp
    font_engine_freetype.cpp
    font_set.cpp
    function_call.cpp
    gradient.cpp
    path_expression_grammar_x3.cpp
    parse_path.cpp
    image_reader.cpp
    cairo_io.cpp
    image.cpp
    image_view.cpp
    image_view_any.cpp
    image_any.cpp
    image_options.cpp
    image_util.cpp
    image_util_jpeg.cpp
    image_util_png.cpp
    image_util_tiff.cpp
    image_util_webp.cpp
    layer.cpp
    map.cpp
    load_map.cpp
    palette.cpp
    marker_helpers.cpp
    plugin.cpp
    rule.cpp
    save_map.cpp
    wkb.cpp
    twkb.cpp
    projection.cpp
    proj_transform.cpp
    scale_denominator.cpp
    simplify.cpp
    parse_transform.cpp
    memory_datasource.cpp
    symbolizer.cpp
    symbolizer_keys.cpp
    symbolizer_enumerations.cpp
    unicode.cpp
    raster_colorizer.cpp
    mapped_memory_cache.cpp
    marker_cache.cpp
    css/css_color_grammar_x3.cpp
    css/css_grammar_x3.cpp
    svg/svg_parser.cpp
    svg/svg_path_parser.cpp
    svg/svg_points_parser.cpp
    svg/svg_transform_parser.cpp
    svg/svg_path_grammar_x3.cpp
    warp.cpp
    vertex_cache.cpp
    vertex_adapters.cpp
    text/font_library.cpp
    text/text_layout.cpp
    text/text_line.cpp
    text/itemizer.cpp
    text/scrptrun.cpp
    text/face.cpp
    text/glyph_positions.cpp
    text/placement_finder.cpp
    text/properties_util.cpp
    text/renderer.cpp
    text/color_font_renderer.cpp
    text/symbolizer_helpers.cpp
    text/text_properties.cpp
    text/font_feature_settings.cpp
    text/formatting/base.cpp
    text/formatting/list.cpp
    text/formatting/text.cpp
    text/formatting/format.cpp
    text/formatting/layout.cpp
    text/formatting/registry.cpp
    text/placements/registry.cpp
    text/placements/base.cpp
    text/placements/dummy.cpp
    text/placements/list.cpp
    text/placements/simple.cpp
    group/group_layout_manager.cpp
    group/group_rule.cpp
    group/group_symbolizer_helper.cpp
    xml_tree.cpp
    config_error.cpp
    color_factory.cpp
    renderer_common.cpp
    renderer_common/render_group_symbolizer.cpp
    renderer_common/render_markers_symbolizer.cpp
    renderer_common/render_pattern.cpp
    renderer_common/render_thunk_extractor.cpp
    renderer_common/pattern_alignment.cpp
    util/math.cpp
    value.cpp
    png_io.cpp
    """
    )

if env['PLUGIN_LINKING'] == 'static':
    hit = False
    lib_env.AppendUnique(CPPPATH='../plugins/')
    for plugin in env['REQUESTED_PLUGINS']:
        details = env['PLUGINS'][plugin]
        if not details['lib'] or details['lib'] in env['LIBS']:
            plugin_env = SConscript('../plugins/input/%s/build.py' % plugin)
            if not plugin_env:
                print("Notice: no 'plugin_env' variable found for plugin: '%s'" % plugin)
            else:
                hit = True
                DEF = '-DMAPNIK_STATIC_PLUGIN_%s' % plugin.upper()
                lib_env.Append(CPPDEFINES = DEF)
                if DEF not in libmapnik_defines:
                    libmapnik_defines.append(DEF)
                if 'SOURCES' in plugin_env and plugin_env['SOURCES']:
                    source += ['../plugins/input/%s/%s' % (plugin, src) for src in plugin_env['SOURCES']]
                if 'CPPDEFINES' in plugin_env  and plugin_env['CPPDEFINES']:
                    lib_env.AppendUnique(CPPDEFINES=plugin_env['CPPDEFINES'])
                if 'CXXFLAGS' in plugin_env and plugin_env['CXXFLAGS']:
                    lib_env.AppendUnique(CXXFLAGS=plugin_env['CXXFLAGS'])
                if 'LINKFLAGS' in plugin_env and plugin_env['LINKFLAGS']:
                    lib_env.AppendUnique(LINKFLAGS=plugin_env['LINKFLAGS'])
                if 'CPPPATH' in plugin_env and plugin_env['CPPPATH']:
                    lib_env.AppendUnique(CPPPATH=copy(plugin_env['CPPPATH']))
                if 'LIBS' in plugin_env and plugin_env['LIBS']:
                    lib_env.AppendUnique(LIBS=plugin_env['LIBS'])
        else:
            print("Notice: dependencies not met for plugin '%s', not building..." % plugin)
    if hit:
        lib_env.Append(CPPDEFINES = '-DMAPNIK_STATIC_PLUGINS')
        libmapnik_defines.append('-DMAPNIK_STATIC_PLUGINS')

# add these to the compile flags no matter what
# to make it safe to try to compile them from Makefile wrapper
source += Split("""
cairo/process_markers_symbolizer.cpp
cairo/process_group_symbolizer.cpp
""")

if env['ENABLE_GLIBC_WORKAROUND']:
    source += Split(
            """
            glibc_workaround.cpp
            """
        )

if env['HAS_CAIRO']:
    lib_env.AppendUnique(LIBPATH=env['CAIRO_LIBPATHS'])
    lib_env.Append(CPPDEFINES = '-DHAVE_CAIRO')
    libmapnik_defines.append('-DHAVE_CAIRO')
    lib_env.AppendUnique(CPPPATH=copy(env['CAIRO_CPPPATHS']))
    source += Split("""
    cairo/cairo_context.cpp
    cairo/cairo_renderer.cpp
    cairo/cairo_render_vector.cpp
    cairo/process_text_symbolizer.cpp
    cairo/process_line_symbolizer.cpp
    cairo/process_line_pattern_symbolizer.cpp
    cairo/process_polygon_symbolizer.cpp
    cairo/process_polygon_pattern_symbolizer.cpp
    cairo/process_debug_symbolizer.cpp
    cairo/process_point_symbolizer.cpp
    cairo/process_raster_symbolizer.cpp
    cairo/process_building_symbolizer.cpp
    """)

for cpp in enabled_imaging_libraries:
    source.append(cpp)

# agg backend
source += Split(
    """
    agg/agg_renderer.cpp
    agg/process_dot_symbolizer.cpp
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
    agg/process_group_symbolizer.cpp
    agg/process_debug_symbolizer.cpp
    """
    )

# libimagequant
lib_env.Append(CFLAGS = "-O3 -fno-math-errno -funroll-loops -fomit-frame-pointer -std=c99")
if env["USE_SSE"] == "yes":
    lib_env.Append(CFLAGS="-msse -mfpmath=sse")
    # As of GCC 4.5, 387 fp math is significantly slower in C99 mode without this.
    # Note: CPUs without SSE2 use 387 for doubles, even when SSE fp math is set.
    if 'gcc' in env['CC']:
        lib_env.Append(CFLAGS='-fexcess-precision=fast')
elif env["USE_SSE"] == "no":
    lib_env.Append(CFLAGS="-mno-sse")
elif env["USE_SSE"] == "platform_default":
    # Let compiler decide
    None

source += glob.glob("../deps/pngquant/" + "*.c")

if env['RUNTIME_LINK'] == "static":
    source += glob.glob('../deps/agg/src/' + '*.cpp')

# add these to the compile flags no matter what
# to make it safe to try to compile them from Makefile wrapper
source += Split("""
grid/process_markers_symbolizer.cpp
grid/process_group_symbolizer.cpp
""")

# grid backend
if env['GRID_RENDERER']:
    source += Split(
        """
        grid/grid.cpp
        grid/grid_renderer.cpp
        grid/process_building_symbolizer.cpp
        grid/process_line_pattern_symbolizer.cpp
        grid/process_line_symbolizer.cpp
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
    svg/output/svg_output_grammars.cpp
    svg/output/svg_renderer.cpp
    svg/output/svg_generator.cpp
    svg/output/svg_output_attributes.cpp
    svg/output/process_symbolizers.cpp
    svg/output/process_line_symbolizer.cpp
    svg/output/process_polygon_symbolizer.cpp
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
env['LIBMAPNIK_CXXFLAGS'] = libmapnik_cxxflags
env['LIBMAPNIK_DEFINES'] = libmapnik_defines

mapnik = None

if env['PLATFORM'] == 'Darwin' or not env['ENABLE_SONAME']:
    target_path = env['MAPNIK_LIB_BASE_DEST']
    if 'uninstall' not in COMMAND_LINE_TARGETS:
        if env['LINKING'] == 'static':
            mapnik = lib_env_final.StaticLibrary(env['MAPNIK_NAME'], source)
        else:
            mapnik = lib_env_final.SharedLibrary(env['MAPNIK_NAME'], source)
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
            mapnik = lib_env_final.StaticLibrary(env['MAPNIK_NAME'], source)
        else:
            mapnik = lib_env_final.SharedLibrary(env['MAPNIK_NAME'], source)
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
