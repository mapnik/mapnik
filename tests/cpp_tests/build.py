import os
import glob
from copy import copy

Import ('env')

test_env = env.Clone()

test_env['LIBS'] = copy(env['LIBMAPNIK_LIBS'])
test_env.AppendUnique(LIBS='mapnik')
test_env.AppendUnique(LIBS='sqlite3')
test_env.AppendUnique(CXXFLAGS='-g')

for cpp_test in glob.glob('*_test.cpp'):
    name = cpp_test.replace('.cpp','-bin')
    source_files = [cpp_test]
    test_program = None
    if 'agg_blend_src_over_test' in cpp_test:
        # customization here for faster compile
        agg_env = Environment(ENV=os.environ)
        agg_env['CXX'] = env['CXX']
        agg_env['CXXFLAGS'] = env['CXXFLAGS']
        if 'agg' in test_env['LIBS']:
            agg_env.AppendUnique(LIBS='agg')
        agg_env.Append(CPPPATH = '#deps/agg/include')
        agg_env.Append(LIBPATH = '#deps/agg')
        agg_env['CPPPATH'] = ['#deps/agg/include',env['BOOST_INCLUDES']]
        test_program = agg_env.Program(name, source=source_files, LINKFLAGS=env['CUSTOM_LDFLAGS'])
    else:
        test_env_local = test_env.Clone()
        if 'csv_parse' in cpp_test:
            source_files += glob.glob('../../plugins/input/csv/' + '*.cpp')
        test_program = test_env_local.Program(name, source=source_files, LINKFLAGS=env['CUSTOM_LDFLAGS'])
        Depends(test_program, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))
    # build locally if installing
    if 'install' in COMMAND_LINE_TARGETS:
        env.Alias('install',test_program)
