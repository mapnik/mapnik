/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include <mapnik/json/topojson_grammar.hpp>
#include <mapnik/json/generic_json.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#pragma GCC diagnostic pop

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::coordinate,
    (double, x)
    (double, y)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::arc,
    (std::list<mapnik::topojson::coordinate>, coordinates)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::transform,
    (double, scale_x)
    (double, scale_y)
    (double, translate_x)
    (double, translate_y)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::bounding_box,
    (double, minx)
    (double, miny)
    (double, maxx)
    (double, maxy)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::topology,
    (std::vector<mapnik::topojson::geometry>, geometries)
    (std::vector<mapnik::topojson::arc>, arcs)
    (boost::optional<mapnik::topojson::transform>, tr)
    (boost::optional<mapnik::topojson::bounding_box>, bbox)
   )



namespace mapnik { namespace topojson {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

template <typename Iterator, typename ErrorHandler>
topojson_grammar<Iterator, ErrorHandler>::topojson_grammar()
    : topojson_grammar::base_type(topology, "topojson")
{
    qi::lit_type lit;
    qi::double_type double_;
    qi::int_type int_;
    qi::omit_type omit;
    qi::_val_type _val;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_r1_type _r1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_c_type _c;
    qi::_d_type _d;
    using qi::fail;
    using qi::on_error;
    using phoenix::push_back;
    using phoenix::construct;

    geometry_type_dispatch.add
        ("\"Point\"",1)
        ("\"LineString\"",2)
        ("\"Polygon\"",3)
        ("\"MultiPoint\"",4)
        ("\"MultiLineString\"",5)
        ("\"MultiPolygon\"",6)
        ("\"GeometryCollection\"",7)
        ;

    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;
    boost::phoenix::function<create_geometry_impl> const create_geometry;
    // generic JSON types
    json.value = json.object | json.array | json.string_ | json.number
        ;

    json.key_value = json.string_ > lit(':') > json.value
        ;

    json.object = lit('{')
        > -(json.key_value % lit(','))
        > lit('}')
        ;

    json.array = lit('[')
        > -(json.value % lit(','))
        > lit(']')
        ;

    json.number = json.strict_double[_val = json.double_converter(_1)]
        | json.int__[_val = json.integer_converter(_1)]
        | lit("true")[_val = true]
        | lit("false")[_val = false]
        | lit("null")[_val = construct<value_null>()]
        ;

    // topo json
    topology = lit('{') >> lit("\"type\"") >> lit(':') >> lit("\"Topology\"")
                        >> ( (lit(',') >> objects ) ^ ( lit(',') >> arcs) ^ (lit(',') >> transform) ^ (lit(',') >> bbox))
                        >> lit('}')
        ;

    transform = lit("\"transform\"") >> lit(':') >> lit('{')
                                     >> lit("\"scale\"") >> lit(':')
                                     >> lit('[')
                                     >> double_ >> lit(',')
                                     >> double_ >> lit(']') >> lit(',')
                                     >> lit("\"translate\"") >> lit(':')
                                     >> lit('[') >> double_ >> lit(',') >> double_ >> lit(']')
                                     >> lit('}')
        ;

    bbox = lit("\"bbox\"") >> lit(':')
                           >> lit('[') >> double_ >> lit(',') >> double_
                           >> lit(',') >> double_ >> lit(',') >> double_
                           >> lit(']')
        ;

    objects = lit("\"objects\"")
        >> lit(':')
        >> lit('{')
        >> -((omit[json.string_]
              >> lit(':')
              >>  (geometry_collection(_val) | geometry[push_back(_val, _1)]) % lit(',')))
        >> lit('}')
        ;

    geometry = lit('{')[_a = 0]
        > ((lit("\"type\"") > lit(':') > geometry_type_dispatch[_a = _1])
           |
           (lit("\"coordinates\"") > lit(':') > coordinates[_b = _1])
           |
           (lit("\"arcs\"") > lit(':') > rings_array[_c = _1])
           |
           properties_[_d = _1]
           |
           json.key_value) % lit(',')
        > lit('}')[_val = create_geometry(_a, _b, _c, _d)]
        ;


    geometry_collection =  lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"GeometryCollection\"")
        >> lit(',') >> lit("\"geometries\"") >> lit(':')
        >> lit('[')
        >> -(geometry[push_back(_r1, _1)] % lit(','))
        >> lit(']')
        >> lit('}')
        ;

    ring = lit('[') >> -(int_ % lit(',')) >> lit(']')
        ;
    rings = lit('[') >> -(ring % lit(',')) >> lit(']')
        ;
    rings_array = lit('[') >> -(rings % lit(',')) >> lit(']')
        |
        rings
        |
        ring
        ;

    properties_ = lit("\"properties\"")
        >> lit(':')
        >> lit('{') >> (json.string_ >> lit(':') >> json.value) % lit(',') >> lit('}')
        ;

    arcs = lit("\"arcs\"") >> lit(':')
                           >> lit('[') >> -( arc % lit(',')) >> lit(']') ;

    arc = lit('[') >> -(coordinate_ % lit(',')) >> lit(']') ;

    coordinate_ = lit('[') > double_ > lit(',') > double_ > lit(']');

    coordinates = (lit('[') >> coordinate_ % lit(',') > lit(']'))
        | coordinate_;

    topology.name("topology");
    transform.name("transform");
    objects.name("objects");
    arc.name("arc");
    arcs.name("arcs");
    json.value.name("value");
    coordinate_.name("coordinate");
    geometry.name("geometry");
    properties_.name("properties");
    geometry_collection.name("geometry_collection");
    // error handler
    on_error<fail>(topology, error_handler(_1, _2, _3, _4));
}

}}
