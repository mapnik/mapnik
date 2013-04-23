#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik
mapnik.logger.set_severity(mapnik.severity_type.None)

import sys
import os.path
from reporting import Reporting
from renderjob import RenderJob
import argparse

visual_output_dir = "/tmp/mapnik-visual-images"

defaults = {
    'sizes': [(500, 100)],
    'scales':[1.0, 2.0],
    'agg': True,
    'cairo': True,
    'grid': True,
    'agg_benchmark': False,
}

dirname = os.path.dirname(__file__)

sizes_many_in_big_range = [(800, 100), (600, 100), (400, 100),
    (300, 100), (250, 100), (150, 100), (100, 100)]

sizes_few_square = [(800, 800), (600, 600), (400, 400), (200, 200)]
sizes_many_in_small_range = [(490, 100), (495, 100), (497, 100), (498, 100),
    (499, 100), (500, 100), (501, 100), (502, 100), (505, 100), (510, 100)]

default_text_box = mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)


text_tests = [
    {'name': "list", 'sizes': sizes_many_in_big_range,'bbox':default_text_box},
    {'name': "simple", 'sizes': sizes_many_in_big_range,'bbox':default_text_box},
    {'name': "lines-1", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "lines-2", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "lines-3", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "lines-4", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "lines-5", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "lines-6", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "formatting", 'bbox':default_text_box},
    # https://github.com/mapnik/mapnik/issues/1696
    # https://github.com/mapnik/mapnik/issues/1521
    # fails with clang++ on os x
    #{'name': "lines-shield", 'sizes': sizes_few_square,'bbox':default_text_box},
    {'name': "collision", 'sizes':[(600,400)]},
    {'name': "text-halo-rasterizer", 'sizes':[(600,400)]},
    {'name': "simple-E", 'bbox':default_text_box},
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
    {'name': "shieldsymbolizer-2", 'bbox':default_text_box},
    {'name': "shieldsymbolizer-3", 'bbox':default_text_box},
    {'name': "shieldsymbolizer-4", 'bbox':default_text_box},
    {'name': "orientation", 'sizes': [(800, 200)], 'bbox':default_text_box},
    {'name': "hb-fontsets", 'sizes': [(800, 200)], 'bbox':default_text_box},
    {'name': "charspacing", 'sizes': [(200, 400)], 'bbox':default_text_box},
    {'name': "charspacing-lines", 'sizes': [(300, 300)], 'bbox':default_text_box},
    {'name': "line_break", 'sizes': [(800, 800)], 'bbox':default_text_box},
    {'name': "rtl-point", 'sizes': [(200, 200)],'bbox':default_text_box},
    {'name': "jalign-auto", 'sizes': [(200, 200)],'bbox':default_text_box},
    {'name': "line-offset", 'sizes':[(900, 250)],'bbox': mapnik.Box2d(-5.192, 50.189, -5.174, 50.195)},
    {'name': "text-bug1532", 'sizes':[(600, 165)]},
    {'name': "text-bug1533", 'sizes':[(600, 600)]},
    {'name': "text-bug1820-1", 'sizes':[(600, 300)], 'bbox':default_text_box},
    {'name': "text-bug1820+0", 'sizes':[(600, 300)], 'bbox':default_text_box},
    {'name': "text-bug1820+1", 'sizes':[(600, 300)], 'bbox':default_text_box},
    {'name': "text-expressionformat-color", 'sizes':[(800, 100)], 'bbox':default_text_box},
    # Disabled by default as the required font isn't shipped with mapnik
    #{'name': "text-malayalam", 'sizes':[(800, 100)], 'bbox':default_text_box},
]

tiff_tests = [
    {'name': "tiff-alpha-gdal", 'sizes':[(600,400)]},
    {'name': "tiff-alpha-broken-assoc-alpha-gdal", 'sizes':[(600,400)]},
    {'name': "tiff-alpha-gradient-gdal", 'sizes':[(600,400)]},
    {'name': "tiff-nodata-edge-gdal", 'sizes':[(600,400),(969,793)]},
    {'name': "tiff-opaque-edge-gdal", 'sizes':[(256,256),(969,793)]},
    {'name': "tiff-opaque-edge-gdal2", 'sizes':[(600,400),(969,793)]},
    {'name': "tiff-opaque-edge-raster2", 'sizes':[(600,400),(969,793)]},
    {'name': "tiff-resampling", 'sizes':[(600,400)]},
    # https://github.com/mapnik/mapnik/issues/1622
    {'name': "tiff-edge-alignment-gdal1", 'sizes':[(256,256),(255,257)],
	'bbox':mapnik.Box2d(-13267022.12540147,4618019.500877209,-13247454.246160466,4637587.380118214)
    },
    {'name': "tiff-edge-alignment-gdal2", 'sizes':[(256,256),(255,257)],
	'bbox':mapnik.Box2d(-13267022.12540147,4598451.621636203,-13247454.246160466,4618019.500877209)
    },
    # https://github.com/mapnik/mapnik/issues/1520
    # commented because these are not critical failures
    #{'name': "tiff-alpha-raster", 'sizes':[(600,400)]},
    #{'name': "tiff-alpha-broken-assoc-alpha-raster", 'sizes':[(600,400)]},
    #{'name': "tiff-nodata-edge-raster", 'sizes':[(600,400)]},
    #{'name': "tiff-opaque-edge-raster", 'sizes':[(256,256)]},
]

other_tests = [
    {'name': "marker-svg-opacity"},
    {'name': "marker-multi-policy", 'sizes':[(600,400)]},
    {'name': "marker-on-line", 'sizes':[(600,400)],
	'bbox': mapnik.Box2d(-10, 0, 15, 20)},
    {'name': "marker-on-line-spacing-eq-width", 'sizes':[(600,400)]},
    {'name': "marker-on-line-spacing-eq-width-overlap", 'sizes':[(600,400)]},
    {'name': "marker_line_placement_on_points"},
    {'name': "marker-with-background-image", 'sizes':[(600,400),(400,600),(257,256)]},
    {'name': "marker-with-background-image-and-hsla-transform", 'sizes':[(600,400),(400,600),(257,256)]},
    {'name': "marker-on-hex-grid", 'sizes':[(600,400),(400,600),(257,256)]},
    {'name': "whole-centroid", 'sizes':[(600,400)],
	'bbox': mapnik.Box2d(736908, 4390316, 2060771, 5942346)},
]

test_names = {
    '!text': text_tests,
    '!tiff': tiff_tests,
    '!other': other_tests,
}

files = text_tests + tiff_tests + other_tests

def find_file(filename):
    if filename.startswith('styles/'):
        filename = filename[7:]
    if filename.endswith('.xml'):
        filename = filename[:-4]
    for f in files:
        if f['name'] == filename:
            return f
    #not found => default sizes
    return {'name': filename, 'sizes': sizes_few_square}
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run mapnik\'s visual tests.')
    parser.add_argument('-q', '--quiet', action='store_true',
        help='output colored dots on stderr instead of detailed information on stdout')
    parser.add_argument('-O', '--overwrite', action='store_true',
        help='overwrite reference images')
    parser.add_argument('-g', '--generate', action='store_true',
        help='generate missing reference images')
    parser.add_argument('-r', '--repeat', action='store',
        help='repeat rendering N times. Useful for finding bugs and benchmarking',
        default=1, type=int, metavar='N')
    parser.add_argument('-b', '--benchmark', action='store_true',
        help='disable all normal renderers and enable special agg renderer writing no output file.')
    parser.add_argument('-d', '--disable-renderer', action='append',
        help='disable a renderer (options: agg, cairo, grid)',
        choices=['agg', 'cairo', 'grid'], metavar='NAME', default=[])
    parser.add_argument('-s', '--scale-factor', action='append',
        help='use fixed scale factor',
        type=float, metavar='FACTOR', default=None)
    parser.add_argument('-o', '--output-dir', action='store',
        default=visual_output_dir, help='output directory (default: %(default)s)', metavar='DIR')
    parser.add_argument('file', nargs='*', action='store',
        help='only render these files (select groups of tests with "!text", "!tiff", "!other")')
    args = parser.parse_args()
    
    reporting = Reporting(quiet=args.quiet)
    render_job = RenderJob(reporting, dirname, args.output_dir)
    render_job.repeat = args.repeat
    render_job.set_overwrite_failures(args.overwrite)
    render_job.set_generate(args.generate)
    if args.file:
        new_files = []
        for f in args.file:
            if f in test_names:
                new_files += test_names[f]
            else:
                new_files.append(find_file(f))
        files = new_files

    for renderer in args.disable_renderer:
        defaults[renderer] = False

    if args.benchmark:
        defaults['agg'] = False
        defaults['cairo'] = False
        defaults['grid'] = False
        defaults['agg_benchmark'] = True

    if args.scale_factor is not None:
        defaults['scales'] = args.scale_factor

    if 'osm' not in mapnik.DatasourceCache.plugin_names():
        print "OSM plugin required"
        sys.exit(2)


    if not os.path.exists(args.output_dir):
        os.makedirs(args.output_dir)

    for f in files:
        config = dict(defaults)
        config.update(f)
        if not render_job.load_and_save(config['name'], os.path.join(dirname, 'xml_output', "%s-out.xml" % config['name'])):
            continue
        for size in config['sizes']:
            for scale_factor in config['scales']:
                render_job.render(config,
                            size[0],
                            size[1],
                            scale_factor)

    sys.exit(reporting.summary())
