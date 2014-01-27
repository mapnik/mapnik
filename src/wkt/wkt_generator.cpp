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

#include <mapnik/geometry.hpp>
#include <mapnik/util/geometry_wkt_generator.hpp>
#include <mapnik/util/geometry_to_wkt.hpp>
#include <mapnik/util/path_iterator.hpp>
#include <mapnik/util/container_adapter.hpp>

namespace mapnik { namespace util {

template <typename T>
std::tuple<unsigned,bool> detail::multi_geometry_type<T>::operator() (T const& geom) const
{
    typedef T geometry_container;
    unsigned type = 0u;
    bool collection = false;

    typename geometry_container::const_iterator itr = geom.begin();
    typename geometry_container::const_iterator end = geom.end();

    for ( ; itr != end; ++itr)
    {
        if (type != 0 && itr->type() != type)
        {
            collection = true;
            break;
        }
        type = itr->type();
    }
    return std::tuple<unsigned,bool>(type, collection);
}

template <typename OutputIterator, typename Geometry>
wkt_generator<OutputIterator, Geometry>::wkt_generator(bool single)
    : wkt_generator::base_type(wkt)
{
    boost::spirit::karma::uint_type uint_;
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::_a_type _a;
    boost::spirit::karma::_b_type _b;
    boost::spirit::karma::_c_type _c;
    boost::spirit::karma::_r1_type _r1;
    boost::spirit::karma::eps_type eps;
    boost::spirit::karma::string_type kstring;

    wkt = point | linestring | polygon
        ;

    point = &uint_(mapnik::geometry_type::types::Point)[_1 = _type(_val)]
        << kstring[ phoenix::if_ (single) [_1 = "Point("]
                   .else_[_1 = "("]]
        << point_coord [_1 = _first(_val)] << lit(')')
        ;

    linestring = &uint_(mapnik::geometry_type::types::LineString)[_1 = _type(_val)]
        << kstring[ phoenix::if_ (single) [_1 = "LineString("]
                   .else_[_1 = "("]]
        << coords
        << lit(')')
        ;

    polygon = &uint_(mapnik::geometry_type::types::Polygon)[_1 = _type(_val)]
        << kstring[ phoenix::if_ (single) [_1 = "Polygon("]
                   .else_[_1 = "("]]
        << coords2
        << lit("))")
        ;

    point_coord = &uint_ << coordinate << lit(' ') << coordinate
        ;

    polygon_coord %= ( &uint_(mapnik::SEG_MOVETO)
                       << eps[_r1 += 1][_a  = _x(_val)][ _b = _y(_val)]
                       << kstring[ if_ (_r1 > 1) [_1 = "),("]
                                  .else_[_1 = "("]]
                       |
                       &uint_(mapnik::SEG_LINETO)
                       << lit(',') << eps[_a = _x(_val)][_b = _y(_val)]
        )
        << coordinate[_1 = _a]
        << lit(' ')
        << coordinate[_1 = _b]
        ;

    coords2 %= *polygon_coord(_a,_b,_c)
        ;

    coords = point_coord % lit(',')
        ;
}

template <typename OutputIterator, typename GeometryContainer>
wkt_multi_generator<OutputIterator, GeometryContainer>::wkt_multi_generator()
    : wkt_multi_generator::base_type(wkt)
{
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::eps_type eps;
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::_a_type _a;

    geometry_types.add
        (mapnik::geometry_type::types::Point,"Point")
        (mapnik::geometry_type::types::LineString,"LineString")
        (mapnik::geometry_type::types::Polygon,"Polygon")
        ;

    wkt =  eps(phoenix::at_c<1>(_a))[_a = _multi_type(_val)]
        << lit("GeometryCollection(") << geometry << lit(")")
        | eps(is_multi(_val)) << lit("Multi") << geometry_types[_1 = phoenix::at_c<0>(_a)]
        << "(" << multi_geometry << ")"
        |  geometry
        ;

    geometry =  -(single_geometry % lit(','))
        ;

    single_geometry = geometry_types[_1 = _type(_val)] << path
        ;

    multi_geometry = -(path % lit(','))
        ;

}

template struct mapnik::util::wkt_generator<std::back_insert_iterator<std::string>, mapnik::geometry_type>;
template struct mapnik::util::wkt_multi_generator<std::back_insert_iterator<std::string>, mapnik::geometry_container >;

bool to_wkt(std::string & wkt, mapnik::geometry_type const& geom)
{
    typedef std::back_insert_iterator<std::string> sink_type;
    sink_type sink(wkt);
    wkt_generator<sink_type, mapnik::geometry_type> generator(true);
    bool result = karma::generate(sink, generator, geom);
    return result;
}

bool to_wkt(std::string & wkt, mapnik::geometry_container const& geom)
{
    typedef std::back_insert_iterator<std::string> sink_type;
    sink_type sink(wkt);
    wkt_multi_generator<sink_type, mapnik::geometry_container> generator;
    bool result = karma::generate(sink, generator, geom);
    return result;
}

}}
