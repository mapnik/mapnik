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

#include <boost/version.hpp>

#if BOOST_VERSION >= 104700

#include <mapnik/geometry.hpp>
#include <mapnik/util/geometry_wkt_generator.hpp>
#include <mapnik/util/path_iterator.hpp>
#include <mapnik/util/container_adapter.hpp>

namespace mapnik { namespace util {

template <typename T>
boost::tuple<unsigned,bool> detail::multi_geometry_type<T>::operator() (T const& geom) const
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
    return boost::tuple<unsigned,bool>(type, collection);
}

template <typename OutputIterator, typename Geometry>
wkt_generator<OutputIterator, Geometry>::wkt_generator(bool single)
    : wkt_generator::base_type(wkt)
{
    using boost::spirit::karma::uint_;
    using boost::spirit::karma::_val;
    using boost::spirit::karma::_1;
    using boost::spirit::karma::lit;
    using boost::spirit::karma::_a;
    using boost::spirit::karma::_r1;
    using boost::spirit::karma::eps;
    using boost::spirit::karma::string;

    wkt = point | linestring | polygon
        ;

    point = &uint_(mapnik::Point)[_1 = _type(_val)]
        << string[ phoenix::if_ (single) [_1 = "Point("]
                   .else_[_1 = "("]]
        << point_coord [_1 = _first(_val)] << lit(')')
        ;

    linestring = &uint_(mapnik::LineString)[_1 = _type(_val)]
        << string[ phoenix::if_ (single) [_1 = "LineString("]
                   .else_[_1 = "("]]
        << coords
        << lit(')')
        ;

    polygon = &uint_(mapnik::Polygon)[_1 = _type(_val)]
        << string[ phoenix::if_ (single) [_1 = "Polygon("]
                   .else_[_1 = "("]]
        << coords2
        << lit("))")
        ;

    point_coord = &uint_ << coordinate << lit(' ') << coordinate
        ;

    polygon_coord %= ( &uint_(mapnik::SEG_MOVETO) << eps[_r1 += 1]
                       << string[ if_ (_r1 > 1) [_1 = "),("]
                                  .else_[_1 = "("] ] | &uint_ << ",")
        << coordinate
        << lit(' ')
        << coordinate
        ;

    coords2 %= *polygon_coord(_a)
        ;

    coords = point_coord % lit(',')
        ;
}

template <typename OutputIterator, typename GeometryContainer>
wkt_multi_generator<OutputIterator, GeometryContainer>::wkt_multi_generator()
    : wkt_multi_generator::base_type(wkt)
{
    using boost::spirit::karma::lit;
    using boost::spirit::karma::eps;
    using boost::spirit::karma::_val;
    using boost::spirit::karma::_1;
    using boost::spirit::karma::_a;

    geometry_types.add
        (mapnik::Point,"Point")
        (mapnik::LineString,"LineString")
        (mapnik::Polygon,"Polygon")
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


}}

#endif
