import os
from glob import glob

Import('env')

subdirs =  {
  './sparsehash':{'dir':'sparsehash','glob':'*'},
  './sparsehash/internal':{'dir':'sparsehash/internal','glob':'*'},
  '../agg/include':{'dir':'agg','glob':'agg*'},
  '../mapbox/variant/include':{'dir':'mapbox','glob':'*/*.hpp'},
  '../mapbox/geometry/include':{'dir':'mapbox','glob':'*/*.hpp'},
  '../mapbox/geometry/include/mapbox':{'dir':'mapbox/geometry','glob':'*/*.hpp'}
}

if 'install' in COMMAND_LINE_TARGETS:
    for k,v in subdirs.items():
        pathdir = os.path.join(k,v['glob'])
        includes = glob(pathdir)
        inc_target = os.path.normpath(env['INSTALL_PREFIX']+'/include/mapnik/'+v['dir'])
        env.Alias(target='install', source=env.Install(inc_target, includes))
