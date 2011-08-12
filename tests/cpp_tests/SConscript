import os
import glob
from copy import copy

Import ('env')

test_env = env.Clone()

headers = env['CPPPATH'] 

libraries =  copy(env['LIBMAPNIK_LIBS'])
libraries.append('mapnik2')

for cpp_test in glob.glob('*_test.cpp'):
    test_program = test_env.Program(cpp_test.replace('.cpp',''), [cpp_test], CPPPATH=headers, LIBS=libraries, LINKFLAGS=env['CUSTOM_LDFLAGS'])
    Depends(test_program, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))
