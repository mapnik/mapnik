# script to build boost python versions
# this should be run with the boost source directory as the cwd

import os
import sys

USER_JAM = """
import option ;
import feature ;
if ! darwin in [ feature.values <toolset> ]
{
    using darwin ; 
}
project : default-build <toolset>darwin ;
using python
     : %(ver)s # version
     : %(system)s/Library/Frameworks/Python.framework/Versions/%(ver)s/bin/python%(ver)s%(variant)s # cmd-or-prefix
     : %(system)s/Library/Frameworks/Python.framework/Versions/%(ver)s/include/python%(ver)s%(variant)s # includes
     : %(system)s/Library/Frameworks/Python.framework/Versions/%(ver)s/lib/python%(ver)s/config%(variant)s # a lib actually symlink
     : <toolset>darwin # condition
     ;
libraries = --with-python ;
"""

def compile_lib(ver,arch='32_64'):
    if ver in ('3.2'):
        open('user-config.jam','w').write(USER_JAM % {'ver':ver,'system':'','variant':'m'})
    elif ver in ('2.5','2.6'):
        # build against system pythons so we can reliably link against FAT binaries
        open('user-config.jam','w').write(USER_JAM % {'ver':ver,'system':'/System','variant':''})
    else:
        # for 2.7 and above hope that python.org provides 64 bit ready binaries...
        open('user-config.jam','w').write(USER_JAM % {'ver':ver,'system':'','variant':''})    
    cmd = "./bjam -q --with-python -a -j2 --ignore-site-config --user-config=user-config.jam link=shared toolset=darwin -d2 address-model=%s architecture=x86 variant=release stage" % arch#linkflags=-search_paths_first
    print cmd
    os.system(cmd)

if __name__ == '__main__':
    if not len(sys.argv) > 2:
        sys.exit('usage: %s <ver> <arch>' % os.path.basename(sys.argv[0]))
    compile_lib(sys.argv[1],sys.argv[2])
    
    