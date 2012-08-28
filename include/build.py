import os
from glob import glob

Import('env')

base = '../include/mapnik/'
subdirs = ['svg','wkt','grid','json','util']

#if env['SVG_RENDERER']:
#    subdirs.append('svg/output')

inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik')

if 'install' in COMMAND_LINE_TARGETS:
    
    includes = glob('../include/mapnik/*.hpp')

    for subdir in subdirs:
        pathdir = os.path.join(base,subdir,'*.hpp')
        includes.extend(glob(pathdir))

    env.Alias(target='install', source=env.Install(inc_target, includes))
    
    # special case these as duplicate named headers break scons
    for subdir in ['text_placements','formatting']:
        includes = glob('../include/mapnik/%s*.hpp' % subdir)
        env.Alias(target='install', source=env.Install(inc_target, includes))

env['create_uninstall_target'](env, inc_target)
