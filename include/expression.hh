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

#ifndef EXPRESSION_HH
#define EXPRESSION_HH

#include "value.hh"
#include "filter_visitor.hh"

namespace mapnik
{
    template <typename FeatureT> class filter_visitor;
    template <typename FeatureT>
    struct expression
    {
	virtual value get_value(FeatureT const& feature) const=0;
	virtual void accept(filter_visitor<FeatureT>& v)=0;
	virtual expression<FeatureT>* clone() const=0;
	virtual ~expression() {}
    };

    template <typename FeatureT> 
    class literal : public expression<FeatureT>
    {
    public:
	literal(int val)
	    : expression<FeatureT>(),
	      value_(val) {}
	literal(double val)
	    : expression<FeatureT>(),
	      value_(val) {}
	literal(std::string const& val)
	    : expression<FeatureT>(),
	      value_(val) {}
	literal(literal const& other)
	    : expression<FeatureT>(),
	      value_(other.value_) {}
	
	value get_value(FeatureT const& /*feature*/) const
	{
	    return value_;
	}
	void accept(filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}
	expression<FeatureT>* clone() const
	{
	    return new literal(*this); 
	}

        ~literal() {}
    private:
	value value_;
	
    };
  

    template <typename FeatureT> 
    class property : public expression<FeatureT>
    {
    public:
	property(std::string const& name)
	    : expression<FeatureT>(),
	      name_(name) {}
	
	property(property const& other)
	    : expression<FeatureT>(),
	      name_(other.name_) {}

	value get_value(FeatureT const& feature) const
	{
	    const attribute& attr=feature.attribute_by_name(name_);
	    return value(attr);
	}
	void accept(filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}
	expression<FeatureT>* clone() const
	{
	    return new property(*this); 
	}
	std::string const& name() const
	{
	    return name_;
	}
        ~property() {}
    private:
	std::string name_;
    };

}

#endif //EXPRESSION_HH
