#!/usr/bin/env python
# Utility to interrogate ESRI shape files

import os
import sys
import struct

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

    # SHP header
    _,_,_,_,_,_,shp_file_length = header[0].unpack_from(shp.read(28))
    _,_,lox,loy,hix,hiy,_,_,_,_ = header[1].unpack_from(shp.read(72))

    print "SHX FILE_LENGTH=",shx_file_length,"bytes"
    print "SHP FILE_LENGTH=",shp_file_length,"bytes"

    print "BBOX(",lox,loy,hix,hiy,")"
    record_header = struct.Struct(">II")
    record = struct.Struct(">II")
    calc_total_size = 50
    count = 0
    while shx.tell() < shx_file_length * 2 :
        offset,shx_content_length =  record.unpack_from(shx.read(8))
        shp.seek(offset*2, os.SEEK_SET)
        record_number,content_length = record_header.unpack_from(shp.read(8))
        #print (2*offset),record_number,content_length
        if shx_content_length <> content_length:
            print "BAD SHAPE FILE: content_lenght mismatch in SHP and SHX",shx_content_length,content_length
            sys.exit(1)
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
