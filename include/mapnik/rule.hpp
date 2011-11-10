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

#ifndef MAPNIK_RULE_HPP
#define MAPNIK_RULE_HPP

// mapnik
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/line_pattern_symbolizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/polygon_pattern_symbolizer.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/raster_symbolizer.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/glyph_symbolizer.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/expression_string.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/variant.hpp>

// stl
#include <string>
#include <vector>

namespace mapnik
{
inline bool operator==(point_symbolizer const& lhs,
                       point_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
inline bool operator==(line_symbolizer const& lhs,
                       line_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
inline bool operator==(line_pattern_symbolizer const& lhs,
                       line_pattern_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}

inline bool operator==(polygon_symbolizer const& lhs,
                       polygon_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
    
inline bool operator==(polygon_pattern_symbolizer const& lhs,
                       polygon_pattern_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
    
inline bool operator==(raster_symbolizer const& lhs,
                       raster_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
    
inline bool operator==(text_symbolizer const& lhs,
                       text_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
    
inline bool operator==(shield_symbolizer const& lhs,
                       shield_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
    
inline bool operator==(building_symbolizer const& lhs,
                       building_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
   
inline bool operator==(markers_symbolizer const& lhs,
                       markers_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}

inline bool operator==(glyph_symbolizer const& lhs,
                       glyph_symbolizer const& rhs)
{
    return (&lhs == &rhs); 
}
typedef boost::variant<point_symbolizer,
                       line_symbolizer,
                       line_pattern_symbolizer,
                       polygon_symbolizer,
                       polygon_pattern_symbolizer,
                       raster_symbolizer,
                       shield_symbolizer,
                       text_symbolizer,
                       building_symbolizer,
                       markers_symbolizer,
                       glyph_symbolizer> symbolizer;
    
        
class rule
{    
public:
    typedef std::vector<symbolizer> symbolizers;
private:
    
    std::string name_;
    std::string title_;
    std::string abstract_;
    double min_scale_;
    double max_scale_;
    symbolizers syms_;
    expression_ptr filter_;
    bool else_filter_;
    bool also_filter_;
public:
    rule()
        : name_(),
          title_(),
          abstract_(),
          min_scale_(0),
          max_scale_(std::numeric_limits<double>::infinity()),
          syms_(),
          filter_(boost::make_shared<mapnik::expr_node>(true)),
          else_filter_(false), 
          also_filter_(false) {}
    
    rule(const std::string& name,
         const std::string& title="",
         double min_scale_denominator=0,
         double max_scale_denominator=std::numeric_limits<double>::infinity())
        : name_(name),
          title_(title),
          min_scale_(min_scale_denominator),
          max_scale_(max_scale_denominator),
          syms_(),
          filter_(boost::make_shared<mapnik::expr_node>(true)),
          else_filter_(false), 
          also_filter_(false)  {}
    
    rule(const rule& rhs, bool deep_copy = false)
        : name_(rhs.name_),
          title_(rhs.title_),
          abstract_(rhs.abstract_),
          min_scale_(rhs.min_scale_),
          max_scale_(rhs.max_scale_),
          syms_(rhs.syms_),
          filter_(rhs.filter_),
          else_filter_(rhs.else_filter_), 
          also_filter_(rhs.also_filter_) 
    {
        if (deep_copy) {
            //std::string expr = to_expression_string(rhs.filter_);
            //filter_ = parse_expression(expr,"utf8");
            
            symbolizers::iterator it  = syms_.begin(),
                                  end = syms_.end();
            
            // FIXME - metawriter_ptr?
            
            for(; it != end; ++it) {
                
                /*if (polygon_symbolizer *sym = boost::get<polygon_symbolizer>(&(*it))) {
                    // no shared pointers
                } else if (line_symbolizer *sym = boost::get<line_symbolizer>(&(*it))) {
                    // no shared pointers
                } else if (building_symbolizer *sym = boost::get<building_symbolizer>(&(*it))) {
                    // no shared pointers
                }*/
                
                if (markers_symbolizer *sym = boost::get<markers_symbolizer>(&(*it))) {
                    copy_path_ptr(sym);
                } else if (point_symbolizer *sym = boost::get<point_symbolizer>(&(*it))) {
                    copy_path_ptr(sym);
                } else if (polygon_pattern_symbolizer *sym = boost::get<polygon_pattern_symbolizer>(&(*it))) {
                    copy_path_ptr(sym);
                } else if (line_pattern_symbolizer *sym = boost::get<line_pattern_symbolizer>(&(*it))) {
                    copy_path_ptr(sym);
                } else if (raster_symbolizer *sym = boost::get<raster_symbolizer>(&(*it))) {
                    raster_colorizer_ptr old_colorizer = sym->get_colorizer(),
                                         new_colorizer = raster_colorizer_ptr();
                    
                    new_colorizer->set_stops( old_colorizer->get_stops());
                    new_colorizer->set_default_mode( old_colorizer->get_default_mode() );
                    new_colorizer->set_default_color( old_colorizer->get_default_color() );
                    new_colorizer->set_epsilon( old_colorizer->get_epsilon() );
                    
                    sym->set_colorizer(new_colorizer);
                } else if (shield_symbolizer *sym = boost::get<shield_symbolizer>(&(*it))) {
                    copy_path_ptr(sym);
                    copy_text_ptr(sym);
                } else if (text_symbolizer *sym = boost::get<text_symbolizer>(&(*it))) {
                    copy_text_ptr(sym);
                }
            }
        }
    }
    
    rule& operator=(rule const& rhs) 
    {
        rule tmp(rhs);
        swap(tmp);
        return *this;
    }
    bool operator==(rule const& other)
    {
        return  (this == &other); 
    }
    
    void set_max_scale(double scale)
    {
        max_scale_=scale;
    }
    
    double get_max_scale() const
    {
        return max_scale_;
    }
    
    void set_min_scale(double scale)
    {
        min_scale_=scale;
    }
    
    double get_min_scale() const
    {
        return min_scale_;
    }
    
    void set_name(std::string const& name)
    {
        name_=name;
    }
    
    std::string const& get_name() const
    {
        return name_;
    }
    
    std::string const& get_title() const
    {
        return  title_;
    }
    
    void set_title(std::string const& title)
    {
        title_=title;
    }
    
    void set_abstract(std::string const& abstract)
    {
        abstract_=abstract;
    }
    
    std::string const& get_abstract() const
    {
        return abstract_;
    }
    
    void append(const symbolizer& sym)
    {
        syms_.push_back(sym);
    }
    
    void remove_at(size_t index)
    {
        if (index < syms_.size())
        {
            syms_.erase(syms_.begin()+index);
        }
    }
    
    const symbolizers& get_symbolizers() const
    {
        return syms_;
    }
    
    symbolizers::const_iterator begin() const
    {
        return syms_.begin();
    }
    
    symbolizers::const_iterator end() const
    {
        return syms_.end();
    }
    
    symbolizers::iterator begin()
    {
        return syms_.begin();
    }
    
    symbolizers::iterator end()
    {
        return syms_.end();
    }
    
    void set_filter(const expression_ptr& filter)
    {
        filter_=filter;
    }
    
    expression_ptr const& get_filter() const
    {
        return filter_;
    }
    
    void set_else(bool else_filter)
    {
        else_filter_=else_filter;
    }
    
    bool has_else_filter() const
    {
        return else_filter_;
    }
    
    void set_also(bool also_filter)
    {
        also_filter_=also_filter;
    }
    
    bool has_also_filter() const
    {
        return also_filter_;
    }
    
    bool active(double scale) const
    {
        return ( scale >= min_scale_ - 1e-6 && scale < max_scale_ + 1e-6);
    }
    
private:
    
    void swap(rule& rhs) throw()
    {
        name_=rhs.name_;
        title_=rhs.title_;
        abstract_=rhs.abstract_;
        min_scale_=rhs.min_scale_;
        max_scale_=rhs.max_scale_;
        syms_=rhs.syms_;
        filter_=rhs.filter_;
        else_filter_=rhs.else_filter_;
        also_filter_=rhs.also_filter_;
    }
    
    template <class T>
    void copy_path_ptr(T* sym)
    {
        std::string path = path_processor_type::to_string(*sym->get_filename());
        sym->set_filename( parse_path(path) );
    }
    
    template <class T>
    void copy_text_ptr(T* sym)
    {
        std::string name = to_expression_string(*sym->get_name());
        sym->set_name( parse_expression(name) );
        
        // FIXME - orientation doesn't appear to be initialized in constructor?
        //std::string orientation = to_expression_string(*sym->get_orientation());
        //sym->set_orientation( parse_expression(orientation) );
        
        unsigned text_size = sym->get_text_size();
        position displace = sym->get_displacement();
        vertical_alignment_e valign = sym->get_vertical_alignment();
        horizontal_alignment_e halign = sym->get_horizontal_alignment();
        justify_alignment_e jalign = sym->get_justify_alignment();
        
        text_placements_ptr placements = text_placements_ptr(boost::make_shared<text_placements_dummy>());
        sym->set_placement_options( placements );
        
        sym->set_text_size(text_size);
        sym->set_displacement(displace);
        sym->set_vertical_alignment(valign);
        sym->set_horizontal_alignment(halign);
        sym->set_justify_alignment(jalign);
    }
};

}

#endif // MAPNIK_RULE_HPP
