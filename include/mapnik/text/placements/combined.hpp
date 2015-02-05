/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Walid Ibrahim
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

#ifndef TEXT_PLACEMENTS_COMBINED_HPP
#define TEXT_PLACEMENTS_COMBINED_HPP

#include <mapnik/text/placements/base.hpp>

namespace mapnik {

    class text_placements_info_combined;
    class feature_impl;
    struct attribute;
    
    class text_placements_combined: public text_placements
    {
        
    public:
        text_placements_combined(text_placements_ptr simple_placement, text_placements_ptr list_placement);
        text_placement_info_ptr get_placement_info(double scale_factor, feature_impl const& feature, attributes const& vars) const;
        virtual void add_expressions(expression_set & output) const;
        static text_placements_ptr from_xml(xml_node const& xml, fontset_map const& fontsets, bool is_shield);
        
        text_placements_ptr get_simple_placement() { return simple_placement_; }
        text_placements_ptr get_list_placement() { return list_placement_; }
        
    private:
        text_placements_ptr simple_placement_;
        text_placements_ptr list_placement_;
        
        friend class text_placements_info_combined;
    };
    
    
    class text_placement_info_combined: public text_placement_info
    {
        
    public:
        text_placement_info_combined(text_placements_combined const* parent, double scale_factor, text_placement_info_ptr simple_placement_info, text_placement_info_ptr list_placement_info);
        bool next() const;
        virtual void reset_state();
    private:
        text_placements_combined const* parent_;
        text_placement_info_ptr simple_placement_info_;
        text_placement_info_ptr list_placement_info_;
    };
    
} //namespace

#endif
