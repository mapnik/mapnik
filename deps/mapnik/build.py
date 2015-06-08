import os
from glob import glob

Import('env')

subdirs =  {
  'sparsehash':'sparsehash',
  'sparsehash/internal':'sparsehash/internal',
  '../agg/include':'agg',
}

if 'install' in COMMAND_LINE_TARGETS:
    for k,v in subdirs.items():
        pathdir = os.path.join(k,'*')
        includes = glob(pathdir)
        inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'+v)
        env.Alias(target='install', source=env.Install(inc_target, includes))
