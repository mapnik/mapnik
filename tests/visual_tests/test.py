#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik
mapnik.logger.set_severity(mapnik.severity_type.None)

import sys
import os.path
from compare import compare, summary, fail

visual_output_dir = "/tmp/mapnik-visual-images"

defaults = {
    'sizes': [(500, 100)]
}

sizes_many_in_big_range = [(800, 100), (600, 100), (400, 100),
    (300, 100), (250, 100), (150, 100), (100, 100)]

sizes_few_square = [(800, 800), (600, 600), (400, 400), (200, 200)]
sizes_many_in_small_range = [(490, 100), (495, 100), (497, 100), (498, 100),
    (499, 100), (500, 100), (501, 100), (502, 100), (505, 100), (510, 100)]

default_text_box = mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)

dirname = os.path.dirname(__file__)

files = [
    {'name': "list", 'sizes': sizes_many_in_big_range,'bbox':default_text_box},
    {'name': "simple", 'sizes': sizes_many_in_big_range,'bbox':default_text_box},
    {'name': "lines-1", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "lines-2", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "lines-3", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "lines-shield", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "simple-E", 'bbox':mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)},
    {'name': "simple-NE",'bbox':default_text_box},
    {'name': "simple-NW",'bbox':default_text_box},
    {'name': "simple-N",'bbox':default_text_box},
    {'name': "simple-SE",'bbox':default_text_box},
    {'name': "simple-SW",'bbox':default_text_box},
    {'name': "simple-S",'bbox':default_text_box},
    {'name': "simple-W",'bbox':default_text_box},
    {'name': "formatting-1",'bbox':default_text_box},
    {'name': "formatting-2",'bbox':default_text_box},
    {'name': "formatting-3",'bbox':default_text_box},
    {'name': "formatting-4",'bbox':default_text_box},
    {'name': "expressionformat",'bbox':default_text_box},
    {'name': "shieldsymbolizer-1", 'sizes': sizes_many_in_small_range,'bbox':default_text_box},
    {'name': "rtl-point", 'sizes': [(200, 200)],'bbox':default_text_box},
    {'name': "jalign-auto", 'sizes': [(200, 200)],'bbox':default_text_box},
    {'name': "line-offset", 'sizes':[(900, 250)],'bbox': mapnik.Box2d(-5.192, 50.189, -5.174, 50.195)},
    {'name': "tiff-alpha-gdal", 'sizes':[(600,400)]},
    {'name': "tiff-alpha-raster", 'sizes':[(600,400)]},
    {'name': "tiff-alpha-broken-assoc-alpha-gdal", 'sizes':[(600,400)]},
    {'name': "tiff-alpha-broken-assoc-alpha-raster", 'sizes':[(600,400)]},
    {'name': "tiff-alpha-gradient-gdal", 'sizes':[(600,400)]},
    {'name': "tiff-nodata-edge-gdal", 'sizes':[(600,400)]},
    {'name': "tiff-nodata-edge-raster", 'sizes':[(600,400)]},
    {'name': "tiff-opaque-edge-gdal", 'sizes':[(256,256)]},
    {'name': "tiff-opaque-edge-raster", 'sizes':[(256,256)]},
    {'name': "tiff-opaque-edge-gdal2", 'sizes':[(600,400)]},
    {'name': "tiff-opaque-edge-raster2", 'sizes':[(600,400)]},
    ]

def render(filename, width, height, bbox, quiet=False):
    m = mapnik.Map(width, height)
    expected = os.path.join(dirname, "images", '%s-%d-reference.png' % (filename, width))
    actual = '%s-%d' % (filename, width)
    try:
        mapnik.load_map(m, os.path.join(dirname, "styles", "%s.xml" % filename), False)
        if bbox is not None:
            m.zoom_to_box(bbox)
        else:
            m.zoom_all()
    except Exception, e:
        sys.stderr.write(e.message + '\n')
        fail(actual,expected,str(e.message))
        return
    actual_agg = os.path.join(visual_output_dir, '%s-agg.png' % actual)
    if not quiet:
        print "\"%s\" with size %dx%d with agg..." % (filename, width, height),
    try:
        mapnik.render_to_file(m, actual_agg)
        diff = compare(actual_agg, expected)
        if not quiet:
            if diff > 0:
                print '\x1b[31m✘\x1b[0m (\x1b[34m%u different pixels\x1b[0m)' % diff
            else:
                print '\x1b[32m✓\x1b[0m'
    except Exception, e:
        sys.stderr.write(e.message + '\n')
        fail(actual_agg,expected,str(e.message))
    if 'tiff' in actual:
        actual_cairo = os.path.join(visual_output_dir, '%s-cairo.png' % actual)
        if not quiet:
            print "\"%s\" with size %dx%d with cairo..." % (filename, width, height),
        try:
            mapnik.render_to_file(m, actual_cairo,'ARGB32')
            diff = compare(actual_cairo, expected)
            if not quiet:
                if diff > 0:
                    print '\x1b[31m✘\x1b[0m (\x1b[34m%u different pixels\x1b[0m)' % diff
                else:
                    print '\x1b[32m✓\x1b[0m'
        except Exception, e:
            sys.stderr.write(e.message + '\n')
            fail(actual_cairo,expected,str(e.message))
    return m

if __name__ == "__main__":
    if '-q' in sys.argv:
       quiet = True
       sys.argv.remove('-q')
    else:
       quiet = False

    if len(sys.argv) == 2:
        files = [{"name": sys.argv[1], "sizes": sizes_few_square}]
    elif len(sys.argv) > 2:
        files = []
        for name in sys.argv[1:]:
            files.append({"name": name})

    if not os.path.exists(visual_output_dir):
        os.makedirs(visual_output_dir)

    if 'osm' in mapnik.DatasourceCache.plugin_names():
        for f in files:
            config = dict(defaults)
            config.update(f)
            for size in config['sizes']:
                m = render(config['name'], size[0], size[1], config.get('bbox'), quiet=quiet)
            mapnik.save_map(m, os.path.join(dirname, 'xml_output', "%s-out.xml" % config['name']))

        summary(generate=False)
