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
from exceptions import OGCException
from wms130 import ExceptionHandler
from lxml import etree as ElementTree
from ConfigParser import SafeConfigParser

class Handler(cgi.DebugHandler):

    def __init__(self):
        conf = SafeConfigParser()
        conf.readfp(open(self.configpath))
        self.conf = conf
        mapfactorymodule = __import__(conf.get('server', 'module'))
        self.mapfactory = getattr(mapfactorymodule, 'MapFactory')()
        self.requesthandlers = {}

    def process(self, req):
        exceptionhandler = ExceptionHandler
        reqparams = {}
        for key, value in req.params.items():
            reqparams[key.lower()] = value
        onlineresource = 'http://%s:%s%s' % (req.environ['SERVER_NAME'], req.environ['SERVER_PORT'], req.environ['SCRIPT_NAME'])
        try:
            if not reqparams.has_key('request'):
                raise OGCException('Missing request parameter.')
            request = reqparams['request'].lower()
            if request == 'getcapabilities' and not reqparams.has_key('service'):
                raise OGCException('Missing service parameter.')
            if request in ['getmap', 'getfeatureinfo']:
                reqparams['service'] = 'wms'
            service = reqparams['service'].lower()
            srkey = (service, request)
            if self.requesthandlers.has_key(srkey):
                requesthandler = self.requesthandlers[srkey]
            else:
                try:
                    mapnikmodule = __import__('mapnik.ogcserver.' + service)
                except:
                    raise OGCException('Unsupported service "%s".' % service)
                ServiceHandlerFactory = getattr(mapnikmodule.ogcserver, service).ServiceHandlerFactory
                servicehandler, exceptionhandler = ServiceHandlerFactory(self.conf, self.mapfactory, onlineresource, reqparams.get('version', None))
                if request not in servicehandler.SERVICE_PARAMS.keys():
                    raise OGCException('Operation "%s" not supported.' % request, 'OperationNotSupported')
                ogcparams = servicehandler.processParameters(request, reqparams)
                try:
                    requesthandler = getattr(servicehandler, request)
                except:
                    raise OGCException('Operation "%s" not supported.' % request, 'OperationNotSupported')
                else:
                    self.requesthandlers[srkey] = requesthandler
            response = requesthandler(ogcparams)
        except OGCException:
            eh = exceptionhandler()
            req.set_header('Content-Type', eh.mimetype)
            req.write(ElementTree.tostring(eh.getexcetree(sys.exc_info()[1])))
        else:
            req.set_header('Content-Type', response.content_type)
            req.write(response.content)