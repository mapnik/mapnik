#!/usr/bin/env python
# Utility to interrogate ESRI shape files

import os
import sys
import struct

ShapeType = { 0 : "NullShape",
              1 : "Point",
              3 : "PolyLine",
              5 : "Polygon",
              8 : "MultiPoint",
              11: "PointZ",
              13: "PolyLineZ",
              15: "PolygonZ",
              18: "MultiPointZ",
              21: "PointM",
              23: "PolyLineM",
              25: "PolygonM",
              28: "MultiPointM",
              31: "MultiPatch"}

def test_record(_type, record) :
    if _type == 0:
        print "NULL shape"
    elif _type == 11: #PointZ
        test_pointz(record)
    elif _type == 5:
        test_polygon(record)

def test_pointz(record):
    if len(record) != 36 :
        print>>sys.stderr,"BAD SHAPE FILE: expected 36 bytes got",len(record)
        sys.exit(1)
    _type,x,y,z,m = struct.unpack("<idddd",record)
    if _type != 11:
        print>>sys.stderr,"BAD SHAPE FILE: expected PointZ or NullShape got",_type
        sys.exit(1)

def test_polygon(record):
    _type, x0, y0, x1, y0, num_parts, num_points = struct.unpack("<iddddii", record[0:44])
    if _type != 5:
        print>>sys.stderr, "BAD SHAPE FILE: expected Polygon or NullShape got", _type
        sys.exit(1)
    length = len(record)
    rec_length = 44 + num_parts * 4 + num_points * 16
    if rec_length <> length:
        print>>sys.stderr, "BAD SHAPE FILE: expected", rec_length, "got", length
        sys.exit(1)

if __name__ == "__main__" :

    if len(sys.argv) !=2:
        print>>sys.stderr, "Usage:",sys.argv[0],"<shapefile>"
        sys.exit(1)

    shx_filename = sys.argv[1][:-3]+"shx"
    shp_filename = sys.argv[1][:-3]+"shp"

    shx = open(shx_filename)
    shp = open(shp_filename)

    header = (struct.Struct(">IIIIIII"),struct.Struct("<IIdddddddd"))
    # SHX header
    _,_,_,_,_,_,shx_file_length = header[0].unpack_from(shx.read(28))
    _,_,lox,loy,hix,hiy,_,_,_,_ = header[1].unpack_from(shx.read(72))

    shx_bbox = [lox,loy,hix,hiy]

    # SHP header
    _,_,_,_,_,_,shp_file_length = header[0].unpack_from(shp.read(28))
    version,_type,lox,loy,hix,hiy,_,_,_,_ = header[1].unpack_from(shp.read(72))

    shp_bbox = [lox,loy,hix,hiy]
    if shx_bbox <> shp_bbox :
        print "BAD SHAPE FILE: bounding box mismatch in *.shp and *.shx", shp_bbox, shx_bbox
        sys.exit(1)

    print "SHX FILE_LENGTH=",shx_file_length,"bytes"
    print "SHP FILE_LENGTH=",shp_file_length,"bytes"

    print "TYPE", ShapeType[_type]
    print "BBOX(",lox,loy,hix,hiy,")"
    record_header = struct.Struct(">II")
    record = struct.Struct(">II")
    calc_total_size = 50
    count = 0
    while shx.tell() < shx_file_length * 2 :
        offset,shx_content_length = record.unpack_from(shx.read(8))
        shp.seek(offset*2, os.SEEK_SET)
        record_number,content_length = record_header.unpack_from(shp.read(8))
        if shx_content_length <> content_length:
            print "BAD SHAPE FILE: content_lenght mismatch in SHP and SHX",shx_content_length,content_length
            sys.exit(1)
        ##
        test_record(_type, shp.read(2*content_length))
        calc_total_size +=(4 + content_length)
        count+=1

    print "SHAPES COUNT=",count
    delta = shp_file_length-calc_total_size
    if  delta  > 0 :
        print "BAD SHAPE FILE: extra ", 2*delta,"bytes"
    elif delta < 0:
        print "BAD SHAPE FILE: missing ", 2*delta,"bytes"
    else:
        print "SHAPE FILE LOOKS GOOD!"
