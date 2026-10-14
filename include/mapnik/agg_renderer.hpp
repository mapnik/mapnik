/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_AGG_RENDERER_HPP
#define MAPNIK_AGG_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp> // for MAPNIK_DECL
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/util/noncopyable.hpp>  // for noncopyable
#include <mapnik/rule.hpp>              // for rule, symbolizers
#include <mapnik/geometry/box2d.hpp>    // for box2d
#include <mapnik/view_transform.hpp>    // for view_transform
#include <mapnik/image_compositing.hpp> // for composite_mode_e
#include <mapnik/pixel_position.hpp>
#include <mapnik/request.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/image_util.hpp>
// stl
#include <cstdint>
#include <cstring>
#include <memory>
#include <stack>
#include <vector>

// fwd declaration to avoid dependence on agg headers
namespace agg {
struct trans_affine;
}

// fwd declarations to speed up compile
namespace mapnik {
class Map;
class feature_impl;
class feature_type_style;
class label_collision_detector4;
class layer;
class color;
struct marker;
class proj_transform;
struct rasterizer;
struct rgba8_t;
template<typename T>
class image;
} // namespace mapnik

namespace mapnik {

// Buffers for styles and layers that are composited. A buffer starts out transparent only on the
// rows that are clean: the others are cleared when a style first draws on them (the rasterizer
// clears the rows it sweeps), or all at once by clean_top() before anything else reads or draws.
template<typename T>
class buffer_stack
{
  public:
    buffer_stack(std::size_t width, std::size_t height)
        : width_(width),
          height_(height),
          buffers_(),
          position_(buffers_.begin())
    {}

    T& push()
    {
        if (position_ == buffers_.begin())
        {
            buffers_.emplace_front(width_, height_);
            position_ = buffers_.begin();
        }
        else
        {
            --position_;
            if (position_->image.painted())
            {
                // cleared again as it is drawn on
                std::fill(position_->clean.begin(), position_->clean.end(), 0);
            }
        }
        position_->image.painted(false);
        return position_->image;
    }
    bool in_range() const { return (position_ != buffers_.end()); }

    void pop()
    {
        // ^ ensure iterator is not out-of-range
        // prior calling this method
        ++position_;
    }

    T& top() const { return position_->image; }

    // per row of top(), whether it is transparent
    std::uint8_t* top_clean() const { return position_->clean.data(); }

    // Makes every row of top() transparent.
    void clean_top() const
    {
        T& image = position_->image;
        std::uint8_t* clean = position_->clean.data();
        for (std::size_t y = 0; y < image.height(); ++y)
        {
            if (!clean[y])
            {
                std::memset(image.bytes() + y * image.row_size(), 0, image.row_size());
                clean[y] = 1;
            }
        }
    }

  private:
    struct entry
    {
        entry(std::size_t width, std::size_t height)
            : image(width, height, false),
              clean(height, 0)
        {}
        T image;
        std::vector<std::uint8_t> clean;
    };
    std::size_t const width_;
    std::size_t const height_;
    std::deque<entry> buffers_;
    typename std::deque<entry>::iterator position_;
};

template<typename T0, typename T1 = label_collision_detector4>
class MAPNIK_DECL agg_renderer : public feature_style_processor<agg_renderer<T0>>,
                                 private util::noncopyable
{
  public:
    using buffer_type = T0;
    using processor_impl_type = agg_renderer<T0>;
    using detector_type = T1;
    // create with default, empty placement detector
    agg_renderer(Map const& m,
                 buffer_type& pixmap,
                 double scale_factor = 1.0,
                 unsigned offset_x = 0,
                 unsigned offset_y = 0);
    // create with external placement detector, possibly non-empty
    agg_renderer(Map const& m,
                 buffer_type& pixmap,
                 std::shared_ptr<detector_type> detector,
                 double scale_factor = 1.0,
                 unsigned offset_x = 0,
                 unsigned offset_y = 0);
    // pass in mapnik::request object to provide the mutable things per render
    agg_renderer(Map const& m,
                 request const& req,
                 attributes const& vars,
                 buffer_type& pixmap,
                 double scale_factor = 1.0,
                 unsigned offset_x = 0,
                 unsigned offset_y = 0);
    ~agg_renderer();
    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay, box2d<double> const& query_extent);
    void end_layer_processing(layer const& lay);

    void start_style_processing(feature_type_style const& st);
    void end_style_processing(feature_type_style const& st);

    void render_marker(pixel_position const& pos,
                       marker const& marker,
                       agg::trans_affine const& tr,
                       double opacity,
                       composite_mode_e comp_op);

    void process(point_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(line_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(line_pattern_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(polygon_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(polygon_pattern_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(raster_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(shield_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(text_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(building_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(markers_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(group_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);
    void process(debug_symbolizer const& sym, feature_impl& feature, proj_transform const& prj_trans);
    void process(dot_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans);

    inline bool process(rule::symbolizers const& symbolizers, mapnik::feature_impl&, proj_transform const&)
    {
        // Line and polygon symbolizers draw only through the rasterizer, which notes whether it
        // rendered anything; the others may draw into the style's buffer in other ways.
        if (!composited_ || drew_otherwise_)
        {
            // matters only for a composited style's buffer, until one has
            return false;
        }
        for (symbolizer const& sym : symbolizers)
        {
            // polygon patterns, too, are drawn only through the rasterizer (their scanlines' spans)
            if (!sym.is<line_symbolizer>() && !sym.is<polygon_symbolizer>() && !sym.is<polygon_pattern_symbolizer>())
            {
                drew_otherwise();
                break;
            }
        }
        // agg renderer doesn't support processing of multiple symbolizers.
        return false;
    }

    void painted(bool painted);
    bool painted();

    // Notes a compositing mode something is about to blend with: an opaque image stays opaque
    // under most, and if it does under all of them, there is nothing to demultiply at the end.
    inline void blends_with(composite_mode_e mode)
    {
        switch (mode)
        {
            case clear:
            case src:
            case src_in:
            case dst_in:
            case src_out:
            case dst_out:
            case dst_atop:
            case _xor:
                opaque_ = false;
                break;
            default:
                break;
        }
    }

    inline eAttributeCollectionPolicy attribute_collection_policy() const { return DEFAULT; }

    inline double scale_factor() const { return common_.scale_factor_; }

    inline attributes const& variables() const { return common_.vars_; }

  protected:
    template<typename R>
    void debug_draw_box(R& buf, box2d<double> const& extent, double x, double y, double angle = 0.0);
    void debug_draw_box(box2d<double> const& extent, double x, double y, double angle = 0.0);
    void draw_geo_extent(box2d<double> const& extent, mapnik::color const& color);

  private:
    std::stack<std::reference_wrapper<buffer_type>> buffers_;
    buffer_stack<buffer_type> internal_buffers_;
    std::unique_ptr<buffer_type> inflated_buffer_;
    std::unique_ptr<rasterizer> const ras_ptr;
    gamma_method_enum gamma_method_;
    double gamma_;
    // every pixel of the map has been opaque since the background was filled (see blends_with)
    bool opaque_ = false;
    renderer_common common_;
    // the style being processed draws into a buffer of its own, composited when it ends
    bool composited_ = false;
    // a symbolizer of the style being processed may have drawn other than through ras_ptr
    bool drew_otherwise_ = false;
    // the style's buffer is cleared row by row as ras_ptr sweeps it
    bool lazy_rows_ = false;
    // Notes that the style's symbolizers draw other than through ras_ptr, before they do.
    void drew_otherwise();
    void setup(Map const& m, buffer_type& pixmap);
};

extern template class MAPNIK_DECL agg_renderer<image<rgba8_t>>;

} // namespace mapnik

#endif // MAPNIK_AGG_RENDERER_HPP
