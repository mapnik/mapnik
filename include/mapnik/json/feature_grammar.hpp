/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_FEATURE_GRAMMAR_HPP
#define MAPNIK_FEATURE_GRAMMAR_HPP

// mapnik
#include <mapnik/geometry.hpp>

// spirit::qi
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

// stl
#include <iostream>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

class attribute_value_visitor
    : public boost::static_visitor<mapnik::value>
{
public:
    attribute_value_visitor(mapnik::transcoder const& tr)
        : tr_(tr) {}

    mapnik::value operator()(std::string const& val) const
    {
        return mapnik::value(tr_.transcode(val.c_str()));
    }

    template <typename T>
    mapnik::value operator()(T const& val) const
    {
        return mapnik::value(val);
    }
    mapnik::transcoder const& tr_;
};

struct put_property
{
    template <typename T0,typename T1, typename T2>
    struct result
    {
        typedef void type;
    };
    explicit put_property(mapnik::transcoder const& tr)
        : tr_(tr) {}

    template <typename T0,typename T1, typename T2>
    void operator() (T0 & feature, T1 const& key, T2 const& val) const
    {
        mapnik::value v = boost::apply_visitor(attribute_value_visitor(tr_),val); // TODO: optimize
        feature.put_new(key, v);
    }

    mapnik::transcoder const& tr_;
};

struct extract_geometry
{
    template <typename T>
    struct result
    {
        typedef boost::ptr_vector<mapnik::geometry_type>& type;
    };

    template <typename T>
    boost::ptr_vector<mapnik::geometry_type>& operator() (T & feature) const
    {
        return feature.paths();
    }
};

struct push_vertex
{
    template <typename T0,typename T1, typename T2, typename T3>
    struct result
    {
        typedef void type;
    };

    template <typename T0,typename T1, typename T2, typename T3>
    void operator() (T0 c, T1 path, T2 x, T3 y) const
    {
        BOOST_ASSERT( path!=0 );
        path->push_vertex(x,y,c);
    }
};

struct cleanup
{
    template <typename T0>
    struct result
    {
        typedef void type;
    };

    template <typename T0>
    void operator() (T0 & path) const
    {
        if (path) delete path, path=0;
    }
};

template <typename Iterator, typename FeatureType>
struct feature_grammar :
    qi::grammar<Iterator, void(FeatureType&),
                space_type>
{
    feature_grammar(mapnik::transcoder const& tr)
        : feature_grammar::base_type(feature,"feature"),
          put_property_(put_property(tr))
    {
        using qi::lit;
        using qi::int_;
        using qi::double_;
#if BOOST_VERSION > 104200
        using qi::no_skip;
#else
        using qi::lexeme;
#endif
        using standard_wide::char_;
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::_3;
        using qi::_4;
        using qi::_a;
        using qi::_b;
        using qi::_r1;
        using qi::_r2;
        using qi::fail;
        using qi::on_error;
        using qi::_pass;
        using qi::eps;
        using qi::raw;
        
        using phoenix::new_;
        using phoenix::push_back;
        using phoenix::construct;

        // generic json types
        value =  object | array | string_
            | number
            ;

        pairs = key_value % lit(',')
            ;

        key_value = (string_ >> lit(':') >> value)
            ;

        object = lit('{')
            >> *pairs
            >> lit('}')
            ;
        array = lit('[')
            >> value >> *(lit(',') >> value)
            >> lit(']')
            ;

        number %= strict_double
            | int_
            | lit("true") [_val = true]
            | lit ("false") [_val = false]
            | lit("null")[_val = construct<value_null>()]
            ;

        unesc_char.add
            ("\\\"", '\"') // quotation mark
            ("\\\\", '\\') // reverse solidus
            ("\\/", '/')   // solidus            
            ("\\b", '\b')  // backspace
            ("\\f", '\f')  // formfeed
            ("\\n", '\n')  // newline
            ("\\r", '\r')  // carrige return
            ("\\t", '\t')  // tab
            ;

        string_ %= lit('"') >> *(unesc_char | "\\u" >> hex4 | (char_ - lit('"'))) >> lit('"')
            ;
        
        // geojson types

        feature_type = lit("\"type\"")
            >> lit(':')
            >> lit("\"Feature\"")
            ;

        feature = lit('{')
            >> (feature_type | (lit("\"geometry\"") > lit(':') > geometry(_r1)) | properties(_r1) | key_value) % lit(',')
            >> lit('}')
            ;

        properties = lit("\"properties\"")
            >> lit(':') >> (lit('{') >>  attributes(_r1) >> lit('}')) | lit("null")
            ;

        attributes = (string_ [_a = _1] >> lit(':') >> attribute_value [put_property_(_r1,_a,_1)]) % lit(',')
            ;

        attribute_value %= number | string_  ;

        // Nabialek trick - FIXME: how to bind argument to dispatch rule?
        // geometry = lit("\"geometry\"")
        //    >> lit(':') >> lit('{')
        //    >> lit("\"type\"") >> lit(':') >> geometry_dispatch[_a = _1]
        //    >> lit(',') >> lit("\"coordinates\"") >> lit(':')
        //    >> qi::lazy(*_a)
        //    >> lit('}')
        //    ;
        // geometry_dispatch.add
        //    ("\"Point\"",&point_coordinates)
        //    ("\"LineString\"",&linestring_coordinates)
        //    ("\"Polygon\"",&polygon_coordinates)
        //    ;
        //////////////////////////////////////////////////////////////////

        geometry = (lit('{')[_a = 0 ] 
                    >> lit("\"type\"") >> lit(':') >> geometry_dispatch[_a = _1] // <---- should be Nabialek trick!
                    >> lit(',') 
                    >> (lit("\"coordinates\"") > lit(':') > coordinates(_r1,_a)
                        | 
                        lit("\"geometries\"") > lit(':') 
                        >> lit('[') >> geometry_collection(_r1) >> lit(']'))
                    >> lit('}'))
            | lit("null")
            ;
                
        geometry_dispatch.add
            ("\"Point\"",1)
            ("\"LineString\"",2)
            ("\"Polygon\"",3)
            ("\"MultiPoint\"",4)
            ("\"MultiLineString\"",5)
            ("\"MultiPolygon\"",6)
            ("\"GeometryCollection\"",7)
            //
            ;
        
        coordinates = (eps(_r2 == 1) > point_coordinates(extract_geometry_(_r1)))
            | (eps(_r2 == 2) > linestring_coordinates(extract_geometry_(_r1)))
            | (eps(_r2 == 3) > polygon_coordinates(extract_geometry_(_r1)))
            | (eps(_r2 == 4) > multipoint_coordinates(extract_geometry_(_r1)))
            | (eps(_r2 == 5) > multilinestring_coordinates(extract_geometry_(_r1)))
            | (eps(_r2 == 6) > multipolygon_coordinates(extract_geometry_(_r1)))
            ;
        
        point_coordinates =  eps[ _a = new_<geometry_type>(Point) ]
            > ( point(SEG_MOVETO,_a) [push_back(_r1,_a)] | eps[cleanup_(_a)][_pass = false] )
            ;

        linestring_coordinates = eps[ _a = new_<geometry_type>(LineString)]
            > (points(_a) [push_back(_r1,_a)]
               | eps[cleanup_(_a)][_pass = false])
            ;

        polygon_coordinates = eps[ _a = new_<geometry_type>(Polygon) ]
            > ((lit('[')
                > points(_a) % lit(',')
                > lit(']')) [push_back(_r1,_a)]
               | eps[cleanup_(_a)][_pass = false])
            ;
        
        multipoint_coordinates =  lit('[') 
            > (point_coordinates(_r1) % lit(',')) 
            > lit(']')
            ;
        
        multilinestring_coordinates =  lit('[') 
            > (linestring_coordinates(_r1) % lit(','))
            > lit(']')
            ;
        
        multipolygon_coordinates =  lit('[') 
            > (polygon_coordinates(_r1) % lit(','))
            > lit(']')
            ;

        geometry_collection = *geometry(_r1) >> *(lit(',') >> geometry(_r1))
            ;
        
        // point
        point = (lit('[') > double_ > lit(',') > double_ > lit(']')) [push_vertex_(_r1,_r2,_1,_2)];
        // points
        points = lit('[')[_a = SEG_MOVETO] > point (_a,_r1) % lit(',') [_a = SEG_LINETO] > lit(']');

        on_error<fail>
            (
                feature
                , std::clog
                << phoenix::val("Error! Expecting ")
                << _4                               // what failed?
                << phoenix::val(" here: \"")
                << construct<std::string>(_3, _2)   // iterators to error-pos, end
                << phoenix::val("\"")
                << std::endl
                );

    }

    // start
    // generic JSON
    qi::rule<Iterator,space_type> value;
    qi::symbols<char const, char const> unesc_char;
    qi::uint_parser< unsigned, 16, 4, 4 > hex4 ;
    qi::rule<Iterator,std::string(), space_type> string_;
    qi::rule<Iterator,space_type> key_value;
    qi::rule<Iterator,boost::variant<value_null,bool,int,double>(),space_type> number;
    qi::rule<Iterator,space_type> object;
    qi::rule<Iterator,space_type> array;
    qi::rule<Iterator,space_type> pairs;
    qi::real_parser<double, qi::strict_real_policies<double> > strict_double;

    // geoJSON
    qi::rule<Iterator,void(FeatureType&),space_type> feature; // START
    qi::rule<Iterator,space_type> feature_type;

    // Nabialek trick //////////////////////////////////////
    //typedef typename qi::rule<Iterator,void(FeatureType &), space_type> dispatch_rule;
    //qi::rule<Iterator,qi::locals<dispatch_rule*>, void(FeatureType&),space_type> geometry;
    //qi::symbols<char, dispatch_rule*> geometry_dispatch;
    ////////////////////////////////////////////////////////

    qi::rule<Iterator,qi::locals<int>, void(FeatureType&),space_type> geometry;
    qi::symbols<char, int> geometry_dispatch;

    qi::rule<Iterator,void(CommandType,geometry_type*),space_type> point;
    qi::rule<Iterator,qi::locals<CommandType>,void(geometry_type*),space_type> points;
    qi::rule<Iterator,void(FeatureType &,int),space_type> coordinates;
    //
    qi::rule<Iterator,qi::locals<geometry_type*>,
             void(boost::ptr_vector<mapnik::geometry_type>& ),space_type> point_coordinates;
    qi::rule<Iterator,qi::locals<geometry_type*>,
             void(boost::ptr_vector<mapnik::geometry_type>& ),space_type> linestring_coordinates;
    qi::rule<Iterator,qi::locals<geometry_type*>,
             void(boost::ptr_vector<mapnik::geometry_type>& ),space_type> polygon_coordinates;    

    qi::rule<Iterator,void(boost::ptr_vector<mapnik::geometry_type>& ),space_type> multipoint_coordinates;
    qi::rule<Iterator,void(boost::ptr_vector<mapnik::geometry_type>& ),space_type> multilinestring_coordinates;
    qi::rule<Iterator,void(boost::ptr_vector<mapnik::geometry_type>& ),space_type> multipolygon_coordinates;
    qi::rule<Iterator,void(FeatureType&),space_type> geometry_collection;

    qi::rule<Iterator,void(FeatureType &),space_type> properties;
    qi::rule<Iterator,qi::locals<std::string>, void(FeatureType &),space_type> attributes;
    qi::rule<Iterator,boost::variant<value_null,bool,int,double,std::string>(), space_type> attribute_value;

    phoenix::function<put_property> put_property_;
    phoenix::function<extract_geometry> extract_geometry_;
    boost::phoenix::function<push_vertex> push_vertex_;
    boost::phoenix::function<cleanup> cleanup_;

};

}}

#endif // MAPNIK_FEATURE_GRAMMAR_HPP
