/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef RASTER_FEATURESET_HPP
#define RASTER_FEATURESET_HPP

#include "raster_datasource.hpp"
#include "raster_info.hpp"

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/debug.hpp>

// stl
#include <vector>

// boost
#include <boost/utility.hpp>

class single_file_policy
{
    raster_info info_;

  public:
    class const_iterator
    {
        enum iterator_e { start, end };
        bool status_;
        const single_file_policy* p_;

      public:
        explicit const_iterator(const single_file_policy* p)
            : status_(start)
            , p_(p)
        {}

        const_iterator()
            : status_(end)
        {}

        const_iterator(const const_iterator& other)
            : status_(other.status_)
            , p_(other.p_)
        {}

        const_iterator& operator++()
        {
            status_ = end;
            return *this;
        }

        const raster_info& operator*() const { return p_->info_; }

        const raster_info* operator->() const { return &(p_->info_); }

        bool operator!=(const const_iterator& itr) { return status_ != itr.status_; }
    };

    explicit single_file_policy(const raster_info& info)
        : info_(info)
    {}

    const_iterator begin() { return const_iterator(this); }

    const_iterator query(const box2d<double>& box)
    {
        if (box.intersects(info_.envelope()))
        {
            return begin();
        }
        return end();
    }

    const_iterator end() { return const_iterator(); }

    inline int img_width(int reader_width) const { return reader_width; }

    inline int img_height(int reader_height) const { return reader_height; }

    inline box2d<double> transform(box2d<double>&) const { return box2d<double>(0, 0, 0, 0); }
};

class tiled_file_policy
{
  public:

    using const_iterator = std::vector<raster_info>::const_iterator;

    tiled_file_policy(std::string const& file,
                      std::string const& format,
                      unsigned tile_size,
                      box2d<double> const& extent,
                      box2d<double> const& bbox,
                      unsigned width,
                      unsigned height)
    {
        double lox = extent.minx();
        double loy = extent.miny();

        int max_x = int(std::ceil(double(width) / double(tile_size)));
        int max_y = int(std::ceil(double(height) / double(tile_size)));

        double pixel_x = extent.width() / double(width);
        double pixel_y = extent.height() / double(height);

        MAPNIK_LOG_DEBUG(raster) << "tiled_file_policy: Raster Plugin PIXEL SIZE(" << pixel_x << "," << pixel_y << ")";

        box2d<double> e = bbox.intersect(extent);

        for (int x = 0; x < max_x; ++x)
        {
            for (int y = 0; y < max_y; ++y)
            {
                double x0 = lox + x * tile_size * pixel_x;
                double y0 = loy + y * tile_size * pixel_y;
                double x1 = x0 + tile_size * pixel_x;
                double y1 = y0 + tile_size * pixel_y;

                if (e.intersects(box2d<double>(x0, y0, x1, y1)))
                {
                    box2d<double> tile_box = e.intersect(box2d<double>(x0, y0, x1, y1));
                    raster_info info(file, format, tile_box, tile_size, tile_size);
                    infos_.push_back(info);
                }
            }
        }

        MAPNIK_LOG_DEBUG(raster) << "tiled_file_policy: Raster Plugin INFO SIZE=" << infos_.size() << " " << file;
    }

    const_iterator begin() { return infos_.begin(); }

    const_iterator end() { return infos_.end(); }

    inline int img_width(int reader_width) const { return reader_width; }

    inline int img_height(int reader_height) const { return reader_height; }

    inline box2d<double> transform(box2d<double>&) const { return box2d<double>(0, 0, 0, 0); }

  private:

    std::vector<raster_info> infos_;
};

class tiled_multi_file_policy
{
  public:

    using const_iterator = std::vector<raster_info>::const_iterator;

    tiled_multi_file_policy(std::string const& file_pattern,
                            std::string const& format,
                            unsigned tile_size,
                            box2d<double> extent,
                            box2d<double> bbox,
                            unsigned width,
                            unsigned height,
                            unsigned tile_stride)
        : image_width_(width)
        , image_height_(height)
        , tile_size_(tile_size)
        , tile_stride_(tile_stride)
    {
        double lox = extent.minx();
        double loy = extent.miny();

        // int max_x = int(std::ceil(double(width) / double(tile_size)));
        // int max_y = int(std::ceil(double(height) / double(tile_size)));

        double pixel_x = extent.width() / double(width);
        double pixel_y = extent.height() / double(height);

        MAPNIK_LOG_DEBUG(raster) << "tiled_multi_file_policy: Raster Plugin PIXEL SIZE(" << pixel_x << "," << pixel_y
                                 << ")";

        // intersection of query with extent => new query
        box2d<double> e = bbox.intersect(extent);

        const int x_min = int(std::floor((e.minx() - lox) / (tile_size * pixel_x)));
        const int y_min = int(std::floor((e.miny() - loy) / (tile_size * pixel_y)));
        const int x_max = int(std::ceil((e.maxx() - lox) / (tile_size * pixel_x)));
        const int y_max = int(std::ceil((e.maxy() - loy) / (tile_size * pixel_y)));

        for (int x = x_min; x < x_max; ++x)
        {
            for (int y = y_min; y < y_max; ++y)
            {
                // x0, y0, x1, y1 => projection-space image coordinates.
                double x0 = lox + x * tile_size * pixel_x;
                double y0 = loy + y * tile_size * pixel_y;
                double x1 = x0 + tile_size * pixel_x;
                double y1 = y0 + tile_size * pixel_y;

                // check if it intersects the query
                if (e.intersects(box2d<double>(x0, y0, x1, y1)))
                {
                    // tile_box => intersection of tile with query in projection-space.
                    box2d<double> tile_box = e.intersect(box2d<double>(x0, y0, x1, y1));
                    std::string file = interpolate(file_pattern, x, y);
                    raster_info info(file, format, tile_box, tile_size, tile_size);
                    infos_.push_back(info);
                }
            }
        }

        MAPNIK_LOG_DEBUG(raster) << "tiled_multi_file_policy: Raster Plugin INFO SIZE=" << infos_.size() << " "
                                 << file_pattern;
    }

    const_iterator begin() { return infos_.begin(); }

    const_iterator end() { return infos_.end(); }

    inline int img_width(int) const { return image_width_; }

    inline int img_height(int) const { return image_height_; }

    inline box2d<double> transform(box2d<double>& box) const
    {
        int x_offset = int(std::floor(box.minx() / tile_size_));
        int y_offset = int(std::floor(box.miny() / tile_size_));
        box2d<double> rem(x_offset * tile_size_, y_offset * tile_size_, x_offset * tile_size_, y_offset * tile_size_);
        box.init(box.minx() - rem.minx(), box.miny() - rem.miny(), box.maxx() - rem.maxx(), box.maxy() - rem.maxy());
        return rem;
    }

  private:

    std::string interpolate(std::string const& pattern, int x, int y) const;

    unsigned int image_width_, image_height_, tile_size_, tile_stride_;
    std::vector<raster_info> infos_;
};

template<typename LookupPolicy>
class raster_featureset : public mapnik::Featureset
{
    using iterator_type = typename LookupPolicy::const_iterator;

  public:
    raster_featureset(LookupPolicy const& policy, box2d<double> const& exttent, mapnik::query const& q);
    virtual ~raster_featureset();
    mapnik::feature_ptr next();

  private:
    LookupPolicy policy_;
    mapnik::value_integer feature_id_;
    mapnik::context_ptr ctx_;
    mapnik::box2d<double> extent_;
    mapnik::box2d<double> bbox_;
    iterator_type curIter_;
    iterator_type endIter_;
    double filter_factor_;
};

#endif // RASTER_FEATURESET_HPP
