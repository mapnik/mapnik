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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/layer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/cairo_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/symbolizer_helpers.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/warp.hpp>
#include <mapnik/config.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker_helpers.hpp>

// cairo
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <cairo-ft.h>
#include <cairo-version.h>

// boost
#include <boost/utility.hpp>
#include <boost/make_shared.hpp>

// agg
#include "agg_conv_clip_polyline.h"
#include "agg_conv_clip_polygon.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"

// markers
#include "agg_path_storage.h"
#include "agg_ellipse.h"

// stl
#include <deque>

namespace mapnik
{
class cairo_pattern : private boost::noncopyable
{
public:
    cairo_pattern(image_data_32 const& data)
    {
        int pixels = data.width() * data.height();
        const unsigned int *in_ptr = data.getData();
        const unsigned int *in_end = in_ptr + pixels;
        unsigned int *out_ptr;

        surface_ = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, data.width(), data.height());

        out_ptr = reinterpret_cast<unsigned int *>(surface_->get_data());

        while (in_ptr < in_end)
        {
            unsigned int in = *in_ptr++;
            unsigned int r = (in >> 0) & 0xff;
            unsigned int g = (in >> 8) & 0xff;
            unsigned int b = (in >> 16) & 0xff;
            unsigned int a = (in >> 24) & 0xff;

            //r = r * a / 255;
            //g = g * a / 255;
            //b = b * a / 255;

            *out_ptr++ = (a << 24) | (r << 16) | (g << 8) | b;
        }
        // mark the surface as dirty as we've modified it behind cairo's back
        surface_->mark_dirty();
        pattern_ = Cairo::SurfacePattern::create(surface_);
    }

    ~cairo_pattern()
    {
    }

    void set_matrix(Cairo::Matrix const& matrix)
    {
        pattern_->set_matrix(matrix);
    }

    void set_origin(double x, double y)
    {
        Cairo::Matrix matrix;

        pattern_->get_matrix(matrix);

        matrix.x0 = -x;
        matrix.y0 = -y;

        pattern_->set_matrix(matrix);
    }

    void set_extend(Cairo::Extend extend)
    {
        pattern_->set_extend(extend);
    }

    void set_filter(Cairo::Filter filter)
    {
        pattern_->set_filter(filter);
    }

    Cairo::RefPtr<Cairo::SurfacePattern> const& pattern() const
    {
        return pattern_;
    }

private:
    Cairo::RefPtr<Cairo::ImageSurface> surface_;
    Cairo::RefPtr<Cairo::SurfacePattern> pattern_;
};

class cairo_gradient : private boost::noncopyable
{
public:
    cairo_gradient(const mapnik::gradient &grad, double opacity=1.0)
    {
        double x1,x2,y1,y2,r;
        grad.get_control_points(x1,y1,x2,y2,r);
        if (grad.get_gradient_type() == LINEAR)
        {
            pattern_ = Cairo::LinearGradient::create(x1, y1, x2, y2);
        }
        else if (grad.get_gradient_type() == RADIAL)
        {
            pattern_ = Cairo::RadialGradient::create(x1, y1, 0, x2, y2, r);
        }

        units_ = grad.get_units();

        BOOST_FOREACH ( mapnik::stop_pair const& st, grad.get_stop_array() )
        {
            mapnik::color const& stop_color = st.second;
            double r= static_cast<double> (stop_color.red())/255.0;
            double g= static_cast<double> (stop_color.green())/255.0;
            double b= static_cast<double> (stop_color.blue())/255.0;
            double a= static_cast<double> (stop_color.alpha())/255.0;
            pattern_->add_color_stop_rgba(st.first, r, g, b, a*opacity);
        }

        double m[6];
        agg::trans_affine tr = grad.get_transform();
        tr.invert();
        tr.store_to(m);
        pattern_->set_matrix(Cairo::Matrix(m[0],m[1],m[2],m[3],m[4],m[5]));
    }

    ~cairo_gradient()
    {
    }


    Cairo::RefPtr<Cairo::Gradient> const& gradient() const
    {
        return pattern_;
    }

    gradient_unit_e units() const
    {
        return units_;
    }

private:
    Cairo::RefPtr<Cairo::Gradient> pattern_;
    gradient_unit_e units_;

};

class cairo_face : private boost::noncopyable
{
public:
    cairo_face(boost::shared_ptr<freetype_engine> const& engine, face_ptr const& face)
        : face_(face)
    {
        static cairo_user_data_key_t key;
        cairo_font_face_t *c_face;

        c_face = cairo_ft_font_face_create_for_ft_face(face->get_face(), FT_LOAD_NO_HINTING);
        cairo_font_face_set_user_data(c_face, &key, new handle(engine, face), destroy);

        cairo_face_ = Cairo::RefPtr<Cairo::FontFace>(new Cairo::FontFace(c_face));
    }

    Cairo::RefPtr<Cairo::FontFace> const& face() const
    {
        return cairo_face_;
    }

private:
    class handle
    {
    public:
        handle(boost::shared_ptr<freetype_engine> const& engine, face_ptr const& face)
            : engine_(engine), face_(face) {}

    private:
        boost::shared_ptr<freetype_engine> engine_;
        face_ptr face_;
    };

    static void destroy(void *data)
    {
        handle *h = static_cast<handle *>(data);

        delete h;
    }

private:
    face_ptr face_;
    Cairo::RefPtr<Cairo::FontFace> cairo_face_;
};

cairo_face_manager::cairo_face_manager(boost::shared_ptr<freetype_engine> engine)
    : font_engine_(engine)
{
}

cairo_face_ptr cairo_face_manager::get_face(face_ptr face)
{
    cairo_face_cache::iterator itr = cache_.find(face);
    cairo_face_ptr entry;

    if (itr != cache_.end())
    {
        entry = itr->second;
    }
    else
    {
        entry = boost::make_shared<cairo_face>(font_engine_, face);
        cache_.insert(std::make_pair(face, entry));
    }

    return entry;
}

class cairo_context : private boost::noncopyable
{
public:
    cairo_context(Cairo::RefPtr<Cairo::Context> const& context)
        : context_(context)
    {
        context_->save();
    }

    ~cairo_context()
    {
        context_->restore();
    }

    void set_color(color const &color, double opacity = 1.0)
    {
        set_color(color.red(), color.green(), color.blue(), color.alpha() * opacity / 255.0);
    }

    void set_color(int r, int g, int b, double opacity = 1.0)
    {
        context_->set_source_rgba(r / 255.0, g / 255.0, b / 255.0, opacity);
    }

    void set_operator(composite_mode_e comp_op)
    {
        switch (comp_op)
        {
        case clear:
            context_->set_operator(Cairo::OPERATOR_CLEAR);
            break;
        case src:
            context_->set_operator(Cairo::OPERATOR_SOURCE);
            break;
        case dst:
            context_->set_operator(Cairo::OPERATOR_DEST);
            break;
        case src_over:
            context_->set_operator(Cairo::OPERATOR_OVER);
            break;
        case dst_over:
            context_->set_operator(Cairo::OPERATOR_DEST_OVER);
            break;
        case src_in:
            context_->set_operator(Cairo::OPERATOR_IN);
            break;
        case dst_in:
            context_->set_operator(Cairo::OPERATOR_DEST_IN);
            break;
        case src_out:
            context_->set_operator(Cairo::OPERATOR_OUT);
            break;
        case dst_out:
            context_->set_operator(Cairo::OPERATOR_DEST_OUT);
            break;
        case src_atop:
            context_->set_operator(Cairo::OPERATOR_ATOP);
            break;
        case dst_atop:
            context_->set_operator(Cairo::OPERATOR_DEST_ATOP);
            break;
        case _xor:
            context_->set_operator(Cairo::OPERATOR_XOR);
            break;
        case plus:
            context_->set_operator(Cairo::OPERATOR_ADD);
            break;
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0)
        case multiply:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_MULTIPLY));
            break;
        case screen:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_SCREEN));
            break;
        case overlay:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_OVERLAY));
            break;
        case darken:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_DARKEN));
            break;
        case lighten:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_LIGHTEN));
            break;
        case color_dodge:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_COLOR_DODGE));
            break;
        case color_burn:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_COLOR_BURN));
            break;
        case hard_light:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_HARD_LIGHT));
            break;
        case soft_light:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_SOFT_LIGHT));
            break;
        case difference:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_DIFFERENCE));
            break;
        case exclusion:
            context_->set_operator(static_cast<Cairo::Operator>(CAIRO_OPERATOR_EXCLUSION));
            break;
#else
#warning building against cairo older that 1.10.0, some compositing options are disabled
        case multiply:
        case screen:
        case overlay:
        case darken:
        case lighten:
        case color_dodge:
        case color_burn:
        case hard_light:
        case soft_light:
        case difference:
        case exclusion:
            break;
#endif
        case contrast:
        case minus:
        case invert:
        case invert_rgb:
        case grain_merge:
        case grain_extract:
        case hue:
        case saturation:
        case _color:
        case _value:
        //case colorize_alpha:
            break;
        }
    }

    void set_line_join(line_join_e join)
    {
        if (join == MITER_JOIN)
            context_->set_line_join(Cairo::LINE_JOIN_MITER);
        else if (join == MITER_REVERT_JOIN)
            context_->set_line_join(Cairo::LINE_JOIN_MITER);
        else if (join == ROUND_JOIN)
            context_->set_line_join(Cairo::LINE_JOIN_ROUND);
        else
            context_->set_line_join(Cairo::LINE_JOIN_BEVEL);
    }

    void set_line_cap(line_cap_e cap)
    {
        if (cap == BUTT_CAP)
            context_->set_line_cap(Cairo::LINE_CAP_BUTT);
        else if (cap == SQUARE_CAP)
            context_->set_line_cap(Cairo::LINE_CAP_SQUARE);
        else
            context_->set_line_cap(Cairo::LINE_CAP_ROUND);
    }

    void set_miter_limit(double limit)
    {
        context_->set_miter_limit(limit);
    }

    void set_line_width(double width)
    {
        context_->set_line_width(width);
    }

    void set_dash(dash_array const &dashes, double scale_factor)
    {
        std::valarray<double> d(dashes.size() * 2);
        dash_array::const_iterator itr = dashes.begin();
        dash_array::const_iterator end = dashes.end();
        int index = 0;

        for (; itr != end; ++itr)
        {
            d[index++] = itr->first * scale_factor;
            d[index++] = itr->second * scale_factor;
        }

        context_->set_dash(d, 0.0);
    }

    void set_fill_rule(Cairo::FillRule fill_rule)
    {
        context_->set_fill_rule(fill_rule);
    }

    void move_to(double x, double y)
    {
#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 6, 0)
        if (x < -32767.0) x = -32767.0;
        else if (x > 32767.0) x = 32767.0;
        if (y < -32767.0) y = -32767.0;
        else if (y > 32767.0) y = 32767.0;
#endif

        context_->move_to(x, y);
    }

    void curve_to(double ct1_x, double ct1_y, double ct2_x, double ct2_y, double end_x, double end_y)
    {
        context_->curve_to(ct1_x,ct1_y,ct2_x,ct2_y,end_x,end_y);
    }

    void close_path()
    {
        context_->close_path();
    }

    void line_to(double x, double y)
    {
#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 6, 0)
        if (x < -32767.0) x = -32767.0;
        else if (x > 32767.0) x = 32767.0;
        if (y < -32767.0) y = -32767.0;
        else if (y > 32767.0) y = 32767.0;
#endif

        context_->line_to(x, y);
    }

    template <typename T>
    void add_path(T& path, unsigned start_index = 0)
    {
        double x, y;

        path.rewind(start_index);

        for (unsigned cm = path.vertex(&x, &y); cm != SEG_END; cm = path.vertex(&x, &y))
        {
            if (cm == SEG_MOVETO)
            {
                move_to(x, y);
            }
            else if (cm == SEG_LINETO)
            {
                line_to(x, y);
            }
            else if (cm == SEG_CLOSE)
            {
                close_path();
            }
        }
    }

    template <typename T>
    void add_agg_path(T& path, unsigned start_index = 0)
    {
        double x=0;
        double y=0;

        path.rewind(start_index);

        for (unsigned cm = path.vertex(&x, &y); !agg::is_stop(cm); cm = path.vertex(&x, &y))
        {
            if (agg::is_move_to(cm))
            {
                move_to(x, y);
            }
            else if (agg::is_drawing(cm))
            {
                if (agg::is_curve3(cm))
                {
                    double end_x=0;
                    double end_y=0;

                    MAPNIK_LOG_WARN(cairo_renderer) << "Curve 3 not implemented";

                    path.vertex(&end_x, &end_y);

                    curve_to(x,y,x,y,end_x,end_y);
                }
                else if (agg::is_curve4(cm))
                {
                    double ct2_x=0;
                    double ct2_y=0;
                    double end_x=0;
                    double end_y=0;

                    path.vertex(&ct2_x, &ct2_y);
                    path.vertex(&end_x, &end_y);

                    curve_to(x,y,ct2_x,ct2_y,end_x,end_y);
                }
                else if (agg::is_line_to(cm))
                {
                    line_to(x, y);
                }
                else
                {
                    MAPNIK_LOG_WARN(cairo_renderer) << "Unimplemented drawing command: " << cm;
                    move_to(x, y);
                }
            }
            else if (agg::is_close(cm))
            {
                close_path();
            }
            else
            {
                MAPNIK_LOG_WARN(cairo_renderer) << "Unimplemented path command: " << cm;
            }
        }
    }

    void rectangle(double x, double y, double w, double h)
    {
        context_->rectangle(x, y, w, h);
    }

    void stroke()
    {
        context_->stroke();
    }

    void fill()
    {
        context_->fill();
    }

    void paint()
    {
        context_->paint();
    }

    void set_pattern(cairo_pattern const& pattern)
    {
        context_->set_source(pattern.pattern());
    }

    void set_gradient(cairo_gradient const& pattern, const box2d<double> &bbox)
    {
        Cairo::RefPtr<Cairo::Gradient> p = pattern.gradient();

        double bx1=bbox.minx();
        double by1=bbox.miny();
        double bx2=bbox.maxx();
        double by2=bbox.maxy();
        if (pattern.units() != USER_SPACE_ON_USE)
        {
            if (pattern.units() == OBJECT_BOUNDING_BOX)
            {
                context_->get_path_extents (bx1, by1, bx2, by2);
            }
            Cairo::Matrix m = p->get_matrix();
            m.scale(1.0/(bx2-bx1),1.0/(by2-by1));
            m.translate(-bx1,-by1);
            p->set_matrix(m);
        }

        context_->set_source(p);
    }

    void add_image(double x, double y, image_data_32 & data, double opacity = 1.0)
    {
        cairo_pattern pattern(data);

        pattern.set_origin(x, y);

        context_->save();
        context_->set_source(pattern.pattern());
        context_->paint_with_alpha(opacity);
        context_->restore();
    }

    void add_image(agg::trans_affine const& tr, image_data_32 & data, double opacity = 1.0)
    {
        cairo_pattern pattern(data);
        if (!tr.is_identity())
        {
            double m[6];
            tr.store_to(m);
            Cairo::Matrix cairo_matrix(m[0],m[1],m[2],m[3],m[4],m[5]);
            cairo_matrix.invert();
            pattern.set_matrix(cairo_matrix);
        }
        context_->save();
        context_->set_source(pattern.pattern());
        context_->paint_with_alpha(opacity);
        context_->restore();
    }


    void set_font_face(cairo_face_manager & manager, face_ptr face)
    {
        context_->set_font_face(manager.get_face(face)->face());
    }

    void set_font_matrix(Cairo::Matrix const& matrix)
    {
        context_->set_font_matrix(matrix);
    }

    void set_matrix(Cairo::Matrix const& matrix)
    {
        context_->set_matrix(matrix);
    }

    void transform(Cairo::Matrix const& matrix)
    {
        context_->transform(matrix);
    }

    void translate(double x, double y)
    {
        context_->translate(x,y);
    }

    void save()
    {
        context_->save();
    }

    void restore()
    {
        context_->restore();
    }

    void show_glyph(unsigned long index, double x, double y)
    {
        Cairo::Glyph glyph;

        glyph.index = index;
        glyph.x = x;
        glyph.y = y;

        cairo_show_glyphs(context_->cobj(), &glyph, 1);
        if (context_->get_status() != CAIRO_STATUS_SUCCESS)
        {
            throw std::runtime_error("cairo: show_glyph");
        }
    }

    void glyph_path(unsigned long index, double x, double y)
    {
        Cairo::Glyph glyph;

        glyph.index = index;
        glyph.x = x;
        glyph.y = y;

        cairo_glyph_path(context_->cobj(), &glyph, 1);
        if (context_->get_status() != CAIRO_STATUS_SUCCESS)
        {
            throw std::runtime_error("cairo: glyph_path");
        }
    }

    void add_text(text_path const& path,
                  cairo_face_manager & manager,
                  face_manager<freetype_engine> &font_manager,
                  double scale_factor = 1.0)
    {
        double sx = path.center.x;
        double sy = path.center.y;

        path.rewind();

        for (int iii = 0; iii < path.num_nodes(); iii++)
        {
            char_info_ptr c;
            double x, y, angle;

            path.vertex(&c, &x, &y, &angle);

            face_set_ptr faces = font_manager.get_face_set(c->format->face_name, c->format->fontset);
            double text_size = c->format->text_size * scale_factor;
            faces->set_character_sizes(text_size);

            glyph_ptr glyph = faces->get_glyph(c->c);

            if (glyph)
            {
                Cairo::Matrix matrix;

                matrix.xx = text_size * cos(angle);
                matrix.xy = text_size * sin(angle);
                matrix.yx = text_size * -sin(angle);
                matrix.yy = text_size * cos(angle);
                matrix.x0 = 0;
                matrix.y0 = 0;

                set_font_matrix(matrix);

                set_font_face(manager, glyph->get_face());

                glyph_path(glyph->get_index(), sx + x, sy - y);
                set_line_width(2.0 * c->format->halo_radius * scale_factor);
                set_line_join(ROUND_JOIN);
                set_color(c->format->halo_fill);
                stroke();
                set_color(c->format->fill);
                show_glyph(glyph->get_index(), sx + x, sy - y);
            }
        }
    }


private:
    Cairo::RefPtr<Cairo::Context> context_;
};

cairo_renderer_base::cairo_renderer_base(Map const& m,
                                         Cairo::RefPtr<Cairo::Context> const& context,
                                         double scale_factor,
                                         unsigned offset_x,
                                         unsigned offset_y)
    : m_(m),
      context_(context),
      width_(m.width()),
      height_(m.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(boost::make_shared<freetype_engine>()),
      font_manager_(*font_engine_),
      face_manager_(font_engine_),
      detector_(boost::make_shared<label_collision_detector4>(
          box2d<double>(-m.buffer_size(), -m.buffer_size(),
          m.width() + m.buffer_size(), m.height() + m.buffer_size())))
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Scale=" << m.scale();
}

cairo_renderer_base::cairo_renderer_base(Map const& m,
                                         Cairo::RefPtr<Cairo::Context> const& context,
                                         boost::shared_ptr<label_collision_detector4> detector,
                                         double scale_factor,
                                         unsigned offset_x,
                                         unsigned offset_y)
    : m_(m),
      context_(context),
      width_(m.width()),
      height_(m.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(boost::make_shared<freetype_engine>()),
      font_manager_(*font_engine_),
      face_manager_(font_engine_),
      detector_(detector)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Scale=" << m.scale();
}

template <>
cairo_renderer<Cairo::Context>::cairo_renderer(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,context,scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<Cairo::Surface>::cairo_renderer(Map const& m, Cairo::RefPtr<Cairo::Surface> const& surface, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,Cairo::Context::create(surface),scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<Cairo::Context>::cairo_renderer(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, boost::shared_ptr<label_collision_detector4> detector, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,context,detector,scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<Cairo::Surface>::cairo_renderer(Map const& m, Cairo::RefPtr<Cairo::Surface> const& surface, boost::shared_ptr<label_collision_detector4> detector, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,Cairo::Context::create(surface),detector,scale_factor,offset_x,offset_y) {}

cairo_renderer_base::~cairo_renderer_base() {}

void cairo_renderer_base::start_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Start map processing bbox=" << map.get_current_extent();

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 6, 0)
    box2d<double> bounds = t_.forward(t_.extent());
    context_->rectangle(bounds.minx(), bounds.miny(), bounds.maxx(), bounds.maxy());
    context_->clip();
#else
#warning building against cairo older that 1.6.0, map clipping is disabled
#endif

    boost::optional<color> bg = m_.background();
    if (bg)
    {
        cairo_context context(context_);
        context.set_color(*bg);
        context.paint();
    }
}

template <>
void cairo_renderer<Cairo::Context>::end_map_processing(Map const& )
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: End map processing";
}

template <>
void cairo_renderer<Cairo::Surface>::end_map_processing(Map const& )
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: End map processing";

    context_->show_page();
}

void cairo_renderer_base::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Start processing layer=" << lay.name() ;
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: -- datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: -- query_extent=" << query_extent;

    if (lay.clear_label_cache())
    {
        detector_->clear();
    }
    query_extent_ = query_extent;
}

void cairo_renderer_base::end_layer_processing(layer const&)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: End layer processing";
}

void cairo_renderer_base::start_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer:start style processing";
}

void cairo_renderer_base::end_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer:end style processing";
}

void cairo_renderer_base::process(polygon_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    cairo_context context(context_);
    context.set_operator(sym.comp_op());
    context.set_color(sym.get_fill(), sym.get_opacity());

    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform());

    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, cairo_context, polygon_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(query_extent_,context,sym,t_,prj_trans,tr,1.0);

    if (prj_trans.equal() && sym.clip()) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }
    // fill polygon
    context.fill();
}

void cairo_renderer_base::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform<CoordTransform,geometry_type> path_type;

    cairo_context context(context_);
    context.set_operator(sym.comp_op());
    color const& fill = sym.get_fill();
    double height = 0.0;
    expression_ptr height_expr = sym.height();
    if (height_expr)
    {
        value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature), *height_expr);
        height = result.to_double() * scale_factor_;
    }

    for (unsigned i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        if (geom.size() > 2)
        {
            boost::scoped_ptr<geometry_type> frame(new geometry_type(LineString));
            boost::scoped_ptr<geometry_type> roof(new geometry_type(Polygon));
            std::deque<segment_t> face_segments;
            double x0 = 0;
            double y0 = 0;
            double x, y;
            geom.rewind(0);
            for (unsigned cm = geom.vertex(&x, &y); cm != SEG_END;
                 cm = geom.vertex(&x, &y))
            {
                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x,y);
                }
                else if (cm == SEG_LINETO || cm == SEG_CLOSE)
                {
                    frame->line_to(x,y);
                    face_segments.push_back(segment_t(x0,y0,x,y));
                }
                x0 = x;
                y0 = y;
            }

            std::sort(face_segments.begin(), face_segments.end(), y_order);
            std::deque<segment_t>::const_iterator itr = face_segments.begin();
            std::deque<segment_t>::const_iterator end=face_segments.end();
            for (; itr != end; ++itr)
            {
                boost::scoped_ptr<geometry_type> faces(new geometry_type(Polygon));
                faces->move_to(itr->get<0>(), itr->get<1>());
                faces->line_to(itr->get<2>(), itr->get<3>());
                faces->line_to(itr->get<2>(), itr->get<3>() + height);
                faces->line_to(itr->get<0>(), itr->get<1>() + height);

                path_type faces_path(t_, *faces, prj_trans);
                context.set_color(int(fill.red() * 0.8), int(fill.green() * 0.8),
                                  int(fill.blue() * 0.8), fill.alpha() * sym.get_opacity() / 255.0);
                context.add_path(faces_path);
                context.fill();

                frame->move_to(itr->get<0>(), itr->get<1>());
                frame->line_to(itr->get<0>(), itr->get<1>() + height);
            }

            geom.rewind(0);
            for (unsigned cm = geom.vertex(&x, &y); cm != SEG_END;
                 cm = geom.vertex(&x, &y))
            {
                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x,y+height);
                    roof->move_to(x,y+height);
                }
                else if (cm == SEG_LINETO || cm == SEG_CLOSE)
                {
                    frame->line_to(x,y+height);
                    roof->line_to(x,y+height);
                }
            }

            path_type path(t_, *frame, prj_trans);
            context.set_color(fill.red()*0.8, fill.green()*0.8, fill.blue()*0.8,sym.get_opacity());
            context.set_line_width(scale_factor_);
            context.add_path(path);
            context.stroke();

            path_type roof_path(t_, *roof, prj_trans);
            context.set_color(fill, fill.alpha() * sym.get_opacity() / 255.0 );
            context.add_path(roof_path);
            context.fill();
        }
    }
}

void cairo_renderer_base::process(line_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef boost::mpl::vector<clip_line_tag, transform_tag,
                               offset_transform_tag, affine_transform_tag,
                               simplify_tag, smooth_tag, dash_tag, stroke_tag> conv_types;
    cairo_context context(context_);
    mapnik::stroke const& stroke_ = sym.get_stroke();
    context.set_operator(sym.comp_op());

    context.set_color(stroke_.get_color(), stroke_.get_opacity());
    context.set_line_join(stroke_.get_line_join());
    context.set_line_cap(stroke_.get_line_cap());
    context.set_miter_limit(stroke_.get_miterlimit());
    context.set_line_width(stroke_.get_width() * scale_factor_);
    if (stroke_.has_dash())
    {
        context.set_dash(stroke_.get_dash_array(), scale_factor_);
    }

    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform());

    box2d<double> clipping_extent = query_extent_;
    if (sym.clip())
    {
        double padding = (double)(query_extent_.width()/width_);
        float half_stroke = stroke_.get_width()/2.0;
        if (half_stroke > 1)
            padding *= half_stroke;
        if (fabs(sym.offset()) > 0)
            padding *= fabs(sym.offset()) * 1.2;
        double x0 = query_extent_.minx();
        double y0 = query_extent_.miny();
        double x1 = query_extent_.maxx();
        double y1 = query_extent_.maxy();
        clipping_extent.init(x0 - padding, y0 - padding, x1 + padding , y1 + padding);
    }
    vertex_converter<box2d<double>, cairo_context, line_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(clipping_extent,context,sym,t_,prj_trans,tr,scale_factor_);

    if (sym.clip()) converter.set<clip_line_tag>(); // optional clip (default: true)
    converter.set<transform_tag>(); // always transform
    if (fabs(sym.offset()) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 1)
        {
            converter.apply(geom);
        }
    }
    // stroke
    context.stroke();
}

void cairo_renderer_base::render_box(box2d<double> const& b)
{
    cairo_context context(context_);
    context.move_to(b.minx(), b.miny());
    context.line_to(b.minx(), b.maxy());
    context.line_to(b.maxx(), b.maxy());
    context.line_to(b.maxx(), b.miny());
    context.close_path();

    context.stroke();
}

void render_vector_marker(cairo_context & context, pixel_position const& pos, mapnik::svg_storage_type & vmarker,
                          agg::pod_bvector<svg::path_attributes> const & attributes,
                          agg::trans_affine const& tr, double opacity, bool recenter)
{
    using namespace mapnik::svg;
    box2d<double> bbox = vmarker.bounding_box();

    agg::trans_affine mtx = tr;

    if (recenter)
    {
        coord<double,2> c = bbox.center();
        mtx = agg::trans_affine_translation(-c.x,-c.y);
        mtx *= tr;
        mtx.translate(pos.x, pos.y);
    }

    agg::trans_affine transform;

    for(unsigned i = 0; i < attributes.size(); ++i)
    {
        mapnik::svg::path_attributes const& attr = attributes[i];
        if (!attr.visibility_flag)
            continue;

        context.save();

        transform = attr.transform;
        transform *= mtx;

        // TODO - this 'is_valid' check is not used in the AGG renderer and also
        // appears to lead to bogus results with
        // tests/data/good_maps/markers_symbolizer_lines_file.xml
        if (/*transform.is_valid() && */ !transform.is_identity())
        {
            double m[6];
            transform.store_to(m);
            context.transform(Cairo::Matrix(m[0],m[1],m[2],m[3],m[4],m[5]));
        }

        vertex_stl_adapter<svg_path_storage> stl_storage(vmarker.source());
        svg_path_adapter svg_path(stl_storage);

        if (attr.fill_flag || attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
        {
            context.add_agg_path(svg_path,attr.index);
            if (attr.even_odd_flag)
            {
                context.set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
            }
            else
            {
                context.set_fill_rule(Cairo::FILL_RULE_WINDING);
            }
            if(attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
            {
                cairo_gradient g(attr.fill_gradient,attr.fill_opacity*opacity);

                context.set_gradient(g,bbox);
                context.fill();
            }
            else if(attr.fill_flag)
            {
                double fill_opacity = attr.fill_opacity * opacity * attr.fill_color.opacity();
                context.set_color(attr.fill_color.r,attr.fill_color.g,attr.fill_color.b, fill_opacity);
                context.fill();
            }
        }

        if (attr.stroke_gradient.get_gradient_type() != NO_GRADIENT || attr.stroke_flag)
        {
            context.add_agg_path(svg_path,attr.index);
            if(attr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
            {
                context.set_line_width(attr.stroke_width);
                context.set_line_cap(line_cap_enum(attr.line_cap));
                context.set_line_join(line_join_enum(attr.line_join));
                context.set_miter_limit(attr.miter_limit);
                cairo_gradient g(attr.stroke_gradient,attr.fill_opacity*opacity);
                context.set_gradient(g,bbox);
                context.stroke();
            }
            else if (attr.stroke_flag)
            {
                double stroke_opacity = attr.stroke_opacity * opacity * attr.stroke_color.opacity();
                context.set_color(attr.stroke_color.r,attr.stroke_color.g,attr.stroke_color.b, stroke_opacity);
                context.set_line_width(attr.stroke_width);
                context.set_line_cap(line_cap_enum(attr.line_cap));
                context.set_line_join(line_join_enum(attr.line_join));
                context.set_miter_limit(attr.miter_limit);
                context.stroke();
            }
        }

        context.restore();
    }
}


void cairo_renderer_base::render_marker(pixel_position const& pos, marker const& marker, const agg::trans_affine & tr, double opacity, bool recenter)

{
    cairo_context context(context_);

    if (marker.is_vector())
    {
        mapnik::svg_path_ptr vmarker = *marker.get_vector_data();
        if (vmarker)
        {
            agg::pod_bvector<svg::path_attributes> const & attributes = vmarker->attributes();
            render_vector_marker(context, pos, *vmarker, attributes, tr, opacity, recenter);
        }
    }
    else if (marker.is_bitmap())
    {
        agg::trans_affine matrix = tr;
        matrix *= agg::trans_affine_translation(pos.x, pos.y);
        context.add_image(matrix, **marker.get_bitmap_data(), opacity);
    }
}

void cairo_renderer_base::process(point_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);

    boost::optional<marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance().find(filename, true);
    }
    else
    {
        marker.reset(boost::make_shared<mapnik::marker>());
    }


    if (marker)
    {
        for (unsigned i = 0; i < feature.num_geometries(); ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            double x;
            double y;
            double z = 0;

            if (sym.get_point_placement() == CENTROID_POINT_PLACEMENT)
            {
                if (!label::centroid(geom, x, y))
                    return;
            }
            else
            {
                if (!label::interior_position(geom ,x, y))
                    return;
            }

            prj_trans.backward(x, y, z);
            t_.forward(&x, &y);

            double dx = 0.5 * (*marker)->width();
            double dy = 0.5 * (*marker)->height();
            agg::trans_affine tr = agg::trans_affine_scaling(scale_factor_);
            evaluate_transform(tr, feature, sym.get_image_transform());
            box2d<double> label_ext (-dx, -dy, dx, dy);
            label_ext *= tr;
            label_ext *= agg::trans_affine_translation(x,y);
            if (sym.get_allow_overlap() ||
                detector_->has_placement(label_ext))
            {
                render_marker(pixel_position(x,y),**marker, tr, sym.get_opacity());

                if (!sym.get_ignore_placement())
                    detector_->insert(label_ext);
            }
        }
    }
}

void cairo_renderer_base::process(shield_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    shield_symbolizer_helper<face_manager<freetype_engine>,
        label_collision_detector4> helper(
            sym, feature, prj_trans,
            width_, height_,
            scale_factor_,
            t_, font_manager_, *detector_, query_extent_);
    cairo_context context(context_);
    context.set_operator(sym.comp_op());

    while (helper.next())
    {
        placements_type const& placements = helper.placements();
        for (unsigned int ii = 0; ii < placements.size(); ++ii)
        {
            pixel_position marker_pos = helper.get_marker_position(placements[ii]);
            double dx = 0.5 * helper.get_marker_width();
            double dy = 0.5 * helper.get_marker_height();
            agg::trans_affine marker_tr = agg::trans_affine_translation(-dx,-dy);
            marker_tr *= agg::trans_affine_scaling(scale_factor_);
            marker_tr *= agg::trans_affine_translation(dx,dy);
            marker_tr *= helper.get_image_transform();
            render_marker(marker_pos,
                          helper.get_marker(),
                          marker_tr,
                          sym.get_opacity());

            context.add_text(placements[ii], face_manager_, font_manager_, scale_factor_);
        }
    }
}

void cairo_renderer_base::process(line_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
    typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;

    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<mapnik::marker_ptr> marker = mapnik::marker_cache::instance().find(filename,true);
    if (!marker && !(*marker)->is_bitmap()) return;

    unsigned width((*marker)->width());
    unsigned height((*marker)->height());

    cairo_context context(context_);
    context.set_operator(sym.comp_op());
    cairo_pattern pattern(**((*marker)->get_bitmap_data()));

    pattern.set_extend(Cairo::EXTEND_REPEAT);
    pattern.set_filter(Cairo::FILTER_BILINEAR);
    context.set_line_width(height * scale_factor_);

    for (unsigned i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type & geom = feature.get_geometry(i);

        if (geom.size() > 1)
        {
            clipped_geometry_type clipped(geom);
            clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
            path_type path(t_,clipped,prj_trans);

            double length(0);
            double x0(0), y0(0);
            double x, y;

            for (unsigned cm = path.vertex(&x, &y); cm != SEG_END; cm = path.vertex(&x, &y))
            {
                if (cm == SEG_MOVETO)
                {
                    length = 0.0;
                }
                else if (cm == SEG_LINETO)
                {
                    double dx = x - x0;
                    double dy = y - y0;
                    double angle = atan2(dy, dx);
                    double offset = fmod(length, width);

                    Cairo::Matrix matrix;
                    cairo_matrix_init_identity(&matrix);
                    cairo_matrix_translate(&matrix,x0,y0);
                    cairo_matrix_rotate(&matrix,angle);
                    cairo_matrix_translate(&matrix,-offset,0.5*height);
                    cairo_matrix_invert(&matrix);

                    pattern.set_matrix(matrix);

                    context.set_pattern(pattern);

                    context.move_to(x0, y0);
                    context.line_to(x, y);
                    context.stroke();

                    length = length + hypot(x - x0, y - y0);
                }

                x0 = x;
                y0 = y;
            }
        }
    }
}

void cairo_renderer_base::process(polygon_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef agg::conv_clip_polygon<geometry_type> clipped_geometry_type;
    typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;

    cairo_context context(context_);
    context.set_operator(sym.comp_op());

    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<mapnik::marker_ptr> marker = mapnik::marker_cache::instance().find(filename,true);
    if (!marker && !(*marker)->is_bitmap()) return;

    cairo_pattern pattern(**((*marker)->get_bitmap_data()));

    pattern.set_extend(Cairo::EXTEND_REPEAT);

    context.set_pattern(pattern);

    //pattern_alignment_e align = sym.get_alignment();
    //unsigned offset_x=0;
    //unsigned offset_y=0;

    //if (align == LOCAL_ALIGNMENT)
    //{
    //    double x0 = 0;
    //    double y0 = 0;
    //    if (feature.num_geometries() > 0)
    //    {
    //        clipped_geometry_type clipped(feature.get_geometry(0));
    //        clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
    //        path_type path(t_,clipped,prj_trans);
    //        path.vertex(&x0,&y0);
    //    }
    //    offset_x = unsigned(width_ - x0);
    //    offset_y = unsigned(height_ - y0);
    //}

    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform());

    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, cairo_context, polygon_pattern_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(query_extent_,context,sym,t_,prj_trans,tr, scale_factor_);

    if (prj_trans.equal() && sym.clip()) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }
    // fill polygon
    context.fill();
}

void cairo_renderer_base::process(raster_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    raster_ptr const& source = feature.get_raster();
    if (source)
    {
        // If there's a colorizer defined, use it to color the raster in-place
        raster_colorizer_ptr colorizer = sym.get_colorizer();
        if (colorizer)
            colorizer->colorize(source,feature);

        box2d<double> target_ext = box2d<double>(source->ext_);
        prj_trans.backward(target_ext, PROJ_ENVELOPE_POINTS);
        box2d<double> ext = t_.forward(target_ext);
        int start_x = static_cast<int>(ext.minx());
        int start_y = static_cast<int>(ext.miny());
        int end_x = static_cast<int>(ceil(ext.maxx()));
        int end_y = static_cast<int>(ceil(ext.maxy()));
        int raster_width = end_x - start_x;
        int raster_height = end_y - start_y;
        if (raster_width > 0 && raster_height > 0)
        {
            raster target(target_ext, raster_width,raster_height);
            scaling_method_e scaling_method = sym.get_scaling_method();
            double filter_radius = sym.calculate_filter_factor();
            bool premultiply_source = !source->premultiplied_alpha_;
            boost::optional<bool> is_premultiplied = sym.premultiplied();
            if (is_premultiplied)
            {
                if (*is_premultiplied) premultiply_source = false;
                else premultiply_source = true;
            }
            if (premultiply_source)
            {
                agg::rendering_buffer buffer(source->data_.getBytes(),source->data_.width(),source->data_.height(),source->data_.width() * 4);
                agg::pixfmt_rgba32 pixf(buffer);
                pixf.premultiply();
            }
            if (!prj_trans.equal())
            {
                double offset_x = ext.minx() - start_x;
                double offset_y = ext.miny() - start_y;
                reproject_and_scale_raster(target, *source, prj_trans,
                                 offset_x, offset_y,
                                 sym.get_mesh_size(),
                                 filter_radius,
                                 scaling_method);
            }
            else
            {
                if (scaling_method == SCALING_BILINEAR8)
                {
                    scale_image_bilinear8<image_data_32>(target.data_,source->data_, 0.0, 0.0);
                } else
                {
                    double scaling_ratio = ext.width() / source->data_.width();
                    scale_image_agg<image_data_32>(target.data_,
                                                   source->data_,
                                                   scaling_method,
                                                   scaling_ratio,
                                                   0.0,
                                                   0.0,
                                                   filter_radius);
                }
            }
            cairo_context context(context_);
            context.set_operator(sym.comp_op());
            context.add_image(start_x, start_y, target.data_, sym.get_opacity());
        }
    }
}

namespace detail {

template <typename Context, typename SvgPath, typename Attributes, typename Detector>
struct markers_dispatch
{
    markers_dispatch(Context & ctx,
                     SvgPath & marker,
                     Attributes const& attributes,
                     Detector & detector,
                     markers_symbolizer const& sym,
                     box2d<double> const& bbox,
                     agg::trans_affine const& marker_trans,
                     double scale_factor)
        :ctx_(ctx),
         marker_(marker),
         attributes_(attributes),
         detector_(detector),
         sym_(sym),
         bbox_(bbox),
         marker_trans_(marker_trans),
         scale_factor_(scale_factor) {}


    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();

        if (placement_method != MARKER_LINE_PLACEMENT)
        {
            double x = 0;
            double y = 0;
            if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                if (!label::interior_position(path, x, y))
                    return;
            }
            else
            {
                if (!label::centroid(path, x, y))
                    return;
            }
            coord2d center = bbox_.center();
            agg::trans_affine matrix = agg::trans_affine_translation(-center.x, -center.y);
            matrix *= marker_trans_;
            matrix *=agg::trans_affine_translation(x, y);

            box2d<double> transformed_bbox = bbox_ * matrix;

            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {
                render_vector_marker(ctx_, pixel_position(x,y), marker_, attributes_, marker_trans_, sym_.get_opacity(), true);

                if (!sym_.get_ignore_placement())
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      sym_.get_spacing() * scale_factor_,
                                                                      sym_.get_max_error(),
                                                                      sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                render_vector_marker(ctx_, pixel_position(x,y),marker_, attributes_, matrix, sym_.get_opacity(), true);

            }
        }
    }

    Context & ctx_;
    SvgPath & marker_;
    Attributes const& attributes_;
    Detector & detector_;
    markers_symbolizer const& sym_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    double scale_factor_;
};

template <typename Context, typename ImageMarker, typename Detector>
struct markers_dispatch_2
{
    markers_dispatch_2(Context & ctx,
                     ImageMarker & marker,
                     Detector & detector,
                     markers_symbolizer const& sym,
                     box2d<double> const& bbox,
                     agg::trans_affine const& marker_trans,
                     double scale_factor)
        :ctx_(ctx),
         marker_(marker),
         detector_(detector),
         sym_(sym),
         bbox_(bbox),
         marker_trans_(marker_trans),
         scale_factor_(scale_factor) {}


    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();

        if (placement_method != MARKER_LINE_PLACEMENT)
        {
            double x = 0;
            double y = 0;
            if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                if (!label::interior_position(path, x, y))
                    return;
            }
            else
            {
                if (!label::centroid(path, x, y))
                    return;
            }
            coord2d center = bbox_.center();
            agg::trans_affine matrix = agg::trans_affine_translation(-center.x, -center.y);
            matrix *= marker_trans_;
            matrix *=agg::trans_affine_translation(x, y);

            box2d<double> transformed_bbox = bbox_ * matrix;

            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {
                ctx_.add_image(matrix, *marker_, sym_.get_opacity());
                if (!sym_.get_ignore_placement())
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      sym_.get_spacing() * scale_factor_,
                                                                      sym_.get_max_error(),
                                                                      sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle))
            {
                coord2d center = bbox_.center();
                agg::trans_affine matrix = agg::trans_affine_translation(-center.x, -center.y);
                matrix *= marker_trans_;
                matrix *= agg::trans_affine_rotation(angle);
                matrix *= agg::trans_affine_translation(x, y);
                ctx_.add_image(matrix, *marker_, sym_.get_opacity());
            }
        }
    }

    Context & ctx_;
    ImageMarker & marker_;
    Detector & detector_;
    markers_symbolizer const& sym_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    double scale_factor_;
};

}
void cairo_renderer_base::process(markers_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef boost::mpl::vector<clip_line_tag,clip_poly_tag,transform_tag,smooth_tag> conv_types;

    cairo_context context(context_);
    context.set_operator(sym.comp_op());

    agg::trans_affine tr = agg::trans_affine_scaling(scale_factor_);

    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance().find(filename, true);
        if (mark && *mark)
        {
            agg::trans_affine geom_tr;
            evaluate_transform(geom_tr, feature, sym.get_transform());
            box2d<double> const& bbox = (*mark)->bounding_box();
            setup_transform_scaling(tr, bbox, feature, sym);
            evaluate_transform(tr, feature, sym.get_image_transform());

            if ((*mark)->is_vector())
            {
                using namespace mapnik::svg;
                typedef agg::pod_bvector<path_attributes> svg_attributes_type;
                typedef detail::markers_dispatch<cairo_context, mapnik::svg_storage_type,
                                             svg_attributes_type,label_collision_detector4> dispatch_type;

                boost::optional<svg_path_ptr> const& stock_vector_marker = (*mark)->get_vector_data();

                expression_ptr const& width_expr = sym.get_width();
                expression_ptr const& height_expr = sym.get_height();

                // special case for simple ellipse markers
                // to allow for full control over rx/ry dimensions
                if (filename == "shape://ellipse"
                    && (width_expr || height_expr))
                {
                    svg_storage_type marker_ellipse;
                    vertex_stl_adapter<svg_path_storage> stl_storage(marker_ellipse.source());
                    svg_path_adapter svg_path(stl_storage);
                    build_ellipse(sym, feature, marker_ellipse, svg_path);
                    svg_attributes_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    agg::trans_affine marker_tr = agg::trans_affine_scaling(scale_factor_);
                    evaluate_transform(marker_tr, feature, sym.get_image_transform());
                    box2d<double> bbox = marker_ellipse.bounding_box();

                    dispatch_type dispatch(context, marker_ellipse, result?attributes:(*stock_vector_marker)->attributes(),
                                           *detector_, sym, bbox, marker_tr, scale_factor_);
                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, dispatch, sym, t_, prj_trans, marker_tr, scale_factor_);

                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.set<transform_tag>(); //always transform
                    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
                    BOOST_FOREACH(geometry_type & geom, feature.paths())
                    {
                        converter.apply(geom);
                    }
                }
                else
                {
                    svg_attributes_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);

                    dispatch_type dispatch(context, **stock_vector_marker, result?attributes:(*stock_vector_marker)->attributes(),
                                           *detector_, sym, bbox, tr, scale_factor_);
                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, dispatch, sym, t_, prj_trans, tr, scale_factor_);

                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.set<transform_tag>(); //always transform
                    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
                    BOOST_FOREACH(geometry_type & geom, feature.paths())
                    {
                        converter.apply(geom);
                    }
                }
            }
            else // raster markers
            {
                typedef detail::markers_dispatch_2<cairo_context,
                                                   mapnik::image_ptr,
                                                   label_collision_detector4> dispatch_type;
                boost::optional<mapnik::image_ptr> marker = (*mark)->get_bitmap_data();
                if ( marker )
                {
                    dispatch_type dispatch(context, *marker,
                                           *detector_, sym, bbox, tr, scale_factor_);

                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, dispatch, sym, t_, prj_trans, tr, scale_factor_);

                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.set<transform_tag>(); //always transform
                    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
                    BOOST_FOREACH(geometry_type & geom, feature.paths())
                    {
                        converter.apply(geom);
                    }
                }
            }
        }
    }
}

void cairo_renderer_base::process(text_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    text_symbolizer_helper<face_manager<freetype_engine>,
        label_collision_detector4> helper(
            sym, feature, prj_trans,
            width_, height_,
            scale_factor_,
            t_, font_manager_, *detector_, query_extent_);

    cairo_context context(context_);
    context.set_operator(sym.comp_op());

    while (helper.next())
    {
        placements_type const& placements = helper.placements();
        for (unsigned int ii = 0; ii < placements.size(); ++ii)
        {
            context.add_text(placements[ii], face_manager_, font_manager_, scale_factor_);
        }
    }
}

template class cairo_renderer<Cairo::Surface>;
template class cairo_renderer<Cairo::Context>;
}

#endif // HAVE_CAIRO
