// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2013 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2013 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2013 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.
// Copyright (c) 2017 Artem Pavlenko, UK

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGY_CARTESIAN_CLOSEST_POINT_HPP
#define BOOST_GEOMETRY_STRATEGY_CARTESIAN_CLOSEST_POINT_HPP

#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>

#include <boost/geometry/strategies/tags.hpp>
#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/default_distance_result.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>

#include <boost/geometry/util/select_coordinate_type.hpp>

#include <boost/geometry/algorithms/detail/overlay/segment_identifier.hpp>


namespace boost { namespace geometry
{

template <typename Point>
struct closest_point_result
{
    typedef Point point_type;
    typedef typename default_distance_result<Point>::type distance_type;
    distance_type distance;
    Point closest_point;

    inline closest_point_result()
        : distance(distance_type())
    {}
};


namespace strategy { namespace distance
{

template
<
    typename CalculationType = void,
    typename Strategy = pythagoras<CalculationType>
>
struct calculate_closest_point
{
public :
    // The three typedefs below are necessary to calculate distances
    // from segments defined in integer coordinates.

    // Integer coordinates can still result in FP distances.
    // There is a division, which must be represented in FP.
    // So promote.
    template <typename Point, typename PointOfSegment>
    struct calculation_type
        : promote_floating_point
          <
              typename strategy::distance::services::return_type
                  <
                      Strategy,
                      Point,
                      PointOfSegment
                  >::type
          >
    {};


public :

    // Helper function
    template <typename Point1, typename Point2>
    inline typename calculation_type<Point1, Point2>::type
    apply_point_point(Point1 const& p1, Point2 const& p2) const
    {
        Strategy point_point_strategy;
        boost::ignore_unused_variable_warning(point_point_strategy);
        return point_point_strategy.apply(p1, p2);
    }

    template <typename Point, typename PointOfSegment, typename Result>
    inline void apply(Point const& p,
                    PointOfSegment const& p1, PointOfSegment const& p2,
                    Result& result) const
    {
        assert_dimension_equal<Point, PointOfSegment>();

        typedef typename calculation_type<Point, PointOfSegment>::type calculation_type;

        //// A projected point of points in Integer coordinates must be able to be
        //// represented in FP.
        typedef model::point
            <
                calculation_type,
                dimension<PointOfSegment>::value,
                typename coordinate_system<PointOfSegment>::type
            > fp_point_type;

        // For convenience
        typedef fp_point_type fp_vector_type;
        fp_vector_type v, w;

        geometry::convert(p2, v);
        geometry::convert(p, w);
        subtract_point(v, p1);
        subtract_point(w, p1);

        calculation_type const zero = calculation_type();

        calculation_type const c1 = dot_product(w, v);
        calculation_type const c2 = dot_product(v, v);

        if (c1 < zero)
        {
            result.distance = apply_point_point(p, p1);
            result.closest_point = p1;
        }
        else if(c1 > c2)
        {
            result.distance = apply_point_point(p, p2);
            result.closest_point = p2;
        }
        else
        {
            Strategy point_point_strategy;
            boost::ignore_unused_variable_warning(point_point_strategy);

            if (geometry::math::equals(c2, zero))
            {
                geometry::convert(p1, result.closest_point);
                result.distance = apply_point_point(p, p1);
                return;
            }

            calculation_type const b = c1 / c2;
            geometry::convert(p1, result.closest_point);
            multiply_value(v, b);
            add_point(result.closest_point, v);
            result.distance = apply_point_point(p, result.closest_point);
        }
    }
};

}} // namespace strategy::distance


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGY_CARTESIAN_CLOSEST_POINT_HPP
