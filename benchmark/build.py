import os
import glob
from copy import copy

Import ('env')

test_env = env.Clone()

test_env['LIBS'] = [env['MAPNIK_NAME']]
test_env.AppendUnique(LIBS=copy(env['LIBMAPNIK_LIBS']))
test_env.AppendUnique(LIBS='mapnik-wkt')
if env['PLATFORM'] == 'Linux':
    test_env.AppendUnique(LIBS='dl')
    test_env.AppendUnique(LIBS='rt')
test_env.AppendUnique(CXXFLAGS='-g')
test_env['CXXFLAGS'] = copy(test_env['LIBMAPNIK_CXXFLAGS'])
test_env.Append(CPPDEFINES = env['LIBMAPNIK_DEFINES'])
if test_env['HAS_CAIRO']:
    test_env.PrependUnique(CPPPATH=test_env['CAIRO_CPPPATHS'])
    test_env.Append(CPPDEFINES = '-DHAVE_CAIRO')
test_env.PrependUnique(CPPPATH='include', delete_existing=True)
test_env['LINKFLAGS'] = copy(test_env['LIBMAPNIK_LINKFLAGS'])
if env['PLATFORM'] == 'Darwin':
    test_env.Append(LINKFLAGS='-F/ -framework CoreFoundation')

test_env_local = test_env.Clone()

#benchmarks = glob.glob('test*cpp')
benchmarks = [
    #"test_array_allocation.cpp",
    #"test_png_encoding1.cpp",
    #"test_png_encoding2.cpp",
    #"test_to_string1.cpp",
    #"test_to_string2.cpp",
    #"test_to_bool.cpp",
    #"test_to_double.cpp",
    #"test_to_int.cpp",
    #"test_utf_encoding.cpp"
    "test_polygon_clipping.cpp",
    #"test_polygon_clipping_rendering.cpp",
    "test_proj_transform1.cpp",
    "test_expression_parse.cpp",
    "test_face_ptr_creation.cpp",
    "test_font_registration.cpp",
    "test_rendering.cpp",
    "test_rendering_shared_map.cpp",
    "test_offset_converter.cpp",
    "test_marker_cache.cpp",
    "test_quad_tree.cpp",
    "test_noop_rendering.cpp",
    "test_getline.cpp",
#    "test_numeric_cast_vs_static_cast.cpp",
]
for cpp_test in benchmarks:
    test_program = test_env_local.Program('out/'+cpp_test.replace('.cpp',''), source=[cpp_test])
    if 'install' in COMMAND_LINE_TARGETS:
        env.Alias('install',test_program)
    #Depends(test_program, env.subst('../src/%s' % env['MAPNIK_LIB_NAME']))
