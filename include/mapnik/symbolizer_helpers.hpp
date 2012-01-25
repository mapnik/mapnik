/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#ifndef SYMBOLIZER_HELPERS_HPP
#define SYMBOLIZER_HELPERS_HPP

#include <mapnik/text_symbolizer.hpp>
#include <mapnik/text_processing.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/feature.hpp>

#include <boost/shared_ptr.hpp>


namespace mapnik {

template <typename FaceManagerT, typename DetectorT>
class text_symbolizer_helper
{
public:
    text_symbolizer_helper(unsigned width,
                           unsigned height,
                           double scale_factor,
                           CoordTransform const &t,
                           FaceManagerT &font_manager,
                           DetectorT &detector) :
        width_(width),
        height_(height),
        scale_factor_(scale_factor),
        t_(t),
        font_manager_(font_manager),
        detector_(detector),
        text_()
    {

    }

    text_placement_info_ptr get_placement(text_symbolizer const& sym,
                                          Feature const& feature,
                                          proj_transform const& prj_trans);
private:
    bool initialize_geometries(text_symbolizer const& sym,
                               Feature const& feature,
                               proj_transform const& prj_trans);

    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform const &t_;
    FaceManagerT &font_manager_;
    DetectorT &detector_;
    boost::shared_ptr<processed_text> text_; /*TODO: Use shared pointers for text placement so we don't need to keep a reference here! */
    // Use a boost::ptr_vector here instread of std::vector?
    std::vector<geometry_type*> geometries_to_process_;
};


template <typename FaceManagerT, typename DetectorT>
text_placement_info_ptr text_symbolizer_helper<FaceManagerT, DetectorT>::get_placement(
    text_symbolizer const& sym,
    Feature const& feature,
    proj_transform const& prj_trans)
{
    if (!initialize_geometries(sym, feature, prj_trans)) return text_placement_info_ptr();

    text_ = boost::shared_ptr<processed_text>(new processed_text(font_manager_, scale_factor_));
    metawriter_with_properties writer = sym.get_metawriter();

    box2d<double> dims(0, 0, width_, height_);

//    typedef coord_transform2<CoordTransform,geometry_type> path_type;
    text_placement_info_ptr placement = sym.get_placement_options()->get_placement_info();
    placement->init(scale_factor_, width_, height_);
    if (writer.first)
        placement->collect_extents = true;

    unsigned num_geom = feature.num_geometries();
    if (!num_geom) return text_placement_info_ptr(); //Nothing to do

    while (placement->next())
    {
        text_processor &processor = placement->properties.processor;
        text_symbolizer_properties const& p = placement->properties;
        /* TODO: Simplify this. */
        text_->clear();
        processor.process(*text_, feature);
        string_info &info = text_->get_string_info();
        /* END TODO */
        double angle = 0.0;
        if (p.orientation)
        {
            angle = boost::apply_visitor(evaluate<Feature,value_type>(feature),*(p.orientation)).to_double();
        }
        placement_finder<DetectorT> finder(*placement, info, detector_, dims);

        unsigned num_geom = feature.num_geometries();
        for (unsigned i=0; i<num_geom; ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            if (geom.num_points() == 0) continue; // don't bother with empty geometries
            finder.find_placement(angle, geom, t_, prj_trans);
            if (!placement->placements.size())
                continue;
            if (writer.first) writer.first->add_text(*placement, font_manager_, feature, t_, writer.second);
            return placement;
        }
    }
    return text_placement_info_ptr();
}

template <typename FaceManagerT, typename DetectorT>
bool text_symbolizer_helper<FaceManagerT, DetectorT>::initialize_geometries(
    text_symbolizer const& sym,
    Feature const& feature,
    proj_transform const& prj_trans)
{
    unsigned num_geom = feature.num_geometries();
    for (unsigned i=0; i<num_geom; ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        // don't bother with empty geometries
        if (geom.num_points() == 0) continue;

        if ((geom.type() == Polygon) && sym.get_minimum_path_length() > 0)
        {
            // TODO - find less costly method than fetching full envelope
            box2d<double> gbox = t_.forward(geom.envelope(),prj_trans);
            if (gbox.width() < sym.get_minimum_path_length())
            {
                continue;
            }
        }
        // TODO - calculate length here as well
        geometries_to_process_.push_back(const_cast<geometry_type*>(&geom));
    }

    if (!geometries_to_process_.size() > 0)
    {
        // early return to avoid significant overhead of rendering setup
        return false;
    }
    return true;
}

}
#endif // SYMBOLIZER_HELPERS_HPP
