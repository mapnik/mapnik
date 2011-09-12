/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2008 Tom Hughes
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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/cairo_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/arrow.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/expression_evaluator.hpp>

// cairo
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <cairo-ft.h>

// boost
#include <boost/utility.hpp>
#include <boost/make_shared.hpp>

// stl
#ifdef MAPNIK_DEBUG
#include <iostream>
#endif

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

            r = r * a / 255;
            g = g * a / 255;
            b = b * a / 255;

            *out_ptr++ = (a << 24) | (r << 16) | (g << 8) | b;
        }
        // mark the surface as dirty as we've modified it behind cairo's back
        surface_->mark_dirty();
        pattern_ = Cairo::SurfacePattern::create(surface_);
    }

    ~cairo_pattern(void)
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

    Cairo::RefPtr<Cairo::SurfacePattern> const& pattern(void) const
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

    ~cairo_gradient(void)
    {
    }


    Cairo::RefPtr<Cairo::Gradient> const& gradient(void) const
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

    Cairo::RefPtr<Cairo::FontFace> const& face(void) const
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

cairo_face_manager::cairo_face_manager(boost::shared_ptr<freetype_engine> engine,
                                       face_manager<freetype_engine> & manager)
    : font_engine_(engine),
      font_manager_(manager)
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
        entry = cairo_face_ptr(new cairo_face(font_engine_, face));

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

    ~cairo_context(void)
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

    void set_dash(dash_array const &dashes)
    {
        std::valarray<double> d(dashes.size() * 2);
        dash_array::const_iterator itr = dashes.begin();
        dash_array::const_iterator end = dashes.end();
        int index = 0;

        for (; itr != end; ++itr)
        {
            d[index++] = itr->first;
            d[index++] = itr->second;
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
                    std::cerr << "Curve 3 not implemented" << std::endl;
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
                    std::cerr << "Unimplemented drawing command: " << cm << std::endl;
                    move_to(x, y);
                }
            }
            else if (agg::is_close(cm))
            {
                close_path();
            }
            else
            {
                std::cerr << "Unimplemented path command: " << cm << std::endl;
            }
        }
    }

    void rectangle(double x, double y, double w, double h)
    {
        context_->rectangle(x, y, w, h);
    }

    void stroke(void)
    {
        context_->stroke();
    }

    void fill(void)
    {
        context_->fill();
    }

    void paint(void)
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

        std::vector<Cairo::Glyph> glyphs;

        glyphs.push_back(glyph);

        context_->show_glyphs(glyphs);
    }

    void glyph_path(unsigned long index, double x, double y)
    {
        Cairo::Glyph glyph;

        glyph.index = index;
        glyph.x = x;
        glyph.y = y;

        std::vector<Cairo::Glyph> glyphs;

        glyphs.push_back(glyph);

        context_->glyph_path(glyphs);
    }

    void add_text(text_path & path,
                  cairo_face_manager & manager,
                  face_set_ptr const& faces,
                  unsigned text_size,
                  color const& fill,
                  unsigned halo_radius,
                  color const& halo_fill)
    {
        double sx = path.starting_x;
        double sy = path.starting_y;

        path.rewind();

        for (int iii = 0; iii < path.num_nodes(); iii++)
        {
            int c;
            double x, y, angle;

            path.vertex(&c, &x, &y, &angle);

            glyph_ptr glyph = faces->get_glyph(c);
 
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
            }
        }

        set_line_width(halo_radius);
        set_line_join(ROUND_JOIN);
        set_color(halo_fill);
        stroke();

        set_color(fill);

        path.rewind();

        for (int iii = 0; iii < path.num_nodes(); iii++)
        {
            int c;
            double x, y, angle;

            path.vertex(&c, &x, &y, &angle);

            glyph_ptr glyph = faces->get_glyph(c);
 
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

                show_glyph(glyph->get_index(), sx + x, sy - y);
            }
        }
    }


private:
    Cairo::RefPtr<Cairo::Context> context_;
};

cairo_renderer_base::cairo_renderer_base(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, unsigned offset_x, unsigned offset_y)
    : m_(m),
      context_(context),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(new freetype_engine()),
      font_manager_(*font_engine_),
      face_manager_(font_engine_,font_manager_),
      detector_(box2d<double>(-m.buffer_size() ,-m.buffer_size() , m.width() + m.buffer_size() ,m.height() + m.buffer_size()))
{
#ifdef MAPNIK_DEBUG
    std::clog << "scale=" << m.scale() << "\n";
#endif
}

template <>
cairo_renderer<Cairo::Context>::cairo_renderer(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m),
      cairo_renderer_base(m,context,offset_x,offset_y)
{
}

template <>
cairo_renderer<Cairo::Surface>::cairo_renderer(Map const& m, Cairo::RefPtr<Cairo::Surface> const& surface, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m),
      cairo_renderer_base(m,Cairo::Context::create(surface),offset_x,offset_y)
{
}

cairo_renderer_base::~cairo_renderer_base() {}

#ifdef MAPNIK_DEBUG
void cairo_renderer_base::start_map_processing(Map const& map)
{
    std::clog << "start map processing bbox="
              << map.get_current_extent() << "\n";
#else
void cairo_renderer_base::start_map_processing(Map const& /*map*/)
{
#endif

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 6, 0)
    box2d<double> bounds = t_.forward(t_.extent());
    context_->rectangle(bounds.minx(), bounds.miny(), bounds.maxx(), bounds.maxy());
    context_->clip();
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
#ifdef MAPNIK_DEBUG
    std::clog << "end map processing\n";
#endif
}

template <>
void cairo_renderer<Cairo::Surface>::end_map_processing(Map const& )
{
#ifdef MAPNIK_DEBUG
    std::clog << "end map processing\n";
#endif
    context_->show_page();
}

void cairo_renderer_base::start_layer_processing(layer const& lay)
{
#ifdef MAPNIK_DEBUG
    std::clog << "start layer processing : " << lay.name()  << "\n";
    std::clog << "datasource = " << lay.datasource().get() << "\n";
#endif
    if (lay.clear_label_cache())
    {
        detector_.clear();
    }
}

void cairo_renderer_base::end_layer_processing(layer const&)
{
#ifdef MAPNIK_DEBUG
    std::clog << "end layer processing\n";
#endif
}

void cairo_renderer_base::process(polygon_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;

    cairo_context context(context_);

    context.set_color(sym.get_fill(), sym.get_opacity());

    for (unsigned i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        if (geom.num_points() > 2)
        {
            path_type path(t_, geom, prj_trans);

            context.add_path(path);
            context.fill();
        }
    }
}

void cairo_renderer_base::process(building_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;
    typedef coord_transform3<CoordTransform,geometry_type> path_type_roof;

    cairo_context context(context_);

    color const& fill = sym.get_fill();
    double height = 0.7071 * sym.height(); // height in meters

    for (unsigned i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        if (geom.num_points() > 2)
        {
            boost::scoped_ptr<geometry_type> frame(new geometry_type(LineString));
            boost::scoped_ptr<geometry_type> roof(new geometry_type(Polygon));
            std::deque<segment_t> face_segments;
            double x0(0);
            double y0(0);
            unsigned cm = geom.vertex(&x0, &y0);

            for (unsigned j = 1; j < geom.num_points(); ++j)
            {
                double x=0;
                double y=0;

                cm = geom.vertex(&x,&y);

                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x,y);
                }
                else if (cm == SEG_LINETO)
                {
                    frame->line_to(x,y);
                }

                if (j != 0)
                {
                    face_segments.push_back(segment_t(x0, y0, x, y));
                }

                x0 = x;
                y0 = y;
            }

            std::sort(face_segments.begin(), face_segments.end(), y_order);
            std::deque<segment_t>::const_iterator itr = face_segments.begin();
            for (; itr != face_segments.end(); ++itr)
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

            for (unsigned j = 0; j < geom.num_points(); ++j)
            {
                double x, y;
                unsigned cm = geom.vertex(&x, &y);

                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x, y + height);
                    roof->move_to(x, y + height);
                }
                else if (cm == SEG_LINETO)
                {
                    frame->line_to(x, y + height);
                    roof->line_to(x, y + height);
                }
            }

            path_type path(t_, *frame, prj_trans);
            context.set_color(128, 128, 128, sym.get_opacity());
            context.add_path(path);
            context.stroke();

            path_type roof_path(t_, *roof, prj_trans);
            context.set_color(sym.get_fill(), sym.get_opacity());
            context.add_path(roof_path);
            context.fill();
        }
    }
}

void cairo_renderer_base::process(line_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;

    cairo_context context(context_);
    mapnik::stroke const& stroke_ = sym.get_stroke();

    context.set_color(stroke_.get_color(), stroke_.get_opacity());

    for (unsigned i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        if (geom.num_points() > 1)
        {
            cairo_context context(context_);
            path_type path(t_, geom, prj_trans);

            if (stroke_.has_dash())
            {
                context.set_dash(stroke_.get_dash_array());
            }

            context.set_line_join(stroke_.get_line_join());
            context.set_line_cap(stroke_.get_line_cap());
            context.set_miter_limit(4.0);
            context.set_line_width(stroke_.get_width());
            context.add_path(path);
            context.stroke();
        }
    }
}

void cairo_renderer_base::render_marker(const int x, const int y, marker &marker, const agg::trans_affine & tr, double opacity)

{
    cairo_context context(context_);
    if (marker.is_vector())
    {
        box2d<double> bbox;
        bbox = (*marker.get_vector_data())->bounding_box();

        coord<double,2> c = bbox.center();
        // center the svg marker on '0,0'
        agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
        // apply symbol transformation to get to map space
        mtx *= tr;
        // render the marker at the center of the marker box
        mtx.translate(x+0.5 * marker.width(), y+0.5 * marker.height());

        typedef coord_transform2<CoordTransform,geometry_type> path_type;
        mapnik::path_ptr vmarker = *marker.get_vector_data();

        agg::pod_bvector<path_attributes> const & attributes_ = vmarker->attributes();
        for(unsigned i = 0; i < attributes_.size(); ++i)
        {
            mapnik::svg::path_attributes const& attr = attributes_[i];
            if (!attr.visibility_flag)
                continue;

            context.save();

            agg::trans_affine transform = attr.transform;
            transform *= mtx;

            if (transform.is_valid() && !transform.is_identity())
            {
                double m[6];
                transform.store_to(m);
                context.transform(Cairo::Matrix(m[0],m[1],m[2],m[3],m[4],m[5]));
            }

            vertex_stl_adapter<svg_path_storage> stl_storage(vmarker->source());
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
                    cairo_gradient g(attr.fill_gradient,attr.opacity*opacity);

                    context.set_gradient(g,bbox);
                    context.fill();
                }
                else if(attr.fill_flag)
                {
                    context.set_color(attr.fill_color.r,attr.fill_color.g,attr.fill_color.b,attr.opacity*opacity);
                    context.fill();
                }
            }


            if(attr.stroke_gradient.get_gradient_type() != NO_GRADIENT || attr.stroke_flag)
            {
                context.add_agg_path(svg_path,attr.index);
                if(attr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
                {
                    context.set_line_width(attr.stroke_width);
                    context.set_line_cap(line_cap_enum(attr.line_cap));
                    context.set_line_join(line_join_enum(attr.line_join));
                    context.set_miter_limit(attr.miter_limit);
                    cairo_gradient g(attr.stroke_gradient,attr.opacity*opacity);
                    context.set_gradient(g,bbox);
                    context.stroke();
                }
                else if(attr.stroke_flag)
                {
                    context.set_color(attr.stroke_color.r,attr.stroke_color.g,attr.stroke_color.b,attr.opacity*opacity);
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
    else if (marker.is_bitmap())
    {
        context.add_image(x, y, **marker.get_bitmap_data(), opacity);
    }
}

void cairo_renderer_base::process(point_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{   
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);

    boost::optional<marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance()->find(filename, true);
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
                geom.label_position(&x, &y);
            else
                geom.label_interior_position(&x, &y);

            prj_trans.backward(x, y, z);
            t_.forward(&x, &y);

            int w = (*marker)->width();
            int h = (*marker)->height();

            int px = int(floor(x - 0.5 * w));
            int py = int(floor(y - 0.5 * h));
            box2d<double> label_ext (px, py, px + w, py + h);
            if (sym.get_allow_overlap() ||
                    detector_.has_placement(label_ext))
            {
                agg::trans_affine mtx;
                boost::array<double,6> const& m = sym.get_transform();
                mtx.load_from(&m[0]);

                render_marker(px,py,**marker, mtx, sym.get_opacity());

                if (!sym.get_ignore_placement())
                    detector_.insert(label_ext);
                metawriter_with_properties writer = sym.get_metawriter();
                if (writer.first)
                {
                    writer.first->add_box(label_ext, feature, t_, writer.second);
                }
            }
        }
    }
}

void cairo_renderer_base::process(shield_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;

    text_placement_info_ptr placement_options = sym.get_placement_options()->get_placement_info();
    placement_options->next();
    placement_options->next_position_only();

    UnicodeString text;
    if( sym.get_no_text() )
        text = UnicodeString( " " );  // TODO: fix->use 'space' as the text to render
    else
    {
        expression_ptr name_expr = sym.get_name();
        if (!name_expr) return;
        value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*name_expr);
        text = result.to_unicode();
    }

    if ( sym.get_text_transform() == UPPERCASE)
    {
        text = text.toUpper();
    }
    else if ( sym.get_text_transform() == LOWERCASE)
    {
        text = text.toLower();
    }
    else if ( sym.get_text_transform() == CAPITALIZE)
    {
        text = text.toTitle(NULL);
    }
    
    agg::trans_affine tr;
    boost::array<double,6> const& m = sym.get_transform();
    tr.load_from(&m[0]);

    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance()->find(filename, true);
    }
    else
    {
        marker.reset(boost::shared_ptr<mapnik::marker> (new mapnik::marker()));
    }

    if (text.length() > 0 && marker)
    {
        face_set_ptr faces;

        if (sym.get_fontset().size() > 0)
        {
            faces = font_manager_.get_face_set(sym.get_fontset());
        }
        else 
        {
            faces = font_manager_.get_face_set(sym.get_face_name());
        }

        if (faces->size() > 0)
        {
            cairo_context context(context_);
            string_info info(text);

            placement_finder<label_collision_detector4> finder(detector_);

            faces->set_pixel_sizes(placement_options->text_size);
            faces->get_string_info(info);

            int w = (*marker)->width();
            int h = (*marker)->height();

            metawriter_with_properties writer = sym.get_metawriter();

            for (unsigned i = 0; i < feature.num_geometries(); ++i)
            {
                geometry_type const& geom = feature.get_geometry(i);
                if (geom.num_points() > 0) // don't bother with empty geometries
                {
                    path_type path(t_, geom, prj_trans);

                    label_placement_enum how_placed = sym.get_label_placement();
                    if (how_placed == POINT_PLACEMENT || how_placed == VERTEX_PLACEMENT || how_placed == INTERIOR_PLACEMENT)
                    {
                        // for every vertex, try and place a shield/text
                        geom.rewind(0);
                        placement text_placement(info, sym, 1.0, w, h, false);
                        text_placement.avoid_edges = sym.get_avoid_edges();
                        text_placement.allow_overlap = sym.get_allow_overlap();
                        if (writer.first)
                            text_placement.collect_extents = true; // needed for inmem metawriter
                        position const& pos = sym.get_displacement();
                        position const& shield_pos = sym.get_shield_displacement();
                        for( unsigned jj = 0; jj < geom.num_points(); jj++ )
                        {
                            double label_x;
                            double label_y;
                            double z=0.0;

                            if( how_placed == VERTEX_PLACEMENT )
                                geom.vertex(&label_x,&label_y);  // by vertex
                            else if( how_placed == INTERIOR_PLACEMENT )
                                geom.label_interior_position(&label_x,&label_y);
                            else
                                geom.label_position(&label_x, &label_y);  // by middle of line or by point
                            prj_trans.backward(label_x,label_y, z);
                            t_.forward(&label_x,&label_y);

                            label_x += boost::get<0>(shield_pos);
                            label_y += boost::get<1>(shield_pos);

                            finder.find_point_placement(text_placement, placement_options,
                                                        label_x, label_y, 0.0,
                                                        sym.get_line_spacing(),
                                                        sym.get_character_spacing());

                            for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ ii)
                            {
                                double x = text_placement.placements[ii].starting_x;
                                double y = text_placement.placements[ii].starting_y;

                                int px;
                                int py;
                                box2d<double> label_ext;

                                if( !sym.get_unlock_image() )
                                {
                                    // center image at text center position
                                    // remove displacement from image label
                                    double lx = x - boost::get<0>(pos);
                                    double ly = y - boost::get<1>(pos);
                                    px=int(floor(lx - (0.5 * w)));
                                    py=int(floor(ly - (0.5 * h)));
                                    label_ext.init( floor(lx - 0.5 * w), floor(ly - 0.5 * h), ceil (lx + 0.5 * w), ceil (ly + 0.5 * h) );
                                }
                                else
                                {  // center image at reference location
                                    px=int(floor(label_x - 0.5 * w));
                                    py=int(floor(label_y - 0.5 * h));
                                    label_ext.init( floor(label_x - 0.5 * w), floor(label_y - 0.5 * h), ceil (label_x + 0.5 * w), ceil (label_y + 0.5 * h));
                                }

                                if ( sym.get_allow_overlap() || detector_.has_placement(label_ext) )
                                {
                                    render_marker(px,py,**marker, tr, sym.get_opacity());

                                    context.add_text(text_placement.placements[ii],
                                                     face_manager_,
                                                     faces,
                                                     placement_options->text_size,
                                                     sym.get_fill(),
                                                     sym.get_halo_radius(),
                                                     sym.get_halo_fill()
                                        );
                                    if (writer.first) {
                                        writer.first->add_box(box2d<double>(px,py,px+w,py+h), feature, t_, writer.second);
                                        writer.first->add_text(text_placement, faces, feature, t_, writer.second); //Only 1 placement
                                    }
                                    detector_.insert(label_ext);
                                }
                            }

                            finder.update_detector(text_placement);
                        }
                    }
                    else if (geom.num_points() > 1 && how_placed == LINE_PLACEMENT)
                    {
                        placement text_placement(info, sym, 1.0, w, h, true);

                        text_placement.avoid_edges = sym.get_avoid_edges();
                        finder.find_point_placements<path_type>(text_placement, placement_options, path);

                        position const&  pos = sym.get_displacement();
                        for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ ii)
                        {
                            double x = text_placement.placements[ii].starting_x;
                            double y = text_placement.placements[ii].starting_y;
                            double lx = x - boost::get<0>(pos);
                            double ly = y - boost::get<1>(pos);
                            int px=int(floor(lx - (0.5*w)));
                            int py=int(floor(ly - (0.5*h)));

                            render_marker(px,py,**marker, tr, sym.get_opacity());

                            context.add_text(text_placement.placements[ii],
                                             face_manager_,
                                             faces,
                                             placement_options->text_size,
                                             sym.get_fill(),
                                             sym.get_halo_radius(),
                                             sym.get_halo_fill()
                                );
                            if (writer.first) writer.first->add_box(box2d<double>(px,py,px+w,py+h), feature, t_, writer.second);
                        }
                        finder.update_detector(text_placement);
                        if (writer.first) writer.first->add_text(text_placement, faces, feature, t_, writer.second); //More than one placement
                    }
                }
            }
        }
    }
}

void cairo_renderer_base::process(line_pattern_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;
    
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<mapnik::marker_ptr> marker = mapnik::marker_cache::instance()->find(filename,true);
    if (!marker && !(*marker)->is_bitmap()) return;
    
    unsigned width((*marker)->width());
    unsigned height((*marker)->height());

    cairo_context context(context_);
    cairo_pattern pattern(**((*marker)->get_bitmap_data()));

    pattern.set_extend(Cairo::EXTEND_REPEAT);
    pattern.set_filter(Cairo::FILTER_BILINEAR);
    context.set_line_width(height);

    for (unsigned i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        if (geom.num_points() > 1)
        {
            path_type path(t_, geom, prj_trans);
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
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;

    cairo_context context(context_);
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<mapnik::marker_ptr> marker = mapnik::marker_cache::instance()->find(filename,true);
    if (!marker && !(*marker)->is_bitmap()) return;

    cairo_pattern pattern(**((*marker)->get_bitmap_data()));

    pattern.set_extend(Cairo::EXTEND_REPEAT);

    context.set_pattern(pattern);

    for (unsigned i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        if (geom.num_points() > 2)
        {
            path_type path(t_, geom, prj_trans);

            context.add_path(path);
            context.fill();
        }
    }
}

//FIXME: Port reprojection code from agg/process_raster_symbolizer.cpp
void cairo_renderer_base::process(raster_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& /*prj_trans*/)
{
    // TODO -- at the moment raster_symbolizer is an empty class
    // used for type dispatching, but we can have some fancy raster
    // processing in a future (filters??). Just copy raster into pixmap for now.
    raster_ptr const& raster = feature.get_raster();
    if (raster)
    {
        // If there's a colorizer defined, use it to color the raster in-place
        raster_colorizer_ptr colorizer = sym.get_colorizer();
        if (colorizer)
            colorizer->colorize(raster,feature.props());

        box2d<double> ext = t_.forward(raster->ext_);
        int start_x = (int)ext.minx();
        int start_y = (int)ext.miny();
        int end_x = (int)ceil(ext.maxx());
        int end_y = (int)ceil(ext.maxy());
        int raster_width = end_x - start_x;
        int raster_height = end_y - start_y;
        double err_offs_x = ext.minx() - start_x;
        double err_offs_y = ext.miny() - start_y;

        if (raster_width > 0 && raster_height > 0)
        {
            double scale_factor = ext.width() / raster->data_.width();
            image_data_32 target(raster_width, raster_height);
            //TODO -- use cairo matrix transformation for scaling
            if (sym.get_scaling() == "bilinear8"){
                scale_image_bilinear8<image_data_32>(target,raster->data_, err_offs_x, err_offs_y);
            } else {
                scaling_method_e scaling_method = get_scaling_method_by_name(sym.get_scaling());
                scale_image_agg<image_data_32>(target,raster->data_, scaling_method, scale_factor, err_offs_x, err_offs_y, sym.calculate_filter_factor());
            }
            cairo_context context(context_);
            //TODO -- support for advanced image merging
            context.add_image(start_x, start_y, target, sym.get_opacity());
        }
    }
}

void cairo_renderer_base::process(markers_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;
    arrow arrow_;
    cairo_context context(context_);

    color const& fill_ = sym.get_fill();
    context.set_color(fill_.red(), fill_.green(), fill_.blue(), fill_.alpha());

    for (unsigned i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        if (geom.num_points() > 1)
        {
            path_type path(t_, geom, prj_trans);

            markers_placement<path_type, label_collision_detector4> placement(path, arrow_.extent(), detector_, sym.get_spacing(), sym.get_max_error(), sym.get_allow_overlap());

            double x, y, angle;
            while (placement.get_point(&x, &y, &angle)) {
                Cairo::Matrix matrix = Cairo::rotation_matrix(angle) * Cairo::translation_matrix(x,y) ;
                context.set_matrix(matrix);
                context.add_path(arrow_);
            }
        }
        context.fill();
    }
}

void cairo_renderer_base::process(glyph_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    face_set_ptr faces = font_manager_.get_face_set(sym.get_face_name());
    if (faces->size() > 0)
    {
        // Get x and y from geometry and translate to pixmap coords.
        double x, y, z=0.0;
        feature.get_geometry(0).label_position(&x, &y);
        prj_trans.backward(x,y,z);
        t_.forward(&x, &y);

        // set font size
        unsigned size = sym.eval_size(feature);
        faces->set_pixel_sizes(size);

        // Get and render text path
        //
        text_path_ptr path = sym.get_text_path(faces, feature);
        // apply displacement
        position pos = sym.get_displacement();
        double dx = boost::get<0>(pos);
        double dy = boost::get<1>(pos);
        path->starting_x = x = x+dx;
        path->starting_y = y = y+dy;

        // get fill and halo params
        color fill = sym.eval_color(feature);
        color halo_fill = sym.get_halo_fill();
        double halo_radius = sym.get_halo_radius();
        if (fill==color("transparent"))
            halo_radius = 0;

        double bsize = size/2 + 1;
        box2d<double> glyph_ext(
            floor(x-bsize), floor(y-bsize), ceil(x+bsize), ceil(y+bsize)
            );
        if ((sym.get_allow_overlap() || detector_.has_placement(glyph_ext)) &&
            (!sym.get_avoid_edges() || detector_.extent().contains(glyph_ext)))
        {    
            // Placement is valid, render glyph and update detector.
            cairo_context context(context_);
            context.add_text(*path,
                             face_manager_,
                             faces,
                             size,
                             fill,
                             halo_radius,
                             halo_fill
                );
            detector_.insert(glyph_ext);
            metawriter_with_properties writer = sym.get_metawriter();
            if (writer.first) writer.first->add_box(glyph_ext, feature, t_, writer.second);
        }
    }
    else
    {
        throw config_error(
            "Unable to find specified font face in GlyphSymbolizer"
            );
    }
}

void cairo_renderer_base::process(text_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;

    bool placement_found = false;
    text_placement_info_ptr placement_options = sym.get_placement_options()->get_placement_info();
    while (!placement_found && placement_options->next())
    {
        expression_ptr name_expr = sym.get_name();
        if (!name_expr) return;
        value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*name_expr);
        UnicodeString text = result.to_unicode();

        if ( sym.get_text_transform() == UPPERCASE)
        {
            text = text.toUpper();
        }
        else if ( sym.get_text_transform() == LOWERCASE)
        {
            text = text.toLower();
        }
        else if ( sym.get_text_transform() == CAPITALIZE)
        {
            text = text.toTitle(NULL);
        }

        if (text.length() <= 0) continue;

        face_set_ptr faces;

        if (sym.get_fontset().size() > 0)
        {
            faces = font_manager_.get_face_set(sym.get_fontset());
        }
        else
        {
            faces = font_manager_.get_face_set(sym.get_face_name());
        }

        if (faces->size() == 0)
        {
            throw config_error("Unable to find specified font face '" + sym.get_face_name() + "'");
        }
        cairo_context context(context_);
        string_info info(text);

        faces->set_pixel_sizes(placement_options->text_size);
        faces->get_string_info(info);

        placement_finder<label_collision_detector4> finder(detector_);

        metawriter_with_properties writer = sym.get_metawriter();

        unsigned num_geom = feature.num_geometries();
        for (unsigned i=0; i<num_geom; ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            if (geom.num_points() == 0) continue;// don't bother with empty geometries
            while (!placement_found && placement_options->next_position_only())
            {
                placement text_placement(info, sym, 1.0);
                text_placement.avoid_edges = sym.get_avoid_edges();
                if (writer.first)
                    text_placement.collect_extents = true; // needed for inmem metawriter

                if (sym.get_label_placement() == POINT_PLACEMENT ||
                        sym.get_label_placement() == INTERIOR_PLACEMENT)
                {
                    double label_x, label_y, z=0.0;
                    if (sym.get_label_placement() == POINT_PLACEMENT)
                        geom.label_position(&label_x, &label_y);
                    else
                        geom.label_interior_position(&label_x, &label_y);
                    prj_trans.backward(label_x,label_y, z);
                    t_.forward(&label_x,&label_y);

                    double angle = 0.0;
                    expression_ptr angle_expr = sym.get_orientation();
                    if (angle_expr)
                    {
                        // apply rotation
                        value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*angle_expr);
                        angle = result.to_double();
                    }

                    finder.find_point_placement(text_placement, placement_options,
                                                label_x, label_y,
                                                angle, sym.get_line_spacing(),
                                                sym.get_character_spacing());
                    finder.update_detector(text_placement);
                }
                else if ( geom.num_points() > 1 && sym.get_label_placement() == LINE_PLACEMENT)
                {
                    path_type path(t_, geom, prj_trans);
                    finder.find_line_placements<path_type>(text_placement, placement_options, path);
                }

                if (!text_placement.placements.size()) continue;
                placement_found = true;

                for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ii)
                {
                    context.add_text(text_placement.placements[ii],
                                     face_manager_,
                                     faces,
                                     placement_options->text_size,
                                     sym.get_fill(),
                                     sym.get_halo_radius(),
                                     sym.get_halo_fill()
                                     );
                }

                if (writer.first) writer.first->add_text(text_placement, faces, feature, t_, writer.second);
            }
        }
    }
}

template class cairo_renderer<Cairo::Surface>;
template class cairo_renderer<Cairo::Context>;
}

#endif
