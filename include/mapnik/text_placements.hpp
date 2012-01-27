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

#ifndef MAPNIK_TEXT_PLACEMENTS_HPP
#define MAPNIK_TEXT_PLACEMENTS_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/text_processing.hpp>

// stl
#include <vector>
#include <string>
#include <queue>
#include <set>

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/property_tree/ptree.hpp>

namespace mapnik {

class text_placements;

typedef boost::tuple<double,double> position;

enum label_placement_enum {
    POINT_PLACEMENT,
    LINE_PLACEMENT,
    VERTEX_PLACEMENT,
    INTERIOR_PLACEMENT,
    label_placement_enum_MAX
};

DEFINE_ENUM( label_placement_e, label_placement_enum );

enum vertical_alignment
{
    V_TOP = 0,
    V_MIDDLE,
    V_BOTTOM,
    V_AUTO,
    vertical_alignment_MAX
};

DEFINE_ENUM( vertical_alignment_e, vertical_alignment );

enum horizontal_alignment
{
    H_LEFT = 0,
    H_MIDDLE,
    H_RIGHT,
    H_AUTO,
    horizontal_alignment_MAX
};

DEFINE_ENUM( horizontal_alignment_e, horizontal_alignment );

enum justify_alignment
{
    J_LEFT = 0,
    J_MIDDLE,
    J_RIGHT,
    justify_alignment_MAX
};

DEFINE_ENUM( justify_alignment_e, justify_alignment );

/** Contains all text symbolizer properties which are not directly related to text formating. */
struct text_symbolizer_properties
{
    text_symbolizer_properties();
    /** Load all values and also the ```processor``` object from XML ptree. */
    void set_values_from_xml(boost::property_tree::ptree const &sym, std::map<std::string,font_set> const & fontsets);
    /** Save all values to XML ptree (but does not create a new parent node!). */
    void to_xml(boost::property_tree::ptree &node, bool explicit_defaults, text_symbolizer_properties const &dfl=text_symbolizer_properties()) const;

    //Per symbolizer options
    expression_ptr orientation;
    position displacement;
    label_placement_e label_placement;
    horizontal_alignment_e halign;
    justify_alignment_e jalign;
    vertical_alignment_e valign;
    /** distance between repeated labels on a single geometry */
    unsigned label_spacing;
    /** distance the label can be moved on the line to fit, if 0 the default is used */
    unsigned label_position_tolerance;
    bool avoid_edges;
    double minimum_distance;
    double minimum_padding;
    double minimum_path_length;
    double max_char_angle_delta;
    /** Always try render an odd amount of labels */
    bool force_odd_labels;
    bool allow_overlap;
    unsigned text_ratio;
    unsigned wrap_width;
    /** Contains everything related to text formating */
    text_processor processor;
};


/** Generate a possible placement and store results of placement_finder.
 * This placement has first to be tested by placement_finder to verify it
 * can actually be used.
 */
class text_placement_info : boost::noncopyable
{
public:
    /** Constructor. Takes the parent text_placements object as a parameter
     * to read defaults from it. */
    text_placement_info(text_placements const* parent);
    /** Get next placement.
      * This function is also called before the first placement is tried.
      * Each class has to return at least one position!
      * If this functions returns false the placement data should be
      * considered invalid!
      */
    virtual bool next()=0;
    virtual ~text_placement_info() {}
    /** Initialize values used by placement finder. Only has to be done once
     * per object.
     */
    void init(double scale_factor_,
              unsigned w = 0, unsigned h = 0, bool has_dimensions_ = false);

    /** Properties actually used by placement finder and renderer. Values in
     * here are modified each time next() is called. */
    text_symbolizer_properties properties;
    
    /** Scale factor used by the renderer. */
    double scale_factor;
    /* TODO: Don't know what this is used for. */
    bool has_dimensions;
    /* TODO: Don't know what this is used for. */
    std::pair<double, double> dimensions;
    /** Set scale factor. */
    void set_scale_factor(double factor) { scale_factor = factor; }
    /** Get scale factor. */
    double get_scale_factor() { return scale_factor; }
    /** Get label spacing taking the scale factor into account. */
    double get_actual_label_spacing() { return scale_factor * properties.label_spacing; }
    /** Get minimum distance taking the scale factor into account. */
    double get_actual_minimum_distance() { return scale_factor * properties.minimum_distance; }
     /** Get minimum padding taking the scale factor into account. */
    double get_actual_minimum_padding() { return scale_factor * properties.minimum_padding; }

    /** Collect a bounding box of all texts placed. */
    bool collect_extents;
    //Output by placement finder
    /** Bounding box of all texts placed. */
    box2d<double> extents;
    /* TODO */
    std::queue< box2d<double> > envelopes;
    /* TODO */
    boost::ptr_vector<text_path> placements;
};

typedef boost::shared_ptr<text_placement_info> text_placement_info_ptr;

/** This object handles the management of all TextSymbolizer properties. It can
 * be used as a base class for own objects which implement new processing 
 * semantics. Basically this class just makes sure a pointer of the right 
 * class is returned by the get_placement_info call.
 */
class text_placements
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
    virtual text_placement_info_ptr get_placement_info() const =0;
    /** Get a list of all expressions used in any placement.
      * This function is used to collect attributes.
      */
    virtual std::set<expression_ptr> get_all_expressions();

    /** Destructor. */
    virtual ~text_placements() {}
    
    /** List of all properties used as the default for the subclasses. */
    text_symbolizer_properties properties;
};

/** Pointer to object of class text_placements */
typedef boost::shared_ptr<text_placements> text_placements_ptr;


class text_placements_info_dummy;

/** Dummy placement algorithm. Always takes the default value. */
class MAPNIK_DECL text_placements_dummy: public text_placements
{
public:
    text_placement_info_ptr get_placement_info() const;
    friend class text_placement_info_dummy;
};

/** Placement info object for dummy placement algorithm. Always takes the default value. */
class MAPNIK_DECL text_placement_info_dummy : public text_placement_info
{
public:
    text_placement_info_dummy(text_placements_dummy const* parent) : text_placement_info(parent),
        state(0), parent_(parent) {}
    bool next();
private:
    unsigned state;
    text_placements_dummy const* parent_;
};



} //namespace

#endif // MAPNIK_TEXT_PLACEMENTS_HPP
