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
# $Id: modserver.py 283 2006-07-22 18:54:53Z jdoyon $

import sys
import traceback
from mod_python import apache, util
from mod_python.util import parse_qsl
from exceptions import OGCException, ServerConfigurationError
from wms111 import ExceptionHandler as ExceptionHandler111
from wms130 import ExceptionHandler as ExceptionHandler130
from configparser import SafeConfigParser
from common import Version


class ModHandler(object):
    def __init__(self, configpath):
        conf = SafeConfigParser()
        conf.readfp(open(configpath))
        self.conf = conf
        if not conf.has_option_with_value('server', 'module'):
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

    def __call__(self, apacheReq):
        #if not apacheReq:
        try:
            # This doesn't use the obvious way because the port is incorrect
            # on Apache 2.0
            
            print util.FieldStorage(apacheReq)
            apacheReqparams = lowerparams(apacheReq.args)
            #TODO: CGI SCRIPT_NAME not supported on mod_python
            port = apacheReq.connection.local_addr[1]
            onlineresource = 'http://%s:%s/%s?' % (apacheReq.hostname, port, 'wms.py') 
            if not apacheReqparams.has_key('apacheRequest'):
                raise OGCException('Missing apacheRequest parameter.')
            apacheRequest = apacheReqparams['apacheRequest']
            del apacheReqparams['apacheRequest']
            if apacheRequest == 'GetCapabilities' and not apacheReqparams.has_key('service'):
                raise OGCException('Missing service parameter.')
            if apacheRequest in ['GetMap', 'GetFeatureInfo']:
                service = 'WMS'
            else:
                service = apacheReqparams['service']
            if apacheReqparams.has_key('service'):
                del apacheReqparams['service']
            try:
                mapnikmodule = __import__('mapnik.ogcserver.' + service)
            except:
                raise OGCException('Unsupported service "%s".' % service)
            ServiceHandlerFactory = getattr(mapnikmodule.ogcserver, service).ServiceHandlerFactory
            servicehandler = ServiceHandlerFactory(self.conf, self.mapfactory, onlineresource, apacheReqparams.get('version', None))
            if apacheReqparams.has_key('version'):
                del apacheReqparams['version']
            if apacheRequest not in servicehandler.SERVICE_PARAMS.keys():
                raise OGCException('Operation "%s" not supported.' % apacheRequest, 'OperationNotSupported')
    
            # Get parameters and pass to WMSFactory in custom "setup" method
            ogcparams = servicehandler.processParameters(apacheRequest, apacheReqparams)
            try:
                apacheRequesthandler = getattr(servicehandler, apacheRequest)
            except:
                raise OGCException('Operation "%s" not supported.' % apacheRequest, 'OperationNotSupported')
            response = apacheRequesthandler(ogcparams)
            apacheReq.content_type = response.content_type
            apacheReq.status = apache.HTTP_OK
            apacheReq.send_http_header()
            apacheReq.write(response.content)
        except Exception, E:
            #self.traceback(apacheReq)
            apacheReq.content_type = "text/plain"
            apacheReq.status = apache.HTTP_NOT_FOUND
            apacheReq.send_http_header()
            apacheReq.write("An error occurred: %s\n%s\n" % (
                str(E), 
                "".join(traceback.format_tb(sys.exc_traceback))))
        return apache.OK

    def traceback(self, apacheReq):
        print >>sys.stderr, util.FieldStorage(apacheReq)
        apacheReqparams = lowerparams(parse_qsl(apacheReq.args))
        version = apacheReqparams.get('version', None)
        if not version:
            version = Version('1.3.0')
        else:
            version = Version(version)
        if version >= '1.3.0':
            eh = ExceptionHandler130(self.debug)
        else:
            eh = ExceptionHandler111(self.debug)
        response = eh.getresponse(apacheReqparams)
        apacheReq.content_type = response.content_type
        apacheReq.status = apache.HTTP_INTERNAL_SERVER_ERROR
        apacheReq.send_http_header()
        apacheReq.write(response.content)

def lowerparams(params):
    apacheReqparams = {}
    for key, value in params:
        apacheReqparams[key.lower()] = value
    return apacheReqparams