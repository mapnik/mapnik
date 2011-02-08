import sys

ver_int = int('%s%s' % (sys.version_info[0],sys.version_info[1]))
ver_str = '%s.%s' % (sys.version_info[0],sys.version_info[1])

path_insert = '/Library/Frameworks/Mapnik.framework/Versions/2.0/unix/lib/python%s/site-packages/'

if ver_int < 26:
    raise ImportError('Cairo bindings are only available for python versions >= 2.6')
elif ver_int in (26,27,31):
    sys.path.insert(0, path_insert % ver_str)
    from cairo import *
elif ver_int > 31:
    raise ImportError('Cairo bindings are only available for python versions <= 3.1')
else:
    raise ImportError('Cairo bindings are only available for python versions 2.6, 2.7, and 3.1')

