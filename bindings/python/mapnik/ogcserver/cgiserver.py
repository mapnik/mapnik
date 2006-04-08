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

import jon.cgi as cgi
from os import environ, access
from exceptions import OGCException
import sys
from copy import deepcopy
from tempfile import gettempdir

environ['PYTHON_EGG_CACHE'] = gettempdir()
from lxml import etree as ElementTree

class Handler(cgi.DebugHandler):

    ogcexcetree = ElementTree.fromstring("""<?xml version='1.0' encoding="UTF-8"?>
    <ServiceExceptionReport version="1.1.1">
      <ServiceException />
    </ServiceExceptionReport>
    """)

    def __init__(self):
        self.requesthandlers = {}

    def process(self, req):
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
                except ImportError:
                    raise OGCException('Service "%s" not supported.' % service)
                ServiceFactory = getattr(mapnikmodule.ogcserver, service).ServiceFactory
                servicehandler = ServiceFactory(self.configpath, self.factory, onlineresource, reqparams.get('version', None))
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
            req.set_header('Content-Type', 'text/xml')
            ogcexcetree = deepcopy(self.ogcexcetree)
            e = ogcexcetree.find('ServiceException')
            e.text = sys.exc_value.args[0]
            if len(sys.exc_value.args) == 2:
                e.set('code', sys.exc_value.args[1])
            req.write(ElementTree.tostring(ogcexcetree))
        else:
            req.set_header('Content-Type', response.content_type)
            req.write(response.content)