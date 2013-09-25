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

#ifndef PLACEMENTS_BASE_HPP
#define PLACEMENTS_BASE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/text_properties.hpp>
#include <mapnik/formatting/base.hpp>

namespace mapnik
{
typedef std::pair<double,double> dimension_type;

class MAPNIK_DECL text_placements;
/** Generate a possible placement and store results of placement_finder.
 * This placement has first to be tested by placement_finder to verify it
 * can actually be used.
 */
class MAPNIK_DECL text_placement_info : mapnik::noncopyable
{
public:
    /** Constructor. Takes the parent text_placements object as a parameter
     * to read defaults from it. */
    text_placement_info(text_placements const* parent, double scale_factor_);
    /** Get next placement.
     * This function is also called before the first placement is tried.
     * Each class has to return at least one position!
     * If this functions returns false the placement data should be
     * considered invalid!
     */
    virtual bool next()=0;
    virtual ~text_placement_info() {}

    /** Properties actually used by placement finder and renderer. Values in
     * here are modified each time next() is called. */
    text_symbolizer_properties properties;

    /** Scale factor used by the renderer. */
    double scale_factor;
    /** Set scale factor. */
    void set_scale_factor(double factor) { scale_factor = factor; }
    /** Get scale factor. */
    double get_scale_factor() const { return scale_factor; }
    /** Get label spacing taking the scale factor into account. */
    double get_actual_label_spacing() const { return scale_factor * properties.label_spacing; }
    /** Get minimum distance taking the scale factor into account. */
    double get_actual_minimum_distance() const { return scale_factor * properties.minimum_distance; }
    /** Get minimum padding taking the scale factor into account. */
    double get_actual_minimum_padding() const { return scale_factor * properties.minimum_padding; }
};

typedef std::shared_ptr<text_placement_info> text_placement_info_ptr;

/** This object handles the management of all TextSymbolizer properties. It can
 * be used as a base class for own objects which implement new processing
 * semantics. Basically this class just makes sure a pointer of the right
 * class is returned by the get_placement_info call.
 */
class MAPNIK_DECL text_placements
{
public:
    text_placements();
    /** Get a text_placement_info object to use in rendering.
     * The returned object creates a list of settings which is
     * used to try to find a placement and stores all
     * information that is generated by
     * the placement finder.
     *
     * This function usually is implemented as
     * text_placement_info_ptr text_placements_XXX::get_placement_info() const
     * {
     *     return text_placement_info_ptr(new text_placement_info_XXX(this));
     * }
     */
    virtual text_placement_info_ptr get_placement_info(
        double scale_factor_) const =0;
    /** Get a list of all expressions used in any placement.
     * This function is used to collect attributes.
     */
    virtual void add_expressions(expression_set &output);

    /** Destructor. */
    virtual ~text_placements() {}

    /** List of all properties used as the default for the subclasses. */
    text_symbolizer_properties defaults;
};

/** Pointer to object of class text_placements */
typedef std::shared_ptr<text_placements> text_placements_ptr;

} //ns mapnik

#endif // PLACEMENTS_BASE_HPP
