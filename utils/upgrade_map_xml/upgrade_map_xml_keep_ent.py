#!/usr/bin/env python
import os
import sys
from lxml import etree
from lxml import objectify
import re
import StringIO

# APPROACH:
# There is no way get the original DOCTYPE declaration with lxml, thus
# first I have to get it with regular expressions, after updating
# the xml it is appended under the xml declaration
# 
# A dummy_map tree is created to resolve some layers entities.
# 
# That is, the script looks into the includes folder and resolves the layer
# entities manually to append them at the end of the xml tree 
# and update them.

# NOTE: It will only resolve entities that are declared like
# <!ENTITY layer{-amenity-symbols} SYSTEM "layer-amenity-symbols.xml.inc">
# 
# If your entity name starts with `layer', the script will try to 
# find a file where to extract from the layers to be updated
dummy_map = """
<Map>
%s
</Map>
"""

def name2expr(sym):
    if 'name' not in sym.attrib: return
    name = sym.attrib['name']
    if re.match('^\[.*\]$',name) is None \
        and '[' not in name \
        and ']' not in name \
        and not name.startswith("'") \
        and not name.endswith("'") \
        and re.match("^\'.*\'",name) is None:
            print >> sys.stderr,"Fixing %s" % name
            name = '[%s]' % name
    sym.attrib.pop('name')
    sym._setText(name)
    
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
    
    # Required parameters:
    #   map_xml_file: outdated stylesheet file
    #   output_file: new stylesheet file
    #   includes folder

    if len(sys.argv) < 4:
        print >> sys.stderr,'Usage: %s <map_xml_file> <output_file> <includes_folder>' % sys.argv[0]
        sys.exit(1)
    xml = sys.argv[1]
    
    if sys.argv[3] is not None:
        includes_folder = sys.argv[3]

    # Get the good doctype with the unresolved external entities
    # since it is forever lost once the xml is parsed
    file = open(xml, 'r')
    xml_string = file.read()
    file.close()
    match = re.match(r'(?ims).*DOCTYPE.*\[.*\]\>', xml_string)
    good_doctype = ""
    if match:
        good_doctype = match.group()

    # Two trees. One with the unresolved entities...
    parser = objectify.makeparser(resolve_entities=False)    
    # tree = objectify.parse(xml, parser=parser)
    # root = tree.getroot()

    # ...and another with the entities resolved.
    # This dummy tree expands the entities that I found and puts them 
    # in a dictionary (entity) => resolved_entity
    # NOTE: `findall' makes the script very slow
    
    # First get the entities declared in the header
    temp_xml = ''.join([good_doctype, dummy_map % '<dummy_tag></dummy_tag>'])
    expanded_tree = objectify.parse(StringIO.StringIO(temp_xml))
    expanded_tree_string = etree.tostring(expanded_tree, 
                                          pretty_print=True,
                                          xml_declaration=True,
                                          encoding="utf-8")
    match = re.match(r'(?ims).*DOCTYPE.*\[.*\]\>', expanded_tree_string)
    resolved_doctype = ""
    if match:
        resolved_doctype = match.group()

    doctype_entities = {}
    # e.g.:
    # <!ENTITY layer-amenity-symbols SYSTEM "layer-amenity-symbols.xml.inc">
    for line in StringIO.StringIO(resolved_doctype).readlines():
        entity_kv = re.match(r'(?ims)(.*ENTITY.*?(?P<entity_key>\b[a-z09].*?\b) .*\"(?P<entity_value>.*)\").*', line)
        # Only consider internal entities
        if (entity_kv is not None) and not (re.search("%", line)):
            doctype_entities[''.join(['&',entity_kv.groupdict()['entity_key']])] = entity_kv.groupdict()['entity_value']

    layer_entities = []
    for entity in doctype_entities:
        if re.search('layer', entity):
            layer_entities.append(entity)


    # Remove the layer entities
    fixed_xml_string = xml_string
    for layer in layer_entities:
        pattern = '(?ims)%s;' % layer
        fixed_xml_string = re.sub(pattern, '', fixed_xml_string)
        print "removed ", layer

    # Tree to be updated to be mapnik compliant
    tree = objectify.parse(StringIO.StringIO(fixed_xml_string), parser=parser)
    root = tree.getroot()
            
    for layer in layer_entities:
        file = open("%s/%s" % (includes_folder, doctype_entities[layer]))
        layer_xml_string = file.read()
        file.close()

        print "Found this layer:", layer
        # Parse without resolving entities
        layer_xml = ''.join([good_doctype, dummy_map % layer_xml_string])
        layer_tree = objectify.parse(StringIO.StringIO(layer_xml), parser=parser)
        layer_root = layer_tree.getroot()
        layer_children = layer_root.getchildren()
        # Append this layer's styles and layers to the tree to be updated
        # print layer_children
        root.extend(layer_children)

    # Update the styles
    for style in root.Style:
        if len(style.Rule):
            # fix [name] thing 
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
                        
    updated_xml = etree.tostring(tree,
                                 pretty_print=True, 
                                 xml_declaration=True,
                                 encoding="utf-8",
                                 standalone=True)

    # Insert the original doctype declaration
    fixed_updated_xml = re.sub(r'(?ims)^.*DOCTYPE.*\[.*\]\>', good_doctype, updated_xml)
    
    output_file = open(sys.argv[2], 'w')
    output_file.write(fixed_updated_xml)
    output_file.close()    
    
    # print fixed_updated_xml
