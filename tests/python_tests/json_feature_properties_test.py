#encoding: utf8

from nose.tools import *
import os,sys
import mapnik
from utilities import execution_path, run_all
try:
    import json
except ImportError:
    import simplejson as json

chars = [
 {
   "name":"single_quote",
   "test": "string with ' quote",
   "json": '{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \' quote"}}'
 },
 {
   "name":"escaped_single_quote",
   "test":"string with \' quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \' quote"}}'
 },
 {
   "name":"double_quote",
   "test":'string with " quote',
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\" quote"}}'
 },
 {
   "name":"double_quote2",
   "test":"string with \" quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\" quote"}}'
 },
 {
   "name":"reverse_solidus", # backslash
   "test":"string with \\ quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\\ quote"}}'
 },
 {
   "name":"solidus", # forward slash
   "test":"string with / quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with / quote"}}'
 },
 {
   "name":"backspace",
   "test":"string with \b quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\b quote"}}'
 },
 {
   "name":"formfeed",
   "test":"string with \f quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\f quote"}}'
 },
 {
   "name":"newline",
   "test":"string with \n quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\n quote"}}'
 },
 {
   "name":"carriage_return",
   "test":"string with \r quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\r quote"}}'
 },
 {
   "name":"horiztonal_tab",
   "test":"string with \t quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\t quote"}}'
 },
 # remainder are c++ reserved, but not json
 {
   "name":"vert_tab",
   "test":"string with \v quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \\u000b quote"}}'
 },
 {
   "name":"alert",
   "test":"string with \a quote",
   "json":'{"type":"Feature","id":1,"geometry":null,"properties":{"name":"string with \u0007 quote"}}'
 }
]

ctx = mapnik.Context()
ctx.push('name')

def test_char_escaping():
    for char in chars:
        feat = mapnik.Feature(ctx,1)
        expected = char['test']
        feat["name"] = expected
        eq_(feat["name"],expected)
        # confirm the python json module
        # is working as we would expect
        pyjson2 = json.loads(char['json'])
        eq_(pyjson2['properties']['name'],expected)
        # confirm our behavior is the same as python json module
        # for the original string
        geojson_feat_string = feat.to_geojson()
        eq_(geojson_feat_string,char['json'],"Mapnik's json escaping is not to spec: actual(%s) and expected(%s)" % (geojson_feat_string,char['json']))
        # and the round tripped string
        pyjson = json.loads(geojson_feat_string)
        eq_(pyjson['properties']['name'],expected)

if __name__ == "__main__":
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
