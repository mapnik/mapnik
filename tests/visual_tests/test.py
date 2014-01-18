#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik
mapnik.logger.set_severity(mapnik.severity_type.None)

import shutil
import sys
import os.path
from compare import compare, compare_grids

try:
    import json
except ImportError:
    import simplejson as json

visual_output_dir = "/tmp/mapnik-visual-images"

defaults = {
    'sizes': [(500, 100)],
    'scales':[1.0,2.0],
    'agg': True,
    'cairo': mapnik.has_cairo(),
    'grid': mapnik.has_grid_renderer()
}

cairo_threshold = 10
if 'Linux' == os.uname()[0]:
    # we assume if linux then you are running packaged cairo
    # which is older than the 1.12.14 version we used on OS X
    # to generate the expected images, so we'll rachet back the threshold
    # https://github.com/mapnik/mapnik/issues/1868
    cairo_threshold = 181

def render_cairo(m, output, scale_factor):
    mapnik.render_to_file(m, output, 'ARGB32', scale_factor)
    # open and re-save as png8 to save space
    new_im = mapnik.Image.open(output)
    new_im.save(output, 'png8:m=h')

def render_grid(m, output, scale_factor):
    grid = mapnik.Grid(m.width, m.height)
    mapnik.render_layer(m, grid, layer=0)
    utf1 = grid.encode('utf', resolution=4)
    open(output,'wb').write(json.dumps(utf1, indent=1))

renderers = [
    { 'name': 'agg',
      'render': lambda m, output, scale_factor: mapnik.render_to_file(m, output, 'png8:m=h', scale_factor),
      'compare': lambda actual, reference: compare(actual, reference, alpha=True),
      'threshold': 0,
      'filetype': 'png',
      'dir': 'images'
    },
    { 'name': 'cairo',
      'render': render_cairo,
      'compare': lambda actual, reference: compare(actual, reference, alpha=False),
      'threshold': cairo_threshold,
      'filetype': 'png',
      'dir': 'images'
    },
    { 'name': 'grid',
      'render': render_grid,
      'compare': lambda actual, reference: compare_grids(actual, reference, alpha=False),
      'threshold': 0,
      'filetype': 'json',
      'dir': 'grids'
    }
]

sizes_many_in_big_range = [(800, 100), (600, 100), (400, 100),
    (300, 100), (250, 100), (150, 100), (100, 100)]

sizes_few_square = [(800, 800), (600, 600), (400, 400), (200, 200)]
sizes_many_in_small_range = [(490, 100), (495, 100), (497, 100), (498, 100),
    (499, 100), (500, 100), (501, 100), (502, 100), (505, 100), (510, 100)]

default_text_box = mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)

merc_z1_bboxes = {
  '0,0':mapnik.Box2d(-20037508.342,0,0,20037508.342), # upper left
  '1,0':mapnik.Box2d(0,0,20037508.342,20037508.342), # upper right
  '0,1':mapnik.Box2d(-20037508.342,-20037508.342,0,0), # lower left
  '1,1':mapnik.Box2d(0,-20037508.342,20037508.342,0) # lower right
}

dirname = os.path.dirname(__file__)

files = {
    'list': {'sizes': sizes_many_in_big_range,'bbox':default_text_box},
    'simple': {'sizes': sizes_many_in_big_range,'bbox':default_text_box},
    'lines-1': {'sizes': sizes_few_square,'bbox':default_text_box},
    'lines-2': {'sizes': sizes_few_square,'bbox':default_text_box},
    'lines-3': {'sizes': sizes_few_square,'bbox':default_text_box},
    'lines-4': {'sizes': sizes_few_square,'bbox':default_text_box},
    'lines-5': {'sizes': sizes_few_square,'bbox':default_text_box},
    'lines-6': {'sizes': sizes_few_square,'bbox':default_text_box},
    'lines-shield': {'sizes': sizes_few_square,'bbox':default_text_box},
    'collision': {'sizes':[(600,400)]},
    'shield-on-polygon': {'sizes':[(600,400)]},
    'shield-on-line-spacing-eq-width': {'sizes':[(600,400)]},
    'geometry-transform-translate': {'sizes':[(200,200)]},
    'geometry-transform-translate-patterns': {'sizes':[(200,200)]},
    'marker-svg-opacity':{},
    'marker-svg-opacity2':{},
    'marker-svg-empty-g-element':{},
    'marker-multi-policy': {'sizes':[(600,400)]},
    'marker-on-line': {'sizes':[(600,400)],
        'bbox': mapnik.Box2d(-10, 0, 15, 20)},
    'marker-on-line-and-line-placement': {'sizes':[(600,400)],
        'bbox': mapnik.Box2d(-10, 0, 15, 20)},
    'marker-on-line-spacing-eq-width': {'sizes':[(600,400)]},
    'marker-on-line-spacing-eq-width-overlap': {'sizes':[(600,400)]},
    'marker_line_placement_on_points':{},
    'marker-with-background-image': {'sizes':[(600,400),(400,600),(257,256)]},
    'marker-with-background-image-and-hsla-transform': {'sizes':[(600,400),(400,600),(257,256)]},
    'marker-on-hex-grid': {'sizes':[(600,400),(400,600),(257,256)]},
    'whole-centroid': {'sizes':[(600,400)],
        'bbox': mapnik.Box2d(736908, 4390316, 2060771, 5942346)},
    'text-halo-rasterizer': {'sizes':[(600,400)]},
    'simple-E': {'bbox':mapnik.Box2d(-0.05, -0.01, 0.95, 0.01)},
    'simple-NE': {'bbox':default_text_box},
    'simple-NW': {'bbox':default_text_box},
    'simple-N': {'bbox':default_text_box},
    'simple-SE': {'bbox':default_text_box},
    'simple-SW': {'bbox':default_text_box},
    'simple-S': {'bbox':default_text_box},
    'simple-W': {'bbox':default_text_box},
    'formatting-1': {'bbox':default_text_box},
    'formatting-2': {'bbox':default_text_box},
    'formatting-3': {'bbox':default_text_box},
    'formatting-4': {'bbox':default_text_box},
    'expressionformat': {'bbox':default_text_box},
    'shieldsymbolizer-1': {'sizes': sizes_many_in_small_range,'bbox':default_text_box},
    'shieldsymbolizer-2': {'sizes': sizes_many_in_small_range,'bbox':default_text_box},
    'shieldsymbolizer-3': {'sizes': sizes_many_in_small_range,'bbox':default_text_box},
    'shieldsymbolizer-4': {'sizes': sizes_many_in_small_range,'bbox':default_text_box},
    'orientation': {'sizes': [(800, 200)], 'bbox': default_text_box},
    'harfbuzz': {'sizes': [(800, 200)], 'bbox': default_text_box},
    'hb-fontsets': {'sizes': [(800, 200)], 'bbox': default_text_box},
    'khmer': {'sizes': [(800, 200)], 'bbox': default_text_box},
    'charspacing': {'sizes': [(200, 400)], 'bbox': default_text_box},
    'charspacing-lines': {'sizes': [(300, 300)], 'bbox': default_text_box},
    'line_break': {'sizes': [(800, 800)], 'bbox': default_text_box},
    'rtl-point': {'sizes': [(200, 200)],'bbox':default_text_box},
    'jalign-auto': {'sizes': [(200, 200)],'bbox':default_text_box},
    'line-offset': {'sizes':[(900, 250)],'bbox': mapnik.Box2d(-5.192, 50.189, -5.174, 50.195)},
    'text-bug1532': {'sizes': [(600, 165)]},
    'text-bug1533': {'sizes': [(600, 600)]},
    'text-bug1820-1': {'sizes': [(600, 300)], 'bbox': default_text_box},
    'text-bug1820+0': {'sizes': [(600, 300)], 'bbox': default_text_box},
    'text-bug1820+1': {'sizes': [(600, 300)], 'bbox': default_text_box},
    'text-bug2037': {'sizes': [(800, 300)], 'bbox': default_text_box},
    'text-expressionformat-color': {'sizes': [(800, 100)], 'bbox': default_text_box},
    'text-halign': {'sizes': [(800,800)], 'bbox': default_text_box},
    'text-malayalam': {'sizes': [(800, 100)], 'bbox': default_text_box},
    'text-bengali': {'sizes': [(800, 100)], 'bbox': default_text_box},
    'line-pattern-symbolizer': {'sizes':[(900, 250)],'bbox': mapnik.Box2d(-5.192, 50.189, -5.174, 50.195)},
    'tiff-alpha-gdal': {'sizes':[(600,400)]},
    'tiff-alpha-broken-assoc-alpha-gdal': {'sizes':[(600,400)]},
    'tiff-alpha-gradient-gdal': {'sizes':[(600,400)]},
    'tiff-nodata-edge-gdal': {'sizes':[(600,400),(969,793)]},
    'tiff-opaque-edge-gdal': {'sizes':[(256,256),(969,793)]},
    'tiff-opaque-edge-gdal2': {'sizes':[(600,400),(969,793)]},
    'tiff-opaque-edge-raster2': {'sizes':[(600,400),(969,793)]},
    'tiff-resampling': {'sizes':[(600,400)]},
    'gdal-filter-factor': {'sizes':[(600,400)]},
    # https://github.com/mapnik/mapnik/issues/1622
    'tiff-edge-alignment-gdal1': {'sizes':[(256,256),(255,257)],
        'bbox':mapnik.Box2d(-13267022.12540147,4618019.500877209,-13247454.246160466,4637587.380118214)
    },
    'tiff-edge-alignment-gdal2': {'sizes':[(256,256),(255,257)],
        'bbox':mapnik.Box2d(-13267022.12540147,4598451.621636203,-13247454.246160466,4618019.500877209)
    },
    'tiff-reprojection-1': {'sizes':[(250,250)]},

    # disabled since fixing is not actionable: https://github.com/mapnik/mapnik/issues/1913
    #'tiff-reprojection-2': {'sizes':[(250,250)]},

    # https://github.com/mapnik/mapnik/issues/1520
    # commented because these are not critical failures
    #'tiff-alpha-raster': {'sizes':[(600,400)]},
    #'tiff-alpha-broken-assoc-alpha-raster': {'sizes':[(600,400)]},
    #'tiff-nodata-edge-raster': {'sizes':[(600,400)]},
    #'tiff-opaque-edge-raster': {'sizes':[(256,256)]},
    'road-casings-grouped-rendering': {'sizes':[(600,600)],
        'bbox':mapnik.Box2d(1477001.12245,6890242.37746,1480004.49012,6892244.62256)
    },
    'road-casings-non-grouped-rendering': {'sizes':[(600,600)],
        'bbox':mapnik.Box2d(1477001.12245,6890242.37746,1480004.49012,6892244.62256)
    },
    'style-level-compositing-tiled-0,0':{'sizes':[(512,512)],'bbox':merc_z1_bboxes['0,0']},
    'style-level-compositing-tiled-1,0':{'sizes':[(512,512)],'bbox':merc_z1_bboxes['1,0']},
    'style-level-compositing-tiled-0,1':{'sizes':[(512,512)],'bbox':merc_z1_bboxes['0,1']},
    'style-level-compositing-tiled-1,1':{'sizes':[(512,512)],'bbox':merc_z1_bboxes['1,1']},
    'marker-path-expression':{},
    'map-background-image-compositing':{'sizes':[(512,512)]},
    'building-symbolizer-opacity':{'sizes':[(512,512)]},
    'line-pattern-symbolizer-opacity':{'sizes':[(512,512)]},
    'dst-over-compositing':{'sizes':[(512,512)]},
    'tiff_colortable':{'sizes':[(256,256)]},
    'tiff_colortable_custom_nodata':{'sizes':[(256,256)]},
    'vrt_colortable':{'sizes':[(256,256)]},
    'raster_colorizer':{'sizes':[(512,512)]},
    'raster_symbolizer':{'sizes':[(512,512)]},
    'raster-color-to-alpha1':{'sizes':[(512,512)]},
    'raster-color-to-alpha2':{'sizes':[(512,512)]},
    'raster-color-to-alpha3':{'sizes':[(512,512)]},
    'raster-color-to-alpha4':{'sizes':[(512,512)]},
    'raster-color-to-alpha5':{'sizes':[(512,512)]},
    'colorize-alpha1':{'sizes':[(512,512)]},
    'colorize-alpha2':{'sizes':[(512,512)]},
    'colorize-alpha3':{'sizes':[(512,512)]},
    'image-filters-galore':{'sizes':[(512,512)]},
    'image-filters-multi-blur':{'sizes':[(512,512)]},
    'line-opacity-multi-render':{'sizes':[(512,512)]},
    'tiff-nodata-rgb':{'sizes':[(512,512)]},
    'tiff-nodata-rgba':{'sizes':[(512,512)]},
    'tiff-nodata-tolerance':{'sizes':[(512,512)]},
    'tiff-nodata-edge-rgba':{'sizes':[(512,512)]}
    }

class Reporting:
    DIFF = 1
    NOT_FOUND = 2
    OTHER = 3
    REPLACE = 4
    def __init__(self, quiet, overwrite_failures = False):
        self.quiet = quiet
        self.passed = 0
        self.failed = 0
        self.overwrite_failures = overwrite_failures
        self.errors = [ #(type, actual, expected, diff, message)
         ]

    def result_fail(self, actual, expected, diff):
        self.failed += 1
        if self.quiet:
            sys.stderr.write('\x1b[31m.\x1b[0m')
        else:
            print '\x1b[31m✘\x1b[0m (\x1b[34m%u different pixels\x1b[0m)' % diff

        if self.overwrite_failures:
            self.errors.append((self.REPLACE, actual, expected, diff, None))
            contents = open(actual, 'r').read()
            open(expected, 'wb').write(contents)
        else:
            self.errors.append((self.DIFF, actual, expected, diff, None))

    def result_pass(self, actual, expected, diff):
        self.passed += 1
        if self.quiet:
            sys.stderr.write('\x1b[32m.\x1b[0m')
        else:
            print '\x1b[32m✓\x1b[0m'

    def not_found(self, actual, expected):
        self.failed += 1
        self.errors.append((self.NOT_FOUND, actual, expected, 0, None))
        if self.quiet:
            sys.stderr.write('\x1b[33m.\x1b[0m')
        else:
            print '\x1b[33m?\x1b[0m (\x1b[34mReference file not found, creating\x1b[0m)'
        contents = open(actual, 'r').read()
        open(expected, 'wb').write(contents)

    def other_error(self, expected, message):
        self.failed += 1
        self.errors.append((self.OTHER, None, expected, 0, message))
        if self.quiet:
            sys.stderr.write('\x1b[31m.\x1b[0m')
        else:
            print '\x1b[31m✘\x1b[0m (\x1b[34m%s\x1b[0m)' % message

    def make_html_item(self,actual,expected,diff):
        item = '''
             <div class="expected">
               <a href="%s">
                 <img src="%s" width="100%s">
               </a>
             </div>
              ''' % (expected,expected,'%')
        item += '<div class="text">%s</div>' % (diff)
        item += '''
             <div class="actual">
               <a href="%s">
                 <img src="%s" width="100%s">
               </a>
             </div>
              ''' % (actual,actual,'%')
        return item

    def summary(self):
        if len(self.errors) == 0:
            print '\nAll %s visual tests passed: \x1b[1;32m✓ \x1b[0m' % self.passed
            return 0
        sortable_errors = []
        print "\nVisual rendering: %s failed / %s passed" % (len(self.errors), self.passed)
        for idx, error in enumerate(self.errors):
            if error[0] == self.OTHER:
                print str(idx+1) + ") \x1b[31mfailure to run test:\x1b[0m %s (\x1b[34m%s\x1b[0m)" % (error[2],error[4])
            elif error[0] == self.NOT_FOUND:
                print str(idx+1) + ") Generating reference image: '%s'" % error[2]
                continue
            elif error[0] == self.DIFF:
                print str(idx+1) + ") \x1b[34m%s different pixels\x1b[0m:\n\t%s (\x1b[31mactual\x1b[0m)\n\t%s (\x1b[32mexpected\x1b[0m)" % (error[3], error[1], error[2])
                if '.png' in error[1]: # ignore grids
                    sortable_errors.append((error[3],error))
            elif error[0] == self.REPLACE:
                print str(idx+1) + ") \x1b[31mreplaced reference with new version:\x1b[0m %s" % error[2]
        if len(sortable_errors):
            # drop failure results in folder
            vdir = os.path.join(visual_output_dir,'visual-test-results')
            if not os.path.exists(vdir):
                os.makedirs(vdir)
            html_template = open(os.path.join(dirname,'html_report_template.html'),'r').read()
            name = 'comparison.html'
            failures_realpath = os.path.join(vdir,name)
            html_out = open(failures_realpath,'w+')
            sortable_errors.sort(reverse=True)
            html_body = ''
            for item in sortable_errors:
                # copy images into single directory
                actual = item[1][1]
                expected = item[1][2]
                diff = item[0]
                actual_new = os.path.join(vdir,os.path.basename(actual))
                shutil.copy(actual,actual_new)
                expected_new = os.path.join(vdir,os.path.basename(expected))
                shutil.copy(expected,expected_new)
                html_body += self.make_html_item(os.path.relpath(actual_new,vdir),os.path.relpath(expected_new,vdir),diff)
            html_out.write(html_template.replace('{{RESULTS}}',html_body))
            print 'View failures by opening %s' % failures_realpath
        return 1

def render(filename,config, width, height, bbox, scale_factor, reporting):
    m = mapnik.Map(width, height)
    postfix = "%s-%d-%d-%s" % (filename, width, height, scale_factor)

    try:
        mapnik.load_map(m, os.path.join(dirname, "styles", "%s.xml" % filename), True)
        if bbox is not None:
            m.zoom_to_box(bbox)
        else:
            m.zoom_all()
    except Exception, e:
        if 'Could not create datasource' in str(e):
            return m
        reporting.other_error(filename, repr(e))
        return m

    for renderer in renderers:
        # TODO - grid renderer does not support scale_factor yet via python
        if renderer['name'] == 'grid' and scale_factor != 1.0:
            continue
        if config.get(renderer['name'], True):
            expected = os.path.join(dirname, renderer['dir'], '%s-%s-reference.%s' %
                (postfix, renderer['name'], renderer['filetype']))
            actual = os.path.join(visual_output_dir, '%s-%s.%s' %
                (postfix, renderer['name'], renderer['filetype']))
            if not quiet:
                print "\"%s\" with %s..." % (postfix, renderer['name']),
            try:
                renderer['render'](m, actual, scale_factor)
                if not os.path.exists(expected):
                    reporting.not_found(actual, expected)
                else:
                    diff = renderer['compare'](actual, expected)
                    if diff > renderer['threshold']:
                        reporting.result_fail(actual, expected, diff)
                    else:
                        reporting.result_pass(actual, expected, diff)
            except Exception, e:
                reporting.other_error(expected, repr(e))
    return m

if __name__ == "__main__":
    if '-q' in sys.argv:
       quiet = True
       sys.argv.remove('-q')
    else:
       quiet = False

    if '--overwrite' in sys.argv:
       overwrite_failures = True
       sys.argv.remove('--overwrite')
    else:
       overwrite_failures = False

    select_files = {}
    if len(sys.argv) > 1:
        for name in sys.argv[1:]:
            if name in files:
                select_files[name]=files[name]
            else:
                select_files[name]={}
    if len(select_files) > 0:
        files = select_files

    if not os.path.exists(visual_output_dir):
        os.makedirs(visual_output_dir)

    reporting = Reporting(quiet, overwrite_failures)
    for filename in files:
        config = dict(defaults)
        config.update(files[filename])
        for size in config['sizes']:
            for scale_factor in config['scales']:
                m = render(filename,
                           config,
                           size[0],
                           size[1],
                           config.get('bbox'),
                           scale_factor,
                           reporting)
        mapnik.save_map(m, os.path.join(dirname, 'xml_output', "%s-out.xml" % filename))

    sys.exit(reporting.summary())
