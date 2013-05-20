/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#include <mapnik/cairo_context.hpp>
#include <mapnik/font_util.hpp>
#include <mapnik/text_properties.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/font_set.hpp>

#include <valarray>
namespace mapnik {

cairo_face::cairo_face(boost::shared_ptr<freetype_engine> const& engine, face_ptr const& face)
    : face_(face)
{
    static cairo_user_data_key_t key;
    c_face_ = cairo_ft_font_face_create_for_ft_face(face->get_face(), FT_LOAD_NO_HINTING);
    cairo_font_face_set_user_data(c_face_, &key, new handle(engine, face), destroy);
}

cairo_face::~cairo_face()
{
    if (c_face_) cairo_font_face_destroy(c_face_);
}

cairo_font_face_t * cairo_face::face() const
{
    return c_face_;
}

cairo_context::cairo_context(cairo_ptr const& cairo)
    : cairo_(cairo)
{}

void cairo_context::clip()
{
    cairo_clip(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::show_page()
{
    cairo_show_page(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_color(color const &color, double opacity)
{
    set_color(color.red()/255.0, color.green()/255.0, color.blue()/255.0, color.alpha() * opacity / 255.0);
}

void cairo_context::set_color(double r, double g, double b, double opacity)
{
    // http://lists.cairographics.org/archives/cairo/2008-August/014759.html
    cairo_set_source_rgba(cairo_.get(), r, g, b, opacity);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_operator(composite_mode_e comp_op)
{
    switch (comp_op)
    {
    case clear:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_CLEAR);
        break;
    case src:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_SOURCE);
        break;
    case dst:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_DEST);
        break;
    case src_over:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_OVER);
        break;
    case dst_over:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_DEST_OVER);
        break;
    case src_in:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_IN);
        break;
    case dst_in:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_DEST_IN);
        break;
    case src_out:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_OUT);
        break;
    case dst_out:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_DEST_OUT);
        break;
    case src_atop:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_ATOP);
        break;
    case dst_atop:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_DEST_ATOP);
        break;
    case _xor:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_XOR);
        break;
    case plus:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_ADD);
        break;
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0)
    case multiply:
        cairo_set_operator(cairo_.get(), CAIRO_OPERATOR_MULTIPLY);
        break;
    case screen:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_SCREEN);
        break;
    case overlay:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_OVERLAY);
        break;
    case darken:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_DARKEN);
        break;
    case lighten:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_LIGHTEN);
        break;
    case color_dodge:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_COLOR_DODGE);
        break;
    case color_burn:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_COLOR_BURN);
        break;
    case hard_light:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_HARD_LIGHT);
        break;
    case soft_light:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_SOFT_LIGHT);
        break;
    case difference:
        cairo_set_operator(cairo_.get(),CAIRO_OPERATOR_DIFFERENCE);
        break;
    case exclusion:
        cairo_set_operator(cairo_.get(), CAIRO_OPERATOR_EXCLUSION);
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
    //
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_line_join(line_join_e join)
{
    if (join == MITER_JOIN)
        cairo_set_line_join(cairo_.get(), CAIRO_LINE_JOIN_MITER);
    else if (join == MITER_REVERT_JOIN)
        cairo_set_line_join(cairo_.get(), CAIRO_LINE_JOIN_MITER);
    else if (join == ROUND_JOIN)
        cairo_set_line_join(cairo_.get(), CAIRO_LINE_JOIN_ROUND);
    else
        cairo_set_line_join(cairo_.get(), CAIRO_LINE_JOIN_BEVEL);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_line_cap(line_cap_e cap)
{
    if (cap == BUTT_CAP)
        cairo_set_line_cap(cairo_.get(), CAIRO_LINE_CAP_BUTT);
    else if (cap == SQUARE_CAP)
        cairo_set_line_cap(cairo_.get(), CAIRO_LINE_CAP_SQUARE);
    else
        cairo_set_line_cap(cairo_.get(), CAIRO_LINE_CAP_ROUND);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_miter_limit(double limit)
{
    cairo_set_miter_limit(cairo_.get(), limit);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_line_width(double width)
{
    cairo_set_line_width(cairo_.get(), width);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_dash(dash_array const &dashes, double scale_factor)
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

    cairo_set_dash(cairo_.get() , &d[0], d.size(), 0/*offset*/);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_fill_rule(cairo_fill_rule_t fill_rule)
{
    cairo_set_fill_rule(cairo_.get(),fill_rule);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::move_to(double x, double y)
{
#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 6, 0)
    if (x < -32767.0) x = -32767.0;
    else if (x > 32767.0) x = 32767.0;
    if (y < -32767.0) y = -32767.0;
    else if (y > 32767.0) y = 32767.0;
#endif
    cairo_move_to(cairo_.get(), x, y);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::curve_to(double ct1_x, double ct1_y, double ct2_x, double ct2_y, double end_x, double end_y)
{
    cairo_curve_to(cairo_.get(), ct1_x,ct1_y,ct2_x,ct2_y,end_x,end_y);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::close_path()
{
    cairo_close_path(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::line_to(double x, double y)
{
#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 6, 0)
    if (x < -32767.0) x = -32767.0;
    else if (x > 32767.0) x = 32767.0;
    if (y < -32767.0) y = -32767.0;
    else if (y > 32767.0) y = 32767.0;
#endif
    cairo_line_to(cairo_.get(), x, y);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::rectangle(double x, double y, double w, double h)
{
    cairo_rectangle(cairo_.get(), x, y, w, h);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::stroke()
{
    cairo_stroke(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::fill()
{
    cairo_fill(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::paint()
{
    cairo_paint(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_pattern(cairo_pattern const& pattern)
{
    cairo_set_source(cairo_.get(), pattern.pattern());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_gradient(cairo_gradient const& pattern, const box2d<double> &bbox)
{
    cairo_pattern_t * gradient = pattern.gradient();
    double bx1=bbox.minx();
    double by1=bbox.miny();
    double bx2=bbox.maxx();
    double by2=bbox.maxy();
    if (pattern.units() != USER_SPACE_ON_USE)
    {
        if (pattern.units() == OBJECT_BOUNDING_BOX)
        {
            cairo_path_extents(cairo_.get(), &bx1, &by1, &bx2, &by2);
        }
        cairo_matrix_t cairo_matrix;
        cairo_pattern_get_matrix(gradient, &cairo_matrix);
        cairo_matrix_scale(&cairo_matrix,1.0/(bx2-bx1),1.0/(by2-by1));
        cairo_matrix_translate(&cairo_matrix, -bx1,-by1);
        cairo_pattern_set_matrix(gradient, &cairo_matrix);
    }
    cairo_set_source(cairo_.get(), const_cast<cairo_pattern_t*>(gradient));
    check_object_status_and_throw_exception(*this);
}

void cairo_context::add_image(double x, double y, image_data_32 & data, double opacity)
{
    cairo_pattern pattern(data);
    pattern.set_origin(x, y);
    cairo_save(cairo_.get());
    cairo_set_source(cairo_.get(), const_cast<cairo_pattern_t*>(pattern.pattern()));
    cairo_paint_with_alpha(cairo_.get(), opacity);
    cairo_restore(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::add_image(agg::trans_affine const& tr, image_data_32 & data, double opacity)
{
    cairo_pattern pattern(data);
    if (!tr.is_identity())
    {
        double m[6];
        tr.store_to(m);
        cairo_matrix_t cairo_matrix;
        cairo_matrix_init(&cairo_matrix,m[0],m[1],m[2],m[3],m[4],m[5]);
        cairo_matrix_invert(&cairo_matrix);
        pattern.set_matrix(cairo_matrix);
    }
    cairo_save(cairo_.get());
    cairo_set_source(cairo_.get(), const_cast<cairo_pattern_t*>(pattern.pattern()));
    cairo_paint_with_alpha(cairo_.get(), opacity);
    cairo_restore(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_font_face(cairo_face_manager & manager, face_ptr face)
{
    cairo_set_font_face(cairo_.get(), manager.get_face(face)->face());
}

void cairo_context::set_font_matrix(cairo_matrix_t const& matrix)
{
    cairo_set_font_matrix(cairo_.get(), &matrix);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::set_matrix(cairo_matrix_t const& matrix)
{
    cairo_set_matrix(cairo_.get(), &matrix);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::transform(cairo_matrix_t const& matrix)
{
    cairo_transform(cairo_.get(), &matrix);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::translate(double x, double y)
{
    cairo_translate(cairo_.get(), x, y);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::save()
{
    cairo_save(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::restore()
{
    cairo_restore(cairo_.get());
    check_object_status_and_throw_exception(*this);
}

void cairo_context::show_glyph(unsigned long index, double x, double y)
{
    cairo_glyph_t glyph;
    glyph.index = index;
    glyph.x = x;
    glyph.y = y;

    cairo_show_glyphs(cairo_.get(), &glyph, 1);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::glyph_path(unsigned long index, double x, double y)
{
    cairo_glyph_t glyph;
    glyph.index = index;
    glyph.x = x;
    glyph.y = y;

    cairo_glyph_path(cairo_.get(), &glyph, 1);
    check_object_status_and_throw_exception(*this);
}

void cairo_context::add_text(text_path const& path,
                             cairo_face_manager & manager,
                             face_manager<freetype_engine> & font_manager,
                             double scale_factor)
{
    double sx = path.center.x;
    double sy = path.center.y;

    path.rewind();

    for (int iii = 0; iii < path.num_nodes(); iii++)
    {
        char_info_ptr c;
        double x, y, angle;

        path.vertex(c, x, y, angle);

        face_set_ptr faces = font_manager.get_face_set(c->format->face_name, c->format->fontset);
        double text_size = c->format->text_size * scale_factor;
        faces->set_character_sizes(text_size);

        glyph_ptr glyph = faces->get_glyph(c->c);

        if (glyph)
        {
            cairo_matrix_t matrix;
            matrix.xx = text_size * std::cos(angle);
            matrix.xy = text_size * std::sin(angle);
            matrix.yx = text_size * -std::sin(angle);
            matrix.yy = text_size * std::cos(angle);
            matrix.x0 = 0;
            matrix.y0 = 0;
            set_font_matrix(matrix);
            set_font_face(manager, glyph->get_face());
            glyph_path(glyph->get_index(), sx + x, sy - y);
            set_line_width(2.0 * c->format->halo_radius * scale_factor);
            set_line_join(ROUND_JOIN);
            set_color(c->format->halo_fill);
            stroke();
        }
    }

    path.rewind();

    for (int iii = 0; iii < path.num_nodes(); iii++)
    {
        char_info_ptr c;
        double x, y, angle;

        path.vertex(c, x, y, angle);

        face_set_ptr faces = font_manager.get_face_set(c->format->face_name, c->format->fontset);
        double text_size = c->format->text_size * scale_factor;
        faces->set_character_sizes(text_size);

        glyph_ptr glyph = faces->get_glyph(c->c);

        if (glyph)
        {
            cairo_matrix_t matrix;
            matrix.xx = text_size * std::cos(angle);
            matrix.xy = text_size * std::sin(angle);
            matrix.yx = text_size * -std::sin(angle);
            matrix.yy = text_size * std::cos(angle);
            matrix.x0 = 0;
            matrix.y0 = 0;
            set_font_matrix(matrix);
            set_font_face(manager, glyph->get_face());
            set_color(c->format->fill);
            show_glyph(glyph->get_index(), sx + x, sy - y);
        }
    }

}
}
