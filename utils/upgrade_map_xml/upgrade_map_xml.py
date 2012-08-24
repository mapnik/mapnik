#!/usr/bin/env python

import os
import re
import sys
import optparse
import tempfile

__version__ = '0.1.0'

HAS_LXML = False

try:
    import lxml.etree as etree
    HAS_LXML = True
except ImportError:
    try:
        import elementtree.ElementTree as etree
    except ImportError:
        import xml.etree.ElementTree as etree

def color_text(color, text):
    if os.name == 'nt':
        return text
    return "\033[9%sm%s\033[0m" % (color,text)

def indent(elem, level=0):
    """ http://infix.se/2007/02/06/gentlemen-indent-your-xml
    """
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        for e in elem:
            indent(e, level+1)
            if not e.tail or not e.tail.strip():
                e.tail = i + "  "
        if not e.tail or not e.tail.strip():
            e.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i
            
def name2expr(sym):
    if 'name' not in sym.attrib: return
    name = sym.attrib['name']
    if re.match('^\[.*\]',name) is None \
        and '[' not in name \
        and ']' not in name \
        and not name.startswith("'") \
        and not name.endswith("'") \
        and re.match("^\'.*\'",name) is None:
            print>>sys.stderr,"Fixing %s" % name
            name = '[%s]' % name
    sym.attrib.pop('name')
    sym.text = name

def handle_attr_changes(sym):
    # http://www.w3schools.com/css/pr_text_text-transform.asp
    # http://code.google.com/p/mapnik-utils/issues/detail?id=32&colspec=ID%20Type%20Status%20Priority%20Component%20Owner%20Summary
    text_convert = sym.attrib.get('text_convert',sym.attrib.get('text_transform',sym.attrib.get('text_transform')))
    if text_convert:
        # note: css supports text-transform:capitalize but Mapnik does not (yet)
        t_ = {'tolower':'lowercase','toupper':'uppercase','none':'none'}
        new = t_.get(text_convert)
        if new:
            sym.attrib['text-transform'] = new
        else:
            sym.attrib['text-transform'] = text_convert
        if sym.attrib.get('text_convert'):
            sym.attrib.pop('text_convert')
        if sym.attrib.get('text_transform'):
            sym.attrib.pop('text_transform')
    
    # https://github.com/mapnik/mapnik/issues/807
    justify_alignment = sym.attrib.get('justify_alignment',sym.attrib.get('justify-alignment'))
    if justify_alignment and justify_alignment == "middle":
        sym.attrib['justify-alignment'] = 'center'    
    
    minimum_distance = sym.attrib.get('min_distance')
    if minimum_distance:
        sym.attrib['minimum-distance'] = minimum_distance
        sym.attrib.pop('min_distance')

    minimum_padding = sym.attrib.get('min_padding')
    if minimum_padding:
        sym.attrib['minimum-padding'] = minimum_padding
        sym.attrib.pop('min_padding')

def fixup_sym_with_image(sym):
    if sym.attrib.get('width'):
        sym.attrib.pop('width')
    if sym.attrib.get('height'):
        sym.attrib.pop('height')
    if sym.attrib.get('type'):
        sym.attrib.pop('type')

def fixup_sym_attributes(sym):
    if len(sym.findall('CssParameter')):
        # copy, so we don't loose after clear()
        attrib = dict(sym.attrib)
        for css in sym.findall('CssParameter'):
            key = css.attrib.get('name')
            value = css.text
            attrib[key] = value
        sym.clear() # remove CssParameter elements
        for k,v in attrib.items(): # insert attributes instead
            sym.attrib[k] = v

def underscore2dash(elem):
    for i in elem.attrib.items():
        if '_' in i[0]:
           new = i[0].replace('_','-')
           old = i[0]
           elem.attrib[new] = i[1]
           elem.attrib.pop(old)
           print>>sys.stderr,"Changing %s to %s" % (old,new)


def upgrade(input_xml,output_xml=None,indent_xml=True):

    if not os.path.exists(input_xml):
        sys.stderr.write('input xml "%s" does not exist' % input_xml)
        sys.exit(1)
    
    pre_read = open(input_xml,'r')
    if '!ENTITY' in pre_read.read() and not HAS_LXML:
        sys.stderr.write('\nSorry, it appears the xml you are trying to upgrade has entities, which requires lxml (python bindings to libxml2)\n')
        sys.stderr.write('Install lxml with: "easy_install lxml" or download from http://codespeak.net/lxml/\n')

        sys.exit(1)        

    try:
        tree = etree.parse(input_xml)
    except:
        print 'Could not parse "%s" invalid XML' % input_xml
        return
    
    if hasattr(tree,'xinclude'):
        tree.xinclude()
    root = tree.getroot()

    # rename 'bgcolor' to 'background-color'
    if root.attrib.get('bgcolor'):
        root.attrib['background-color'] = root.attrib.get('bgcolor')
        root.attrib.pop('bgcolor')
    
    # underscores to spaces for <Map ..>
    underscore2dash(root)
    
    root.set('minimum-version', '0.7.2')
    
    # underscores to spaces for <FontSet ..>
    fontset = root.findall('FontSet') or root.findall('*/FontSet')
    for f in fontset:
        font = f.findall('Font')
        for f_ in font:
            underscore2dash(f_)

    # underscores to spaces for <Layer ..>
    layers = root.findall('Layer') or root.findall('*/Layer')
    for l in layers:
        underscore2dash(l)
    
    
    styles = root.findall('Style') or root.findall('*/Style')
    if not len(styles):
        sys.stderr.write('### Warning, no styles encountered and nothing able to be upgraded!\n')
    else:
        for style in styles:
            for rule in style.findall('Rule'):
                for sym in rule.findall('TextSymbolizer') or []:
                    name2expr(sym)
                    handle_attr_changes(sym)
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                for sym in rule.findall('ShieldSymbolizer') or []:
                    name2expr(sym) 
                    handle_attr_changes(sym)
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                    fixup_sym_with_image(sym)
                for sym in rule.findall('PointSymbolizer') or []:
                    fixup_sym_with_image(sym)
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                for sym in rule.findall('PolygonPatternSymbolizer') or []:
                    fixup_sym_with_image(sym)
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                for sym in rule.findall('LinePatternSymbolizer') or []:
                    fixup_sym_with_image(sym)
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                for sym in rule.findall('LineSymbolizer') or []:
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                for sym in rule.findall('PolygonSymbolizer') or []:
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                for sym in rule.findall('RasterSymbolizer') or []:
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                for sym in rule.findall('BuildingSymbolizer') or []:
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)
                for sym in rule.findall('MarkersSymbolizer') or []:
                    fixup_sym_attributes(sym)
                    underscore2dash(sym)

    if indent_xml:
        indent(root)
    
    if output_xml:
        tree.write(output_xml)
    else:
        tree.write(input_xml)

parser = optparse.OptionParser(usage="""%prog <input xml> [options]

Upgrade a Mapnik XML stylesheet to Mapnik 2.0

Full help:
 $ %prog -h (or --help for possible options)

Read 'map.xml' and write new 'map2.xml'
 $ %prog map.xml map2.xml

Update 'map.xml' in place (*Careful*)
 $ %prog map.xml --in-place

""", version='%prog ' + __version__)

parser.add_option('--indent',
                  dest='indent_xml',
                  action='store_true',
                  default=False,
                  help='Indent the resulting XML')

parser.add_option('--in-place',
                  dest='update_in_place',
                  action='store_true',
                  default=False,
                  help='Update and overwrite the map in place')

if __name__ == "__main__":

    (options, args) = parser.parse_args()
    if not len(args) > 0:
        parser.error("Please provide the path to a map.xml and a new xml to write")
    
    input_xml = args[0]
    output_xml = None

    if len(args) < 3 and not options.update_in_place:
        if len(args) == 2:
            output_xml = args[1]
            
        if (len(args) == 1) or (input_xml == output_xml):
            parser.error(color_text(1,'\n\nAre you sure you want to overwrite "%s"?\nIf so, then pass --in-place to confirm.\nOtherwise pass a different filename to write an upgraded copy to.\n' % input_xml))

        print 'Upgrading "%s" to "%s"...' % (input_xml,output_xml)
        upgrade(input_xml,output_xml=output_xml,indent_xml=options.indent_xml)
    
    elif len(args) == 1:
        print 'Upgrading "%s"...' % (input_xml)
        upgrade(input_xml,output_xml=output_xml,indent_xml=options.indent_xml)

    elif len(args) > 1: # assume a list of inputs
        found = []
        for input_xml in args:
            if os.path.exists(input_xml) and input_xml not in found:
                print 'Upgrading "%s" in place...' % input_xml
                found.append(input_xml)
                upgrade(input_xml,output_xml=None,indent_xml=options.indent_xml)
