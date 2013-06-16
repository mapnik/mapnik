import os
import glob
from copy import copy

Import ('env')

filesystem = 'boost_filesystem%s' % env['BOOST_APPEND']
system = 'boost_system%s' % env['BOOST_APPEND']

libraries =  [filesystem,system]

source = Split(
    """
    singleton_helper.cpp
    singtest.cpp
    """)

liba_env = env.Clone()
liba_env.Append(CPPPATH = '#tests/cpp_tests/singleton_test')
liba_env.Append(LIBPATH = '#tests/cpp_tests/singleton_test')
liba_env.Append(CXXFLAGS="-DMAPNIK_EXPORTS")
utils_a = liba_env.SharedLibrary('utils-a', source,LIBS=[filesystem,system])

source = Split(
    """
    singleton_helperb.cpp
    """)

libb_env = env.Clone()
libb_env.Append(CPPPATH = '#tests/cpp_tests/singleton_test')
libb_env.Append(LIBPATH = '#tests/cpp_tests/singleton_test')
libb_env.Append(CXXFLAGS="-DHELPER_EXPORT")
utils_b = libb_env.SharedLibrary('utils-b', source, LIBS=[filesystem,system,'utils-a'])

libraries.append('utils-a')
libraries.append('utils-b')

test_env = env.Clone()
test_env.Append(CPPPATH = '#tests/cpp_tests/singleton_test')
test_env.Append(LIBPATH = '#tests/cpp_tests/singleton_test')
test_program = test_env.Program('singleton_test', ['main.cpp'], LIBS=libraries, LINKFLAGS=env['CUSTOM_LDFLAGS'])

# build locally if installing
if 'install' in COMMAND_LINE_TARGETS:
    env.Alias('install',test_program)