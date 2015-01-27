/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_POLYGON_CLIPPER_HPP
#define MAPNIK_POLYGON_CLIPPER_HPP

// stl
#include <iostream>
#include <iomanip>
#include <deque>

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/util/boost_geometry_adapters.hpp>

// boost
#include <boost/geometry.hpp>

namespace mapnik {

using namespace boost::geometry;

template <typename Geometry>
struct polygon_clipper
{
    using point_2d = mapnik::coord2d;
    using polygon_2d = model::polygon<mapnik::coord2d>;
    using polygon_list = std::deque<polygon_2d>;

    enum
    {
        clip = 1,
        no_clip = 2,
        ignore = 3

    } state_;

    polygon_clipper(Geometry & geom)
        : state_(clip),
          clip_box_(),
          geom_(geom)
    {

    }

    polygon_clipper(box2d<double> const& clip_box, Geometry & geom)
        :state_(clip),
         clip_box_(clip_box),
         geom_(geom)
    {
        init();
    }

    void set_clip_box(box2d<double> const& clip_box)
    {
        state_ = clip;
        clip_box_ = clip_box;
        init();
    }

    unsigned type() const
    {
        return geom_.type();
    }

    void rewind(unsigned path_id)
    {
        if (state_ == clip) output_.rewind(path_id);
        else geom_.rewind(path_id);
    }

    unsigned vertex (double * x, double * y)
    {
        switch (state_)
        {
        case clip:
            return output_.vertex(x,y);
        case no_clip:
            return geom_.vertex(x,y);
        case ignore:
            return SEG_END;
        }
        return SEG_END;
    }

private:

    void init()
    {
        geom_.rewind(0);
        box2d<double> bbox = geom_.envelope();
        if (clip_box_.contains(bbox))
        {
            // shortcut to original geometry (no-clipping)
            state_ = no_clip;
            return;
        }
        else if (!clip_box_.intersects(bbox))
        {
            // polygon is outside of clipping box
            state_ = ignore;
            return;
        }

        polygon_2d subject_poly;
        double x = 0;
        double y = 0;
        double prev_x = 0;
        double prev_y = 0;
        geom_.rewind(0);
        unsigned ring_count = 0;
        while (true)
        {
            unsigned cmd = geom_.vertex(&x,&y);
            if (cmd == SEG_END) break;
            if (cmd == SEG_MOVETO)
            {
                prev_x = x;
                prev_y = y;
                if (ring_count == 0)
                {
                    append(subject_poly, make<point_2d>(x,y));
                }
                else
                {
                    subject_poly.inners().push_back(polygon_2d::inner_container_type::value_type());
                    append(subject_poly.inners().back(),make<point_2d>(x,y));
                }
                ++ring_count;
            }
            else if (cmd == SEG_LINETO)
            {
                if (std::abs(x - prev_x) < 1e-12 && std::abs(y - prev_y) < 1e-12)
                {
#ifdef MAPNIK_LOG
                    MAPNIK_LOG_WARN(polygon_clipper)
                        << std::setprecision(12) << "coincident vertices:(" << prev_x << ","
                        <<  prev_y << ") , (" << x << "," << y <<  ")";
#endif
                    continue;
                }
                prev_x = x;
                prev_y = y;
                if (ring_count == 1)
                {
                    append(subject_poly, make<point_2d>(x,y));
                }
                else
                {
                    append(subject_poly.inners().back(),make<point_2d>(x,y));
                }
            }
        }

        polygon_list clipped_polygons;
#ifdef MAPNIK_LOG
        double area = boost::geometry::area(subject_poly);
        if (area < 0)
        {
            MAPNIK_LOG_ERROR(polygon_clipper) << "negative area detected for polygon indicating incorrect winding order";
        }
#endif
        try
        {
            boost::geometry::intersection(clip_box_, subject_poly, clipped_polygons);
        }
        catch (boost::geometry::exception const& ex)
        {
            std::cerr << ex.what() << std::endl;
        }

        for (polygon_2d const& poly : clipped_polygons)
        {
            bool move_to = true;
            for (point_2d const& c : boost::geometry::exterior_ring(poly))
            {
                if (move_to)
                {
                    move_to = false;
                    output_.move_to(c.x,c.y);
                }
                else
                {
                    output_.line_to(c.x,c.y);
                }
            }
            output_.close_path();
            // interior rings
            for (polygon_2d::inner_container_type::value_type const& ring : boost::geometry::interior_rings(poly))
            {
                move_to = true;
                for (point_2d const& c : ring)
                {
                    if (move_to)
                    {
                        move_to = false;
                        output_.move_to(c.x,c.y);
                    }
                    else
                    {
                        output_.line_to(c.x,c.y);
                    }
                }
                output_.close_path();
            }
        }
    }

    box2d<double> clip_box_;
    Geometry & geom_;
    mapnik::geometry_type output_;

};

}

#endif //MAPNIK_POLYGON_CLIPPER_HPP
