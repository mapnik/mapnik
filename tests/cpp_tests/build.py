import os
import glob
from copy import copy

Import ('env')

test_env = env.Clone()

if not env['CPP_TESTS']:
    for cpp_test_bin in glob.glob('*-bin'):
        os.unlink(cpp_test_bin)
else:
    test_env['LIBS'] = [env['MAPNIK_NAME']]
    test_env.AppendUnique(LIBS=copy(env['LIBMAPNIK_LIBS']))
    if env['RUNTIME_LINK'] == 'static' and env['PLATFORM'] == 'Linux':
        test_env.AppendUnique(LIBS='dl')
    test_env.AppendUnique(CXXFLAGS='-g')
    test_env['CXXFLAGS'] = copy(test_env['LIBMAPNIK_CXXFLAGS'])
    test_env.Append(CPPDEFINES = env['LIBMAPNIK_DEFINES'])
    if test_env['HAS_CAIRO']:
        test_env.PrependUnique(CPPPATH=test_env['CAIRO_CPPPATHS'])
        test_env.Append(CPPDEFINES = '-DHAVE_CAIRO')
    for cpp_test in glob.glob('*_test.cpp'):
        name = cpp_test.replace('.cpp','-bin')
        source_files = [cpp_test]
        test_program = None
        # enable for faster compile while developing just this test
        #if 'agg_blend_src_over_test' in cpp_test:
        if False:
            # customization here for faster compile
            agg_env = Environment(ENV=os.environ)
            agg_env['CXX'] = env['CXX']
            agg_env['CXXFLAGS'] = env['CXXFLAGS']
            if 'agg' in test_env['LIBS']:
                agg_env.AppendUnique(LIBS='agg')
            agg_env.Append(CPPPATH = '#deps/agg/include')
            agg_env.Append(LIBPATH = '#deps/agg')
            agg_env['CPPPATH'] = ['#deps/agg/include',env['BOOST_INCLUDES']]
            test_program = agg_env.Program(name, source=source_files)
        else:
            test_env_local = test_env.Clone()
            if 'csv_parse' in cpp_test:
                source_files += glob.glob('../../plugins/input/csv/' + '*.cpp')
            test_program = test_env_local.Program(name, source=source_files)
            Depends(test_program, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))
        # build locally if installing
        if 'install' in COMMAND_LINE_TARGETS:
            env.Alias('install',test_program)
