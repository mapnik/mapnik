import os
import glob
from copy import copy

Import ('env')

test_env = env.Clone()

if not env['CPP_TESTS']:
    for cpp_test_bin in glob.glob('./*/*-bin'):
        os.unlink(cpp_test_bin)
    if os.path.exists('./unit/run'): os.unlink('./unit/run')
    if os.path.exists('./visual/run'): os.unlink('./visual/run')
else:
    test_env['LIBS'] = [env['MAPNIK_NAME']]
    test_env.AppendUnique(LIBS='mapnik-wkt')
    test_env.AppendUnique(LIBS='mapnik-json')
    test_env.AppendUnique(LIBS=copy(env['LIBMAPNIK_LIBS']))
    if env['RUNTIME_LINK'] == 'static' and env['PLATFORM'] == 'Linux':
        test_env.AppendUnique(LIBS='dl')
    test_env.AppendUnique(CXXFLAGS='-g')
    test_env['CXXFLAGS'] = copy(test_env['LIBMAPNIK_CXXFLAGS'])
    test_env.Append(CPPDEFINES = env['LIBMAPNIK_DEFINES'])

    if test_env['HAS_CAIRO']:
        test_env.PrependUnique(CPPPATH=test_env['CAIRO_CPPPATHS'])
        test_env.Append(CPPDEFINES = '-DHAVE_CAIRO')
    test_env.PrependUnique(CPPPATH=['./'])
    if test_env['PLATFORM'] == 'Linux':
        test_env['LINKFLAGS'].append('-pthread')
    test_env.AppendUnique(LIBS='boost_program_options%s' % env['BOOST_APPEND'])
    test_env_local = test_env.Clone()


    # unit tests
    sources = glob.glob('./unit/*/*.cpp')
    sources.extend(glob.glob('./unit/*.cpp'))
    sources.append('../plugins/input/ogr/ogr_utils.cpp')
    test_program = test_env_local.Program("./unit/run", source=sources)
    Depends(test_program, env.subst('../src/%s' % env['MAPNIK_LIB_NAME']))
    Depends(test_program, env.subst('../src/json/libmapnik-json${LIBSUFFIX}'))
    Depends(test_program, env.subst('../src/wkt/libmapnik-wkt${LIBSUFFIX}'))
    if 'install' in COMMAND_LINE_TARGETS:
        env.Alias('install',test_program)

    # standalone tests
    for standalone in glob.glob('./standalone/*cpp'):
        test_program2 = test_env_local.Program(standalone.replace('.cpp','-bin'), source=standalone)
        Depends(test_program, env.subst('../src/%s' % env['MAPNIK_LIB_NAME']))
        if 'install' in COMMAND_LINE_TARGETS:
            env.Alias('install',test_program2)

    # visual tests
    source = Split(
        """
        visual/report.cpp
        visual/runner.cpp
        visual/run.cpp
        visual/parse_map_sizes.cpp
        """
        )
    test_program3 = test_env_local.Program('visual/run', source=source)
    Depends(test_program, env.subst('../src/%s' % env['MAPNIK_LIB_NAME']))

    # build locally if installing
    if 'install' in COMMAND_LINE_TARGETS:
        env.Alias('install',test_program3)
