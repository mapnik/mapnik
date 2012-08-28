import os
from glob import glob

Import('env')

base = './mapnik/'
subdirs = ['','svg','wkt','grid','json','util','text_placements','formatting']

#if env['SVG_RENDERER']:
#    subdirs.append('svg/output')

if 'install' in COMMAND_LINE_TARGETS:
    for subdir in subdirs:
        pathdir = os.path.join(base,subdir,'*.hpp')
        includes = glob(pathdir)
        inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'+subdir)
        env.Alias(target='install', source=env.Install(inc_target, includes))

env['create_uninstall_target'](env, os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'))
