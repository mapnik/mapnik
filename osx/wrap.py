#!/usr/bin/env python 

import os
import glob
import shutil
from os.path import join
import sys

def sym(target,link):
    relative = os.path.relpath(target,link)
    if relative.startswith('../'):
        relative = relative.replace('../','',1)
    try:
        os.symlink(relative,link)
    except OSError, e:
        raise OSError('%s: %s' % (e,link))

def copy_all_items(pattern,place,recursive=False):
    items = glob.glob(pattern)
    for i in items:
       if recursive:
           # -R is needed to preserve symlinks
           os.system('cp -R %s %s' % (i,place))
       else:
           shutil.copy(i,place)


if __name__ == "__main__":

    # final resting place
    install_path = '/Library/Frameworks'

    # move to this dir
    here = os.path.dirname(os.path.realpath(__file__))
    os.chdir(here)
       
    # relative path to DESTDIR output
    framework = 'Library/Frameworks/Mapnik.framework'
   
    # Framework version, likely should be 2.0.0..
    version = '2.0'
    
    # Versions/Current symlink
    current = join(framework,'Versions',version)

    # HERE WE GO...
    
    # set up symlinks
    # from existing directory, to link
    active = join(framework,'Versions/Current')
    #os.system('rm -rf %s' % active)
    sym(current,active)
    
    # point top level 'unix' to active one
    sym(join(active,'unix'),join(framework,'unix'))
    
    # install boost and icu...
    # recursive to preserve symlinks
    copy_all_items('sources/lib/libicuu*dylib',join(active,'unix/lib'),recursive=True)
    copy_all_items('sources/lib/libicud*dylib',join(active,'unix/lib'),recursive=True)
    copy_all_items('sources/lib/libicui1*dylib',join(active,'unix/lib'),recursive=True)
    copy_all_items('sources/lib/libboost*dylib',join(active,'unix/lib'),recursive=True)
    
    # move over headers
    #TODO

    # Resources
    os.mkdir(join(active,'Resources'))
    # TODO - put docs and other stuff here...
    # or link to /share
    sym(join(active,'Resources'),join(framework,'Resources'))
    shutil.copy('Info.plist',join(active,'Resources'))
    
    # Mapnik Headers
    sym(join(active,'unix/include'),join(active,'Headers'))
    sym(join(active,'Headers'),join(framework,'Headers'))
    
    # Programs
    sym(join(active,'unix/bin'),join(active,'Programs'))
    sym(join(active,'Programs'),join(framework,'Programs'))
    
    # Datasources
    sym(join(active,'unix/lib/mapnik2/input'),join(active,'Datasources'))
    sym(join(active,'Datasources'),join(framework,'Datasources'))
    
    # Fonts
    # TODO - move fonts to main level...
    sym(join(active,'unix/lib/mapnik2/fonts'),join(active,'Fonts'))
    sym(join(active,'Fonts'),join(framework,'Fonts'))
    
    # symlinks to user and system font directories.
    # TODO - NEED TO BENCHMARK performance hit of extra loading time and memory usage
    #sym('/Library/Fonts', join(fonts,'Library'))
    #sym('/System/Library/Fonts', join(fonts,'System'))
    
    # symlink the lib
    sym(join(active,'unix/lib/libmapnik2.dylib'),join(active,'Mapnik'))
    sym(join(active,'Mapnik'),join(framework,'Mapnik'))
    
    # Python
    py_dir = join(active,'unix/lib/python2.6/site-packages')
    sym(py_dir,join(active,'Python'))
    sym(join(active,'Python'),join(framework,'Python'))
    
    # try to start using relative paths..
    #paths_py = '''
    #import os
    #inputpluginspath = os.path.normpath(os.path.join(os.path.dirname(__file__),'../../../../Datasources'))
    #fontscollectionpath = os.path.normpath(os.path.join(os.path.dirname(__file__),'../../../../Fonts'))
    #'''
    
    paths_py = '''
inputpluginspath = '%(install_path)s/Mapnik.framework/Datasources'
fontscollectionpath = '%(install_path)s/Mapnik.framework/Fonts'
'''
    
    # TODO - consider making _mapnik.so import dependent on version
    # so we can simplify install..
    # py26
    mapnik_module = join(py_dir,'mapnik2')
    open(mapnik_module+'/paths.py','w').write(paths_py % locals())
    shutil.copy('../bindings/python/mapnik/__init__.py',mapnik_module)
    
    # write pth
    pth ='''import sys; sys.path.insert(0,'%(install_path)s/Mapnik.framework/Python')
    ''' % locals()
    # TODO - testing hack, will add this local python binding to sys path for snow leopard
    open('/Library/Python/2.6/site-packages/mapnik.pth','w').write(pth)

    # Stash in resources as well
    open(join(active,'mapnik.pth'),'w').write(pth)
    