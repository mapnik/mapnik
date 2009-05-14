/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id$

#ifndef ATTRIBUTE_COLLECTOR_HPP
#define ATTRIBUTE_COLLECTOR_HPP

// mapnik
#include <mapnik/filter.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/rule.hpp>
// stl
#include <set>
#include <iostream>

namespace mapnik {
    
    struct symbolizer_attributes : public boost::static_visitor<>
    {
        symbolizer_attributes(std::set<std::string>& names)
            : names_(names) {}
	
        template <typename T>
        void operator () (T const&) const {}
        void operator () (text_symbolizer const& sym)
        {
            names_.insert(sym.get_name());
        }
      	void operator () (shield_symbolizer const& sym)
      	{
      	    names_.insert(sym.get_name());
      	}
    private:
        std::set<std::string>& names_;
    };

    template <typename FeatureT>
    class attribute_collector : public filter_visitor<FeatureT>
    {
    private:
        std::set<std::string>& names_;
    public:
	
        attribute_collector(std::set<std::string>& names)
            : names_(names) {}
	
        void visit(filter<FeatureT>& /*filter*/) 
        { 
            //not interested
        }
	
        void visit(expression<FeatureT>& exp)
        {
            property<FeatureT>* pf;
            if ((pf = dynamic_cast<property<FeatureT>*>(&exp)))
            {
                names_.insert(pf->name());
            }
        }
        void visit(rule_type const& r)
        {	    
            const symbolizers& symbols = r.get_symbolizers();
            symbolizers::const_iterator symIter=symbols.begin();
            symbolizer_attributes attr(names_);
            while (symIter != symbols.end())
            {
                boost::apply_visitor(attr,*symIter++);
            }
            filter_ptr const& filter = r.get_filter();
            filter->accept(*this);
        }

        virtual ~attribute_collector() {}
    private:	
        // no copying 
        attribute_collector(attribute_collector const&);
        attribute_collector& operator=(attribute_collector const&);
    };   
}

#endif //ATTRIBUTE_COLLECTOR_HPP
