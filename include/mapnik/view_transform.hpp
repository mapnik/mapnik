/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_VIEW_TRANSFORM_HPP
#define MAPNIK_VIEW_TRANSFORM_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/proj_transform.hpp>

// stl
#include <cstddef>

namespace mapnik
{

template <typename Transform, typename Geometry>
struct MAPNIK_DECL coord_transform
{
    // SFINAE value_type detector
    template <typename T>
    struct void_type
    {
        using type = void;
    };

    template <typename T, typename D, typename _ = void>
    struct select_value_type
    {
        using type = D;
    };

    template <typename T, typename D>
    struct select_value_type<T, D, typename void_type<typename T::value_type>::type>
    {
        using type = typename T::value_type;
    };

    using size_type = std::size_t;
    using value_type = typename select_value_type<Geometry, void>::type;

    coord_transform(Transform const& t,
                     Geometry & geom,
                     proj_transform const& prj_trans)
        : t_(&t),
        geom_(geom),
        prj_trans_(&prj_trans)  {}

    explicit coord_transform(Geometry & geom)
        : t_(0),
        geom_(geom),
        prj_trans_(0)  {}

    void set_proj_trans(proj_transform const& prj_trans)
    {
        prj_trans_ = &prj_trans;
    }

    void set_trans(Transform  const& t)
    {
        t_ = &t;
    }

    unsigned vertex(double *x, double *y) const
    {
        unsigned command;
        bool ok = false;
        bool skipped_points = false;
        while (!ok)
        {
            command = geom_.vertex(x,y);
            if (command == SEG_END)
            {
                return command;
            }
            double z=0;
            ok = prj_trans_->backward(*x, *y, z);
            if (!ok) {
                skipped_points = true;
            }
        }
        if (skipped_points && (command == SEG_LINETO))
        {
            command = SEG_MOVETO;
        }
        t_->forward(x,y);
        return command;
    }

    void rewind(unsigned pos) const
    {
        geom_.rewind(pos);
    }

    unsigned type() const
    {
        return static_cast<unsigned>(geom_.type());
    }

    Geometry const& geom() const
    {
        return geom_;
    }

private:
    Transform const* t_;
    Geometry & geom_;
    proj_transform const* prj_trans_;
};

class view_transform
{
private:
    const int width_;
    const int height_;
    const box2d<double> extent_;
    const double sx_;
    const double sy_;
    const double offset_x_;
    const double offset_y_;
    int offset_;
public:

    view_transform(int width, int height, box2d<double> const& extent,
                   double offset_x = 0.0, double offset_y = 0.0)
        : width_(width),
          height_(height),
          extent_(extent),
          sx_(extent_.width() > 0 ? static_cast<double>(width_) / extent_.width() : 1.0),
          sy_(extent_.height() > 0 ? static_cast<double>(height_) / extent_.height() : 1.0),
          offset_x_(offset_x),
          offset_y_(offset_y),
          offset_(0) {}

    view_transform(view_transform const&) = default;

    inline int offset() const
    {
        return offset_;
    }

    inline void set_offset(int offset)
    {
        offset_ = offset;
    }

    inline double offset_x() const
    {
        return offset_x_;
    }

    inline double offset_y() const
    {
        return offset_y_;
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
        *x = (*x - extent_.minx()) * sx_ - (offset_x_ - offset_);
        *y = (extent_.maxy() - *y) * sy_ - (offset_y_ - offset_);
    }

    inline void backward(double *x, double *y) const
    {
        *x = extent_.minx() + (*x + (offset_x_ - offset_)) / sx_;
        *y = extent_.maxy() - (*y + (offset_y_ - offset_)) / sy_;
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

    inline box2d<double> forward(box2d<double> const& e,
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

    inline box2d<double> forward(box2d<double> const& e) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        forward(&x0, &y0);
        forward(&x1, &y1);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline box2d<double> backward(box2d<double> const& e,
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

    inline box2d<double> backward(box2d<double> const& e) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        backward(&x0, &y0);
        backward(&x1, &y1);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline box2d<double> const& extent() const
    {
        return extent_;
    }
};
}

#endif // MAPNIK_VIEW_TRANSFORM_HPP
