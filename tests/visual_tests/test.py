#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import mapnik
#mapnik.logger.set_severity(mapnik.severity_type.None)
#mapnik.logger.set_severity(mapnik.severity_type.Debug)
import shutil
import os.path
from compare import compare, compare_grids
import platform
import glob

try:
    import json
except ImportError:
    import simplejson as json

visual_output_dir = "/tmp/mapnik-visual-images"

defaults = {
    'status': True,
    'sizes': [(500, 100)],
    'scales':[1.0,2.0],
    'agg': True,
    'cairo': mapnik.has_cairo(),
    'grid': mapnik.has_grid_renderer()
}

cairo_threshold = 10
agg_threshold = 0
grid_threshold = 5
if 'Linux' == platform.uname()[0]:
    # we assume if linux then you are running packaged cairo
    # which is older than the 1.12.14 version we used on OS X
    # to generate the expected images, so we'll rachet back the threshold
    # https://github.com/mapnik/mapnik/issues/1868
    cairo_threshold = 230
    agg_threshold = 12
    grid_threshold = 6

def render_cairo(m, output, scale_factor):
    mapnik.render_to_file(m, output, 'ARGB32', scale_factor)
    # open and re-save as png8 to save space
    new_im = mapnik.Image.open(output)
    new_im.save(output, 'png8:m=h')

def render_grid(m, output, scale_factor):
    grid = mapnik.Grid(m.width, m.height)
    mapnik.render_layer(m, grid, layer=0, scale_factor=scale_factor)
    utf1 = grid.encode('utf', resolution=4)
    open(output,'wb').write(json.dumps(utf1, indent=1))

def render_agg(m, output, scale_factor):
    mapnik.render_to_file(m, output, 'png8:m=h', scale_factor),

renderers = [
    { 'name': 'agg',
      'render': render_agg,
      'compare': lambda actual, reference: compare(actual, reference, alpha=True),
      'threshold': agg_threshold,
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
      'threshold': grid_threshold,
      'filetype': 'json',
      'dir': 'grids'
    }
]

dirname = os.path.dirname(__file__)

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
            if platform.uname()[0] == 'Windows':
                sys.stderr.write('.')
            else:
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
            if platform.uname()[0] == 'Windows':
                sys.stderr.write('.')
            else:
                sys.stderr.write('\x1b[32m.\x1b[0m')
        else:
            if platform.uname()[0] == 'Windows':
                print '\x1b[32m✓\x1b[0m'
            else:
                print '✓'

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

def render(filename, config, scale_factor, reporting):
    m = mapnik.Map(*config['sizes'][0])

    try:
        mapnik.load_map(m, os.path.join(dirname, "styles", filename), True)

        if not (m.parameters['status'] if ('status' in m.parameters) else config['status']):
            return
    except Exception, e:
        if 'Could not create datasource' in str(e) \
           or 'Bad connection' in str(e):
            return m
        reporting.other_error(filename, repr(e))
        return m

    sizes = config['sizes'];
    if 'sizes' in m.parameters:
        sizes = [[int(i) for i in size.split(',')] for size in m.parameters['sizes'].split(';')]

    for size in sizes:
        m.width, m.height = size

        if 'bbox' in m.parameters:
            bbox = mapnik.Box2d.from_string(str(m.parameters['bbox']))
            m.zoom_to_box(bbox)
        else:
            m.zoom_all()

        name = filename[0:-4]
        postfix = "%s-%d-%d-%s" % (name, m.width, m.height, scale_factor)
        for renderer in renderers:
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

    files = None
    if len(sys.argv) > 1:
        files = [name + ".xml" for name in sys.argv[1:]]
    else:
        files = [os.path.basename(file) for file in glob.glob(os.path.join(dirname, "styles/*.xml"))]

    if not os.path.exists(visual_output_dir):
        os.makedirs(visual_output_dir)

    reporting = Reporting(quiet, overwrite_failures)
    try:
        for filename in files:
            config = dict(defaults)
            for scale_factor in config['scales']:
                m = render(filename, config, scale_factor, reporting)
                #mapnik.save_map(m, os.path.join(dirname, 'xml_output', "%s-out.xml" % filename))
    except KeyboardInterrupt:
        pass
    sys.exit(reporting.summary())
