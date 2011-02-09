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
        if os.path.exists(link):
            os.unlink(link)
        os.symlink(relative,link)
    except OSError, e:
        raise OSError('%s: %s' % (e,link))

def copy_all_items(pattern,place,recursive=True):
    items = glob.glob(pattern)
    for i in items:
       if recursive:
           # -R is needed to preserve symlinks
           os.system('cp -R %s %s' % (i,place))
       else:
           shutil.copy(i,place)

def drop(path):
    if os.path.islink(path) or os.path.lexists(path):
        if os.path.isabs(path):
            print 'not deleting item as it is an absolute path: %' % path
        else:
            os.system('rm -r %s' % path)

if __name__ == "__main__":

    
    # include headers for a full SDK?
    INCLUDE_HEADERS = True
    
    INCLUDE_NODE = False

    INCLUDE_PYTHON = True
    
    INCLUDE_CAIRO = True
    
    INCLUDE_PYCAIRO = False
    
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
    #drop('rm -rf %s' % active)
    sym(current,active)
    
    # point top level 'unix' to active one
    sym(join(active,'unix'),join(framework,'unix'))

    # create headers directory if needed
    if INCLUDE_HEADERS:
        if not os.path.exists(join(active,'unix/include')):
            os.mkdir(join(active,'unix/include'))

    # Mapnik Headers
    if INCLUDE_HEADERS:
        sym(join(active,'unix/include'),join(active,'Headers'))
        sym(join(active,'Headers'),join(framework,'Headers'))
    else:
        # purge the installed headers of mapnik
        drop('rm -rf %s' % join(active,'unix/include'))
    
    if INCLUDE_CAIRO:
        
        # install cairo libs and deps
        copy_all_items('sources/lib/libcairo*dylib',join(active,'unix/lib'),recursive=True)
        copy_all_items('sources/lib/libfontconfig*dylib',join(active,'unix/lib'),recursive=True)
        copy_all_items('sources/lib/libsigc-2.0*dylib',join(active,'unix/lib'),recursive=True)
        copy_all_items('sources/lib/libpixman*dylib',join(active,'unix/lib'),recursive=True)
    
        # install cairo includes
        if INCLUDE_HEADERS:
            # what is layout?
            for group in ['cairo','cairomm-1.0','fontconfig','pixman-1','pycairo','sigc++-2.0','layout']:
                if not os.path.exists(join(active,'unix/include/%s' % group)):
                    os.mkdir(join(active,'unix/include/%s' % group))
                copy_all_items('sources/include/%s/*' % group,join(active,'unix/include/%s' % group),recursive=True)
            
            if not INCLUDE_PYCAIRO:
                drop('rm -rf %s' % join(active,'unix/include/pycairo'))

            # likely uneeded
            if not os.path.exists(join(active,'unix/lib/sigc++-2.0')):
                os.mkdir(join(active,'unix/lib/sigc++-2.0'))
            copy_all_items('sources/lib/sigc++-2.0/*',join(active,'unix/lib/sigc++-2.0'),recursive=True)
            
    
    # install icu libs
    copy_all_items('sources/lib/libicuu*dylib',join(active,'unix/lib'),recursive=True)
    copy_all_items('sources/lib/libicud*dylib',join(active,'unix/lib'),recursive=True)
    copy_all_items('sources/lib/libicui1*dylib',join(active,'unix/lib'),recursive=True)

    # install icu includes
    if INCLUDE_HEADERS:
        if not os.path.exists(join(active,'unix/include/unicode')):
            os.mkdir(join(active,'unix/include/unicode'))
        copy_all_items('sources/include/unicode/*',join(active,'unix/include/unicode'),recursive=True)

    # install boost libs
    copy_all_items('sources/lib/libboost*dylib',join(active,'unix/lib'),recursive=True)

    # install boost includes
    if INCLUDE_HEADERS:
        if not os.path.exists(join(active,'unix/include/boost')):
            os.mkdir(join(active,'unix/include/boost'))
        copy_all_items('sources/include/boost/*',join(active,'unix/include/boost'),recursive=True)

    # install rasterlite lib
    copy_all_items('sources/lib/librasterlite*dylib',join(active,'unix/lib'),recursive=True)

    # install freetype2 libs
    copy_all_items('sources/lib/libfreetype*dylib',join(active,'unix/lib'),recursive=True)

    # install freetype2 includes
    if INCLUDE_HEADERS:
        if not os.path.exists(join(active,'unix/include/freetype2')):
            os.mkdir(join(active,'unix/include/freetype2'))
        copy_all_items('sources/include/freetype2/*',join(active,'unix/include/freetype2'),recursive=True)
        copy_all_items('sources/include/ft2build.h',join(active,'unix/include/'),recursive=True)
    
    # Node-mapnik bindings location
    if INCLUDE_NODE:
        if not os.path.exists(join(active,'unix/lib/node')):
            os.mkdir(join(active,'unix/lib/node'))
            os.mkdir(join(active,'unix/lib/node/mapnik'))
        sym(join(active,'unix/lib/node'),join(active,'Node'))
        sym(join(active,'Node'),join(framework,'Node'))
    
        # do this later...
        copy_all_items('deps/node-mapnik/mapnik/*',join(active,'unix/lib/node/mapnik/'),recursive=True)
    else:
        drop('rm -r %s' % join(active,'Node'))
        drop('rm -r %s' % join(framework,'Node'))
        drop('rm -r %s' % join(active,'unix/lib/node/'))
    
    # Resources
    if not os.path.exists(join(active,'Resources')):
        os.mkdir(join(active,'Resources'))
    # TODO - put docs and other stuff here...
    # or link to /share
    sym(join(active,'Resources'),join(framework,'Resources'))
    shutil.copy('Info.plist',join(active,'Resources'))
        
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
    if INCLUDE_PYTHON:
        #python_versions =  glob.glob('unix/lib/python*')
        #for py in python_versions:
        #    py_dir = join(active,'%s/site-packages' % py)
        if not os.path.exists(join(active,'Python')):
            os.mkdir(join(active,'Python'))
            os.mkdir(join(active,'Python/mapnik2'))
            if INCLUDE_PYCAIRO:
                os.mkdir(join(active,'Python/cairo'))
        shutil.copy('python/mapnik.py',join(active,'Python/mapnik2/__init__.py'))
        if INCLUDE_PYCAIRO:
            if not os.path.exists(join(active,'Python/cairo/')):
                os.mkdir(join(active,'Python/cairo/'))
            shutil.copy('python/cairo.py',join(active,'Python/cairo/__init__.py'))
        else:
            drop('rm -rf %s' % join(active,'Python/cairo'))
            
        #sym(py_dir,join(active,'Python'))
        sym(join(active,'Python'),join(framework,'Python'))
                    

        # pycairo module
        for pyver in glob.glob('sources/lib/python*'):
            ver = os.path.basename(pyver)
            to = join(active,'unix/lib/%s/site-packages/cairo/' % ver)
            if INCLUDE_PYCAIRO:
                assert os.path.exists(join(active,'unix/lib/%s' % ver))
                assert os.path.exists(join(active,'unix/lib/%s/site-packages' % ver))
                if not os.path.exists(to):
                    os.mkdir(to)
                copy_all_items('sources/lib/%s/site-packages/cairo/*' % ver,to,recursive=True)
            else:
                drop('rm -r %s' % to)
        
        # try to start using relative paths..
        #paths_py = '''
        #import os
        #inputpluginspath = os.path.normpath(os.path.join(os.path.dirname(__file__),'../../../../Datasources'))
        #fontscollectionpath = os.path.normpath(os.path.join(os.path.dirname(__file__),'../../../../Fonts'))
        #'''
        
        #paths_py = '''
        #inputpluginspath = '%(install_path)s/Mapnik.framework/Datasources'
        #fontscollectionpath = '%(install_path)s/Mapnik.framework/Fonts'
        #'''
        
        # TODO - consider making _mapnik.so import dependent on version
        # so we can simplify install..
        
        # done via scons install...
        #mapnik_module = join(py_dir,'mapnik2')
        #open(mapnik_module+'/paths.py','w').write(paths_py % locals())
        #shutil.copy('../bindings/python/mapnik/__init__.py',mapnik_module)
        
        # pth file
        pth ='''import sys; sys.path.insert(0,'%(install_path)s/Mapnik.framework/Python')
        ''' % locals()
        
        # TODO - testing hack, will add this local python binding to sys path for snow leopard
        #open('/Library/Python/2.6/site-packages/mapnik.pth','w').write(pth)
    
        # Stash in resources as well
        open(join(active,'Resources/mapnik2.pth'),'w').write(pth)
    