#!/usr/bin/env python
import os
import sys
from lxml import etree
from lxml import objectify
import re

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
        print>>sys.stderr,'Usage: %s <map_xml_file> <output_file>' % sys.argv[0]
        sys.exit(1)
        
    xml = sys.argv[1]
    tree = objectify.parse(xml)
    root = tree.getroot()
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
                        
    updated_xml = etree.tostring(tree,pretty_print=True,standalone=True)
    
    output_file = open(sys.argv[2], 'w')

    output_file.write(updated_xml)

    output_file.close()
