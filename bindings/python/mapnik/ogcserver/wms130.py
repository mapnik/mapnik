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
                   ColorFactory, CRSFactory, CRS, WMSBaseServiceHandler, \
                   BaseExceptionHandler, Projection
from exceptions import OGCException, ServerConfigurationError
from lxml import etree as ElementTree
from mapnik import Coord

class ServiceHandler(WMSBaseServiceHandler):

    SERVICE_PARAMS = {
        'GetCapabilities': {
            'format': ParameterDefinition(False, str, 'text/xml', ('text/xml',), True),
            'updatesequence': ParameterDefinition(False, str)
        },
        'GetMap': {
            'layers': ParameterDefinition(True, ListFactory(str)),
            'styles': ParameterDefinition(True, ListFactory(str)),
            'crs': ParameterDefinition(True, CRSFactory(['EPSG'])),
            'bbox': ParameterDefinition(True, ListFactory(float)),
            'width': ParameterDefinition(True, int),
            'height': ParameterDefinition(True, int),
            'format': ParameterDefinition(True, str, allowedvalues=('image/png', 'image/jpeg')),
            'transparent': ParameterDefinition(False, str, 'FALSE', ('TRUE', 'FALSE')),
            'bgcolor': ParameterDefinition(False, ColorFactory, ColorFactory('0xFFFFFF')),
            'exceptions': ParameterDefinition(False, str, 'XML', ('XML', 'INIMAGE', 'BLANK')),
        },
        'GetFeatureInfo': {
            'layers': ParameterDefinition(True, ListFactory(str)),
            'styles': ParameterDefinition(False, ListFactory(str)),
            'crs': ParameterDefinition(True, CRSFactory(['EPSG'])),
            'bbox': ParameterDefinition(True, ListFactory(float)),
            'width': ParameterDefinition(True, int),
            'height': ParameterDefinition(True, int),
            'format': ParameterDefinition(False, str, allowedvalues=('image/png', 'image/jpeg')),
            'transparent': ParameterDefinition(False, str, 'FALSE', ('TRUE', 'FALSE')),
            'bgcolor': ParameterDefinition(False, ColorFactory, ColorFactory('0xFFFFFF')),
            'exceptions': ParameterDefinition(False, str, 'XML', ('XML', 'INIMAGE', 'BLANK')),
            'query_layers': ParameterDefinition(True, ListFactory(str)),
            'info_format': ParameterDefinition(True, str, allowedvalues=('text/plain', 'text/xml')),
            'feature_count': ParameterDefinition(False, int, 1),
            'i': ParameterDefinition(True, float),
            'j': ParameterDefinition(True, float)
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
        ['maxheight', 'MaxHeight', int],
        ['keywordlist', 'KeywordList', str]
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
              </HTTP>
            </DCPType>
          </GetCapabilities>
          <GetMap>
            <Format>image/png</Format>
            <Format>image/jpeg</Format>
            <DCPType>
              <HTTP>
                <Get>
                  <OnlineResource xlink:type="simple"/>
                </Get>
              </HTTP>
            </DCPType>
          </GetMap>
          <GetFeatureInfo>
            <Format>text/plain</Format>
            <DCPType>
              <HTTP>
                <Get>
                  <OnlineResource xlink:type="simple"/>
                </Get>
              </HTTP>
            </DCPType>
          </GetFeatureInfo>
        </Request>
        <Exception>
          <Format>XML</Format>
          <Format>INIMAGE</Format>
          <Format>BLANK</Format>
        </Exception>
        <Layer>
          <Title>A Mapnik WMS Server</Title>
          <Abstract>A Mapnik WMS Server</Abstract>
        </Layer>
      </Capability>
    </WMS_Capabilities>
    """

    def __init__(self, conf, mapfactory, opsonlineresource):
        self.conf = conf
        self.mapfactory = mapfactory
        self.opsonlineresource = opsonlineresource
        if self.conf.has_option('service', 'allowedepsgcodes'):
            self.allowedepsgcodes = map(lambda code: 'epsg:%s' % code, self.conf.get('service', 'allowedepsgcodes').split(','))
        else:
            raise ServerConfigurationError('Allowed EPSG codes not properly configured.')
        self.capabilities = None

    def GetCapabilities(self, params):
        if not self.capabilities:
            capetree = ElementTree.fromstring(self.capabilitiesxmltemplate)
    
            elements = capetree.findall('{http://www.opengis.net/wms}Capability//{http://www.opengis.net/wms}OnlineResource')
            for element in elements:
                element.set('{http://www.w3.org/1999/xlink}href', self.opsonlineresource)
    
            self.processServiceCapabilities(capetree)
    
            rootlayerelem = capetree.find('{http://www.opengis.net/wms}Capability/{http://www.opengis.net/wms}Layer')
    
            for epsgcode in self.allowedepsgcodes:
                rootlayercrs = ElementTree.Element('CRS')
                rootlayercrs.text = epsgcode.upper()
                rootlayerelem.append(rootlayercrs)
    
            for layer in self.mapfactory.layers.values():
                layerproj = Projection(layer.srs)
                layername = ElementTree.Element('Name')
                layername.text = layer.name
                env = layer.envelope()
                layerexgbb = ElementTree.Element('EX_GeographicBoundingBox')
                ll = layerproj.inverse(Coord(env.minx, env.miny))
                ur = layerproj.inverse(Coord(env.maxx, env.maxy))
                exgbb_wbl = ElementTree.Element('westBoundLongitude')
                exgbb_wbl.text = str(ll.x)
                layerexgbb.append(exgbb_wbl)
                exgbb_ebl = ElementTree.Element('eastBoundLongitude')
                exgbb_ebl.text = str(ur.x)
                layerexgbb.append(exgbb_ebl)
                exgbb_sbl = ElementTree.Element('southBoundLatitude')
                exgbb_sbl.text = str(ll.y)
                layerexgbb.append(exgbb_sbl)
                exgbb_nbl = ElementTree.Element('northBoundLatitude')
                exgbb_nbl.text = str(ur.y)
                layerexgbb.append(exgbb_nbl)
                layerbbox = ElementTree.Element('BoundingBox')
                layerbbox.set('CRS', layerproj.epsgstring())
                layerbbox.set('minx', str(env.minx))
                layerbbox.set('miny', str(env.miny))
                layerbbox.set('maxx', str(env.maxx))
                layerbbox.set('maxy', str(env.maxy))
                layere = ElementTree.Element('Layer')
                layere.append(layername)
                if layer.title:
                    layertitle = ElementTree.Element('Title')
                    layertitle.text = layer.title
                    layere.append(layertitle)
                if layer.abstract:
                    layerabstract = ElementTree.Element('Abstract')
                    layerabstract.text = layer.abstract
                    layere.append(layerabstract)
                if layer.queryable:
                    layere.set('queryable', '1')
                layere.append(layerexgbb)
                layere.append(layerbbox)
                if len(layer.wmsextrastyles) > 0:
                    for extrastyle in [layer.wmsdefaultstyle] + list(layer.wmsextrastyles):
                        style = ElementTree.Element('Style')
                        stylename = ElementTree.Element('Name')
                        stylename.text = extrastyle
                        styletitle = ElementTree.Element('Title')
                        styletitle.text = extrastyle
                        style.append(stylename)
                        style.append(styletitle)
                        layere.append(style)
                rootlayerelem.append(layere)
            self.capabilities = '<?xml version="1.0" encoding="UTF-8"?>' + ElementTree.tostring(capetree)
        response = Response('text/xml', self.capabilities)
        return response

    def GetMap(self, params):
        if params['width'] > int(self.conf.get('service', 'maxwidth')) or params['height'] > int(self.conf.get('service', 'maxheight')):
            raise OGCException('Requested map size exceeds limits set by this server.')
        return WMSBaseServiceHandler.GetMap(self, params)

class ExceptionHandler(BaseExceptionHandler):

    xmlmimetype = "text/xml"

    xmltemplate = ElementTree.fromstring("""<?xml version='1.0' encoding="UTF-8"?>
    <ServiceExceptionReport version="1.3.0"
                            xmlns="http://www.opengis.net/ogc"
                            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                            xsi:schemaLocation="http://www.opengis.net/ogc http://schemas.opengis.net/wms/1.3.0/exceptions_1_3_0.xsd">
      <ServiceException/>
    </ServiceExceptionReport>
    """)

    xpath = '{http://www.opengis.net/ogc}ServiceException'

    handlers = {'XML': BaseExceptionHandler.xmlhandler,
                'INIMAGE': BaseExceptionHandler.inimagehandler,
                'BLANK': BaseExceptionHandler.blankhandler}

    defaulthandler = 'XML'
