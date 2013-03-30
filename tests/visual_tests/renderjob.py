# -*- coding: utf-8 -*-

import mapnik
import os
from time import time
from compare import compare, compare_grids
try:
    import json
except ImportError:
    import simplejson as json

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



class RenderJob:
    renderers = [
    { 'name': 'agg',
      'render': lambda m, output, scale_factor: mapnik.render_to_file(m, output, 'png8:m=h', scale_factor),
      'compare': lambda actual, reference: compare(actual, reference, alpha=True),
      'threshold': 0,
      'filetype': 'png',
      'dir': 'images'
    },
    { 'name': 'agg_benchmark',
      'render': lambda m, output, scale_factor: mapnik.render(m, mapnik.Image(m.width, m.height), scale_factor),
      'compare': lambda actual, reference: 0,
      'threshold': 0,
      'filetype': 'png',
      'dir': 'images',
      'no_reference': True
    },
    { 'name': 'cairo',
      'render': render_cairo,
      'compare': lambda actual, reference: compare(actual, reference, alpha=False),
      'threshold': 0,
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

    def __init__(self, reporting, dirname, visual_output_dir):
        self.reporting = reporting
        self.generate = False
        self.overwrite_failures = False
        self.dirname = dirname
        self.repeat = 1
        self.visual_output_dir = visual_output_dir

    def set_overwrite_failures(self, state):
        self.reporting.overwrite_failures = state
        self.overwrite_failures = state

    def set_generate(self, state):
        self.generate = state
        self.reporting.generate = state

    def load_and_save(self, filename, output):
        self.m = mapnik.Map(16, 16)
        try:
            start = time()
            mapnik.load_map(self.m, os.path.join(self.dirname, "styles", "%s.xml" % filename), False)
            mapnik.save_map(self.m, output)
        except Exception, e:
            self.reporting.load_error(filename, repr(e))
            return False
        return True

    def render(self, config, width, height, scale_factor):
        filename = config['name']
        postfix = "%s-%d-%d-%s" % (filename, width, height, scale_factor)

        bbox = config.get('bbox')
        self.m.resize(int(width*scale_factor), int(height*scale_factor))
        if bbox is not None:
            self.m.zoom_to_box(bbox)
        else:
            self.m.zoom_all()

        for renderer in self.renderers:
            # TODO - grid renderer does not support scale_factor yet via python
            if renderer['name'] == 'grid' and scale_factor != 1.0:
                continue
            if config.get(renderer['name'], True):
                expected = os.path.join(self.dirname, renderer['dir'], '%s-%s-reference.%s' %
                    (postfix, renderer['name'], renderer['filetype']))
                actual = os.path.join(self.visual_output_dir, '%s-%s.%s' %
                    (postfix, renderer['name'], renderer['filetype']))
                self.reporting.show_file(postfix, renderer['name'])
                try:
                    start = time()
                    for i in range(self.repeat):
                        renderer['render'](self.m, actual, scale_factor)
                    render_time = time() - start

                    if not os.path.exists(expected) and not renderer.get('no_reference', False):
                        self.reporting.not_found(actual, expected)
                    else:
                        diff = renderer['compare'](actual, expected)
                        if diff > renderer['threshold']:
                            self.reporting.result_fail(actual, expected, diff, render_time)
                        else:
                            self.reporting.result_pass(actual, expected, diff, render_time)
                except Exception, e:
                    self.reporting.other_error('%s-%s' % (postfix, renderer['name']), repr(e))