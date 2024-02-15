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

#ifndef MAPNIK_SVG_RENDERER_AGG_HPP
#define MAPNIK_SVG_RENDERER_AGG_HPP

// mapnik
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/gradient.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/util/noncopyable.hpp>

#if defined(GRID_RENDERER)
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <mapnik/grid/grid_pixel.hpp>
MAPNIK_DISABLE_WARNING_POP
#endif

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_pixfmt_rgba.h"
#include "agg_path_storage.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_color_rgba.h"
#include "agg_bounding_rect.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_scanline_p.h"
#include "agg_scanline_bin.h"
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_span_gradient.h"
#include "agg_gradient_lut.h"
#include "agg_gamma_lut.h"
#include "agg_span_interpolator_linear.h"
MAPNIK_DISABLE_WARNING_POP

#include <ostream>

namespace mapnik {
namespace svg {

// Arbitrary linear gradient specified by two control points. Gradient
// value is taken as the normalised distance along the line segment
// represented by the two points.

class linear_gradient_from_segment
{
  public:
    linear_gradient_from_segment(double x1, double y1, double x2, double y2)
        : x1_(x1 * static_cast<double>(agg::gradient_subpixel_scale))
        , y1_(y1 * static_cast<double>(agg::gradient_subpixel_scale))
        , x2_(x2 * static_cast<double>(agg::gradient_subpixel_scale))
        , y2_(y2 * static_cast<double>(agg::gradient_subpixel_scale))
    {
        double dx = x2_ - x1_;
        double dy = y2_ - y1_;
        length_sqr_ = dx * dx + dy * dy;
    }

    int calculate(int x, int y, int d) const
    {
        if (length_sqr_ <= 0)
            return 0;
        double u = ((x - x1_) * (x2_ - x1_) + (y - y1_) * (y2_ - y1_)) / length_sqr_;
        if (u < 0)
            u = 0;
        else if (u > 1)
            u = 1;
        return static_cast<int>(u * d);
    }

  private:
    double x1_;
    double y1_;
    double x2_;
    double y2_;

    double length_sqr_;
};

template<typename VertexSource, typename AttributeSource, typename ScanlineRenderer, typename PixelFormat>
class renderer_agg : util::noncopyable
{
  public:
    using curved_type = agg::conv_curve<VertexSource>;
    // stroke
    using curved_stroked_type = agg::conv_stroke<curved_type>;
    using curved_stroked_trans_type = agg::conv_transform<curved_stroked_type>;
    // stroke dash-array
    using curved_dashed_type = agg::conv_dash<curved_type>;
    using curved_dashed_stroked_type = agg::conv_stroke<curved_dashed_type>;
    using curved_dashed_stroked_trans_type = agg::conv_transform<curved_dashed_stroked_type>;
    // fill
    using curved_trans_type = agg::conv_transform<curved_type>;
    using curved_trans_contour_type = agg::conv_contour<curved_trans_type>;
    // renderer
    using renderer_base = agg::renderer_base<PixelFormat>;
    using vertex_source_type = VertexSource;
    using attribute_source_type = AttributeSource;

    // mapnik::svg_path_adapter, mapnik::svg_attribute_type, renderer_solid, agg::pixfmt_rgba32_pre
    renderer_agg(VertexSource& source, group const& svg_group)
        : source_(source)
        , curved_(source_)
        , curved_dashed_(curved_)
        , curved_stroked_(curved_)
        , curved_dashed_stroked_(curved_dashed_)
        , svg_group_(svg_group)
    {}

    template<typename Rasterizer, typename Scanline, typename Renderer>
    void render(Rasterizer& ras,
                Scanline& sl,
                Renderer& ren,
                agg::trans_affine const& mtx,
                double opacity,
                box2d<double> const& symbol_bbox)

    {
        double adjusted_opacity = opacity * svg_group_.opacity; // adjust top level opacity
        if (adjusted_opacity < 1.0)
        {
            mapnik::image_rgba8 im(ren.width(), ren.height(), true, true);
            agg::rendering_buffer buf(im.bytes(), im.width(), im.height(), im.row_size());
            PixelFormat pixf(buf);
            Renderer ren_g(pixf);
            for (auto const& elem : svg_group_.elements)
            {
                mapbox::util::apply_visitor(
                  group_renderer<Rasterizer, Scanline, Renderer>(*this, ras, sl, ren_g, mtx, symbol_bbox),
                  elem);
            }
            ren.blend_from(ren_g.ren(), 0, 0, 0, unsigned(adjusted_opacity * 255));
        }
        else
        {
            for (auto const& elem : svg_group_.elements)
            {
                mapbox::util::apply_visitor(
                  group_renderer<Rasterizer, Scanline, Renderer>(*this, ras, sl, ren, mtx, symbol_bbox),
                  elem);
            }
        }
    }

    template<typename Rasterizer, typename Scanline, typename Renderer>
    struct group_renderer
    {
        group_renderer(renderer_agg& renderer,
                       Rasterizer& ras,
                       Scanline& sl,
                       Renderer& ren,
                       agg::trans_affine const& mtx,
                       box2d<double> const& symbol_bbox)
            : renderer_(renderer)
            , ras_(ras)
            , sl_(sl)
            , ren_(ren)
            , mtx_(mtx)
            , symbol_bbox_(symbol_bbox)
        {}

        void render_gradient(Rasterizer& ras,
                             Scanline& sl,
                             Renderer& ren,
                             gradient const& grad,
                             agg::trans_affine const& mtx,
                             double opacity,
                             box2d<double> const& symbol_bbox,
                             curved_trans_type& curved_trans,
                             unsigned path_id) const
        {
            using gamma_lut_type = agg::gamma_lut<agg::int8u, agg::int8u>;
            using color_func_type = agg::gradient_lut<agg::color_interpolator<agg::rgba8>, 1024>;
            using interpolator_type = agg::span_interpolator_linear<>;
            using span_allocator_type = agg::span_allocator<agg::rgba8>;

            span_allocator_type m_alloc;
            color_func_type m_gradient_lut;
            gamma_lut_type m_gamma_lut;

            double x1, x2, y1, y2, radius;
            grad.get_control_points(x1, y1, x2, y2, radius);

            m_gradient_lut.remove_all();
            for (mapnik::stop_pair const& st : grad.get_stop_array())
            {
                mapnik::color const& stop_color = st.second;
                unsigned r = stop_color.red();
                unsigned g = stop_color.green();
                unsigned b = stop_color.blue();
                unsigned a = stop_color.alpha();
                m_gradient_lut.add_color(st.first, agg::rgba8_pre(r, g, b, int(a * opacity)));
            }
            if (m_gradient_lut.build_lut())
            {
                agg::trans_affine tr = mtx;
                agg::trans_affine transform = grad.get_transform();
                transform *= tr;
                transform.invert();

                if (grad.get_units() != USER_SPACE_ON_USE)
                {
                    double scale = mtx.scale();
                    double bx1 = symbol_bbox.minx();
                    double by1 = symbol_bbox.miny();
                    double bx2 = symbol_bbox.maxx();
                    double by2 = symbol_bbox.maxy();

                    if (grad.get_units() == OBJECT_BOUNDING_BOX)
                    {
                        bounding_rect_single(curved_trans, path_id, &bx1, &by1, &bx2, &by2);
                    }
                    transform.translate(-bx1 / scale, -by1 / scale);
                    transform.scale(scale / (bx2 - bx1), scale / (by2 - by1));
                }
                else
                {
                    double scaledown = 255;
                    x1 /= scaledown; // fx
                    y1 /= scaledown; // fy
                    x2 /= scaledown; // cx
                    y2 /= scaledown; // cy
                    radius /= scaledown;
                    transform.translate(-symbol_bbox.minx(), -symbol_bbox.miny());
                    transform.scale(1 / scaledown, 1 / scaledown);
                }
                if (grad.get_gradient_type() == RADIAL)
                {
                    using gradient_adaptor_type = agg::gradient_radial_focus;
                    using span_gradient_type =
                      agg::span_gradient<agg::rgba8, interpolator_type, gradient_adaptor_type, color_func_type>;

                    // the agg radial gradient assumes it is centred on 0
                    transform.translate(-x2, -y2);
                    // scale everything up since agg turns things into integers a bit too soon
                    int scaleup = 255;
                    radius *= scaleup;
                    x1 *= scaleup;
                    y1 *= scaleup;
                    x2 *= scaleup;
                    y2 *= scaleup;
                    transform.scale(scaleup, scaleup);

                    interpolator_type span_interpolator(transform);
                    gradient_adaptor_type gradient_adaptor(radius, (x1 - x2), (y1 - y2));

                    span_gradient_type span_gradient(span_interpolator, gradient_adaptor, m_gradient_lut, 0, radius);

                    render_scanlines_aa(ras, sl, ren, m_alloc, span_gradient);
                }
                else
                {
                    using gradient_adaptor_type = linear_gradient_from_segment;
                    using span_gradient_type =
                      agg::span_gradient<agg::rgba8, interpolator_type, gradient_adaptor_type, color_func_type>;
                    // scale everything up since agg turns things into integers a bit too soon
                    int scaleup = 255;
                    x1 *= scaleup;
                    y1 *= scaleup;
                    x2 *= scaleup;
                    y2 *= scaleup;

                    transform.scale(scaleup, scaleup);

                    interpolator_type span_interpolator(transform);
                    gradient_adaptor_type gradient_adaptor(x1, y1, x2, y2);

                    span_gradient_type span_gradient(span_interpolator, gradient_adaptor, m_gradient_lut, 0, scaleup);

                    render_scanlines_aa(ras, sl, ren, m_alloc, span_gradient);
                }
            }
        }

        void operator()(group const& g) const
        {
            double opacity = g.opacity;
            if (opacity < 1.0)
            {
                mapnik::image_rgba8 im(ren_.width(), ren_.height(), true, true);
                agg::rendering_buffer buf(im.bytes(), im.width(), im.height(), im.row_size());
                PixelFormat pixf(buf);
                Renderer ren(pixf);
                for (auto const& elem : g.elements)
                {
                    mapbox::util::apply_visitor(group_renderer(renderer_, ras_, sl_, ren, mtx_, symbol_bbox_), elem);
                }
                ren_.blend_from(ren.ren(), 0, 0, 0, unsigned(opacity * 255));
            }
            else
            {
                for (auto const& elem : g.elements)
                {
                    mapbox::util::apply_visitor(group_renderer(renderer_, ras_, sl_, ren_, mtx_, symbol_bbox_), elem);
                }
            }
        }

        void operator()(path_attributes const& attr) const
        {
            using namespace agg;
            trans_affine transform;

            curved_stroked_trans_type curved_stroked_trans(renderer_.curved_stroked_, transform);
            curved_dashed_stroked_trans_type curved_dashed_stroked_trans(renderer_.curved_dashed_stroked_, transform);
            curved_trans_type curved_trans(renderer_.curved_, transform);
            curved_trans_contour_type curved_trans_contour(curved_trans);
            curved_trans_contour.auto_detect_orientation(true);

            if (!attr.visibility_flag)
                return;

            transform = attr.transform;

            transform *= mtx_;
            double scl = transform.scale();
            // renderer_.curved_.approximation_method(curve_inc);
            renderer_.curved_.approximation_scale(scl);
            renderer_.curved_.angle_tolerance(0.0);

            typename PixelFormat::color_type color;

            if (attr.fill_flag || attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
            {
                ras_.reset();
                // https://github.com/mapnik/mapnik/issues/1129
                if (std::fabs(curved_trans_contour.width()) <= 1)
                {
                    ras_.add_path(curved_trans, attr.index);
                }
                else
                {
                    curved_trans_contour.miter_limit(attr.miter_limit);
                    ras_.add_path(curved_trans_contour, attr.index);
                }

                if (attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
                {
                    render_gradient(ras_,
                                    sl_,
                                    ren_,
                                    attr.fill_gradient,
                                    transform,
                                    attr.fill_opacity,
                                    symbol_bbox_,
                                    curved_trans,
                                    attr.index);
                }
                else
                {
                    ras_.filling_rule(attr.even_odd_flag ? fill_even_odd : fill_non_zero);
                    color = attr.fill_color;
                    color.opacity(color.opacity() * attr.fill_opacity);
                    ScanlineRenderer ren_s(ren_);
                    color.premultiply();
                    ren_s.color(color);
                    render_scanlines(ras_, sl_, ren_s);
                }
            }

            if (attr.stroke_flag || attr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
            {
                if (attr.dash.size() > 0)
                {
                    renderer_.curved_dashed_stroked_.width(attr.stroke_width);
                    renderer_.curved_dashed_stroked_.line_join(attr.line_join);
                    renderer_.curved_dashed_stroked_.line_cap(attr.line_cap);
                    renderer_.curved_dashed_stroked_.miter_limit(attr.miter_limit);
                    renderer_.curved_dashed_stroked_.inner_join(inner_round);
                    renderer_.curved_dashed_stroked_.approximation_scale(scl);

                    // If the *visual* line width is considerable we
                    // turn on processing of curve cups.
                    //---------------------
                    if (attr.stroke_width * scl > 1.0)
                    {
                        renderer_.curved_.angle_tolerance(0.2);
                    }
                    ras_.reset();
                    renderer_.curved_dashed_.remove_all_dashes();
                    for (auto d : attr.dash)
                    {
                        renderer_.curved_dashed_.add_dash(std::get<0>(d), std::get<1>(d));
                    }
                    renderer_.curved_dashed_.dash_start(attr.dash_offset);
                    ras_.add_path(curved_dashed_stroked_trans, attr.index);
                    if (attr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
                    {
                        render_gradient(ras_,
                                        sl_,
                                        ren_,
                                        attr.stroke_gradient,
                                        transform,
                                        attr.stroke_opacity,
                                        symbol_bbox_,
                                        curved_trans,
                                        attr.index);
                    }
                    else
                    {
                        ras_.filling_rule(fill_non_zero);
                        color = attr.stroke_color;
                        color.opacity(color.opacity() * attr.stroke_opacity);
                        ScanlineRenderer ren_s(ren_);
                        color.premultiply();
                        ren_s.color(color);
                        render_scanlines(ras_, sl_, ren_s);
                    }
                }
                else
                {
                    renderer_.curved_stroked_.width(attr.stroke_width);
                    renderer_.curved_stroked_.line_join(attr.line_join);
                    renderer_.curved_stroked_.line_cap(attr.line_cap);
                    renderer_.curved_stroked_.miter_limit(attr.miter_limit);
                    renderer_.curved_stroked_.inner_join(inner_round);
                    renderer_.curved_stroked_.approximation_scale(scl);

                    // If the *visual* line width is considerable we
                    // turn on processing of curve cups.
                    //---------------------
                    if (attr.stroke_width * scl > 1.0)
                    {
                        renderer_.curved_.angle_tolerance(0.2);
                    }
                    ras_.reset();
                    ras_.add_path(curved_stroked_trans, attr.index);
                    if (attr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
                    {
                        render_gradient(ras_,
                                        sl_,
                                        ren_,
                                        attr.stroke_gradient,
                                        transform,
                                        attr.stroke_opacity,
                                        symbol_bbox_,
                                        curved_trans,
                                        attr.index);
                    }
                    else
                    {
                        ras_.filling_rule(fill_non_zero);
                        color = attr.stroke_color;
                        color.opacity(color.opacity() * attr.stroke_opacity);
                        ScanlineRenderer ren_s(ren_);
                        color.premultiply();
                        ren_s.color(color);
                        render_scanlines(ras_, sl_, ren_s);
                    }
                }
            }
        }
        renderer_agg& renderer_;
        Rasterizer& ras_;
        Scanline& sl_;
        Renderer& ren_;
        agg::trans_affine const& mtx_;
        box2d<double> const& symbol_bbox_;
    };

#if defined(GRID_RENDERER)

    template<typename Rasterizer, typename Scanline, typename Renderer>
    struct grid_renderer
    {
        grid_renderer(renderer_agg& renderer,
                      Rasterizer& ras,
                      Scanline& sl,
                      Renderer& ren,
                      agg::trans_affine const& mtx,
                      mapnik::value_integer const& feature_id)
            : renderer_(renderer)
            , ras_(ras)
            , sl_(sl)
            , ren_(ren)
            , mtx_(mtx)
            , feature_id_(feature_id)
        {}

        void operator()(group const& g) const
        {
            for (auto const& elem : g.elements)
            {
                mapbox::util::apply_visitor(grid_renderer(renderer_, ras_, sl_, ren_, mtx_, feature_id_), elem);
            }
        }
        void operator()(path_attributes const& attr) const
        {
            using namespace agg;

            trans_affine transform;
            curved_stroked_trans_type curved_stroked_trans(renderer_.curved_stroked_, transform);
            curved_trans_type curved_trans(renderer_.curved_, transform);
            curved_trans_contour_type curved_trans_contour(curved_trans);

            curved_trans_contour.auto_detect_orientation(true);

            if (!attr.visibility_flag)
                return;

            transform = attr.transform;
            transform *= mtx_;

            double scl = transform.scale();
            // renderer_.curved_.approximation_method(curve_inc);
            renderer_.curved_.approximation_scale(scl);
            renderer_.curved_.angle_tolerance(0.0);

            typename PixelFormat::color_type color{feature_id_};

            if (attr.fill_flag || attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
            {
                ras_.reset();

                if (std::fabs(curved_trans_contour.width()) <= 1)
                {
                    ras_.add_path(curved_trans, attr.index);
                }
                else
                {
                    curved_trans_contour.miter_limit(attr.miter_limit);
                    ras_.add_path(curved_trans_contour, attr.index);
                }

                ras_.filling_rule(attr.even_odd_flag ? fill_even_odd : fill_non_zero);
                ScanlineRenderer ren_s(ren_);
                ren_s.color(color);
                render_scanlines(ras_, sl_, ren_s);
            }

            if (attr.stroke_flag || attr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
            {
                renderer_.curved_stroked_.width(attr.stroke_width);
                // m_curved_stroked.line_join((attr.line_join == miter_join) ? miter_join_round : attr.line_join);
                renderer_.curved_stroked_.line_join(attr.line_join);
                renderer_.curved_stroked_.line_cap(attr.line_cap);
                renderer_.curved_stroked_.miter_limit(attr.miter_limit);
                renderer_.curved_stroked_.inner_join(inner_round);
                renderer_.curved_stroked_.approximation_scale(scl);

                // If the *visual* line width is considerable we
                // turn on processing of curve cusps.
                //---------------------
                if (attr.stroke_width * scl > 1.0)
                {
                    renderer_.curved_.angle_tolerance(0.2);
                }
                ras_.reset();
                ras_.add_path(curved_stroked_trans, attr.index);

                ras_.filling_rule(fill_non_zero);
                ScanlineRenderer ren_s(ren_);
                ren_s.color(color);
                render_scanlines(ras_, sl_, ren_s);
            }
        }

        renderer_agg& renderer_;
        Rasterizer& ras_;
        Scanline& sl_;
        Renderer& ren_;
        agg::trans_affine const& mtx_;
        mapnik::value_integer const& feature_id_;
    };

    template<typename Rasterizer, typename Scanline, typename Renderer>
    void render_id(Rasterizer& ras,
                   Scanline& sl,
                   Renderer& ren,
                   mapnik::value_integer feature_id,
                   agg::trans_affine const& mtx,
                   double /*opacity*/,
                   box2d<double> const& /*symbol_bbox*/)

    {
        for (auto const& elem : svg_group_.elements)
        {
            mapbox::util::apply_visitor(
              grid_renderer<Rasterizer, Scanline, Renderer>(*this, ras, sl, ren, mtx, feature_id),
              elem);
        }
    }
#endif

    inline VertexSource& source() const
    {
        return source_;
    }

    inline group const& svg_group() const
    {
        return svg_group_;
    }

  private:

    VertexSource& source_;
    curved_type curved_;
    curved_dashed_type curved_dashed_;
    curved_stroked_type curved_stroked_;
    curved_dashed_stroked_type curved_dashed_stroked_;
    group const& svg_group_;
};

} // namespace svg
} // namespace mapnik

#endif // MAPNIK_SVG_RENDERER_AGG_HPP
