import os
import glob

Import ('env')

headers = env['CPPPATH'] 

filesystem = 'boost_filesystem%s' % env['BOOST_APPEND']
system = 'boost_system%s' % env['BOOST_APPEND']
regex = 'boost_regex%s' % env['BOOST_APPEND']

libraries =  [filesystem, 'mapnik2']
libraries.append(env['ICU_LIB_NAME'])
libraries.append(regex)
libraries.append(system)

for cpp_test in glob.glob('path_element_test.cpp'):
    env.Program(cpp_test.replace('.cpp',''), [cpp_test], CPPPATH=headers, LIBS=libraries)