#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os, mapnik
from timeit import Timer, time
from nose.tools import *
from utilities import execution_path

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

combinations = ['png',
                'png8',
                'png8:m=o',
                'png8:m=h',
                'png8:m=o:t=0',
                'png8:m=o:t=1',
                'png8:m=h:t=0',
                'png8:m=h:t=1',
                'png:z=1',
                'png8:z=1',
                'png8:z=1:m=o',
                'png8:z=1:m=h',
                'png8:z=1:c=50',
                'png8:z=1:c=1',
                'png8:z=1:c=50:m=h',
                'png8:z=1:c=1:m=o',
                'png8:z=1:c=1:m=o:s=filtered',
                'png:z=1:s=filtered',
                'png:z=1:s=huff',
                'png:z=1:s=rle',
                'png:m=h;g=2.0',
                'png:m=h;g=1.0',
               ]

def do_encoding():

    image = None
    iterations = 10

    results = {}
    sortable = {}

    def run(func, im, format, t):
        global image
        image = im
        start = time.time()
        set = t.repeat(iterations,1)
        elapsed = (time.time() - start)
        min_ = min(set)*1000
        avg = (sum(set)/len(set))*1000
        name = func.__name__ + ' ' + format
        results[name] = [avg,min_,elapsed*1000,name]
        sortable[name] = [avg]

    def blank():
        eval('image.tostring("%s")' % c)
    blank_im = mapnik.Image(512,512)

    for c in combinations:
        t = Timer(blank)
        run(blank,blank_im,c,t)

    def solid():
        eval('image.tostring("%s")' % c)
    solid_im = mapnik.Image(512,512)
    solid_im.background = mapnik.Color("#f2efe9")

    for c in combinations:
        t = Timer(solid)
        run(solid,solid_im,c,t)

    def many_colors():
        eval('image.tostring("%s")' % c)
    # lots of colors: http://tile.osm.org/13/4194/2747.png
    many_colors_im = mapnik.Image.open('../data/images/13_4194_2747.png')

    for c in combinations:
        t = Timer(many_colors)
        run(many_colors,many_colors_im,c,t)

    for key, value in sorted(sortable.iteritems(), key=lambda (k,v): (v,k)):
        s = results[key]
        avg = str(s[0])[:6]
        min_ = str(s[1])[:6]
        elapsed = str(s[2])[:6]
        name = s[3]
        print 'avg: %sms | min: %sms | total: %sms <-- %s' % (min_,avg,elapsed,name)


if __name__ == "__main__":
    setup()
    do_encoding()
    for t in dir():
        if 'test_' in t:
            eval(t)()
