#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik
import sys
import os.path
from compare import compare, summary

defaults = {
    'sizes': [(500, 100)],
    'bbox': mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)
}

sizes_many_in_big_range = [(800, 100), (600, 100), (400, 100),
    (300, 100), (250, 100), (150, 100), (100, 100)]

sizes_few_square = [(800, 800), (600, 600), (400, 400), (200, 200)]
sizes_many_in_small_range = [(490, 100), (495, 100), (497, 100), (498, 100),
    (499, 100), (500, 100), (501, 100), (502, 100), (505, 100), (510, 100)]

dirname = os.path.dirname(__file__)

files = [
    {'name': "list", 'sizes': sizes_many_in_big_range},
    {'name': "simple", 'sizes': sizes_many_in_big_range},
    {'name': "lines-1", 'sizes': sizes_few_square},
    {'name': "lines-2", 'sizes': sizes_few_square},
    {'name': "lines-3", 'sizes': sizes_few_square},
    {'name': "lines-shield", 'sizes': sizes_few_square},
    {'name': "simple-E"},
    {'name': "simple-NE"},
    {'name': "simple-NW"},
    {'name': "simple-N"},
    {'name': "simple-SE"},
    {'name': "simple-SW"},
    {'name': "simple-S"},
    {'name': "simple-W"},
    {'name': "formatting-1"},
    {'name': "formatting-2"},
    {'name': "formatting-3"},
    {'name': "formatting-4"},
    {'name': "expressionformat"},
    {'name': "shieldsymbolizer-1", 'sizes': sizes_many_in_small_range},
    {'name': "rtl-point", 'sizes': [(200, 200)]},
    {'name': "jalign-auto", 'sizes': [(200, 200)]},
    {'name': "line-offset", 'sizes':[(900, 250)],
        'bbox': mapnik.Box2d(-5.192, 50.189, -5.174, 50.195)}
    ]

def render(filename, width, height, bbox, quiet=False):
    if not quiet:
        print "Rendering style \"%s\" with size %dx%d ... \x1b[1;32m✓ \x1b[0m" % (filename, width, height)
        print "-"*80
    m = mapnik.Map(width, height)
    mapnik.load_map(m, os.path.join(dirname, "styles", "%s.xml" % filename), False)
    if bbox is not None:
        m.zoom_to_box(bbox)
    else:
        m.zoom_all()
    expected = os.path.join(dirname, "images", '%s-%d-reference.png' % (filename, width))
    if not os.path.exists('/tmp/mapnik-visual-images'):
        os.makedirs('/tmp/mapnik-visual-images')
    actual = os.path.join("/tmp/mapnik-visual-images", '%s-%d-agg.png' % (filename, width))
    mapnik.render_to_file(m, actual)
    diff = compare(actual, expected)
    if diff > 0:
        print "-"*80
        print '\x1b[33mError:\x1b[0m %u different pixels' % diff
        print "-"*80

    return m

if __name__ == "__main__":
    if '-q' in sys.argv:
       quiet = True
       sys.argv.remove('-q')
    else:
       quiet = False

    if len(sys.argv) == 2:
        files = [(sys.argv[1], (500, 500))]
    elif len(sys.argv) > 2:
        files = [sys.argv[1:]]

    for f in files:
        config = dict(defaults)
        config.update(f)
        for size in config['sizes']:
            m = render(config['name'], size[0], size[1], config['bbox'], quiet=quiet)
        mapnik.save_map(m, os.path.join(dirname, 'xml_output', "%s-out.xml" % config['name']))

    summary()
