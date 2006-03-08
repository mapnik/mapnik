/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#ifndef MATH_EXPR_HPP
#define MATH_EXPR_HPP

#include "expression.hpp"

namespace mapnik
{
    template <typename T>
    struct add
    {
	T operator () (T const& left, T const& right)
	{
	    return left + right;
	}
	static std::string to_string()
	{
	    return "+";
	} 
    };

    template <typename T>
    struct sub
    {
	T operator () (T const& left, T const& right)
	{
	    return left - right;
	}
	static std::string to_string()
	{
	    return "-";
	} 
    };
    
    template <typename T>
    struct mult
    {
	T operator () (T const& left, T const& right)
	{
	    return left * right;
	}
	static std::string to_string()
	{
	    return "*";
	} 
    };
    
    template <typename T>
    struct div
    {
	T operator () (T const& left, T const& right)
	{
	    return left / right;
	}
	static std::string to_string()
	{
	    return "/";
	} 
    };
    
    template <typename FeatureT,typename Op>
    struct math_expr_b : public expression<FeatureT>
    {
	math_expr_b(expression<FeatureT> const& left,
		    expression<FeatureT> const& right)
	    : expression<FeatureT>(),
	      left_(left.clone()), 
	      right_(right.clone()) {}
	math_expr_b(math_expr_b const& other)
	    : expression<FeatureT>(),
	      left_(other.left_->clone()),
	      right_(other.right_->clone()) {}

	value get_value(FeatureT const& feature) const
	{
	    return Op ()(left_->get_value(feature),right_->get_value(feature));
	}

	void accept(filter_visitor<FeatureT>& v)
	{
	    left_->accept(v);
	    right_->accept(v);
	    v.visit(*this);
	}

	expression<FeatureT>* clone() const
	{
	    return new math_expr_b<FeatureT,Op>(*this);
	}
	std::string to_string() const
	{
	    return "("+left_->to_string() + Op::to_string() + right_->to_string()+")";
	}

	~math_expr_b() 
	{
	    delete left_;
	    delete right_;
	}
    private:
	expression<FeatureT>* left_;
	expression<FeatureT>* right_;	
    }; 
}


#endif //
