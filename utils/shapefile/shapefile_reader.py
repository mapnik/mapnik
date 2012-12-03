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
    _,_,_,_,_,_,file_length = header[0].unpack_from(shx.read(28))
    _,_,lox,loy,hix,hiy,_,_,_,_ = header[1].unpack_from(shx.read(72))

    print "FILE_LENGTH=",file_length
    print "BBOX(",lox,loy,hix,hiy,")"
    record_header = struct.Struct(">II")
    record = struct.Struct(">II")
    while shx.tell() < file_length * 2 :
        offset,_ =  record.unpack_from(shx.read(8))
        shp.seek(offset*2, os.SEEK_SET)
        record_number,record_length = record_header.unpack_from(shp.read(8))
        print (2*offset),record_number,record_length
