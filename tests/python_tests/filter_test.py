#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import Todo

import mapnik

if hasattr(mapnik,'Expression'):
    mapnik.Filter = mapnik.Expression

map_ = '''<Map>
    <Style name="s">
        <Rule>
            <Filter><![CDATA[(([region]>=0) and ([region]<=50))]]></Filter>
        </Rule>
        <Rule>
            <Filter><![CDATA[([region]>=0) and ([region]<=50)]]></Filter>
        </Rule>
        <Rule>
            <Filter>
            
            <![CDATA[

            ([region] >= 0) 

            and 

            ([region] <= 50)
            ]]>
            
            </Filter>
        </Rule>
        <Rule>
            <Filter>([region]&gt;=0) and ([region]&lt;=50)</Filter>
        </Rule>
        <Rule>
            <Filter>
            ([region] &gt;= 0)
             and
            ([region] &lt;= 50)
            </Filter>
        </Rule>

    </Style>
    <Style name="s2" filter-mode="first">
        <Rule>
        </Rule>
        <Rule>
        </Rule>
    </Style>
</Map>'''

def test_filter_init():    
    m = mapnik.Map(1,1)
    mapnik.load_map_from_string(m,map_)
    filters = []
    filters.append(mapnik.Filter("([region]>=0) and ([region]<=50)"))
    filters.append(mapnik.Filter("(([region]>=0) and ([region]<=50))"))
    filters.append(mapnik.Filter("((([region]>=0) and ([region]<=50)))"))
    filters.append(mapnik.Filter('((([region]>=0) and ([region]<=50)))'))
    filters.append(mapnik.Filter('''((([region]>=0) and ([region]<=50)))'''))
    filters.append(mapnik.Filter('''
    ((([region]>=0)
    and
    ([region]<=50)))
    '''))
    filters.append(mapnik.Filter('''
    ([region]>=0)
    and
    ([region]<=50)
    '''))
    filters.append(mapnik.Filter('''
    ([region]
    >=
    0)
    and
    ([region]
    <= 
    50)
    '''))
    
    s = m.find_style('s')
    
    for r in s.rules:
        filters.append(r.filter)
    
    first = filters[0]
    for f in filters:
        eq_(str(first),str(f))
    
    s = m.find_style('s2')
    
    eq_(s.filter_mode,mapnik.filter_mode.FIRST)


def test_regex_match():
    f = mapnik.Feature(0)
    f["name"] = 'test'
    expr = mapnik.Expression("[name].match('test')")
    eq_(expr.evaluate(f),True) # 1 == True

def test_unicode_regex_match():
    f = mapnik.Feature(0)
    f["name"] = 'Québec'
    expr = mapnik.Expression("[name].match('Québec')")
    eq_(expr.evaluate(f),True) # 1 == True

def test_regex_replace():
    f = mapnik.Feature(0)
    f["name"] = 'test'
    expr = mapnik.Expression("[name].replace('(\B)|( )','$1 ')")
    eq_(expr.evaluate(f),'t e s t')

def test_unicode_regex_replace():
    f = mapnik.Feature(0)
    f["name"] = 'Québec'
    expr = mapnik.Expression("[name].replace('(\B)|( )','$1 ')")
    eq_(expr.evaluate(f), u'Q u é b e c')

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]


