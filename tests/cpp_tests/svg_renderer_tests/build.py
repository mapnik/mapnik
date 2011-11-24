import os
import glob
from copy import copy

Import ('env')

headers = env['CPPPATH'] 

filesystem = 'boost_filesystem%s' % env['BOOST_APPEND']
system = 'boost_system%s' % env['BOOST_APPEND']
regex = 'boost_regex%s' % env['BOOST_APPEND']

libraries =  copy(env['LIBMAPNIK_LIBS'])
libraries.append('mapnik')

for cpp_test in glob.glob('*_test.cpp'):
    test_program = env.Program(cpp_test.replace('.cpp',''), [cpp_test], CPPPATH=headers, LIBS=libraries, LINKFLAGS=env['CUSTOM_LDFLAGS'])
    Depends(test_program, env.subst('../../../src/%s' % env['MAPNIK_LIB_NAME']))
