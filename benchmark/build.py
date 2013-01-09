import os
import glob
from copy import copy

Import ('env')

test_env = env.Clone()

test_env['LIBS'] = copy(env['LIBMAPNIK_LIBS'])
test_env.AppendUnique(LIBS='mapnik')
#test_env.AppendUnique(LIBS='sqlite3')
test_env.AppendUnique(CXXFLAGS='-g')

for cpp_test in glob.glob('run*.cpp'):
    name = cpp_test.replace('.cpp','')
    source_files = [cpp_test]
    test_env_local = test_env.Clone()
    test_program = test_env_local.Program(name, source=source_files, LINKFLAGS=env['CUSTOM_LDFLAGS'])
    Depends(test_program, env.subst('../src/%s' % env['MAPNIK_LIB_NAME']))
    # build locally if installing
    if 'install' in COMMAND_LINE_TARGETS:
        env.Alias('install',test_program)
