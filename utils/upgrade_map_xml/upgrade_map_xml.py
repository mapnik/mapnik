#!/usr/bin/env python
import os
import re
import sys
import tempfile

HAS_LXML = False

try:
    import lxml.etree as etree
    HAS_LXML = True
except ImportError:
    try:
        import elementtree.ElementTree as etree
    except ImportError:
        import xml.etree.ElementTree as etree

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
    name = sym.attrib['name']
    if re.match('^\[.*\]$',name) is None:
        print>>sys.stderr,"Fixing %s" % name
        expression = '[%s]' % name
        sym.attrib['name'] = expression    

def handle_attr_changes(sym):
    # http://www.w3schools.com/css/pr_text_text-transform.asp
    # http://code.google.com/p/mapnik-utils/issues/detail?id=32&colspec=ID%20Type%20Status%20Priority%20Component%20Owner%20Summary
    text_transform = sym.attrib.get('text_convert')
    # note: css supports text-transform:capitalize but Mapnik does not (yet)
    t_ = {'tolower':'uppercase','toupper':'lowercase','none':'none'}
    if text_transform:
        new = t_.get(text_transform)
        if new:
            sym.attrib['text_transform'] = new

def fixup_pointsym(sym):
    if sym.attrib.get('width'):
        sym.attrib.pop('width')
    if sym.attrib.get('height'):
        sym.attrib.pop('height')
    if sym.attrib.get('type'):
        sym.attrib.pop('type')

def fixup_sym_attributes(sym):
    #if not sym.find('CssParameter'):
    #    return
    attrib = {}
    metawriter = sym.attrib.get('meta-writer')
    if metawriter:
        attrib['meta-writer'] = metawriter
    metaoutput = sym.attrib.get('meta-output')
    if metaoutput:
        attrib['meta-output'] = metaoutput
    
    for css in sym.findall('CssParameter'):
        key = css.attrib.get('name')
        value = css.text
        attrib[key] = value
    sym.clear() # remove CssParameter elements
    for k,v in attrib.items(): # insert attributes instead
        sym.attrib[k] = v
    
        
if __name__ == "__main__":
    
    #required parameters:
    #   map_xml_file: outdated stylesheet file
    #   output_file: new stylesheet file

    if len(sys.argv) != 3:
        sys.stderr.write('Usage: %s <map_xml_file> <output_file>\n' % os.path.basename(sys.argv[0]))
        sys.exit(1)
        
    xml = sys.argv[1]
    pre_read = open(xml,'r')
    if '!ENTITY' in pre_read.read() and not HAS_LXML:
        sys.stderr.write('\nSorry, it appears the xml you are trying to upgrade has entities, which requires lxml (python bindings to libxml2)\n')
        sys.stderr.write('Install lxml with: "easy_install lxml" or download from http://codespeak.net/lxml/\n')

        sys.exit(1)        
        
    tree = etree.parse(xml)
    if hasattr(tree,'xinclude'):
        tree.xinclude()
    root = tree.getroot()
    
    # rename 'bgcolor' to 'background-color'
    if root.attrib.get('bgcolor'):
        root.attrib['background-color'] = root.attrib.get('bgcolor')
        root.attrib.pop('bgcolor')
    
    styles = root.findall('Style')
    if not len(styles):
        sys.stderr.write('### Warning, no styles encountered and nothing able to be upgraded!\n')
    else:
        for style in styles:
            for rule in style.findall('Rule'):
                for sym in rule.findall('TextSymbolizer') or []:
                    name2expr(sym)
                    #handle_attr_changes(sym)
                for sym in rule.findall('ShieldSymbolizer') or []:
                    name2expr(sym) 
                for sym in rule.findall('PointSymbolizer') or []:
                    fixup_pointsym(sym)
                for sym in rule.findall('LineSymbolizer') or []:
                    fixup_sym_attributes(sym)
                for sym in rule.findall('PolygonSymbolizer') or []:
                    fixup_sym_attributes(sym)
                for sym in rule.findall('RasterSymbolizer') or []:
                    fixup_sym_attributes(sym)
                for sym in rule.findall('BuildingSymbolizer') or []:
                    fixup_sym_attributes(sym)

    # TODO - make forcing indent an option
    indent(root)
    tree.write(sys.argv[2])