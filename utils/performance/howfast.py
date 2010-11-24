#!/usr/bin/env python

import sys
import mapnik2 as mapnik
#import mapnik
from timeit import Timer, time

if not len(sys.argv) >= 3:
    sys.exit('usage: howfast.py <stylesheet> <iterations> [minx,miny,maxx,maxy]')

if not hasattr(mapnik,'Envelope'):
    mapnik.Envelope = mapnik.Box2d

if not hasattr(mapnik,'Box2d'):
    mapnik.Box2d = mapnik.Envelope

m = mapnik.Map(256,256)
mapnik.load_map(m,sys.argv[1])
m.zoom_all()
ver = mapnik.mapnik_version()

TOTAL_TIME = 0
BBOX = None

def test():
    global TOTAL_TIME
    start = time.time()
    im = mapnik.Image(m.width,m.height)
    mapnik.render(m,im)
    TOTAL_TIME += (time.time() - start)

def f_(set):
    min_ = str(min(set))[:6]
    avg = str(sum(set)/len(set))[:6]
    print '%s: min: %s | avg: %s | total: %s' % (ver,min_,avg,str(TOTAL_TIME)[:6])

if __name__=='__main__':
    iters = int(sys.argv[2])
    # if passed, set up bbox
    if len(sys.argv) == 4:
        bbox = sys.argv[3]
        if ',' in bbox:
            parts = bbox.split(',')
        else:
            parts = bbox.split(' ')
        env = mapnik.Box2d(*map(float,parts))
        m.zoom_to_box(env)

    # load once - making sure mmap'd shapefiles are loaded
    test_ = "test()"
    eval(test_)
    t = Timer(test_, "from __main__ import test")
    f_(t.repeat(iters,1))