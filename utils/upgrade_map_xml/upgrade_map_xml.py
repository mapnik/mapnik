#!/usr/bin/env python
import os
import sys
import tempfile
from lxml import etree
from lxml import objectify
import re

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
    
def fixup_pointsym(sym):
    if sym.attrib.get('width'):
        sym.attrib.pop('width')
    if sym.attrib.get('height'):
        sym.attrib.pop('height')
    if sym.attrib.get('type'):
        sym.attrib.pop('type')

def fixup_sym_attributes(sym):
    if not hasattr(sym,'CssParameter'):
        return
    attrib = {}
    metawriter = sym.attrib.get('meta-writer')
    if metawriter:
        attrib['meta-writer'] = metawriter
    metaoutput = sym.attrib.get('meta-output')
    if metaoutput:
        attrib['meta-output'] = metaoutput
    
    for css in sym.CssParameter:
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
    tree = objectify.parse(xml)
    tree.xinclude()
    root = tree.getroot()
    
    # rename 'bgcolor' to 'background-color'
    if root.attrib.get('bgcolor'):
        root.attrib['background-color'] = root.attrib.get('bgcolor')
        root.attrib.pop('bgcolor')
    
    if hasattr(root,'Style'):
        for style in root.Style:
            if len(style.Rule):
                for rule in style.Rule:
                    if hasattr(rule,'TextSymbolizer'):
                        for sym in rule.TextSymbolizer:
                            name2expr(sym)
                    if hasattr(rule,'ShieldSymbolizer'):
                        for sym in rule.ShieldSymbolizer:
                            name2expr(sym) 
                    if hasattr(rule,'PointSymbolizer'):
                        for sym in rule.PointSymbolizer:
                            fixup_pointsym(sym)
                    if hasattr(rule,'LineSymbolizer') :
                        for sym in rule.LineSymbolizer:
                            fixup_sym_attributes(sym)
                    if hasattr(rule,'PolygonSymbolizer') :
                        for sym in rule.PolygonSymbolizer:
                            fixup_sym_attributes(sym)
                    if hasattr(rule,'RasterSymbolizer') :
                        for sym in rule.RasterSymbolizer:
                            fixup_sym_attributes(sym)
                    if hasattr(rule,'BuildingSymbolizer') :
                        for sym in rule.BuildingSymbolizer:
                            fixup_sym_attributes(sym)
    else:
        sys.stderr.write('### Warning, no styles encountered and nothing able to be upgraded!\n')

    updated_xml = etree.tostring(tree)
    (handle, path) = tempfile.mkstemp(suffix='.xml', prefix='mapnik-')
    os.close(handle)
    open(path,'w').write(updated_xml)
    indented = etree.parse(path)
    indent(indented.getroot())
    output_file = open(sys.argv[2], 'w')
    output_file.write(etree.tostring(indented))
    output_file.close()