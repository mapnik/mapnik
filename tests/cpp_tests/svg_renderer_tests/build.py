import os
import glob
from copy import copy

Import ('env')

libraries = ['mapnik']
libraries.extend(copy(env['LIBMAPNIK_LIBS']))

for cpp_test in glob.glob('*_test.cpp'):
    test_program = env.Program(cpp_test.replace('.cpp',''), [cpp_test], LIBS=libraries)
    Depends(test_program, env.subst('../../../src/%s' % env['MAPNIK_LIB_NAME']))
