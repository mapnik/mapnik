import os
import glob
from copy import copy

Import ('env')

test_env = env.Clone()

test_env['LIBS'] = copy(env['LIBMAPNIK_LIBS'])
test_env.AppendUnique(LIBS='mapnik')
test_env.AppendUnique(CXXFLAGS='-g')
if 'g++' in env['CXX']:
    test_env.Append(CXXFLAGS='-fPIC')
if env['PLATFORM'] == 'Darwin':
    test_env.Append(LINKFLAGS='-F/ -framework CoreFoundation')

test_env_local = test_env.Clone()

for cpp_test in glob.glob('*cpp'):
    test_program = test_env_local.Program('out/'+cpp_test.replace('.cpp',''), source=[cpp_test])
    if 'install' in COMMAND_LINE_TARGETS:
        env.Alias('install',test_program)
    #Depends(test_program, env.subst('../src/%s' % env['MAPNIK_LIB_NAME']))
