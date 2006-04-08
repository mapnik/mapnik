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

from common import ParameterDefinition, BaseServiceHandler, Response, \
                   PIL_TYPE_MAPPING, Version, ListFactory, ColorFactory
from exceptions import OGCException, ServerConfigurationError
from ConfigParser import SafeConfigParser
from mapnik import Map, Color, Envelope, render, rawdata, Image, Projection, \
                   DEGREES
from PIL.Image import fromstring
from StringIO import StringIO
from lxml import etree as ElementTree

class ServiceHandler111(BaseServiceHandler):

    SERVICE_PARAMS = {
        'getcapabilities': {
            'updatesequence': ParameterDefinition(False, cast=str)
        },
        'getmap': {
            'version': ParameterDefinition(True, allowedvalues=(Version('1.1.1'),), cast=Version),
            'layers': ParameterDefinition(True, cast=ListFactory(str)),
            'styles': ParameterDefinition(True, cast=ListFactory(str)),
            'srs': ParameterDefinition(True, match='^EPSG:\d+$'),
            'bbox': ParameterDefinition(True, cast=ListFactory(float)),
            'width': ParameterDefinition(True, cast=int),
            'height': ParameterDefinition(True, cast=int),
            'format': ParameterDefinition(True, allowedvalues=('image/png','image/jpeg','image/gif'), cast=str.lower),
            'transparent': ParameterDefinition(False, 'FALSE', allowedvalues=('true','false'), cast=str.lower),
            'bgcolor': ParameterDefinition(False, ColorFactory('0xFFFFFF'), cast=ColorFactory),
            'exceptions': ParameterDefinition(False, 'application/vnd.ogc.se_xml', ('application/vnd.ogc.se_xml',), cast=str)
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

    def __init__(self, configpath, factory, opsonlineresource):
        self.factory = factory
        self.conf = SafeConfigParser()
        self.conf.readfp(open(configpath))
        if self.conf.has_option('service', 'epsg'):
            self.epsgcode = self.conf.get('service', 'epsg')
            self.proj = Projection(['init=epsg:' + self.epsgcode])
        else:
            raise "Missing EPSG code"
        if not opsonlineresource.endswith('?'): opsonlineresource += '?'

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
        rootlayersrs.text = 'EPSG:%s' % self.epsgcode

        dict = self.factory()

        for layer in dict['layers']:
            layername = ElementTree.Element('Name')
            layername.text = layer.name()
            layertitle = ElementTree.Element('Title')
            layertitle.text = layer.name()
            env = layer.envelope()
            llp = self.proj.Inverse(env.minx, env.miny)
            urp = self.proj.Inverse(env.maxx, env.maxy)
            latlonbb = ElementTree.Element('LatLonBoundingBox')
            latlonbb.set('minx', str(llp[0]))
            latlonbb.set('miny', str(llp[1]))
            latlonbb.set('maxx', str(urp[0]))
            latlonbb.set('maxy', str(urp[1]))
            layerbbox = ElementTree.Element('BoundingBox')
            layerbbox.set('SRS', 'EPSG:%s' % self.epsgcode)
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

    def getcapabilities(self, params):
        response = Response('application/vnd.ogc.wms_xml', self.capabilities)
        return response
        
    def getmap(self, params):
        m = Map(params['width'], params['height'])
        if params['transparent'].lower() == 'false':
            m.background = params['bgcolor']
        mo = self.factory()
        for layername in params['layers']:
            for layer in mo['layers']:
                if layer.name() == layername:
                    for stylename in layer.styles:
                        if stylename in mo['styles'].keys():
                            m.append_style(stylename, mo['styles'][stylename])
                    m.layers.append(layer)
        m.zoom_to_box(Envelope(params['bbox'][0], params['bbox'][1], params['bbox'][2], params['bbox'][3]))
        im = Image(params['width'], params['height'])
        render(m, im)
        im2 = fromstring('RGBA', (params['width'], params['height']), rawdata(im))
        fh = StringIO()
        im2.save(fh, PIL_TYPE_MAPPING[params['format']])
        fh.seek(0)
        response = Response(params['format'], fh.read())
        return response

class ServiceHandler130(BaseServiceHandler):

    SERVICE_PARAMS = {
        'getcapabilities': {
            'format': ParameterDefinition(False, 'text/xml', ('text/xml',), True, cast=str),
        },
        'getmap': {
            'version': ParameterDefinition(True, allowedvalues=('1.3.0',), cast=Version),
            'layers': ParameterDefinition(True, cast=ListFactory(str)),
            'styles': ParameterDefinition(True, cast=ListFactory(str)),
            'crs': ParameterDefinition(True, match='^EPSG:\d+$'),
            'bbox': ParameterDefinition(True, cast=ListFactory(float)),
            'width': ParameterDefinition(True, cast=int),
            'height': ParameterDefinition(True, cast=int),
            'format': ParameterDefinition(True, allowedvalues=('image/gif','image/png','image/jpeg'), cast=str.lower),
            'transparent': ParameterDefinition(False, allowedvalues=('true','false'), cast=str.lower),
            'bgcolor': ParameterDefinition(False, ColorFactory('0xFFFFFF'), cast=ColorFactory),
            'exceptions': ParameterDefinition(False, 'xml', ('xml',), cast=str.lower),
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

    def __init__(self, configpath, factory, opsonlineresource):
        self.factory = factory
        self.conf = SafeConfigParser()
        self.conf.readfp(open(configpath))
        if self.conf.has_option('service', 'epsg'):
            self.epsgcode = self.conf.get('service', 'epsg')
            self.proj = Projection(['init=epsg:' + self.epsgcode])
        else:
            raise "Missing EPSG code"
        if not opsonlineresource.endswith('?'): opsonlineresource += '?'

        capetree = ElementTree.fromstring(self.capabilitiesxmltemplate)
        
        elements = capetree.findall('{http://www.opengis.net/wms}Capability//{http://www.opengis.net/wms}OnlineResource')
        for element in elements:
            element.set('{http://www.w3.org/1999/xlink}href', opsonlineresource)

        if len(self.conf.items('service')) > 0:
            servicee = capetree.find('{http://www.opengis.net/wms}Service')
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

        
        rootlayerelem = capetree.find('{http://www.opengis.net/wms}Capability/{http://www.opengis.net/wms}Layer')

        rootlayercrs = rootlayerelem.find('{http://www.opengis.net/wms}CRS')
        rootlayercrs.text = 'EPSG:%s' % self.epsgcode
        
        dict = self.factory()
        
        for layer in dict['layers']:
            layername = ElementTree.Element('Name')
            layername.text = layer.name()
            layertitle = ElementTree.Element('Title')
            layertitle.text = layer.name()
            env = layer.envelope()
            layerexgbb = ElementTree.Element('EX_GeographicBoundingBox')
            ll = self.proj.Inverse(env.minx, env.miny)
            ur = self.proj.Inverse(env.maxx, env.maxy)
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
            layerbbox.set('CRS', 'EPSG:%s' % self.epsgcode)
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
        
        self.capabilities = '<?xml version="1.0" encoding="UTF-8"?>\n' + ElementTree.tostring(capetree)

    def getcapabilities(self, params):
        response = Response('text/xml', self.capabilities)
        return response
        
    def getmap(self, params):
        if params['width'] > int(self.conf.get('service', 'maxwidth')) and height > int(self.conf.get('service', 'maxheight')):
            raise OGCException('Bad map size.')
        m = Map(params['width'], params['height'])
        if params.has_key('transparent') and params['transparent'].lower() == 'false':
            m.background = params['bgcolor']
        mo = self.factory()
        for layername in params['layers']:
            for layer in mo['layers']:
                if layer.name() == layername:
                    for stylename in layer.styles:
                        if stylename in mo['styles'].keys():
                            m.append_style(stylename, mo['styles'][stylename])
                    m.layers.append(layer)
        m.zoom_to_box(Envelope(params['bbox'][0], params['bbox'][1], params['bbox'][2], params['bbox'][3]))
        im = Image(params['width'], params['height'])
        render(m, im)
        im2 = fromstring('RGBA', (params['width'], params['height']), rawdata(im))
        fh = StringIO()
        im2.save(fh, PIL_TYPE_MAPPING[params['format']])
        fh.seek(0)
        response = Response(params['format'], fh.read())
        return response

def ServiceFactory(configpath, factory, onlineresource, version):
    if not version:
        version = Version('1.3.0')
    else:
        try:
            version = Version(version)
        except:
            raise OGCException("Parameter 'version' has an illegal value.")
    if version >= '1.3.0':
        return ServiceHandler130(configpath, factory, onlineresource)
    else:
        return ServiceHandler111(configpath, factory, onlineresource)
    
    