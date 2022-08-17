// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2013 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2013 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2013 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.
// Copyright (c) 2017 Artem Pavlenko, UK

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_CLOSEST_POINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_CLOSEST_POINT_HPP

#include <boost/mpl/if.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/static_assert.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/algorithms/not_implemented.hpp>
#include <boost/geometry/algorithms/detail/throw_on_empty_input.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/default_distance_result.hpp>

#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/extensions/strategies/cartesian/closest_point.hpp>

#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace closest_point {


template <typename Point1, typename Point2>
struct point_point
{
    template <typename Strategy, typename Result>
    static inline void apply(Point1 const& point1, Point2 const& point2, Strategy const& strategy, Result& result)
    {
        result.distance
            = strategy.apply_point_point(point1, point2);
        geometry::convert(point2, result.closest_point);
    }
};

template <typename Point, typename Range>
struct point_range
{
    template <typename Strategy, typename Result>
    static inline void apply(Point const& point, Range const& range, Strategy const& strategy, Result& result)
    {
        // This should not occur (see exception on empty input below)
        if (boost::begin(range) == boost::end(range))
        {
            return;
        }

        // line of one point: same case as point-point
        typedef typename boost::range_const_iterator<Range>::type iterator_type;
        iterator_type it = boost::begin(range);
        iterator_type prev = it++;
        if (it == boost::end(range))
        {
            point_point<Point, typename boost::range_value<Range const>::type>::apply(point, *prev, strategy, result);
            return;
        }

        // Initialize with first segment
        strategy.apply(point, *prev, *it, result);

        // Check other segments
        for(prev = it++; it != boost::end(range); prev = it++)
        {
            Result other;
            strategy.apply(point, *prev, *it, other);
            if (other.distance < result.distance)
            {
                result = other;
            }
        }
    }
};




}} // namespace detail::closest_point
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Geometry1, typename Geometry2,
    typename Tag1 = typename tag<Geometry1>::type,
    typename Tag2 = typename tag<Geometry2>::type,
    bool Reverse = reverse_dispatch<Geometry1, Geometry2>::type::value
>
struct closest_point : not_implemented<Tag1, Tag2>
{
};


template
<
    typename Geometry1, typename Geometry2,
    typename Tag1, typename Tag2
>
struct closest_point<Geometry1, Geometry2, Tag1, Tag2, true>
{
    template <typename Strategy, typename Result>
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
                    Strategy const& strategy, Result& result)
    {
        // Reversed version just calls dispatch with reversed arguments
        closest_point
            <
                Geometry2, Geometry1, Tag2, Tag1, false
            >::apply(geometry2, geometry1, strategy, result);
    }

};


template<typename Point1, typename Point2>
struct closest_point
    <
        Point1, Point2,
        point_tag, point_tag, false
    > : public detail::closest_point::point_point<Point1, Point2>
{};


template<typename Point, typename Segment>
struct closest_point
    <
        Point, Segment,
        point_tag, segment_tag,
        false
    >
{
    template <typename Strategy, typename Result>
    static inline void apply(Point const& point, Segment const& segment,
                    Strategy const& strategy, Result& result)
    {

        typename point_type<Segment>::type p[2];
        geometry::detail::assign_point_from_index<0>(segment, p[0]);
        geometry::detail::assign_point_from_index<1>(segment, p[1]);

        strategy.apply(point, p[0], p[1], result);
    }
};


template <typename Point, typename Ring>
struct closest_point<Point, Ring, point_tag, ring_tag, false>
    : detail::closest_point::point_range<Point, Ring>
{};

//
template<typename Point, typename Linestring>
struct closest_point
    <
        Point, Linestring,
        point_tag, linestring_tag,
        false
    >
    : detail::closest_point::point_range<Point, Linestring>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



template <typename Geometry1, typename Geometry2, typename Result>
inline void closest_point(Geometry1 const& geometry1, Geometry2 const& geometry2, Result& result)
{
    concepts::check<Geometry1 const>();
    concepts::check<Geometry2 const>();
    concepts::check<typename Result::point_type>();

    assert_dimension_equal<Geometry1, Geometry2>();
    assert_dimension_equal<Geometry1, typename Result::point_type>();

    detail::throw_on_empty_input(geometry1);
    detail::throw_on_empty_input(geometry2);

    strategy::distance::calculate_closest_point<> info_strategy;


    dispatch::closest_point
            <
                Geometry1,
                Geometry2
            >::apply(geometry1, geometry2, info_strategy, result);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_CLOSEST_POINT_HPP
