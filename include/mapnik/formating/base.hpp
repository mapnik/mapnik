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
#ifndef BASE_HPP
#define BASE_HPP

#include <mapnik/feature.hpp>
#include <mapnik/filter_factory.hpp>

#include <boost/property_tree/ptree.hpp>

namespace mapnik {

typedef std::set<expression_ptr> expression_set;

namespace formating {
class node;
typedef boost::shared_ptr<node> node_ptr;
class node
{
public:
    virtual ~node() {}
    virtual void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(boost::property_tree::ptree const& xml);
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const = 0;
    virtual void add_expressions(expression_set &output) const;
};
} //ns formating
} //ns mapnik
#endif // BASE_HPP
