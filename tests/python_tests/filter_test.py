#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
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
    context = mapnik.Context()
    context.push('name')
    f = mapnik.Feature(context,0)
    f["name"] = 'test'
    expr = mapnik.Expression("[name].match('test')")
    eq_(expr.evaluate(f),True) # 1 == True

def test_unicode_regex_match():
    context = mapnik.Context()
    context.push('name')
    f = mapnik.Feature(context,0)
    f["name"] = 'Québec'
    expr = mapnik.Expression("[name].match('Québec')")
    eq_(expr.evaluate(f),True) # 1 == True

def test_regex_replace():
    context = mapnik.Context()
    context.push('name')
    f = mapnik.Feature(context,0)
    f["name"] = 'test'
    expr = mapnik.Expression("[name].replace('(\B)|( )','$1 ')")
    eq_(expr.evaluate(f),'t e s t')

def test_unicode_regex_replace():
    context = mapnik.Context()
    context.push('name')
    f = mapnik.Feature(context,0)
    f["name"] = 'Québec'
    expr = mapnik.Expression("[name].replace('(\B)|( )','$1 ')")
    eq_(expr.evaluate(f), u'Q u é b e c')

def test_float_precision():
    context = mapnik.Context()
    context.push('num')
    f = mapnik.Feature(context,0)
    f["num"] = 1.0000
    eq_(f["num"],1.0000)
    expr = mapnik.Expression("[num] = 1.0000")
    eq_(expr.evaluate(f),True)
    expr = mapnik.Expression("[num].match('.*0$')")
    eq_(expr.evaluate(f),True)
    expr = mapnik.Expression("[num].match('.*0$')")
    eq_(expr.evaluate(f),True)

def test_string_matching_on_precision():
    context = mapnik.Context()
    context.push('num')
    f = mapnik.Feature(context,0)
    f["num"] = "1.0000"
    eq_(f["num"],"1.0000")
    expr = mapnik.Expression("[num].match('.*(^0|00)$')")
    eq_(expr.evaluate(f),True)

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
