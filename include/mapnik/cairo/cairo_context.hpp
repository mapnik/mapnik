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

#ifndef MAPNIK_CAIRO_CONTEXT_HPP
#define MAPNIK_CAIRO_CONTEXT_HPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/gradient.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/symbolizer_enumerations.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <cairo.h>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <memory>
#include <map>
#include <stdexcept>
#include <random>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template<typename T>
class box2d;

using ErrorStatus = cairo_status_t;

/// Throws the appropriate exception, if exceptions are enabled.
inline void throw_exception(ErrorStatus status)
{
    throw std::runtime_error(std::string("cairo: ") + cairo_status_to_string(status));
}

// We inline this because it is called so often.
inline void check_status_and_throw_exception(ErrorStatus status)
{
    if (status != CAIRO_STATUS_SUCCESS)
        throw_exception(status);
}

template<class T>
void check_object_status_and_throw_exception(T const& object)
{
    check_status_and_throw_exception(object.get_status());
}

class cairo_face : private util::noncopyable
{
  public:
    cairo_face(std::shared_ptr<font_library> const& library, face_ptr const& face);
    ~cairo_face();
    cairo_font_face_t* face() const;

  private:
    class handle
    {
      public:
        handle(std::shared_ptr<font_library> const& library, face_ptr const& face)
            : library_(library),
              face_(face)
        {}

      private:
        std::shared_ptr<font_library> library_;
        face_ptr face_;
    };

    static void destroy(void* data)
    {
        handle* h = static_cast<handle*>(data);
        delete h;
    }

  private:
    face_ptr face_;
    cairo_font_face_t* c_face_;
};

using cairo_face_ptr = std::shared_ptr<cairo_face>;

class cairo_face_manager : private util::noncopyable
{
  public:
    cairo_face_manager(std::shared_ptr<font_library> library);
    cairo_face_ptr get_face(face_ptr face);

  private:
    using cairo_face_cache = std::map<face_ptr, cairo_face_ptr>;
    std::shared_ptr<font_library> font_library_;
    cairo_face_cache cache_;
};

struct cairo_closer
{
    void operator()(cairo_t* obj)
    {
        if (obj)
            cairo_destroy(obj);
    }
};

struct cairo_surface_closer
{
    void operator()(cairo_surface_t* surface)
    {
        if (surface)
            cairo_surface_destroy(surface);
    }
};

using cairo_ptr = std::shared_ptr<cairo_t>;
using cairo_surface_ptr = std::shared_ptr<cairo_surface_t>;

inline cairo_ptr create_context(cairo_surface_ptr const& surface)
{
    return cairo_ptr(cairo_create(&*surface), cairo_closer());
}

class cairo_pattern : private util::noncopyable
{
  public:
    explicit cairo_pattern(image_rgba8 const& data, double opacity = 1.0)
    {
        std::size_t pixels = data.width() * data.height();
        const unsigned int* in_ptr = data.data();
        const unsigned int* in_end = in_ptr + pixels;
        unsigned int* out_ptr;

        surface_ = cairo_surface_ptr(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                                static_cast<int>(data.width()),
                                                                static_cast<int>(data.height())),
                                     cairo_surface_closer());

        out_ptr = reinterpret_cast<unsigned int*>(cairo_image_surface_get_data(surface_.get()));

        while (in_ptr < in_end)
        {
            unsigned int in = *in_ptr++;
            unsigned int r = static_cast<unsigned>(((in >> 0) & 0xff) * opacity);
            unsigned int g = static_cast<unsigned>(((in >> 8) & 0xff) * opacity);
            unsigned int b = static_cast<unsigned>(((in >> 16) & 0xff) * opacity);
            unsigned int a = static_cast<unsigned>(((in >> 24) & 0xff) * opacity);

            // r = r * a / 255;
            // g = g * a / 255;
            // b = b * a / 255;

            *out_ptr++ = (a << 24) | (r << 16) | (g << 8) | b;
        }
        // mark the surface as dirty as we've modified it behind cairo's back
        cairo_surface_mark_dirty(surface_.get());
        pattern_ = cairo_pattern_create_for_surface(surface_.get());
    }

    cairo_pattern(cairo_surface_ptr const& surface)
        : surface_(surface),
          pattern_(cairo_pattern_create_for_surface(surface_.get()))
    {}

    ~cairo_pattern()
    {
        if (pattern_)
            cairo_pattern_destroy(pattern_);
    }

    void set_matrix(cairo_matrix_t const& matrix) { cairo_pattern_set_matrix(pattern_, &matrix); }

    void set_origin(double x, double y)
    {
        cairo_matrix_t matrix;
        cairo_pattern_get_matrix(pattern_, &matrix);
        matrix.x0 = -x;
        matrix.y0 = -y;
        cairo_pattern_set_matrix(pattern_, &matrix);
    }

    void set_extend(cairo_extend_t extend) { cairo_pattern_set_extend(pattern_, extend); }

    void set_filter(cairo_filter_t filter) { cairo_pattern_set_filter(pattern_, filter); }

    cairo_pattern_t* pattern() const { return pattern_; }

  private:
    cairo_surface_ptr surface_;
    cairo_pattern_t* pattern_;
};

class cairo_gradient : private util::noncopyable
{
  public:
    cairo_gradient(mapnik::gradient const& grad, double opacity = 1.0)
    {
        double x1, x2, y1, y2, rad;
        grad.get_control_points(x1, y1, x2, y2, rad);
        if (grad.get_gradient_type() == LINEAR)
        {
            pattern_ = cairo_pattern_create_linear(x1, y1, x2, y2);
        }
        else
        {
            pattern_ = cairo_pattern_create_radial(x1, y1, 0, x2, y2, rad);
        }

        units_ = grad.get_units();

        for (mapnik::stop_pair const& st : grad.get_stop_array())
        {
            mapnik::color const& stop_color = st.second;
            double r = static_cast<double>(stop_color.red()) / 255.0;
            double g = static_cast<double>(stop_color.green()) / 255.0;
            double b = static_cast<double>(stop_color.blue()) / 255.0;
            double a = static_cast<double>(stop_color.alpha()) / 255.0;
            cairo_pattern_add_color_stop_rgba(pattern_, st.first, r, g, b, a * opacity);
        }

        double m[6];
        agg::trans_affine tr = grad.get_transform();
        tr.invert();
        tr.store_to(m);
        cairo_matrix_t matrix;
        cairo_matrix_init(&matrix, m[0], m[1], m[2], m[3], m[4], m[5]);
        cairo_pattern_set_matrix(pattern_, &matrix);
    }

    ~cairo_gradient()
    {
        if (pattern_)
            cairo_pattern_destroy(pattern_);
    }

    cairo_pattern_t* gradient() const { return pattern_; }

    gradient_unit_e units() const { return units_; }

  private:
    cairo_pattern_t* pattern_;
    gradient_unit_e units_;
};

class cairo_context : private util::noncopyable
{
  public:
    cairo_context(cairo_ptr const& cairo);

    inline ErrorStatus get_status() const { return cairo_status(cairo_.get()); }

    void clip();
    void show_page();
    void set_color(color const& color, double opacity = 1.0);
    void set_color(double r, double g, double b, double opacity = 1.0);
    void set_operator(composite_mode_e comp_op);
    void set_line_join(line_join_e join);
    void set_line_cap(line_cap_e cap);
    void set_miter_limit(double limit);
    void set_line_width(double width);
    void set_dash(dash_array const& dashes, double scale_factor);
    void set_fill_rule(cairo_fill_rule_t fill_rule);
    void move_to(double x, double y);
    void curve_to(double ct1_x, double ct1_y, double ct2_x, double ct2_y, double end_x, double end_y);
    void close_path();
    void line_to(double x, double y);
    void rectangle(double x, double y, double w, double h);
    void stroke();
    void fill();
    void paint();
    void paint(double opacity);
    void set_pattern(cairo_pattern const& pattern);
    void set_gradient(cairo_gradient const& pattern, box2d<double> const& bbox);
    void add_image(double x, double y, image_rgba8 const& data, double opacity = 1.0);
    void add_image(agg::trans_affine const& tr, image_rgba8 const& data, double opacity = 1.0);
    void set_font_face(cairo_face_manager& manager, face_ptr face);
    void set_font_matrix(cairo_matrix_t const& matrix);
    void set_matrix(cairo_matrix_t const& matrix);
    void transform(cairo_matrix_t const& matrix);
    void translate(double x, double y);
    void save();
    void restore();
    void show_glyph(unsigned long index, pixel_position const& pos);
    void glyph_path(unsigned long index, pixel_position const& pos);
    void add_text(glyph_positions const& pos,
                  cairo_face_manager& manager,
                  composite_mode_e comp_op = src_over,
                  composite_mode_e halo_comp_op = src_over,
                  double scale_factor = 1.0);

    void push_group();
    void pop_group();

    template<typename T>
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

    template<typename T>
    void add_agg_path(T& path, unsigned start_index = 0)
    {
        double x = 0;
        double y = 0;

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
                    double end_x = 0;
                    double end_y = 0;

                    MAPNIK_LOG_WARN(cairo_renderer) << "Curve 3 not implemented";

                    path.vertex(&end_x, &end_y);

                    curve_to(x, y, x, y, end_x, end_y);
                }
                else if (agg::is_curve4(cm))
                {
                    double ct2_x = 0;
                    double ct2_y = 0;
                    double end_x = 0;
                    double end_y = 0;

                    path.vertex(&ct2_x, &ct2_y);
                    path.vertex(&end_x, &end_y);

                    curve_to(x, y, ct2_x, ct2_y, end_x, end_y);
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
    inline double generate_offset()
    {
        std::mt19937 gen(rd_());
        std::uniform_real_distribution<> dis{0.0, 1e-6};
        return dis(gen);
    }

  private:
    cairo_ptr cairo_;
    std::random_device rd_;
};

template<typename Context>
struct line_pattern_rasterizer
{
    line_pattern_rasterizer(Context& context, cairo_pattern& pattern, unsigned width, unsigned height)
        : context_(context),
          pattern_(pattern),
          width_(width),
          height_(height)
    {}

    template<typename T>
    void add_path(T& path, unsigned start_index = 0)
    {
        double length = 0.0;
        double x0 = 0.0;
        double y0 = 0.0;
        double x, y;
        path.rewind(start_index);
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
                double angle = std::atan2(dy, dx);
                double offset = std::fmod(length, static_cast<double>(width_));

                cairo_matrix_t matrix;
                cairo_matrix_init_identity(&matrix);
                cairo_matrix_translate(&matrix, x0, y0);
                cairo_matrix_rotate(&matrix, angle);
                cairo_matrix_translate(&matrix, -offset, 0.5 * height_);
                cairo_matrix_invert(&matrix);
                pattern_.set_matrix(matrix);
                context_.set_pattern(pattern_);
                context_.move_to(x0, y0);
                context_.line_to(x, y);
                context_.stroke();
                length = length + std::hypot(x - x0, y - y0);
            }

            x0 = x;
            y0 = y;
        }
    }

    Context& context_;
    cairo_pattern& pattern_;
    unsigned width_;
    unsigned height_;
};

} // namespace mapnik

#endif // MAPNIK_CAIRO_CONTEXT_HPP
