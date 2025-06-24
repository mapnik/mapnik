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

// mapnik
#include <mapnik/attribute_collector.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/group/group_layout_manager.hpp>
#include <mapnik/group/group_symbolizer_helper.hpp>
#include <mapnik/group/group_symbolizer_properties.hpp>
#include <mapnik/renderer_common/render_group_symbolizer.hpp>
#include <mapnik/renderer_common/render_thunk_extractor.hpp>
#include <mapnik/util/conversions.hpp>

namespace mapnik {

void render_group_symbolizer(group_symbolizer const& sym,
                             feature_impl& feature,
                             attributes const& vars,
                             proj_transform const& prj_trans,
                             box2d<double> const& clipping_extent,
                             renderer_common& common,
                             render_thunk_list_dispatch& render_thunks)
{
    // find all column names referenced in the group rules and symbolizers
    std::set<std::string> columns;
    group_attribute_collector column_collector(columns, false);
    column_collector(sym);

    auto props = get<group_symbolizer_properties_ptr>(sym, keys::group_properties);

    // create a new context for the sub features of this group
    context_ptr sub_feature_ctx = std::make_shared<mapnik::context_type>();

    // populate new context with column names referenced in the group rules and symbolizers
    for (auto const& col_name : columns)
    {
        sub_feature_ctx->push(col_name);
    }

    // keep track of the sub features that we'll want to symbolize
    // along with the group rules that they matched
    std::vector<std::pair<group_rule_ptr, feature_ptr>> matches;

    // create a copied 'virtual' common renderer for processing sub feature symbolizers
    // create an empty detector for it, so we are sure we won't hit anything
    virtual_renderer_common virtual_renderer(common);

    // keep track of which lists of render thunks correspond to
    // entries in the group_layout_manager.
    std::list<render_thunk_list> layout_thunks;

    // layout manager to store and arrange bboxes of matched features
    group_layout_manager layout_manager(props->get_layout());
    layout_manager.set_input_origin(common.width_ * 0.5, common.height_ * 0.5);

    // run feature or sub feature through the group rules & symbolizers
    // for each index value in the range
    value_integer start = get<value_integer>(sym, keys::start_column);
    value_integer end = start + get<value_integer>(sym, keys::num_columns);
    for (value_integer col_idx = start; col_idx < end; ++col_idx)
    {
        // create sub feature with indexed column values
        feature_ptr sub_feature = feature_factory::create(sub_feature_ctx, col_idx);

        // copy the necessary columns to sub feature
        for (auto const& col_name : columns)
        {
            if (col_name.find('%') != std::string::npos)
            {
                if (col_name.size() == 1)
                {
                    // column name is '%' by itself, so give the index as the value
                    sub_feature->put(col_name, col_idx);
                }
                else
                {
                    // indexed column
                    std::string col_idx_str;
                    if (mapnik::util::to_string(col_idx_str, col_idx))
                    {
                        std::string col_idx_name = col_name;
                        boost::replace_all(col_idx_name, "%", col_idx_str);
                        sub_feature->put(col_name, feature.get(col_idx_name));
                    }
                }
            }
            else
            {
                // non-indexed column
                sub_feature->put(col_name, feature.get(col_name));
            }
        }

        // add a single point geometry at pixel origin
        double x = common.width_ / 2.0, y = common.height_ / 2.0, z = 0.0;
        common.t_.backward(&x, &y);
        prj_trans.forward(x, y, z);
        // note that we choose a point in the middle of the screen to
        // try to ensure that we don't get edge artefacts due to any
        // symbolizers with avoid-edges set: only the avoid-edges of
        // the group symbolizer itself should matter.
        geometry::point<double> origin_pt(x, y);
        sub_feature->set_geometry(origin_pt);
        // get the layout for this set of properties
        for (auto const& rule : props->get_rules())
        {
            if (util::apply_visitor(evaluate<feature_impl, value_type, attributes>(*sub_feature, common.vars_),
                                    *(rule->get_filter()))
                  .to_bool())
            {
                // add matched rule and feature to the list of things to draw
                matches.emplace_back(rule, sub_feature);

                // construct a bounding box around all symbolizers for the matched rule
                box2d<double> bounds;
                render_thunk_list thunks;
                render_thunk_extractor
                  extractor(bounds, thunks, *sub_feature, common.vars_, prj_trans, virtual_renderer, clipping_extent);

                for (auto const& _sym : *rule)
                {
                    // TODO: construct layout and obtain bounding box
                    util::apply_visitor(extractor, _sym);
                }

                // add the bounding box to the layout manager
                layout_manager.add_member_bound_box(bounds);
                layout_thunks.emplace_back(std::move(thunks));
                break;
            }
        }
    }

    // create a symbolizer helper
    group_symbolizer_helper helper(sym,
                                   feature,
                                   vars,
                                   prj_trans,
                                   common.width_,
                                   common.height_,
                                   common.scale_factor_,
                                   common.t_,
                                   *common.detector_,
                                   clipping_extent);

    for (size_t i = 0; i < matches.size(); ++i)
    {
        group_rule_ptr match_rule = matches[i].first;
        feature_ptr match_feature = matches[i].second;
        value_unicode_string rpt_key_value = "";

        // get repeat key from matched group rule
        expression_ptr rpt_key_expr = match_rule->get_repeat_key();

        // if no repeat key was defined, use default from group symbolizer
        if (!rpt_key_expr)
        {
            rpt_key_expr = get<expression_ptr>(sym, keys::repeat_key);
        }

        // evaluate the repeat key with the matched sub feature if we have one
        if (rpt_key_expr)
        {
            rpt_key_value =
              util::apply_visitor(evaluate<feature_impl, value_type, attributes>(*match_feature, common.vars_),
                                  *rpt_key_expr)
                .to_unicode();
        }
        helper.add_box_element(layout_manager.offset_box_at(i), rpt_key_value);
    }

    pixel_position_list const& positions = helper.get();
    for (pixel_position const& pos : positions)
    {
        size_t layout_i = 0;
        for (auto const& thunks : layout_thunks)
        {
            pixel_position const& offset = layout_manager.offset_at(layout_i);
            pixel_position render_offset = pos + offset;
            render_thunks.render_list(thunks, render_offset);
            ++layout_i;
        }
    }
}

} // namespace mapnik
