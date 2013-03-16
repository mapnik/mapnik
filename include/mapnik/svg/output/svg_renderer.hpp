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

#ifndef MAPNIK_SVG_RENDERER_HPP
#define MAPNIK_SVG_RENDERER_HPP

// mapnik
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/map.hpp>
#include <mapnik/svg/output/svg_generator.hpp>
#include <mapnik/svg/output/svg_output_attributes.hpp>

// stl
#include <string>

namespace mapnik
{
// parameterized with the type of output iterator it will use for output.
// output iterators add more flexibility than streams, because iterators
// can target many other output destinations besides streams.
template <typename OutputIterator>
class MAPNIK_DECL svg_renderer : public feature_style_processor<svg_renderer<OutputIterator> >,
                                 private boost::noncopyable
{
public:
    typedef svg_renderer<OutputIterator> processor_impl_type;
    svg_renderer(Map const& m, OutputIterator& output_iterator, unsigned offset_x=0, unsigned offset_y=0);
    ~svg_renderer();

    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay, box2d<double> const& query_extent);
    void end_layer_processing(layer const& lay);
    void start_style_processing(feature_type_style const& st) {}
    void end_style_processing(feature_type_style const& st) {}

    /*!
     * @brief Overloads that process each kind of symbolizer individually.
     */
    void process(point_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(line_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(line_pattern_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(polygon_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(polygon_pattern_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(raster_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(shield_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(text_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(building_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(markers_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(debug_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans) {}

    /*!
     * @brief Overload that process the whole set of symbolizers of a rule.
     * @return true, meaning that this renderer can process multiple symbolizers.
     */
    bool process(rule::symbolizers const& syms,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);

    void painted(bool painted)
    {
        // nothing to do
    }

    inline OutputIterator& get_output_iterator()
    {
        return output_iterator_;
    }

    inline const OutputIterator& get_output_iterator() const
    {
        return output_iterator_;
    }

private:
    OutputIterator& output_iterator_;
    const int width_;
    const int height_;
    CoordTransform t_;
    svg::svg_generator<OutputIterator> generator_;
    svg::path_output_attributes path_attributes_;
    box2d<double> query_extent_;
    bool painted_;

    /*!
     * @brief Visitor that makes the calls to process each symbolizer when stored in a boost::variant.
     * This object follows the model of that found in feature_style_processor. It appears here, because
     * the logic that iterates over the set of symbolizer has been moved to an SVG renderer's internal
     * method.
     */
    struct symbol_dispatch : public boost::static_visitor<>
    {
        symbol_dispatch(svg_renderer<OutputIterator>& processor,
                        mapnik::feature_impl & feature,
                        proj_transform const& prj_trans)
            : processor_(processor),
              feature_(feature),
              prj_trans_(prj_trans) {}

        template <typename Symbolizer>
        void operator()(Symbolizer const& sym) const
        {
            processor_.process(sym, feature_, prj_trans_);
        }

        svg_renderer<OutputIterator>& processor_;
        mapnik::feature_impl & feature_;
        proj_transform const& prj_trans_;
    };
};
}

#endif // MAPNIK_SVG_RENDERER_HPP
