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

#ifndef COMPARISON_HPP
#define COMPARISON_HPP

#include <mapnik/filter.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/attribute.hpp>

namespace mapnik {   
    template <typename T>  
    struct greater_than
    {
        bool operator() (T const& left, T const& right) const
        {
            return left > right;
        }
        static std::string to_string()
        {
            return ">";
        }
    };

    template <typename T>  
    struct greater_than_or_equal
    {
        bool operator() (T const& left, T const& right) const
        {
            return left >= right;
        }
        static std::string to_string()
        {
            return ">=";
        }
    };
    template <typename T>  
    struct less_than
    {
        bool operator() (T const& left, T const& right) const
        {
            return left < right;
        }
        static std::string to_string()
        {
            return "<";
        }
    };
    template <typename T>  
    struct less_than_or_equal
    {
        bool operator() (T const& left, T const& right) const
        {
            return left <= right;
        }
        static std::string to_string()
        {
            return "<=";
        }
    };
    template <typename T>  
    struct equals
    {
        bool operator() (T const& left, T const& right) const
        {
            return left == right;
        }
        static std::string to_string()
        {
            return "=";
        }
    };
    
    template <typename T>  
    struct not_equals
    {
        bool operator() (T const& left, T const& right) const
        {
            return left != right;
        }
        static std::string to_string()
        {
            return "<>";
        }
    };
    
    template <typename FeatureT,typename Op>
    struct compare_filter : public filter<FeatureT>
    {
        compare_filter(expression<FeatureT> const& left,
                       expression<FeatureT> const& right)
            : filter<FeatureT>(),
              left_(left.clone()), right_(right.clone()) {}

        compare_filter(compare_filter const& other)
            : filter<FeatureT>(),
              left_(other.left_->clone()),right_(other.right_->clone()) {}
	
        bool pass(const FeatureT& feature) const
        {   
            return Op()(left_->get_value(feature),right_->get_value(feature));     
        }
        void accept(filter_visitor<FeatureT>& v)
        {
            left_->accept(v);
            right_->accept(v);
            v.visit(*this);
        }
        std::string to_string() const
        {
            return "("+left_->to_string()+Op::to_string()+right_->to_string()+")";
        }

        filter<FeatureT>* clone() const
        {
            return new compare_filter<FeatureT,Op>(*this);
        }
        virtual ~compare_filter() 
        {
            delete left_;
            delete right_;
        }
    private:
        expression<FeatureT>* left_;
        expression<FeatureT>* right_;
    };
}

#endif //COMPARISON_HPP
