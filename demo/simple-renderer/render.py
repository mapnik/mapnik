#!/usr/bin/env python
from __future__ import print_function
import sys
import mapnik

def render(input_file, output_file, width=800, height=800, bbox=None):
    m = mapnik.Map(width, height)
    mapnik.load_map(m, input_file, False)
    if bbox is not None:
        m.zoom_to_box(bbox)
    else:
        m.zoom_all()
    mapnik.render_to_file(m, output_file)

if len(sys.argv) == 2:
    render(sys.argv[1], "output.png")
elif len(sys.argv) == 3:
    render(sys.argv[1], sys.argv[2])
elif len(sys.argv) == 5:
    render(sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4]))
else:
    print ("usage: %s style_file [output_file] [width height]" % sys.argv[0])
    sys.exit(1)
