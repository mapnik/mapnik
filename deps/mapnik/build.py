import os
from glob import glob

Import('env')

subdirs = ['sparsehash','sparsehash/internal']

if 'install' in COMMAND_LINE_TARGETS:
    for subdir in subdirs:
        pathdir = os.path.join(subdir,'*')
        includes = glob(pathdir)
        inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'+subdir)
        env.Alias(target='install', source=env.Install(inc_target, includes))
