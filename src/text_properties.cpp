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
// mapnik
#include <mapnik/text_properties.hpp>
#include <mapnik/processed_text.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/formatting/text.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>

// boost
#include <boost/make_shared.hpp>

namespace mapnik
{
using boost::optional;

text_symbolizer_properties::text_symbolizer_properties() :
    orientation(),
    displacement(0,0),
    label_placement(POINT_PLACEMENT),
    halign(H_AUTO),
    jalign(J_AUTO),
    valign(V_AUTO),
    label_spacing(0),
    label_position_tolerance(0),
    avoid_edges(false),
    minimum_distance(0.0),
    minimum_padding(0.0),
    minimum_path_length(0.0),
    max_char_angle_delta(22.5 * M_PI/180.0),
    force_odd_labels(false),
    allow_overlap(false),
    largest_bbox_only(true),
    text_ratio(0),
    wrap_width(0),
    format(),
    tree_()
{

}

void text_symbolizer_properties::process(processed_text &output, Feature const& feature) const
{
    output.clear();
    if (tree_) {
        tree_->apply(format, feature, output);
    } else {
        MAPNIK_LOG_WARN(text_properties) << "text_symbolizer_properties can't produce text: No formatting tree!";
    }
}

void text_symbolizer_properties::set_format_tree(formatting::node_ptr tree)
{
    tree_ = tree;
}

formatting::node_ptr text_symbolizer_properties::format_tree() const
{
    return tree_;
}

void text_symbolizer_properties::from_xml(xml_node const &sym, fontset_map const & fontsets)
{
    optional<label_placement_e> placement_ = sym.get_opt_attr<label_placement_e>("placement");
    if (placement_) label_placement = *placement_;
    optional<vertical_alignment_e> valign_ = sym.get_opt_attr<vertical_alignment_e>("vertical-alignment");
    if (valign_) valign = *valign_;
    optional<double> text_ratio_ = sym.get_opt_attr<double>("text-ratio");
    if (text_ratio_) text_ratio = *text_ratio_;
    optional<double> wrap_width_ = sym.get_opt_attr<double>("wrap-width");
    if (wrap_width_) wrap_width = *wrap_width_;
    optional<unsigned> label_position_tolerance_ = sym.get_opt_attr<unsigned>("label-position-tolerance");
    if (label_position_tolerance_) label_position_tolerance = *label_position_tolerance_;
    optional<double> spacing_ = sym.get_opt_attr<double>("spacing");
    if (spacing_) label_spacing = *spacing_;
    else {
        // https://github.com/mapnik/mapnik/issues/1427
        spacing_ = sym.get_opt_attr<double>("label-spacing");
        if (spacing_) label_spacing = *spacing_;
    }
    optional<double> minimum_distance_ = sym.get_opt_attr<double>("minimum-distance");
    if (minimum_distance_) minimum_distance = *minimum_distance_;
    optional<double> min_padding_ = sym.get_opt_attr<double>("minimum-padding");
    if (min_padding_) minimum_padding = *min_padding_;
    optional<double> min_path_length_ = sym.get_opt_attr<double>("minimum-path-length");
    if (min_path_length_) minimum_path_length = *min_path_length_;
    optional<boolean> avoid_edges_ = sym.get_opt_attr<boolean>("avoid-edges");
    if (avoid_edges_) avoid_edges = *avoid_edges_;
    optional<boolean> allow_overlap_ = sym.get_opt_attr<boolean>("allow-overlap");
    if (allow_overlap_) allow_overlap = *allow_overlap_;
    optional<boolean> largest_bbox_only_ = sym.get_opt_attr<boolean>("largest-bbox-only");
    if (largest_bbox_only_) largest_bbox_only = *largest_bbox_only_;
    optional<horizontal_alignment_e> halign_ = sym.get_opt_attr<horizontal_alignment_e>("horizontal-alignment");
    if (halign_) halign = *halign_;
    optional<justify_alignment_e> jalign_ = sym.get_opt_attr<justify_alignment_e>("justify-alignment");
    if (jalign_) jalign = *jalign_;
    optional<expression_ptr> orientation_ = sym.get_opt_attr<expression_ptr>("orientation");
    if (orientation_) orientation = *orientation_;
    optional<double> dx = sym.get_opt_attr<double>("dx");
    if (dx) displacement.first = *dx;
    optional<double> dy = sym.get_opt_attr<double>("dy");
    if (dy) displacement.second = *dy;
    optional<double> max_char_angle_delta_ = sym.get_opt_attr<double>("max-char-angle-delta");
    if (max_char_angle_delta_) max_char_angle_delta=(*max_char_angle_delta_)*(M_PI/180);

    optional<expression_ptr> name_ = sym.get_opt_attr<expression_ptr>("name");
    if (name_)
    {
        MAPNIK_LOG_WARN(text_placements) << "Using 'name' in TextSymbolizer/ShieldSymbolizer is deprecated!";

        set_old_style_expression(*name_);
    }

    format.from_xml(sym, fontsets);
    formatting::node_ptr n(formatting::node::from_xml(sym));
    if (n) set_format_tree(n);
}

void text_symbolizer_properties::to_xml(boost::property_tree::ptree &node,
                                        bool explicit_defaults,
                                        text_symbolizer_properties const& dfl) const
{
    if (orientation)
    {
        std::string const& orientationstr = to_expression_string(*orientation);
        if (!dfl.orientation || orientationstr != to_expression_string(*(dfl.orientation)) || explicit_defaults) {
            set_attr(node, "orientation", orientationstr);
        }
    }

    if (displacement.first != dfl.displacement.first || explicit_defaults)
    {
        set_attr(node, "dx", displacement.first);
    }
    if (displacement.second != dfl.displacement.second || explicit_defaults)
    {
        set_attr(node, "dy", displacement.second);
    }
    if (label_placement != dfl.label_placement || explicit_defaults)
    {
        set_attr(node, "placement", label_placement);
    }
    if (valign != dfl.valign || explicit_defaults)
    {
        set_attr(node, "vertical-alignment", valign);
    }
    if (text_ratio != dfl.text_ratio || explicit_defaults)
    {
        set_attr(node, "text-ratio", text_ratio);
    }
    if (wrap_width != dfl.wrap_width || explicit_defaults)
    {
        set_attr(node, "wrap-width", wrap_width);
    }
    if (label_position_tolerance != dfl.label_position_tolerance || explicit_defaults)
    {
        set_attr(node, "label-position-tolerance", label_position_tolerance);
    }
    if (label_spacing != dfl.label_spacing || explicit_defaults)
    {
        set_attr(node, "spacing", label_spacing);
    }
    if (minimum_distance != dfl.minimum_distance || explicit_defaults)
    {
        set_attr(node, "minimum-distance", minimum_distance);
    }
    if (minimum_padding != dfl.minimum_padding || explicit_defaults)
    {
        set_attr(node, "minimum-padding", minimum_padding);
    }
    if (minimum_path_length != dfl.minimum_path_length || explicit_defaults)
    {
        set_attr(node, "minimum-path-length", minimum_path_length);
    }
    if (allow_overlap != dfl.allow_overlap || explicit_defaults)
    {
        set_attr(node, "allow-overlap", allow_overlap);
    }
    if (avoid_edges != dfl.avoid_edges || explicit_defaults)
    {
        set_attr(node, "avoid-edges", avoid_edges);
    }
    if (largest_bbox_only != dfl.largest_bbox_only|| explicit_defaults)
    {
        set_attr(node, "largest-bbox_only", largest_bbox_only);
    }
    if (max_char_angle_delta != dfl.max_char_angle_delta || explicit_defaults)
    {
        set_attr(node, "max-char-angle-delta", max_char_angle_delta);
    }
    if (halign != dfl.halign || explicit_defaults)
    {
        set_attr(node, "horizontal-alignment", halign);
    }
    if (jalign != dfl.jalign || explicit_defaults)
    {
        set_attr(node, "justify-alignment", jalign);
    }
    if (valign != dfl.valign || explicit_defaults)
    {
        set_attr(node, "vertical-alignment", valign);
    }
    format.to_xml(node, explicit_defaults, dfl.format);
    if (tree_) tree_->to_xml(node);
}


void text_symbolizer_properties::add_expressions(expression_set &output) const
{
    output.insert(orientation);
    if (tree_) tree_->add_expressions(output);
}

void text_symbolizer_properties::set_old_style_expression(expression_ptr expr)
{
    tree_ = boost::make_shared<formatting::text_node>(expr);
}

char_properties::char_properties() :
    face_name(),
    fontset(),
    text_size(10.0),
    character_spacing(0),
    line_spacing(0),
    text_opacity(1.0),
    wrap_before(false),
    wrap_char(' '),
    text_transform(NONE),
    fill(color(0,0,0)),
    halo_fill(color(255,255,255)),
    halo_radius(0)
{

}

void char_properties::from_xml(xml_node const& sym, fontset_map const& fontsets)
{
    optional<double> text_size_ = sym.get_opt_attr<double>("size");
    if (text_size_) text_size = *text_size_;
    optional<double> character_spacing_ = sym.get_opt_attr<double>("character-spacing");
    if (character_spacing_) character_spacing = *character_spacing_;
    optional<color> fill_ = sym.get_opt_attr<color>("fill");
    if (fill_) fill = *fill_;
    optional<color> halo_fill_ = sym.get_opt_attr<color>("halo-fill");
    if (halo_fill_) halo_fill = *halo_fill_;
    optional<double> halo_radius_ = sym.get_opt_attr<double>("halo-radius");
    if (halo_radius_) halo_radius = *halo_radius_;
    optional<boolean> wrap_before_ = sym.get_opt_attr<boolean>("wrap-before");
    if (wrap_before_) wrap_before = *wrap_before_;
    optional<text_transform_e> tconvert_ = sym.get_opt_attr<text_transform_e>("text-transform");
    if (tconvert_) text_transform = *tconvert_;
    optional<double> line_spacing_ = sym.get_opt_attr<double>("line-spacing");
    if (line_spacing_) line_spacing = *line_spacing_;
    optional<double> opacity_ = sym.get_opt_attr<double>("opacity");
    if (opacity_) text_opacity = *opacity_;
    optional<std::string> wrap_char_ = sym.get_opt_attr<std::string>("wrap-character");
    if (wrap_char_ && (*wrap_char_).size() > 0) wrap_char = ((*wrap_char_)[0]);
    optional<std::string> face_name_ = sym.get_opt_attr<std::string>("face-name");
    if (face_name_)
    {
        face_name = *face_name_;
    }
    optional<std::string> fontset_name_ = sym.get_opt_attr<std::string>("fontset-name");
    if (fontset_name_) {
        std::map<std::string,font_set>::const_iterator itr = fontsets.find(*fontset_name_);
        if (itr != fontsets.end())
        {
            fontset = itr->second;
        }
        else
        {
            throw config_error("Unable to find any fontset named '" + *fontset_name_ + "'", sym);
        }
    }
    if (!face_name.empty() && fontset)
    {
        throw config_error("Can't have both face-name and fontset-name", sym);
    }
    if (face_name.empty() && !fontset)
    {
        throw config_error("Must have face-name or fontset-name", sym);
    }
}

void char_properties::to_xml(boost::property_tree::ptree &node, bool explicit_defaults, char_properties const &dfl) const
{
    if (fontset)
    {
        set_attr(node, "fontset-name", fontset->get_name());
    }

    if (face_name != dfl.face_name || explicit_defaults)
    {
        set_attr(node, "face-name", face_name);
    }

    if (text_size != dfl.text_size || explicit_defaults)
    {
        set_attr(node, "size", text_size);
    }

    if (fill != dfl.fill || explicit_defaults)
    {
        set_attr(node, "fill", fill);
    }
    if (halo_radius != dfl.halo_radius || explicit_defaults)
    {
        set_attr(node, "halo-radius", halo_radius);
    }
    if (halo_fill != dfl.halo_fill || explicit_defaults)
    {
        set_attr(node, "halo-fill", halo_fill);
    }
    if (wrap_before != dfl.wrap_before || explicit_defaults)
    {
        set_attr(node, "wrap-before", wrap_before);
    }
    if (wrap_char != dfl.wrap_char || explicit_defaults)
    {
        set_attr(node, "wrap-character", std::string(1, wrap_char));
    }
    if (text_transform != dfl.text_transform || explicit_defaults)
    {
        set_attr(node, "text-transform", text_transform);
    }
    if (line_spacing != dfl.line_spacing || explicit_defaults)
    {
        set_attr(node, "line-spacing", line_spacing);
    }
    if (character_spacing != dfl.character_spacing || explicit_defaults)
    {
        set_attr(node, "character-spacing", character_spacing);
    }
    // for shield_symbolizer this is later overridden
    if (text_opacity != dfl.text_opacity || explicit_defaults)
    {
        set_attr(node, "opacity", text_opacity);
    }
}

} //ns mapnik
