#!/usr/bin/env python
import os
import sys
from lxml import etree
from lxml import objectify

def name2expr(sym):
    name = sym.attrib['name']
    expression = '[%s]' % name
    sym.attrib['name'] = expression    

def fixup_pointsym(sym):
    if sym.attrib.get('width'):
        sym.attrib.pop('width')
    if sym.attrib.get('height'):
        sym.attrib.pop('height')
    #if sym.attrib.get('type'):
    #    sym.attrib.pop('type')

def fixup_sym_attributes(sym):
    attrib = {}
    for css in sym.CssParameter:
        key = css.attrib.get('name')
        value = css.text
        attrib[key]=value
    sym.clear() # remove CssParameter children
    for k,v in attrib.items(): # insert attributes
        sym.attrib[k] = v
    

if __name__ == "__main__":

    if len(sys.argv) != 2:
        print>>sys.stderr,'Usage: %s <map_xml_file>' % sys.argv[0]
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
                if hasattr(rule,'LineSymbolizer'):
                    for sym in rule.LineSymbolizer:
                        fixup_sym_attributes(sym)
                        
                            
                
    print etree.tostring(tree,pretty_print=True,standalone=True)
    
