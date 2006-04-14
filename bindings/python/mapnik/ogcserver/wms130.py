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

from common import ParameterDefinition, Response, Version, ListFactory, \
                   ColorFactory, CRSFactory, CRS, WMSBaseServiceHandler
from exceptions import OGCException, ServerConfigurationError, BaseExceptionHandler
from lxml import etree as ElementTree

class ServiceHandler(WMSBaseServiceHandler):

    SERVICE_PARAMS = {
        'GetCapabilities': {
            'format': ParameterDefinition(False, str, 'text/xml', ('text/xml',), True),
            'updatesequence': ParameterDefinition(False, str)
        },
        'GetMap': {
            'version': ParameterDefinition(True, Version, allowedvalues=(Version('1.3.0'),)),
            'layers': ParameterDefinition(True, ListFactory(str)),
            'styles': ParameterDefinition(True, ListFactory(str)),
            'crs': ParameterDefinition(True, CRSFactory(['EPSG'])),
            'bbox': ParameterDefinition(True, ListFactory(float)),
            'width': ParameterDefinition(True, int),
            'height': ParameterDefinition(True, int),
            'format': ParameterDefinition(True, str, allowedvalues=('image/gif','image/png','image/jpeg')),
            'transparent': ParameterDefinition(False, str, 'FALSE', ('TRUE','FALSE')),
            'bgcolor': ParameterDefinition(False, ColorFactory, ColorFactory('0xFFFFFF')),
            'exceptions': ParameterDefinition(False, str, 'XML', ('XML',)),
        }
    }

    CONF_SERVICE = [
        ['title', 'Title', str],
        ['abstract', 'Abstract', str],
        ['onlineresource', 'OnlineResource', str],
        ['fees', 'Fees', str],
        ['accessconstraints', 'AccessConstraints', str],
        ['layerlimit', 'LayerLimit', int],
        ['maxwidth', 'MaxWidth', int],
        ['maxheight', 'MaxHeight', int]
    ]

    capabilitiesxmltemplate = """<?xml version="1.0" encoding="UTF-8"?>
    <WMS_Capabilities version="1.3.0" xmlns="http://www.opengis.net/wms"
                                      xmlns:xlink="http://www.w3.org/1999/xlink"
                                      xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                                      xsi:schemaLocation="http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd">
      <Service>
        <Name>WMS</Name>
      </Service>
      <Capability>
        <Request>
          <GetCapabilities>
            <Format>text/xml</Format>
            <DCPType>
              <HTTP>
                <Get>
                  <OnlineResource xlink:type="simple"/>
                </Get>
                <Post>
                  <OnlineResource xlink:type="simple"/>
                </Post>
              </HTTP>
            </DCPType>
          </GetCapabilities>
          <GetMap>
            <Format>image/png</Format>
            <Format>image/jpeg</Format>
            <Format>image/gif</Format>
            <DCPType>
              <HTTP>
                <Get>
                  <OnlineResource xlink:type="simple"/>
                </Get>
                <Post>
                  <OnlineResource xlink:type="simple"/>
                </Post>
              </HTTP>
            </DCPType>
          </GetMap>
        </Request>
        <Exception>
          <Format>XML</Format>
        </Exception>
        <Layer>
          <Title>A Mapnik WMS Server</Title>
          <Abstract>A Mapnik WMS Server</Abstract>
          <CRS/>
        </Layer>
      </Capability>
    </WMS_Capabilities>
    """

    def __init__(self, conf, mapfactory, opsonlineresource):
        self.conf = conf
        self.mapfactory = mapfactory
        if self.conf.has_option('service', 'epsg'):
            self.crs = CRS('EPSG', self.conf.get('service', 'epsg'))
        else:
            raise ServerConfigurationError('EPSG code not properly configured.')

        capetree = ElementTree.fromstring(self.capabilitiesxmltemplate)
        
        elements = capetree.findall('{http://www.opengis.net/wms}Capability//{http://www.opengis.net/wms}OnlineResource')
        for element in elements:
            element.set('{http://www.w3.org/1999/xlink}href', opsonlineresource)

        if len(self.conf.items('service')) > 0:
            servicee = capetree.find('{http://www.opengis.net/wms}Service')
            for item in self.CONF_SERVICE:
                if self.conf.has_option('service', item[0]):
                    value = self.conf.get('service', item[0]).strip()
                    try:
                        item[2](value)
                    except:
                        raise ServerConfigurationError('Configuration parameter [%s]->%s has an invalid value: %s.' % ('service', item[0], value))
                    if item[0] == 'onlineresource':
                        element = ElementTree.Element('%s' % item[1])
                        servicee.append(element)
                        element.set('{http://www.w3.org/1999/xlink}href', value)
                        element.set('{http://www.w3.org/1999/xlink}type', 'simple')
                    else:
                        element = ElementTree.Element('%s' % item[1])
                        element.text = value
                        servicee.append(element)

        
        rootlayerelem = capetree.find('{http://www.opengis.net/wms}Capability/{http://www.opengis.net/wms}Layer')

        rootlayercrs = rootlayerelem.find('{http://www.opengis.net/wms}CRS')
        rootlayercrs.text = str(self.crs)
        
        for layer in self.mapfactory.layers.values():
            layername = ElementTree.Element('Name')
            layername.text = layer.name()
            layertitle = ElementTree.Element('Title')
            layertitle.text = layer.name()
            env = layer.envelope()
            layerexgbb = ElementTree.Element('EX_GeographicBoundingBox')
            ll = self.crs.Inverse(env.minx, env.miny)
            ur = self.crs.Inverse(env.maxx, env.maxy)
            exgbb_wbl = ElementTree.Element('westBoundLongitude')
            exgbb_wbl.text = str(ll[0])
            layerexgbb.append(exgbb_wbl)
            exgbb_ebl = ElementTree.Element('eastBoundLongitude')
            exgbb_ebl.text = str(ur[0])
            layerexgbb.append(exgbb_ebl)
            exgbb_sbl = ElementTree.Element('southBoundLatitude')
            exgbb_sbl.text = str(ll[1])
            layerexgbb.append(exgbb_sbl)
            exgbb_nbl = ElementTree.Element('northBoundLatitude')
            exgbb_nbl.text = str(ur[1])
            layerexgbb.append(exgbb_nbl)
            layerbbox = ElementTree.Element('BoundingBox')
            layerbbox.set('CRS', str(self.crs))
            layerbbox.set('minx', str(env.minx))
            layerbbox.set('miny', str(env.miny))
            layerbbox.set('maxx', str(env.maxx))
            layerbbox.set('maxy', str(env.maxy))
            layere = ElementTree.Element('Layer')
            layere.append(layername)
            layere.append(layertitle)
            layere.append(layerexgbb)
            layere.append(layerbbox)
            rootlayerelem.append(layere)
        
        self.capabilities = '<?xml version="1.0" encoding="UTF-8"?>' + ElementTree.tostring(capetree)

    def GetCapabilities(self, params):
        response = Response('text/xml', self.capabilities)
        return response
        
    def GetMap(self, params):
        if params['width'] > int(self.conf.get('service', 'maxwidth')) or params['height'] > int(self.conf.get('service', 'maxheight')):
            raise OGCException('Requested map size exceeds limits set by this server.')
        if str(params['crs']) != str(self.crs):
            raise OGCException('Unsupported CRS requested.  Must be "%s" and not "%s".' % (self.crs, params['crs']), 'InvalidCRS')
        return WMSBaseServiceHandler.GetMap(self, params)

class ExceptionHandler(BaseExceptionHandler):
    
    mimetype = "text/xml"
    
    xmltemplate = ElementTree.fromstring("""<?xml version='1.0' encoding="UTF-8"?>
    <ServiceExceptionReport version="1.3.0"
                            xmlns="http://www.opengis.net/ogc"
                            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                            xsi:schemaLocation="http://www.opengis.net/ogc http://schemas.opengis.net/wms/1.3.0/exceptions_1_3_0.xsd">
      <ServiceException/>
    </ServiceExceptionReport>
    """)
    
    xpath = '{http://www.opengis.net/ogc}ServiceException'