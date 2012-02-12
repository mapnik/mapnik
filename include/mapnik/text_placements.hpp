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
typedef std::pair<double,double> dimension_type;

class processed_text : boost::noncopyable
{
public:
    class processed_expression
    {
    public:
        processed_expression(char_properties const& properties, UnicodeString const& text) :
            p(properties), str(text) {}
        char_properties p;
        UnicodeString str;
    };
public:
    processed_text(face_manager<freetype_engine> & font_manager, double scale_factor);
    void push_back(char_properties const& properties, UnicodeString const& text);
    unsigned size() const { return expr_list_.size(); }
    unsigned empty() const { return expr_list_.empty(); }
    void clear();
    typedef std::list<processed_expression> expression_list;
    expression_list::const_iterator begin() const;
    expression_list::const_iterator end() const;
    string_info &get_string_info();
private:
    expression_list expr_list_;
    face_manager<freetype_engine> & font_manager_;
    double scale_factor_;
    string_info info_;
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
    text_placement_info(text_placements const* parent,
                        double scale_factor_, dimension_type dim, bool has_dimensions_);
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
    /* TODO: Don't know what this is used for. */
    bool has_dimensions;
    /* TODO: Don't know what this is used for. */
    dimension_type dimensions;
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
    /** Additional boxes to take into account when finding placement.
     * Used for finding line placements where multiple placements are returned.
     * Boxes are relative to starting point of current placement.
     */
    std::vector<box2d<double> > additional_boxes;

    /* TODO */
    std::queue< box2d<double> > envelopes;
    /** Used to return all placements found. */
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
    virtual text_placement_info_ptr get_placement_info(
        double scale_factor_, dimension_type dim,
        bool has_dimensions_) const =0;
    /** Get a list of all expressions used in any placement.
     * This function is used to collect attributes.
     */
    virtual void add_expressions(expression_set &output);

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
    text_placement_info_ptr get_placement_info(
        double scale_factor, dimension_type dim, bool has_dimensions) const;
    friend class text_placement_info_dummy;
};

/** Placement info object for dummy placement algorithm. Always takes the default value. */
class MAPNIK_DECL text_placement_info_dummy : public text_placement_info
{
public:
    text_placement_info_dummy(text_placements_dummy const* parent,
                              double scale_factor, dimension_type dim, bool has_dimensions)
        : text_placement_info(parent, scale_factor, dim, has_dimensions),
        state(0), parent_(parent) {}
    bool next();
private:
    unsigned state;
    text_placements_dummy const* parent_;
};



} //namespace

#endif // MAPNIK_TEXT_PLACEMENTS_HPP
