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
#include <mapnik/renderer_common/arc_symbolizer_properties.hpp>
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
                     arc_symbolizer_properties const& props,
                     svg::path_output_attributes const& fill_attributes,
                     svg::path_output_attributes const& arc_attributes,
                     svg::path_output_attributes const& radius_attributes)
        : out_(out),
          tr_(tr),
          prj_trans_(prj_trans),
          props_(props),
          fill_attributes_(fill_attributes),
          arc_attributes_(arc_attributes),
          radius_attributes_(radius_attributes),
          emitted_(false)
    {
        auto [a0, a1] = props_.sweep();

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
        util::to_string(radius_str_, props_.radius);
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
    std::pair<double, double> arc_offset(double a) const
    {
        return {props_.radius * std::sin(a), -props_.radius * std::cos(a)};
    }

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
        static svg::svg_path_dash_array_grammar<OutputIterator> const dash_array_grammar;
        karma::lit_type lit;
        karma::string_type kstring;
        karma::generate(out_, lit("<path d=\"") << kstring << lit("\" "), d);
        // unlike generate_path_impl we skip the dash array entirely when it is
        // empty, so solid paths carry no stroke-dasharray="" attribute
        if (!attributes.stroke_dasharray().empty())
            karma::generate(out_, dash_array_grammar << lit(" "), attributes.stroke_dasharray());
        karma::generate(out_, attributes_grammar << lit("/>\n"), attributes);
        emitted_ = true;
    }

    void render_one(double cx, double cy)
    {
        // Closed pie wedge (center -> arc start, arc, arc end -> center), used for fill.
        if (props_.has_fill)
        {
            std::string d = "M ";
            append_coord(d, cx, cy);
            d += " L ";
            append_coord(d, cx + start_offset_.first, cy + start_offset_.second);
            d += arc_commands(cx, cy);
            d += " Z";
            emit_path(d, fill_attributes_);
        }

        // The two radius spokes from the center to the arc endpoints.
        if (props_.has_radius_stroke)
        {
            auto const& end_offset = arc_offsets_.back();
            std::string d = "M ";
            append_coord(d, cx, cy);
            d += " L ";
            append_coord(d, cx + start_offset_.first, cy + start_offset_.second);
            d += " M ";
            append_coord(d, cx, cy);
            d += " L ";
            append_coord(d, cx + end_offset.first, cy + end_offset.second);
            emit_path(d, radius_attributes_);
        }

        // The curved arc line.
        if (props_.has_arc_stroke)
        {
            std::string d = "M ";
            append_coord(d, cx + start_offset_.first, cy + start_offset_.second);
            d += arc_commands(cx, cy);
            emit_path(d, arc_attributes_);
        }
    }

    OutputIterator& out_;
    view_transform const& tr_;
    proj_transform const& prj_trans_;
    arc_symbolizer_properties const& props_;
    svg::path_output_attributes fill_attributes_;
    svg::path_output_attributes arc_attributes_;
    svg::path_output_attributes radius_attributes_;
    std::pair<double, double> start_offset_;
    std::vector<std::pair<double, double>> arc_offsets_;
    std::string radius_str_;
    bool emitted_;
};

} // namespace detail

template<typename T>
void svg_renderer<T>::process(arc_symbolizer const& sym, mapnik::feature_impl& feature, proj_transform const& prj_trans)
{
    arc_symbolizer_properties const props(sym, feature, common_.vars_, common_.scale_factor_);
    if (!props.drawable())
        return;

    // dash lengths in props are unscaled; svg emits final pixel lengths, so
    // apply the scale factor here like the (pre-scaled) stroke widths
    auto scale_dashes = [&](dash_array const& dashes) {
        dash_array scaled;
        scaled.reserve(dashes.size());
        for (auto const& d : dashes)
            scaled.emplace_back(d.first * common_.scale_factor_, d.second * common_.scale_factor_);
        return scaled;
    };

    svg::path_output_attributes fill_attributes;
    fill_attributes.set_fill_color(detail::opaque(props.fill));
    fill_attributes.set_fill_opacity(detail::combined_opacity(props.fill, props.fill_opacity));

    svg::path_output_attributes arc_attributes;
    arc_attributes.set_stroke_color(detail::opaque(props.arc_stroke));
    arc_attributes.set_stroke_opacity(detail::combined_opacity(props.arc_stroke, props.arc_stroke_opacity));
    arc_attributes.set_stroke_width(props.arc_stroke_width);
    arc_attributes.set_stroke_dasharray(scale_dashes(props.arc_dash));
    arc_attributes.set_stroke_dashoffset(props.arc_dash_offset * common_.scale_factor_);

    svg::path_output_attributes radius_attributes;
    radius_attributes.set_stroke_color(detail::opaque(props.radius_stroke));
    radius_attributes.set_stroke_opacity(detail::combined_opacity(props.radius_stroke, props.radius_stroke_opacity));
    radius_attributes.set_stroke_width(props.radius_stroke_width);
    radius_attributes.set_stroke_dasharray(scale_dashes(props.radius_dash));
    radius_attributes.set_stroke_dashoffset(props.radius_dash_offset * common_.scale_factor_);

    detail::svg_arc_renderer<T>
      apply(output_iterator_, common_.t_, prj_trans, props, fill_attributes, arc_attributes, radius_attributes);
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
