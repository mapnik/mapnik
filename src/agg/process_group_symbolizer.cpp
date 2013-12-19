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
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/group/group_layout_manager.hpp>

#include <mapnik/geom_util.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/pixel_position.hpp>

// agg
#include "agg_trans_affine.h"

// stl
#include <string>

// boost
#include <boost/variant/apply_visitor.hpp>

namespace mapnik {

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(group_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    // find all column names referenced in the group rules and symbolizers
    std::set<std::string> columns;
    attribute_collector column_collector(columns);
    expression_attributes<std::set<std::string> > rk_attr(columns);

    expression_ptr repeat_key = get<mapnik::expression_ptr>(sym, keys::repeat_key);
    if (repeat_key)
    {
        boost::apply_visitor(rk_attr, *repeat_key);
    }

    // get columns from child rules and symbolizers
    group_symbolizer_properties_ptr props = get<group_symbolizer_properties_ptr>(sym, keys::group_properties);
    if (props) {
        for (auto const& rule : props->get_rules())
        {
            // note that this recurses down on to the symbolizer
            // internals too, so we get all free variables.
            column_collector(*rule);
            // still need to collect repeat key columns
            if (rule->get_repeat_key())
            {
                boost::apply_visitor(rk_attr, *(rule->get_repeat_key()));
            }
        }
    }

    // create a new context for the sub features of this group
    context_ptr sub_feature_ctx = std::make_shared<mapnik::context_type>();

    // populate new context with column names referenced in the group rules and symbolizers
    // and determine if any indexed columns are present
    bool has_idx_cols = false;
    for (auto const& col_name : columns)
    {
        sub_feature_ctx->push(col_name);
        if (col_name.find('%') != std::string::npos)
        {
            has_idx_cols = true;
        }
    }

    // keep track of the sub features that we'll want to symbolize
    // along with the group rules that they matched
    std::vector< std::pair<group_rule_ptr, feature_ptr> > matches;

    // layout manager to store and arrange bboxes of matched features
    group_layout_manager layout_manager(props->get_layout(), pixel_position(common_.width_ / 2.0, common_.height_ / 2.0));
    text_layout text(common_.font_manager_, common_.scale_factor_);

    // run feature or sub feature through the group rules & symbolizers
    // for each index value in the range
    int start = get<value_integer>(sym, keys::start_column);
    int end = start + get<value_integer>(sym, keys::num_columns);
    for (int col_idx = start; col_idx < end; ++col_idx)
    {
        feature_ptr sub_feature;

        if (has_idx_cols)
        {
            // create sub feature with indexed column values
            sub_feature = feature_factory::create(sub_feature_ctx, col_idx);

            // copy the necessary columns to sub feature
            for(auto const& col_name : columns)
            {
                if (col_name.find('%') != std::string::npos)
                {
                    if (col_name.size() == 1)
                    {
                        // column name is '%' by itself, so give the index as the value
                        sub_feature->put(col_name, (value_integer)col_idx);
                    }
                    else
                    {
                        // indexed column
                        std::string col_idx_name = col_name;
                        boost::replace_all(col_idx_name, "%", boost::lexical_cast<std::string>(col_idx));
                        sub_feature->put(col_name, feature.get(col_idx_name));
                    }
                }
                else
                {
                    // non-indexed column
                    sub_feature->put(col_name, feature.get(col_name));
                }
            }
        }
        else
        {
            // no indexed columns, so use the existing feature instead of copying
            sub_feature = feature_ptr(&feature);
        }

        for (auto const& rule : props->get_rules())
        {
             if (boost::apply_visitor(evaluate<Feature,value_type>(*sub_feature),
                                               *(rule->get_filter())).to_bool())
             {
                // add matched rule and feature to the list of things to draw
                matches.push_back(std::make_pair(rule, sub_feature));

                // construct a bounding box around all symbolizers for the matched rule
                bound_box bounds;
                for (auto const& sym : *rule)
                {
                    // TODO: construct layout and obtain bounding box
                }

                // add the bounding box to the layout manager
                layout_manager.add_member_bound_box(bounds);
                break;
            }
        }
    }
}

template void agg_renderer<image_32>::process(group_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
