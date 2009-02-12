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
                   ColorFactory, CRSFactory, WMSBaseServiceHandler, CRS, \
                   BaseExceptionHandler, Projection
from exceptions import OGCException, ServerConfigurationError
from lxml import etree as ElementTree
from mapnik import Coord

class ServiceHandler(WMSBaseServiceHandler):

    SERVICE_PARAMS = {
        'GetCapabilities': {
            'updatesequence': ParameterDefinition(False, str)
        },
        'GetMap': {
            'layers': ParameterDefinition(True, ListFactory(str)),
            'styles': ParameterDefinition(True, ListFactory(str)),
            'srs': ParameterDefinition(True, CRSFactory(['EPSG'])),
            'bbox': ParameterDefinition(True, ListFactory(float)),
            'width': ParameterDefinition(True, int),
            'height': ParameterDefinition(True, int),
            'format': ParameterDefinition(True, str, allowedvalues=('image/png', 'image/jpeg')),
            'transparent': ParameterDefinition(False, str, 'FALSE', ('TRUE', 'FALSE')),
            'bgcolor': ParameterDefinition(False, ColorFactory, ColorFactory('0xFFFFFF')),
            'exceptions': ParameterDefinition(False, str, 'application/vnd.ogc.se_xml', ('application/vnd.ogc.se_xml', 'application/vnd.ogc.se_inimage', 'application/vnd.ogc.se_blank'))
        },
        'GetFeatureInfo': {
            'layers': ParameterDefinition(True, ListFactory(str)),
            'styles': ParameterDefinition(False, ListFactory(str)),
            'srs': ParameterDefinition(True, CRSFactory(['EPSG'])),
            'bbox': ParameterDefinition(True, ListFactory(float)),
            'width': ParameterDefinition(True, int),
            'height': ParameterDefinition(True, int),
            'format': ParameterDefinition(False, str, allowedvalues=('image/png', 'image/jpeg')),
            'transparent': ParameterDefinition(False, str, 'FALSE', ('TRUE', 'FALSE')),
            'bgcolor': ParameterDefinition(False, ColorFactory, ColorFactory('0xFFFFFF')),
            'exceptions': ParameterDefinition(False, str, 'application/vnd.ogc.se_xml', ('application/vnd.ogc.se_xml', 'application/vnd.ogc.se_inimage', 'application/vnd.ogc.se_blank')),
            'query_layers': ParameterDefinition(True, ListFactory(str)),
            'info_format': ParameterDefinition(True, str, allowedvalues=('text/plain', 'text/xml')),
            'feature_count': ParameterDefinition(False, int, 1),
            'x': ParameterDefinition(True, int),
            'y': ParameterDefinition(True, int)
        }        
    }

    CONF_SERVICE = [
        ['title', 'Title', str],
        ['abstract', 'Abstract', str],
        ['onlineresource', 'OnlineResource', str],
        ['fees', 'Fees', str],
        ['accessconstraints', 'AccessConstraints', str],
        ['keywordlist', 'KeywordList', str]
    ]

    capabilitiesxmltemplate = """<?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <!DOCTYPE WMT_MS_Capabilities SYSTEM "http://www.digitalearth.gov/wmt/xml/capabilities_1_1_1.dtd">
    <WMT_MS_Capabilities version="1.1.1" updateSequence="0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.opengis.net/wms">
      <Service>
        <Name>WMS</Name>
      </Service>
      <Capability>
        <Request>
          <GetCapabilities>
            <Format>application/vnd.ogc.wms_xml</Format>
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
          <Format>application/vnd.ogc.se_xml</Format>
          <Format>application/vnd.ogc.se_inimage</Format>
          <Format>application/vnd.ogc.se_blank</Format>
        </Exception>
        <Layer>
          <Title>A Mapnik WMS Server</Title>
          <Abstract>A Mapnik WMS Server</Abstract>
        </Layer>
      </Capability>
    </WMT_MS_Capabilities>
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
    
            elements = capetree.findall('Capability//OnlineResource')
            for element in elements:
                element.set('{http://www.w3.org/1999/xlink}href', self.opsonlineresource)
    
            self.processServiceCapabilities(capetree)
    
            rootlayerelem = capetree.find('{http://www.opengis.net/wms}Capability/{http://www.opengis.net/wms}Layer')
    
            for epsgcode in self.allowedepsgcodes:
                rootlayercrs = ElementTree.Element('SRS')
                rootlayercrs.text = epsgcode.upper()
                rootlayerelem.append(rootlayercrs)
    
            for layer in self.mapfactory.layers.values():
                layerproj = Projection(layer.srs)
                layername = ElementTree.Element('Name')
                layername.text = layer.name
                env = layer.envelope()
                llp = layerproj.inverse(Coord(env.minx, env.miny))
                urp = layerproj.inverse(Coord(env.maxx, env.maxy))
                latlonbb = ElementTree.Element('LatLonBoundingBox')
                latlonbb.set('minx', str(llp.x))
                latlonbb.set('miny', str(llp.y))
                latlonbb.set('maxx', str(urp.x))
                latlonbb.set('maxy', str(urp.y))
                layerbbox = ElementTree.Element('BoundingBox')
                layerbbox.set('SRS', layerproj.epsgstring())
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
                layere.append(latlonbb)
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
            self.capabilities = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n' + ElementTree.tostring(capetree)
        response = Response('application/vnd.ogc.wms_xml', self.capabilities)
        return response

    def GetMap(self, params):
        params['crs'] = params['srs']
        return WMSBaseServiceHandler.GetMap(self, params)

    def GetFeatureInfo(self, params):
        params['crs'] = params['srs']
        params['i'] = params['x']
        params['j'] = params['y']
        return WMSBaseServiceHandler.GetFeatureInfo(self, params, 'query_map_point')        

class ExceptionHandler(BaseExceptionHandler):

    xmlmimetype = "application/vnd.ogc.se_xml"

    xmltemplate = ElementTree.fromstring("""<?xml version='1.0' encoding="UTF-8" standalone="no"?>
    <!DOCTYPE ServiceExceptionReport SYSTEM "http://www.digitalearth.gov/wmt/xml/exception_1_1_1.dtd">
    <ServiceExceptionReport version="1.1.1">
      <ServiceException />
    </ServiceExceptionReport>
    """)

    xpath = 'ServiceException'

    handlers = {'application/vnd.ogc.se_xml': BaseExceptionHandler.xmlhandler,
                'application/vnd.ogc.se_inimage': BaseExceptionHandler.inimagehandler,
                'application/vnd.ogc.se_blank': BaseExceptionHandler.blankhandler}

    defaulthandler = 'application/vnd.ogc.se_xml'
