#!/usr/bin/env python

import json
import sys

data = ''

with open ("%s/reference.json" % sys.argv[1], "r") as myfile:
    data=myfile.read()

ref = json.loads(data)

chars = [chr(ord('a')+n) for n in range(0,26)]

print "XML Reference"
print "============="
print

def handle_tag(obj):

    if "status" in obj:
        if obj["status"] == "deprecated":
            print ".. warning::  This attribute is deprecated and should no longer be used"
        elif obj["status"] == "experimental":
            print ".. warning:: This attribute is experimental and will likely change"
        elif obj["status"] == "unstable":
            print ".. note:: This attribute is unstable and may change"
        print



    if "doc" in obj:
        print obj["doc"]
        print
   
    if "type" in obj:
        if isinstance(obj["type"], list):
            print ":Values: %s" % " , ".join("``%s``" % x for x in obj["type"])
            print 
        else:
            if obj["type"] == "functions" and "functions" in obj:
                print ":Values: Comma separated list of functions:"
                for function in obj["functions"]:
                    print "      - ``%s(%s)``" % (function[0],",".join(chars[n] for n in range (0,function[1])))
            else:
                print ":Type: `%s`" % obj["type"]
            print 

    if "default-value" in obj:
        print ":Default Value: ``%s``" % (obj["default-value"] or "empty")
        print 
    
    if "default-meaning" in obj:
        print ":Default Meaning: %s" % obj["default-meaning"]
        print 


print "``<style>``"
print "-----------"
print
for key in sorted(ref["style"]):
    print "``%s`` attribute" % key
    print "%s"  % ("`" * (len(key)+14))
    print
    handle_tag(ref["style"][key])


print "``<layer>``"
print "-----------"
print
for key in sorted(ref["style"]):
    print "``%s`` attribute" % key
    print "%s"  % ("`" * (len(key)+14))
    print
    handle_tag(ref["style"][key])


print "Symbolizers"
print "-----------"
print
print "Common attributes"
print "`````````````````"
for key in sorted(ref["symbolizers"]["*"]):
    print "``%s`` attribute" % key
    print "%s"  % ("_" * (len(key)+14))
    print
    handle_tag(ref["symbolizers"]["*"][key])

for symbolizer in sorted(ref["symbolizers"]):
    if symbolizer == "*":
	continue
    print "``<%ssymbolizer>``" % symbolizer
    print "`" * len("``<%ssymbolizer>``" % symbolizer)
    for key in sorted(ref["symbolizers"][symbolizer]):
        print "``%s`` attribute" % key
        print "%s"  % ("_" * (len(key)+14))
        print
    	handle_tag(ref["symbolizers"][symbolizer][key])

print "Colors"
print "------"
print 
for color in sorted(ref["colors"]):
  print ":%s: rgb(%s)" % (color, ','.join("%d" % n for n in ref["colors"][color]))

print


with open ("%s/datasources.json" % sys.argv[1], "r") as myfile:
    data=myfile.read()
ref = json.loads(data)

print "Datasources"
print "-----------"
print

for datasource in sorted(ref["datasources"]):
    print "``<%sdatasource>``" % datasource
    print "`" * len("``<%sdatasource>``" % datasource)
    for key in sorted(ref["datasources"][datasource]):
        print "``%s`` attribute" % key
        print "%s"  % ("_" * (len(key)+14))
        print
    	handle_tag(ref["datasources"][datasource][key])
