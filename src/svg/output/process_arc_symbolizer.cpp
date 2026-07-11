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

#if defined(SVG_RENDERER)

// mapnik
#include <mapnik/svg/output/svg_renderer.hpp>
#include <mapnik/svg/output/svg_output_attributes.hpp>
#include <mapnik/svg/output/svg_output_grammars.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/color.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/util/noncopyable.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <cmath>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace mapnik {
namespace detail {

// SVG colours carry no alpha channel (path_output_attributes emits plain
// #rrggbb via color::to_hex_string only for opaque colours), so the colour's
// own alpha is folded into the *-opacity attribute instead — the same product
// the raster backends compute per pixel.
inline color opaque(color const& c)
{
    return color(c.red(), c.green(), c.blue());
}

inline double combined_opacity(color const& c, double opacity)
{
    return opacity * c.alpha() / 255.0;
}

inline void append_coord(std::string& d, double x, double y)
{
    std::string val;
    util::to_string(val, x);
    d += val;
    d += ',';
    util::to_string(val, y);
    d += val;
}

// Emits a circular arc / sector ("pie wedge") at every point vertex of a
// geometry, mirroring the agg and cairo backends. The fill wedge, the two
// radius spokes and the curved arc line are emitted as separate <path>
// elements so each can carry its own presentation attributes; the attributes
// themselves are generated through the renderer's karma grammars
// (svg_path_attributes_grammar / svg_path_dash_array_grammar), the same
// machinery process_symbolizers.cpp uses for line/polygon paths. Only the
// path data ("d") is produced here, because the shared grammars have no
// support for elliptical-arc commands.
//
// Angle convention: start_angle / end_angle are compass bearings in degrees,
// measured clockwise from north (up), swept clockwise. Kept identical to the
// raster backends so all renderers produce the same geometry.
template<typename OutputIterator>
struct svg_arc_renderer : util::noncopyable
{
    svg_arc_renderer(OutputIterator& out,
                     view_transform const& tr,
                     proj_transform const& prj_trans,
                     double radius,
                     double start_angle,
                     double end_angle,
                     svg::path_output_attributes const& arc_attributes)
        : out_(out),
          tr_(tr),
          prj_trans_(prj_trans),
          radius_(radius),
          arc_attributes_(arc_attributes),
          emitted_(false)
    {
        // Sweep angles in radians, clockwise from north, wrap-around handled
        // (e.g. 350 -> 10 degrees). Equal angles mean a full circle, as
        // charted for all-round lights.
        double a0 = util::radians(start_angle);
        double a1 = util::radians(end_angle);
        if (a1 <= a0)
            a1 += util::tau;

        // All per-vertex geometry is the centre point plus fixed offsets;
        // precompute the offsets once. The sweep is split into sub-arcs of at
        // most pi so a single elliptical-arc command is never ambiguous (and
        // full circles, which "A" cannot express in one command, work too).
        start_offset_ = arc_offset(a0);
        double const span = a1 - a0;
        double const half_turn = util::tau / 2.0;
        int const segs = std::max(1, static_cast<int>(std::ceil(span / half_turn)));
        arc_offsets_.reserve(segs);
        for (int i = 1; i <= segs; ++i)
        {
            arc_offsets_.push_back(arc_offset(a0 + span * (static_cast<double>(i) / segs)));
        }
        util::to_string(radius_str_, radius_);
    }

    template<typename Adapter>
    void operator()(Adapter const& va)
    {
        double x, y, z = 0;
        unsigned cmd = SEG_END;
        va.rewind(0);
        while ((cmd = va.vertex(&x, &y)) != mapnik::SEG_END)
        {
            if (cmd == SEG_CLOSE)
                continue;
            prj_trans_.backward(x, y, z);
            tr_.forward(&x, &y);
            render_one(x, y);
        }
    }

    bool emitted() const { return emitted_; }

  private:
    // Offset of the point on the arc at angle a (bearing clockwise from
    // north, screen space with y pointing down) relative to the centre.
    std::pair<double, double> arc_offset(double a) const { return {radius_ * std::sin(a), -radius_ * std::cos(a)}; }

    // The curved part of the arc as elliptical-arc ("A") commands, assuming
    // the current point is already at the arc start. sweep-flag=1 matches our
    // clockwise (increasing-bearing) direction in y-down space.
    std::string arc_commands(double cx, double cy) const
    {
        std::string d;
        for (auto const& offset : arc_offsets_)
        {
            d += " A ";
            d += radius_str_;
            d += ',';
            d += radius_str_;
            d += " 0 0,1 ";
            append_coord(d, cx + offset.first, cy + offset.second);
        }
        return d;
    }

    void emit_path(std::string const& d, svg::path_output_attributes const& attributes)
    {
        namespace karma = boost::spirit::karma;
        static svg::svg_path_attributes_grammar<OutputIterator> const attributes_grammar;
        karma::lit_type lit;
        karma::string_type kstring;
        karma::generate(out_, lit("<path d=\"") << kstring << lit("\" "), d);
        karma::generate(out_, attributes_grammar << lit("/>\n"), attributes);
        emitted_ = true;
    }

    void render_one(double cx, double cy)
    {
        if (radius_ <= 0.0)
            return;

	std::string d = "M ";
	append_coord(d, cx, cy);
	d += " L ";
	append_coord(d, cx + start_offset_.first, cy + start_offset_.second);
	d += arc_commands(cx, cy);
	d += " Z";
	emit_path(d, arc_attributes_);
    }

    OutputIterator& out_;
    view_transform const& tr_;
    proj_transform const& prj_trans_;
    double radius_;
    svg::path_output_attributes arc_attributes_;
    std::pair<double, double> start_offset_;
    std::vector<std::pair<double, double>> arc_offsets_;
    std::string radius_str_;
    bool emitted_;
};

} // namespace detail

template<typename T>
void svg_renderer<T>::process(arc_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans)
{
    double const radius = get<double>(sym, keys::radius, feature, common_.vars_, 0.0) * common_.scale_factor_;
    double const start_angle = get<double>(sym, keys::start_angle, feature, common_.vars_, 0.0);
    double const end_angle = get<double>(sym, keys::end_angle, feature, common_.vars_, 360.0);

    bool const has_fill = has_key(sym, keys::fill);
    color const fill = get<mapnik::color>(sym, keys::fill, feature, common_.vars_, mapnik::color(128, 128, 128));
    double const fill_opacity = get<double>(sym, keys::fill_opacity, feature, common_.vars_, 1.0);

    bool const has_stroke = has_key(sym, keys::stroke);
    color const stroke = get<mapnik::color>(sym, keys::stroke, feature, common_.vars_, mapnik::color(0, 0, 0));
    double const stroke_width_raw = get<double>(sym, keys::stroke_width, feature, common_.vars_, 0.0);
    double const stroke_width = stroke_width_raw * common_.scale_factor_;
    //    double const stroke_width = has_stroke ? stroke_width_raw * common_.scale_factor_ : 0;
    double const stroke_opacity = get<double>(sym, keys::stroke_opacity, feature, common_.vars_, 1.0);

    svg::path_output_attributes arc_attributes;
    arc_attributes.set_fill_color(detail::opaque(fill));
    arc_attributes.set_fill_opacity(detail::combined_opacity(fill, has_fill ? fill_opacity : 0));

    arc_attributes.set_stroke_color(detail::opaque(stroke));
    arc_attributes.set_stroke_opacity(detail::combined_opacity(stroke, has_stroke ? stroke_opacity : 0));
    arc_attributes.set_stroke_width(stroke_width);

    detail::svg_arc_renderer<T> apply(output_iterator_,
                                      common_.t_,
                                      prj_trans,
                                      radius,
                                      start_angle,
                                      end_angle,
                                      arc_attributes);
    mapnik::util::apply_visitor(geometry::vertex_processor<detail::svg_arc_renderer<T>>(apply), feature.get_geometry());
    if (apply.emitted())
    {
        painted_ = true;
    }
}

template void svg_renderer<std::ostream_iterator<char>>::process(arc_symbolizer const&,
                                                                 mapnik::feature_impl&,
                                                                 proj_transform const&);

} // namespace mapnik

#endif // SVG_RENDERER
