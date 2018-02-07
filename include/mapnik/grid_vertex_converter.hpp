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

#ifndef MAPNIK_GRID_ADAPTERS_HPP
#define MAPNIK_GRID_ADAPTERS_HPP

// mapnik
#include <mapnik/vertex.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/geometry/polygon_vertex_processor.hpp>
#include <mapnik/geometry/envelope.hpp>
#include <mapnik/geometry/interior.hpp>
#include <mapnik/view_strategy.hpp>
#include <mapnik/vertex_adapters.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_gray.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_bin.h"
#include "agg_conv_transform.h"

namespace mapnik { namespace geometry {

// Generates integer coordinates of a spiral similar to the Ulam spiral
// around [0, 0], bounded by size.
class spiral_iterator
{
public:
    spiral_iterator(unsigned size)
        : end_(size * size),
          i_(0),
          x_(0), y_(0)
    {
    }

    bool vertex(int * x, int * y)
    {
        if (i_ < end_)
        {
            *x = x_;
            *y = y_;

            if (std::abs(x_) <= std::abs(y_) && (x_ != y_ || x_ >= 0))
            {
                x_ += ((y_ >= 0) ? 1 : -1);
            }
            else
            {
                y_ += ((x_ >= 0) ? -1 : 1);
            }

            ++i_;

            return true;
        }
        return false;
    }

    void rewind()
    {
        i_ = x_ = y_ = 0;
    }

private:
    const unsigned end_;
    unsigned i_;
    int x_, y_;
};

struct view_transform_agg_adapter
{
    void transform(double * x, double * y) const
    {
        vt.forward(x, y);
    }

    view_transform const& vt;
};

// Generates grid of points laying inside a polygon.
template <typename PathType, typename T, bool Alternating = false>
struct grid_vertex_converter
{
    grid_vertex_converter(PathType & path, T dx, T dy, double scale_factor)
        : grid_vertex_converter(cache_path(path), dx, dy, scale_factor)
    {
    }

    void rewind(unsigned)
    {
        si_.rewind();
    }

    unsigned vertex(T * x, T * y)
    {
        int spiral_x, spiral_y;
        while (si_.vertex(&spiral_x, &spiral_y))
        {
            T pix_x = interior_.x + spiral_x * dx_;
            T pix_y = interior_.y + spiral_y * dy_;

            if (Alternating && spiral_y % 2 != 0)
            {
                // Every odd line is shifted by dx/2.
                pix_x += this->dx_ / 2.0;
            }

            if (pix_x >= 0 && static_cast<std::size_t>(pix_x) < hit_bitmap_.width() &&
                pix_y >= 0 && static_cast<std::size_t>(pix_y) < hit_bitmap_.height() &&
                get_pixel<image_gray8::pixel_type>(hit_bitmap_, pix_x, pix_y))
            {
                *x = pix_x;
                *y = pix_y;
                vt_.backward(x, y);
                return mapnik::SEG_MOVETO;
            }
        }
        return mapnik::SEG_END;
    }

    geometry_types type() const
    {
        return geometry_types::MultiPoint;
    }

private:
    grid_vertex_converter(polygon<T> const& poly, T dx, T dy, double scale_factor)
        : grid_vertex_converter(poly, dx, dy, scale_factor, mapnik::geometry::envelope(poly))
    {
    }

    grid_vertex_converter(polygon<T> const& poly, T dx, T dy, double scale_factor, box2d<T> const& envelope)
        : hit_bitmap_scale_(get_hit_bitmap_scale(envelope)),
          dx_(dx * hit_bitmap_scale_),
          dy_(dy * hit_bitmap_scale_),
          vt_(envelope.valid() ? (envelope.width() * hit_bitmap_scale_) : 0,
              envelope.valid() ? (envelope.height() * hit_bitmap_scale_) : 0, envelope),
          hit_bitmap_(create_hit_bitmap(poly)),
          interior_(interior(poly, envelope, scale_factor)),
          si_(std::max(std::ceil((hit_bitmap_.width() + std::abs((hit_bitmap_.width() / 2.0) - interior_.x) * 2.0) / dx_),
                       std::ceil((hit_bitmap_.height() + std::abs((hit_bitmap_.height() / 2.0) - interior_.y) * 2.0) / dy_)))
    {
    }

    double get_hit_bitmap_scale(box2d<T> const& envelope) const
    {
        T size = envelope.width() * envelope.height();
        // Polygon with huge area can lead to excessive memory allocation.
        // This is more or less arbitrarily chosen limit for the maximum bitmap resolution.
        // Bitmap bigger than this limit is scaled down to fit into this resolution.
        const std::size_t max_size = 8192 * 8192;
        if (size > max_size)
        {
            return std::sqrt(max_size / size);
        }
        return 1;
    }

    // The polygon is rendered to a bitmap for fast hit-testing.
    image_gray8 create_hit_bitmap(polygon<T> const& poly) const
    {
        polygon_vertex_adapter<T> va(poly);
        view_transform_agg_adapter vta{ vt_ };
        agg::conv_transform<polygon_vertex_adapter<T>, view_transform_agg_adapter> tp(va, vta);
        tp.rewind(0);
        agg::rasterizer_scanline_aa<> ras;
        ras.add_path(tp);

        image_gray8 hit_bitmap(vt_.width(), vt_.height());
        agg::rendering_buffer buf(hit_bitmap.data(),
                                  hit_bitmap.width(),
                                  hit_bitmap.height(),
                                  hit_bitmap.row_size());
        agg::pixfmt_gray8 pixfmt(buf);
        using renderer_base = agg::renderer_base<agg::pixfmt_gray8>;
        using renderer_bin = agg::renderer_scanline_bin_solid<renderer_base>;
        renderer_base rb(pixfmt);
        renderer_bin ren_bin(rb);
        ren_bin.color(agg::gray8(1));
        agg::scanline_bin sl_bin;
        agg::render_scanlines(ras, sl_bin, ren_bin);

        return hit_bitmap;
    }

    mapnik::geometry::point<T> interior(polygon<T> const& poly,
                                        box2d<T> const& envelope,
                                        double scale_factor) const
    {
        mapnik::geometry::point<T> interior;
        if (!mapnik::geometry::interior(poly, scale_factor, interior))
        {
            auto center = envelope.center();
            interior.x = center.x;
            interior.y = center.y;
        }

        vt_.forward(&interior.x, &interior.y);

        return interior;
    }

    polygon<T> cache_path(PathType & path) const
    {
        mapnik::geometry::polygon_vertex_processor<T> vertex_processor;
        path.rewind(0);
        vertex_processor.add_path(path);
        return vertex_processor.polygon_;
    }

    const double hit_bitmap_scale_;
    const T dx_, dy_;
    const view_transform vt_;
    const image_gray8 hit_bitmap_;
    const mapnik::geometry::point<T> interior_;
    spiral_iterator si_;
};

template <typename PathType, typename T>
using regular_grid_vertex_converter = grid_vertex_converter<PathType, T, false>;

template <typename PathType, typename T>
using alternating_grid_vertex_converter = grid_vertex_converter<PathType, T, true>;

}
}

#endif //MAPNIK_GRID_ADAPTERS_HPP
