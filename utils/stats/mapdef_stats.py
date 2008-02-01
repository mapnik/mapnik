#!/usr/bin/env python

import sys
from mapnik import *
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "usage : ./mapdef_stats.py <mapdefinition file>"
        sys.exit(0)

    m = Map(100,100)
    styles = []
    num_rules = 0
    num_sym = 0
    load_map(m,sys.argv[1])
    for l in m.layers:
        print "Layer:%s"  % l.name
        for s in l.styles:
            print "    Style:%s" % s
            styles.append(s)
            style = m.find_style(s)
            num_rules += len(style.rules)
            for r in style.rules:
                print "        Filter: %s" % r.filter
                num_sym += len(r.symbols)
                            
    print "Total number of layers      %s" % len(m.layers)
    print "Total number of styles      %s" % len(set(styles)) # unique styles
    print "Total number of rules       %s" % num_rules
    print "Total number of symbolizers %s" % num_sym
