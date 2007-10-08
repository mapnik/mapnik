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

#ifndef REGEX_FILTER_HPP
#define REGEX_FILTER_HPP
// mapnik
#include <mapnik/filter.hpp>
#include <mapnik/expression.hpp>
// boost
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

namespace mapnik
{ 
    template <typename FeatureT>
    struct regex_filter : public filter<FeatureT>
    {

        regex_filter(expression<FeatureT> const& exp,
                     std::string const& pattern)
            : filter<FeatureT>(),
              exp_(exp.clone()),
              pattern_(pattern) {}
	
        regex_filter(regex_filter const& other)
            :  filter<FeatureT>(),
               exp_(other.exp_->clone()),
               pattern_(other.pattern_) {}
	
        bool pass(FeatureT const& feature) const
        {
            std::string text=exp_->get_value(feature).to_string();
            boost::trim_if(text, boost::is_any_of("'"));
            return boost::regex_match(text,pattern_);
        } 
	
        void accept(filter_visitor<FeatureT>& v)
        {
            exp_->accept(v);
            v.visit(*this);
        }
	
        filter<FeatureT>* clone() const
        {
            return new regex_filter(*this);
        }
        std::string to_string() const
        {
            return exp_->to_string()+".match("+pattern_.str()+")";
        }
        ~regex_filter()
        {
            delete exp_;
        }
	
    private:
        expression<FeatureT>* exp_;
        boost::regex pattern_;
	
    };   
}


#endif //REGEX_FILTER_HPP
