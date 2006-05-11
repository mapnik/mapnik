#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2006 Jean-Francois Doyon
#
# Mapnik is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# $Id$

from os import environ
from tempfile import gettempdir
environ['PYTHON_EGG_CACHE'] = gettempdir()

import sys
from jon import cgi
from exceptions import OGCException, ServerConfigurationError
from wms111 import ExceptionHandler as ExceptionHandler111
from wms130 import ExceptionHandler as ExceptionHandler130
from ConfigParser import SafeConfigParser
from common import Version

class Handler(cgi.DebugHandler):

    def __init__(self):
        conf = SafeConfigParser()
        conf.readfp(open(self.configpath))
        self.conf = conf
        if not conf.has_option('server', 'module'):
            raise ServerConfigurationError('The factory module is not defined in the configuration file.')
        try:
            mapfactorymodule = __import__(conf.get('server', 'module'))
        except ImportError:
            raise ServerConfigurationError('The factory module could not be loaded.')
        if hasattr(mapfactorymodule, 'WMSFactory'):
            self.mapfactory = getattr(mapfactorymodule, 'WMSFactory')()
        else:
            raise ServerConfigurationError('The factory module does not have a WMSFactory class.')
        if conf.has_option('server', 'debug'):
            self.debug = int(conf.get('server', 'debug'))
        else:
            self.debug = 0

    def process(self, req):
        reqparams = lowerparams(req.params)
        onlineresource = 'http://%s:%s%s?' % (req.environ['SERVER_NAME'], req.environ['SERVER_PORT'], req.environ['SCRIPT_NAME'])
        if not reqparams.has_key('request'):
            raise OGCException('Missing request parameter.')
        request = reqparams['request']
        del reqparams['request']
        if request == 'GetCapabilities' and not reqparams.has_key('service'):
            raise OGCException('Missing service parameter.')
        if request in ['GetMap', 'GetFeatureInfo']:
            service = 'WMS'
        else:
            service = reqparams['service']
        if reqparams.has_key('service'):
            del reqparams['service']
        try:
            mapnikmodule = __import__('mapnik.ogcserver.' + service)
        except:
            raise OGCException('Unsupported service "%s".' % service)
        ServiceHandlerFactory = getattr(mapnikmodule.ogcserver, service).ServiceHandlerFactory
        servicehandler = ServiceHandlerFactory(self.conf, self.mapfactory, onlineresource, reqparams.get('version', None))
        del reqparams['version']
        if request not in servicehandler.SERVICE_PARAMS.keys():
            raise OGCException('Operation "%s" not supported.' % request, 'OperationNotSupported')
        ogcparams = servicehandler.processParameters(request, reqparams)
        try:
            requesthandler = getattr(servicehandler, request)
        except:
            raise OGCException('Operation "%s" not supported.' % request, 'OperationNotSupported')
        response = requesthandler(ogcparams)
        req.set_header('Content-Type', response.content_type)
        req.write(response.content)

    def traceback(self, req):
        reqparams = lowerparams(req.params)
        version = reqparams.get('version', None)
        if not version:
            version = Version('1.3.0')
        else:
            version = Version(version)
        if version >= '1.3.0':
            eh = ExceptionHandler130(self.debug)
        else:
            eh = ExceptionHandler111(self.debug)
        response = eh.getresponse(reqparams)
        req.set_header('Content-Type', response.content_type)
        req.write(response.content)

def lowerparams(params):
    reqparams = {}
    for key, value in params.items():
        reqparams[key.lower()] = value
    return reqparams