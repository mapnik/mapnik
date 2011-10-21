/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id$

#ifndef CTRANS_HPP
#define CTRANS_HPP

#include <algorithm>

#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/coord_array.hpp>
#include <mapnik/proj_transform.hpp>

namespace mapnik
{

typedef coord_array<coord2d> CoordinateArray;


template <typename Transform, typename Geometry>
struct MAPNIK_DECL coord_transform
{
    coord_transform(Transform const& t, Geometry& geom)
        : t_(t), geom_(geom) {}

    unsigned vertex(double *x, double *y) const
    {
        unsigned command = geom_.vertex(x, y);
        t_.forward(x, y);
        return command;
    }

    void rewind(unsigned pos)
    {
        geom_.rewind(pos);
    }

private:
    Transform const& t_;
    Geometry& geom_;
};


template <typename Transform, typename Geometry>
struct MAPNIK_DECL coord_transform2
{
    typedef std::size_t size_type;
    typedef typename Geometry::value_type value_type;

    coord_transform2(Transform const& t,
                     Geometry const& geom,
                     proj_transform const& prj_trans)
        : t_(t),
        geom_(geom),
        prj_trans_(prj_trans)  {}

    unsigned vertex(double *x, double *y) const
    {
        unsigned command = SEG_MOVETO;
        bool ok = false;
        bool skipped_points = false;
        double z = 0;
        while (!ok && command != SEG_END)
        {
            command = geom_.vertex(x, y);
            ok = prj_trans_.backward(*x, *y, z);
            if (!ok) {
                skipped_points = true;
            }
        }
        if (skipped_points && (command == SEG_LINETO))
        {
            command = SEG_MOVETO;
        }
        t_.forward(x, y);
        return command;
    }

    void rewind(unsigned pos)
    {
        geom_.rewind(pos);
    }

    Geometry const& geom() const
    {
        return geom_;
    }

private:
    Transform const& t_;
    Geometry const& geom_;
    proj_transform const& prj_trans_;
};


template <typename Transform, typename Geometry>
struct MAPNIK_DECL coord_transform3
{
    coord_transform3(Transform const& t,
                     Geometry const& geom,
                     proj_transform const& prj_trans,
                     int dx, int dy)
        : t_(t),
        geom_(geom),
        prj_trans_(prj_trans),
        dx_(dx), dy_(dy) {}

    unsigned vertex(double *x, double *y) const
    {
        unsigned command = geom_.vertex(x, y);
        double z = 0;
        prj_trans_.backward(*x, *y, z);
        t_.forward(x, y);
        *x += dx_;
        *y += dy_;
        return command;
    }

    void rewind(unsigned pos)
    {
        geom_.rewind(pos);
    }

private:
    Transform const& t_;
    Geometry const& geom_;
    proj_transform const& prj_trans_;
    int dx_;
    int dy_;
};


class CoordTransform
{
private:
    int width_;
    int height_;
    double sx_;
    double sy_;
    box2d<double> extent_;
    double offset_x_;
    double offset_y_;

public:
    CoordTransform(int width, int height, const box2d<double>& extent,
                   double offset_x = 0, double offset_y = 0)
        : width_(width), height_(height), extent_(extent),
        offset_x_(offset_x), offset_y_(offset_y)
    {
        sx_ = double(width_) / extent_.width();
        sy_ = double(height_) / extent_.height();
    }

    inline int width() const
    {
        return width_;
    }

    inline int height() const
    {
        return height_;
    }

    inline double scale_x() const
    {
        return sx_;
    }

    inline double scale_y() const
    {
        return sy_;
    }

    inline void forward(double *x, double *y) const
    {
        *x = (*x - extent_.minx()) * sx_ - offset_x_;
        *y = (extent_.maxy() - *y) * sy_ - offset_y_;
    }

    inline void backward(double *x, double *y) const
    {
        *x = extent_.minx() + (*x + offset_x_) / sx_;
        *y = extent_.maxy() - (*y + offset_y_) / sy_;
    }

    inline coord2d& forward(coord2d& c) const
    {
        forward(&c.x, &c.y);
        return c;
    }

    inline coord2d& backward(coord2d& c) const
    {
        backward(&c.x, &c.y);
        return c;
    }

    inline box2d<double> forward(const box2d<double>& e,
                                 proj_transform const& prj_trans) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        double z = 0.0;
        prj_trans.backward(x0, y0, z);
        forward(&x0, &y0);
        prj_trans.backward(x1, y1, z);
        forward(&x1, &y1);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline box2d<double> forward(const box2d<double>& e) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        forward(&x0, &y0);
        forward(&x1, &y1);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline box2d<double> backward(const box2d<double>& e,
                                  proj_transform const& prj_trans) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        double z = 0.0;
        backward(&x0, &y0);
        prj_trans.forward(x0, y0, z);
        backward(&x1, &y1);
        prj_trans.forward(x1, y1, z);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline box2d<double> backward(const box2d<double>& e) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        backward(&x0, &y0);
        backward(&x1, &y1);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline CoordinateArray& forward(CoordinateArray& coords) const
    {
        for (unsigned i = 0; i < coords.size(); ++i)
        {
            forward(coords[i]);
        }
        return coords;
    }

    inline CoordinateArray& backward(CoordinateArray& coords) const
    {
        for (unsigned i = 0; i < coords.size(); ++i)
        {
            backward(coords[i]);
        }
        return coords;
    }

    inline box2d<double> const& extent() const
    {
        return extent_;
    }
};
}

#endif //CTRANS_HPP
