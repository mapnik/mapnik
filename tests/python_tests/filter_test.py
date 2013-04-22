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
    f["num1"] = 1.0000
    f["num2"] = 1.0001
    eq_(f["num1"],1.0000)
    eq_(f["num2"],1.0001)
    expr = mapnik.Expression("[num1] = 1.0000")
    eq_(expr.evaluate(f),True)
    expr = mapnik.Expression("[num1].match('1')")
    eq_(expr.evaluate(f),True)
    expr = mapnik.Expression("[num2] = 1.0001")
    eq_(expr.evaluate(f),True)
    expr = mapnik.Expression("[num2].match('1.0001')")
    eq_(expr.evaluate(f),True)

def test_string_matching_on_precision():
    context = mapnik.Context()
    context.push('num')
    f = mapnik.Feature(context,0)
    f["num"] = "1.0000"
    eq_(f["num"],"1.0000")
    expr = mapnik.Expression("[num].match('.*(^0|00)$')")
    eq_(expr.evaluate(f),True)

def test_creation_of_null_value():
    context = mapnik.Context()
    context.push('nv')
    f = mapnik.Feature(context,0)
    f["nv"] = None
    eq_(f["nv"],None)
    eq_(f["nv"] is None,True)
    # test boolean
    f["nv"] = 0
    eq_(f["nv"],0)
    eq_(f["nv"] is not None,True)

def test_creation_of_bool():
    context = mapnik.Context()
    context.push('bool')
    f = mapnik.Feature(context,0)
    f["bool"] = True
    eq_(f["bool"],True)
    # TODO - will become int of 1 do to built in boost python conversion
    # is this fixable?
    eq_(isinstance(f["bool"],bool) or isinstance(f["bool"],int),True)
    f["bool"] = False
    eq_(f["bool"],False)
    eq_(isinstance(f["bool"],bool) or isinstance(f["bool"],int),True)
    # test NoneType
    f["bool"] = None
    eq_(f["bool"],None)
    eq_(isinstance(f["bool"],bool) or isinstance(f["bool"],int),False)
    # test integer
    f["bool"] = 0
    eq_(f["bool"],0)
    # ugh, boost_python's built into converter does not work right
    #eq_(isinstance(f["bool"],bool),False)

null_equality = [
  ['hello',False,unicode],
  [u'',False,unicode],
  [0,False,int],
  [123,False,int],
  [0.0,False,float],
  [123.123,False,float],
  [.1,False,float],
  [False,False,int], # TODO - should become bool
  [True,False,int], # TODO - should become bool
  [None,True,None],
  [2147483648,False,int],
  [922337203685477580,False,int]
]

def test_expressions_with_null_equality():
    for eq in null_equality:
        context = mapnik.Context()
        f = mapnik.Feature(context,0)
        f["prop"] = eq[0]
        eq_(f["prop"],eq[0])
        if eq[0] is None:
            eq_(f["prop"] is None, True)
        else:
            eq_(isinstance(f['prop'],eq[2]),True,'%s is not an instance of %s' % (f['prop'],eq[2]))
        expr = mapnik.Expression("[prop] = null")
        eq_(expr.evaluate(f),eq[1])
        expr = mapnik.Expression("[prop] is null")
        eq_(expr.evaluate(f),eq[1])

def test_expressions_with_null_equality2():
    for eq in null_equality:
        context = mapnik.Context()
        f = mapnik.Feature(context,0)
        f["prop"] = eq[0]
        eq_(f["prop"],eq[0])
        if eq[0] is None:
            eq_(f["prop"] is None, True)
        else:
            eq_(isinstance(f['prop'],eq[2]),True,'%s is not an instance of %s' % (f['prop'],eq[2]))
        # TODO - support `is not` syntax:
        # https://github.com/mapnik/mapnik/issues/796
        expr = mapnik.Expression("not [prop] is null")
        eq_(expr.evaluate(f),not eq[1])
        # https://github.com/mapnik/mapnik/issues/1642
        expr = mapnik.Expression("[prop] != null")
        eq_(expr.evaluate(f),not eq[1])

truthyness = [
  [u'hello',True,unicode],
  [u'',False,unicode],
  [0,False,int],
  [123,True,int],
  [0.0,False,float],
  [123.123,True,float],
  [.1,True,float],
  [False,False,int], # TODO - should become bool
  [True,True,int], # TODO - should become bool
  [None,False,None],
  [2147483648,True,int],
  [922337203685477580,True,int]
]

def test_expressions_for_thruthyness():
    context = mapnik.Context()
    for eq in truthyness:
        f = mapnik.Feature(context,0)
        f["prop"] = eq[0]
        eq_(f["prop"],eq[0])
        if eq[0] is None:
            eq_(f["prop"] is None, True)
        else:
            eq_(isinstance(f['prop'],eq[2]),True,'%s is not an instance of %s' % (f['prop'],eq[2]))
        expr = mapnik.Expression("[prop]")
        eq_(expr.to_bool(f),eq[1])
        expr = mapnik.Expression("not [prop]")
        eq_(expr.to_bool(f),not eq[1])
        expr = mapnik.Expression("! [prop]")
        eq_(expr.to_bool(f),not eq[1])
    # also test if feature does not have property at all
    f2 = mapnik.Feature(context,1)
    # no property existing will return value_null since
    # https://github.com/mapnik/mapnik/commit/562fada9d0f680f59b2d9f396c95320a0d753479#include/mapnik/feature.hpp
    eq_(f2["prop"] is None,True)
    expr = mapnik.Expression("[prop]")
    eq_(expr.evaluate(f2),None)
    eq_(expr.to_bool(f2),False)

if __name__ == "__main__":
    [eval(run)() for run in dir() if 'test_' in run]
