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
                   ColorFactory, CRSFactory, WMSBaseServiceHandler, CRS
from exceptions import OGCException, ServerConfigurationError, BaseExceptionHandler
from lxml import etree as ElementTree

class ServiceHandler(WMSBaseServiceHandler):

    SERVICE_PARAMS = {
        'GetCapabilities': {
            'updatesequence': ParameterDefinition(False, str)
        },
        'GetMap': {
            'version': ParameterDefinition(True, Version, allowedvalues=(Version('1.1.1'),)),
            'layers': ParameterDefinition(True, ListFactory(str)),
            'styles': ParameterDefinition(True, ListFactory(str)),
            'srs': ParameterDefinition(True, CRSFactory(['EPSG'])),
            'bbox': ParameterDefinition(True, ListFactory(float)),
            'width': ParameterDefinition(True, int),
            'height': ParameterDefinition(True, int),
            'format': ParameterDefinition(True, str, allowedvalues=('image/png','image/jpeg','image/gif')),
            'transparent': ParameterDefinition(False, str, 'FALSE', ('TRUE','FALSE')),
            'bgcolor': ParameterDefinition(False, ColorFactory, ColorFactory('0xFFFFFF')),
            'exceptions': ParameterDefinition(False, str, 'application/vnd.ogc.se_xml', ('application/vnd.ogc.se_xml',))
        }
    }

    CONF_SERVICE = [
        ['title', 'Title', str],
        ['abstract', 'Abstract', str],
        ['onlineresource', 'OnlineResource', str],
        ['fees', 'Fees', str],
        ['accessconstraints', 'AccessConstraints', str],
    ]

    capabilitiesxmltemplate = """<?xml version='1.0' encoding="UTF-8" standalone="no"?>
    <!DOCTYPE WMT_MS_Capabilities SYSTEM "http://www.digitalearth.gov/wmt/xml/capabilities_1_1_1.dtd">
    <WMT_MS_Capabilities version="1.1.1" updateSequence="0" xmlns:xlink="http://www.w3.org/1999/xlink">
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
          <Format>application/vnd.ogc.se_xml</Format>
        </Exception>
        <Layer>
          <Title>A Mapnik WMS Server</Title>
          <Abstract>A Mapnik WMS Server</Abstract>
          <SRS/>
        </Layer>
      </Capability>
    </WMT_MS_Capabilities>
    """

    def __init__(self, conf, mapfactory, opsonlineresource):
        self.conf = conf
        self.mapfactory = mapfactory
        if self.conf.has_option('service', 'epsg'):
            self.crs = CRS('EPSG', self.conf.get('service', 'epsg'))
        else:
            ServerConfigurationError('EPSG code not properly configured.')

        capetree = ElementTree.fromstring(self.capabilitiesxmltemplate)
        
        elements = capetree.findall('Capability//OnlineResource')
        for element in elements:
            element.set('{http://www.w3.org/1999/xlink}href', opsonlineresource)

        if len(self.conf.items('service')) > 0:
            servicee = capetree.find('Service')
            for item in self.CONF_SERVICE:
                if self.conf.has_option('service', item[0]):
                    value = self.conf.get('service', item[0])
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

        rootlayerelem = capetree.find('Capability/Layer')
        
        rootlayersrs = rootlayerelem.find('SRS')
        rootlayersrs.text = str(self.crs)

        for layer in self.mapfactory.getlayers():
            layername = ElementTree.Element('Name')
            layername.text = layer.name()
            layertitle = ElementTree.Element('Title')
            layertitle.text = layer.name()
            env = layer.envelope()
            llp = self.crs.Inverse(env.minx, env.miny)
            urp = self.crs.Inverse(env.maxx, env.maxy)
            latlonbb = ElementTree.Element('LatLonBoundingBox')
            latlonbb.set('minx', str(llp[0]))
            latlonbb.set('miny', str(llp[1]))
            latlonbb.set('maxx', str(urp[0]))
            latlonbb.set('maxy', str(urp[1]))
            layerbbox = ElementTree.Element('BoundingBox')
            layerbbox.set('SRS', str(self.crs))
            layerbbox.set('minx', str(env.minx))
            layerbbox.set('miny', str(env.miny))
            layerbbox.set('maxx', str(env.maxx))
            layerbbox.set('maxy', str(env.maxy))
            layere = ElementTree.Element('Layer')
            layere.append(layername)
            layere.append(layertitle)
            layere.append(latlonbb)
            layere.append(layerbbox)
            rootlayerelem.append(layere)
        
        self.capabilities = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n' + ElementTree.tostring(capetree)

    def GetCapabilities(self, params):
        response = Response('application/vnd.ogc.wms_xml', self.capabilities)
        return response

class ExceptionHandler(BaseExceptionHandler):
    
    mimetype = "application/vnd.ogc.se_xml"
    
    xmltemplate = ElementTree.fromstring("""<?xml version='1.0' encoding="UTF-8" standalone="no"?>
    <!DOCTYPE ServiceExceptionReport SYSTEM "http://www.digitalearth.gov/wmt/xml/exception_1_1_1.dtd">
    <ServiceExceptionReport version="1.1.1">
      <ServiceException />
    </ServiceExceptionReport>
    """)
    
    xpath = 'ServiceException'