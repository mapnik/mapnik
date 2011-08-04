import os
import re
import sys
import datetime
from subprocess import PIPE, Popen, call as subcall

now = datetime.datetime.now()
date = now.strftime('%Y_%m_%d') # year, mon, day

INSTALL = False

plat = 'snow_leopard'
arch = 'intel'
version = '2.0.0'#-dev

def get(cmd):
    try:
        sub = cmd.split(' ')
        response = Popen(sub, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        cm = response.communicate()
        return cm[0]
        if not cm[0]:
            sys.exit(cm[1])
    except OSError, E:
        sys.exit(E)

if plat == 'snow_leopard':
    short_plat = 'snow'
else:
    short_plat = plat

def get_svn():
    pattern = r'(\d+)(.*)'
    svn_version = get('svnversion ../')
    return re.match(pattern,svn_version).groups()[0]

svn = get_svn()

identifier = '''mapnik build log
----------------
version: %(version)s
os: %(plat)s
arch: %(arch)s
date: %(date)s
svn revision: %(svn)s

''' % locals()

if __name__ == '__main__':
    
    
    if not os.path.exists('installer'):
        sys.exit("must be run from directory below 'installer' dir")
    
    print 'removing old files'
    #os.system('rm installer/pkg/Mapnik.pkg')
    os.system('rm installer/*dmg')
    
    print 'writing build log'
    open('installer/pkg/build_log.txt','w').write(identifier)

    cmd = 'packagemaker --doc installer/mapnik.pmdoc --out installer/pkg/Mapnik.pkg'
    print cmd
    #os.system(cmd)

    cmd = 'hdiutil create "installer/mapnik_%s_%s_%s_%s_%s.dmg" -volname "Mapnik %s" -fs HFS+ -srcfolder installer/pkg' % (version,short_plat,arch,date,svn,version)
    print cmd
    os.system(cmd)
