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

#ifndef RULE_HH
#define RULE_HH

#include "symbolizer.hh"
#include "filter.hh"
#include <ptr.hh>
#include <string>
#include <vector>

namespace mapnik
{
    typedef ref_ptr<Symbolizer> symbolizer_ptr;
    typedef std::vector<symbolizer_ptr> symbolizers;    
    template <typename FeatureT> class null_filter;

    template <typename FeatureT,template <typename> class Filter>
    class rule
    {
	typedef Filter<FeatureT> filter_type;
	typedef ref_ptr<filter_type> filter_ptr;
    private:

	std::string name_;
	std::string title_;
	std::string abstract_;
	double min_scale_;
	double max_scale_;
        symbolizers syms_;
	filter_ptr filter_;
	bool else_filter_;
    public:
	rule()
	    : name_(),
	      title_(),
	      abstract_(),
	      min_scale_(0),
	      max_scale_(std::numeric_limits<double>::infinity()),
	      syms_(),
	      filter_(new null_filter<FeatureT>),
	      else_filter_(false) {}
	
	rule(const std::string& name,const std::string& title)
	    : name_(name),
	      title_(title),
	      abstract_(),
	      min_scale_(0),
	      max_scale_(std::numeric_limits<double>::infinity()),
	      syms_(),
	      filter_(new null_filter<FeatureT>),
	      else_filter_(false) {}
        
	rule(const std::string& name,const std::string& title,
	     double min_scale_denominator,double max_scale_denominator)
	    : name_(name),
	      title_(title),
	      min_scale_(min_scale_denominator),
	      max_scale_(max_scale_denominator),
	      syms_(),
	      filter_(new null_filter<FeatureT>),
	      else_filter_(false) {}
	    
	rule(const rule& rhs)    
	    : name_(rhs.name_),
	      title_(rhs.title_),
	      abstract_(rhs.abstract_),
	      min_scale_(rhs.min_scale_),
	      max_scale_(rhs.max_scale_),
	      syms_(rhs.syms_),
	      filter_(rhs.filter_),
	      else_filter_(rhs.else_filter_) {}
	
        rule& operator=(rule const& rhs) 
	{
	    rule tmp(rhs);
	    swap(tmp);
	    return *this;
	}
	
	void abstract(const std::string& abstract)
	{
	    abstract_=abstract;
	}
	const std::string& abstract() const
	{
	    return abstract_;
	}
	
	void add(const symbolizer_ptr& symbol)
	{
	    syms_.push_back(symbol);
	}
	
	const symbolizers& get_symbolizers() const
	{
	    return syms_;
	}
	
	void set_filter(const filter_ptr& filter)
	{
	    filter_=filter;
	}

	const filter_ptr& get_filter() const
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
	
    private:
	
	void swap(rule& rhs) throw()
	{
	    title_=rhs.title_;
	    abstract_=rhs.abstract_;
	    min_scale_=rhs.min_scale_;
	    max_scale_=rhs.max_scale_;
	    syms_=rhs.syms_;
	    filter_=rhs.filter_;
	    else_filter_=rhs.else_filter_;
	}
    };
}

#endif //RULE_HH
