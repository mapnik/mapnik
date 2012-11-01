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


def test_geometry_type_eval():
    # clashing field called 'mapnik::geometry'
    context2 = mapnik.Context()
    context2.push('mapnik::geometry_type')
    f = mapnik.Feature(context2,0)
    f["mapnik::geometry_type"] = 'sneaky'
    expr = mapnik.Expression("[mapnik::geometry_type]")
    eq_(expr.evaluate(f),0)

    expr = mapnik.Expression("[mapnik::geometry_type]")
    context = mapnik.Context()

    # no geometry
    f = mapnik.Feature(context,0)
    eq_(expr.evaluate(f),0)
    eq_(mapnik.Expression("[mapnik::geometry_type]=0").evaluate(f),True)

    # POINT = 1
    f = mapnik.Feature(context,0)
    f.add_geometries_from_wkt('POINT(10 40)')
    eq_(expr.evaluate(f),1)
    eq_(mapnik.Expression("[mapnik::geometry_type]=point").evaluate(f),True)

    # LINESTRING = 2
    f = mapnik.Feature(context,0)
    f.add_geometries_from_wkt('LINESTRING (30 10, 10 30, 40 40)')
    eq_(expr.evaluate(f),2)
    eq_(mapnik.Expression("[mapnik::geometry_type]=linestring").evaluate(f),True)

    # POLYGON = 3
    f = mapnik.Feature(context,0)
    f.add_geometries_from_wkt('POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10))')
    eq_(expr.evaluate(f),3)
    eq_(mapnik.Expression("[mapnik::geometry_type]=polygon").evaluate(f),True)

    # COLLECTION = 4
    f = mapnik.Feature(context,0)
    f.add_geometries_from_wkt('GEOMETRYCOLLECTION(POLYGON((1 1,2 1,2 2,1 2,1 1)),POINT(2 3),LINESTRING(2 3,3 4))')
    eq_(expr.evaluate(f),4)
    eq_(mapnik.Expression("[mapnik::geometry_type]=collection").evaluate(f),True)

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

def test_unicode_regex_replace_to_str():
    expr = mapnik.Expression("[name].replace('(\B)|( )','$1 ')")
    eq_(str(expr),"[name].replace('(\B)|( )','$1 ')")

def test_unicode_regex_replace():
    context = mapnik.Context()
    context.push('name')
    f = mapnik.Feature(context,0)
    f["name"] = 'Québec'
    expr = mapnik.Expression("[name].replace('(\B)|( )','$1 ')")
    # will fail if -DBOOST_REGEX_HAS_ICU is not defined
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
