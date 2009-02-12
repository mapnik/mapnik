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

from exceptions import OGCException, ServerConfigurationError
from mapnik import Map, Color, Envelope, render, Image, Projection as MapnikProjection, render_to_file, Coord
from PIL.Image import fromstring, new
from PIL.ImageDraw import Draw
from StringIO import StringIO
from copy import deepcopy
from traceback import format_exception, format_exception_only
from sys import exc_info
from lxml import etree as ElementTree
import re
import sys
# from elementtree import ElementTree
# ElementTree._namespace_map.update({'http://www.opengis.net/wms': 'wms',
#                                    'http://www.opengis.net/ogc': 'ogc',
#                                    'http://www.w3.org/1999/xlink': 'xlink',
#                                    'http://www.w3.org/2001/XMLSchema-instance': 'xsi'
#                                    })

PIL_TYPE_MAPPING = {'image/jpeg': 'jpeg', 'image/png': 'png'}

class ParameterDefinition:

    def __init__(self, mandatory, cast, default=None, allowedvalues=None, fallback=False):
        """ An OGC request parameter definition.  Used to describe a
            parameter's characteristics.

            @param mandatory: Is this parameter required by the request?
            @type mandatory: Boolean.

            @param default: Default value to use if one is not provided
                            and the parameter is optional.
            @type default: None or any valid value.

            @param allowedvalues: A list of allowed values for the parameter.
                                  If a value is provided that is not in this
                                  list, an error is raised.
            @type allowedvalues: A python tuple of values.

            @param fallback: Whether the value of the parameter should fall
                             back to the default should an illegal value be
                             provided.
            @type fallback: Boolean.

            @return: A L{ParameterDefinition} instance.
        """
        if mandatory not in [True, False]:
            raise ServerConfigurationError("Bad value for 'mandatory' parameter, must be True or False.")
        self.mandatory = mandatory
        if not callable(cast):
            raise ServerConfigurationError('Cast parameter definition must be callable.')
        self.cast = cast
        self.default = default
        if allowedvalues and type(allowedvalues) != type(()):
            raise ServerConfigurationError("Bad value for 'allowedvalues' parameter, must be a tuple.")
        self.allowedvalues = allowedvalues
        if fallback not in [True, False]:
            raise ServerConfigurationError("Bad value for 'fallback' parameter, must be True or False.")
        self.fallback = fallback

class BaseServiceHandler:

    CONF_CONTACT_PERSON_PRIMARY = [
        ['contactperson', 'ContactPerson', str],
        ['contactorganization', 'ContactOrganization', str]
    ]
    
    CONF_CONTACT_ADDRESS = [   
        ['addresstype', 'AddressType', str],
        ['address', 'Address', str],
        ['city', 'City', str],
        ['stateorprovince', 'StateOrProvince', str],
        ['postcode', 'PostCode', str],
        ['country', 'Country', str]
    ]
    
    CONF_CONTACT = [
        ['contactposition', 'ContactPosition', str],
        ['contactvoicetelephone', 'ContactVoiceTelephone', str],
        ['contactelectronicmailaddress', 'ContactElectronicMailAddress', str]
    ]

    def processParameters(self, requestname, params):
        finalparams = {}
        for paramname, paramdef in self.SERVICE_PARAMS[requestname].items():
            if paramname not in params.keys() and paramdef.mandatory:
                raise OGCException('Mandatory parameter "%s" missing from request.' % paramname)
            elif paramname in params.keys():
                try:
                    params[paramname] = paramdef.cast(params[paramname])
                except OGCException:
                    raise
                except:
                    raise OGCException('Invalid value "%s" for parameter "%s".' % (params[paramname], paramname))
                if paramdef.allowedvalues and params[paramname] not in paramdef.allowedvalues:
                    if not paramdef.fallback:
                        raise OGCException('Parameter "%s" has an illegal value.' % paramname)
                    else:
                        finalparams[paramname] = paramdef.default
                else:
                    finalparams[paramname] = params[paramname]
            elif not paramdef.mandatory and paramdef.default:
                finalparams[paramname] = paramdef.default
        return finalparams
    
    def processServiceCapabilities(self, capetree):
        if len(self.conf.items('service')) > 0:
            servicee = capetree.find('{http://www.opengis.net/wms}Service')
            for item in self.CONF_SERVICE:
                if self.conf.has_option_with_value('service', item[0]):
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
                    elif item[0] == 'keywordlist':
                        element = ElementTree.Element('%s' % item[1])
                        servicee.append(element)
                        keywords = value.split(',')
                        keywords = map(str.strip, keywords)
                        for keyword in keywords:
                            kelement = ElementTree.Element('Keyword')
                            kelement.text = keyword
                            element.append(kelement)
                    else:
                        element = ElementTree.Element('%s' % item[1])
                        element.text = value
                        servicee.append(element)
            if len(self.conf.items_with_value('contact')) > 0:
                element = ElementTree.Element('ContactInformation')
                servicee.append(element)
                for item in self.CONF_CONTACT:
                    if self.conf.has_option_with_value('contact', item[0]):
                        value = self.conf.get('contact', item[0]).strip()
                        try:
                            item[2](value)
                        except:
                            raise ServerConfigurationError('Configuration parameter [%s]->%s has an invalid value: %s.' % ('service', item[0], value))
                        celement = ElementTree.Element('%s' % item[1])
                        celement.text = value
                        element.append(celement)
                for item in self.CONF_CONTACT_PERSON_PRIMARY + self.CONF_CONTACT_ADDRESS:
                    if item in self.CONF_CONTACT_PERSON_PRIMARY:
                        tagname = 'ContactPersonPrimary'
                    else:
                        tagname = 'ContactAddress'
                    if self.conf.has_option_with_value('contact', item[0]):
                        if element.find(tagname) == None:
                            subelement = ElementTree.Element(tagname)
                            element.append(subelement)
                        value = self.conf.get('contact', item[0]).strip()
                        try:
                            item[2](value)
                        except:
                            raise ServerConfigurationError('Configuration parameter [%s]->%s has an invalid value: %s.' % ('service', item[0], value))
                        celement = ElementTree.Element('%s' % item[1])
                        celement.text = value
                        subelement.append(celement)

class Response:

    def __init__(self, content_type, content):
        self.content_type = content_type
        self.content = content

class Version:

    def __init__(self, version):
        version = version.split('.')
        if len(version) != 3:
            raise OGCException('Badly formatted version number.')
        try:
            version = map(int, version)
        except:
            raise OGCException('Badly formatted version number.')
        self.version = version

    def __repr__(self):
        return '%s.%s.%s' % (self.version[0], self.version[1], self.version[2])

    def __cmp__(self, other):
        if isinstance(other, str):
            other = Version(other)
        if self.version[0] < other.version[0]:
            return -1
        elif self.version[0] > other.version[0]:
            return 1
        else:
            if self.version[1] < other.version[1]:
                return -1
            elif self.version[1] > other.version[1]:
                return 1
            else:
                if self.version[2] < other.version[2]:
                    return -1
                elif self.version[2] > other.version[2]:
                    return 1
                else:
                    return 0

class ListFactory:

    def __init__(self, cast):
        self.cast = cast

    def __call__(self, string):
        seq = string.split(',')
        return map(self.cast, seq)

def ColorFactory(colorstring):
    if re.match('^0x[a-fA-F0-9]{6}$', colorstring):
        return Color(eval('0x' + colorstring[2:4]), eval('0x' + colorstring[4:6]), eval('0x' + colorstring[6:8]))
    else:
        raise OGCException('Invalid color value. Must be of format "0xFFFFFF".')

class CRS:

    def __init__(self, namespace, code):
        self.namespace = namespace.lower()
        self.code = int(code)
        self.proj = None

    def __repr__(self):
        return '%s:%s' % (self.namespace, self.code)

    def __eq__(self, other):
        if str(other) == str(self):
            return True
        return False

    def inverse(self, x, y):
        if not self.proj:
            self.proj = Projection('+init=%s:%s' % (self.namespace, self.code))
        return self.proj.inverse(Coord(x, y))

    def forward(self, x, y):
        if not self.proj:
            self.proj = Projection('+init=%s:%s' % (self.namespace, self.code))        
        return self.proj.forward(Coord(x, y))

class CRSFactory:

    def __init__(self, allowednamespaces):
        self.allowednamespaces = allowednamespaces

    def __call__(self, crsstring):
        if not re.match('^[A-Z]{3,5}:\d+$', crsstring):
            raise OGCException('Invalid format for the CRS parameter: %s' % crsstring, 'InvalidCRS')
        crsparts = crsstring.split(':')
        if crsparts[0] in self.allowednamespaces:
            return CRS(crsparts[0], crsparts[1])
        else:
            raise OGCException('Invalid CRS Namespace: %s' % crsparts[0], 'InvalidCRS')

class WMSBaseServiceHandler(BaseServiceHandler):

    def GetMap(self, params):
        m = self._buildMap(params)
        im = Image(params['width'], params['height'])
        render(m, im)
        return Response(params['format'], im.tostring(PIL_TYPE_MAPPING[params['format']]))

    def GetFeatureInfo(self, params, querymethodname='query_point'):
        m = self._buildMap(params)
        if params['info_format'] == 'text/plain':
            writer = TextFeatureInfo()
        elif params['info_format'] == 'text/xml':
            writer = XMLFeatureInfo()
        for layerindex, layername in enumerate(params['query_layers']):
            if layername in params['layers']:
                if m.layers[layerindex].queryable:
                    features = getattr(m, querymethodname)(layerindex, params['i'], params['j'])
                    if features:
                        writer.addlayer(m.layers[layerindex].name)
                    for feature in features:
                        writer.addfeature()
                        for prop in feature.properties:
                            writer.addattribute(prop.key(), prop.data())
                else:
                    raise OGCException('Requested query layer "%s" is not marked queryable.' % layername, 'LayerNotQueryable')
            else:
                raise OGCException('Requested query layer "%s" not in the LAYERS parameter.' % layername)
        return Response(params['info_format'], str(writer))

    def _buildMap(self, params):
        if str(params['crs']) not in self.allowedepsgcodes:
            raise OGCException('Unsupported CRS "%s" requested.' % str(params['crs']).upper(), 'InvalidCRS')
        if params['bbox'][0] >= params['bbox'][2]:
            raise OGCException("BBOX values don't make sense.  minx is greater than maxx.")
        if params['bbox'][1] >= params['bbox'][3]:
            raise OGCException("BBOX values don't make sense.  miny is greater than maxy.")
        if params.has_key('styles') and len(params['styles']) != len(params['layers']):
            raise OGCException('STYLES length does not match LAYERS length.')
        m = Map(params['width'], params['height'], '+init=%s' % params['crs'])
        if params.has_key('transparent') and params['transparent'] == 'FALSE':
            m.background = params['bgcolor']
        else:
            m.background = Color(0, 0, 0, 0)
        maplayers = self.mapfactory.layers
        mapstyles = self.mapfactory.styles
        mapaggregatestyles = self.mapfactory.aggregatestyles
        for layerindex, layername in enumerate(params['layers']):
            try:
                layer = maplayers[layername]
            except KeyError:
                raise OGCException('Layer "%s" not defined.' % layername, 'LayerNotDefined')
            reqstyle = params['styles'][layerindex]
            if reqstyle and reqstyle not in layer.wmsextrastyles:
                raise OGCException('Invalid style "%s" requested for layer "%s".' % (reqstyle, layername), 'StyleNotDefined')
            if not reqstyle:
                reqstyle = layer.wmsdefaultstyle
            if reqstyle in mapaggregatestyles.keys():
                for stylename in mapaggregatestyles[reqstyle]:
                    layer.styles.append(stylename)
            else:
                layer.styles.append(reqstyle)
            for stylename in layer.styles:
                if stylename in mapstyles.keys():
                    m.append_style(stylename, mapstyles[stylename])
                else:
                    raise ServerConfigurationError('Layer "%s" refers to non-existent style "%s".' % (layername, stylename))
            m.layers.append(layer)
        m.zoom_to_box(Envelope(params['bbox'][0], params['bbox'][1], params['bbox'][2], params['bbox'][3]))
        return m

class BaseExceptionHandler:

    def __init__(self, debug):
        self.debug = debug

    def getresponse(self, params):
        code = ''
        message = '\n'
        if not params:
            message = '''
            <h2>Welcome to the Mapnik OGCServer.</h2>
            <h3>Ready to accept map requests...</h5>
            <h4>For more info see: <a href="http://trac.mapnik.org/wiki/OgcServer">trac.mapnik.org</a></h4>
            '''
            return self.htmlhandler('', message)
        excinfo = exc_info()
        if self.debug:
            messagelist = format_exception(excinfo[0], excinfo[1], excinfo[2])
        else:
            messagelist = format_exception_only(excinfo[0], excinfo[1])
        message += ''.join(messagelist)
        if isinstance(excinfo[1], OGCException) and len(excinfo[1].args) > 1:
            code = excinfo[1].args[1]
        exceptions = params.get('exceptions', None)
        if self.debug:
            return self.htmlhandler(code, message)
        if not exceptions or not self.handlers.has_key(exceptions):
            exceptions = self.defaulthandler
        return self.handlers[exceptions](self, code, message, params)

    def htmlhandler(self,code,message):
        if code:
           resp_text = '<h2>OGCServer Error:</h2><pre>%s</pre>\n<h3>Traceback:</h3><pre>%s</pre>\n' %  (message, code)
        else:
           resp_text = message
        return Response('text/html', resp_text)

    def xmlhandler(self, code, message, params):
        ogcexcetree = deepcopy(self.xmltemplate)
        e = ogcexcetree.find(self.xpath)
        e.text = message
        if code:
            e.set('code', code)
        return Response(self.xmlmimetype, ElementTree.tostring(ogcexcetree))

    def inimagehandler(self, code, message, params):
        im = new('RGBA', (int(params['width']), int(params['height'])))
        im.putalpha(new('1', (int(params['width']), int(params['height']))))
        draw = Draw(im)
        for count, line in enumerate(message.strip().split('\n')):
            draw.text((12,15*(count+1)), line, fill='#000000')
        fh = StringIO()
        im.save(fh, PIL_TYPE_MAPPING[params['format']])
        fh.seek(0)
        return Response(params['format'], fh.read())

    def blankhandler(self, code, message, params):
        bgcolor = params.get('bgcolor', '#FFFFFF')
        bgcolor = bgcolor.replace('0x', '#')
        transparent = params.get('transparent', 'FALSE')
        if transparent == 'TRUE':
            im = new('RGBA', (int(params['width']), int(params['height'])))
            im.putalpha(new('1', (int(params['width']), int(params['height']))))
        else:
            im = new('RGBA', (int(params['width']), int(params['height'])), bgcolor)
        fh = StringIO()
        im.save(fh, PIL_TYPE_MAPPING[params['format']])
        fh.seek(0)
        return Response(params['format'], fh.read())

class Projection(MapnikProjection):
    
    def epsgstring(self):
        return self.params().split('=')[1].upper()

class TextFeatureInfo:

    def __init__(self):
        self.buffer = ''

    def addlayer(self, name):
        self.buffer += '[%s]\n' % name

    def addfeature(self):
        self.buffer += '\n'

    def addattribute(self, name, value):
        self.buffer += '%s=%s\n' % (name, str(value))

    def __str__(self):
        return self.buffer

class XMLFeatureInfo:

    basexml = """<?xml version="1.0"?>
    <resultset>
    </resultset>
    """

    def __init__(self):
        self.rootelement = ElementTree.fromstring(self.basexml)

    def addlayer(self, name):
        layer = ElementTree.Element('layer')
        layer.set('name', name)
        self.rootelement.append(layer)
        self.currentlayer = layer

    def addfeature(self):
        feature = ElementTree.Element('feature')
        self.currentlayer.append(feature)
        self.currentfeature = feature
    
    def addattribute(self, name, value):
        attribute = ElementTree.Element('attribute')
        attname = ElementTree.Element('name')
        attname.text = name
        attvalue = ElementTree.Element('value')
        attvalue.text = value.unicode()
        attribute.append(attname)
        attribute.append(attvalue)
        self.currentfeature.append(attribute)
    
    def __str__(self):
        return '<?xml version="1.0"?>\n' + ElementTree.tostring(self.rootelement)
