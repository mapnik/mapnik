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

#ifndef FILTER_VISITOR_HPP
#define FILTER_VISITOR_HPP

#include <mapnik/filter.hpp>
#include <mapnik/expression.hpp>

namespace mapnik
{
    template <typename FeatureT> class filter;
    template <typename FeatureT> class expression;
    template <typename FeatureT> class expression;
    template <typename Feature,template <typename> class Filter> class rule;
    template <typename FeatureT>
    class filter_visitor
    {
	public:
		virtual void visit(filter<FeatureT>& filter)=0;
		virtual void visit(expression<FeatureT>&)=0;
		virtual void visit(rule<FeatureT,filter> const& r)=0;
		virtual ~filter_visitor() {}
    };    
}

#endif //FILTER_VISITOR_HPP
