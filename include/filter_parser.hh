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

#ifndef FILTER_PARSER_HH
#define FILTER_PARSER_HH

#include <boost/spirit/core.hpp>
#include <boost/spirit/symbols.hpp>

#include "value.hh"
#include "expression.hh"
#include "filter.hh"

#include <stack>
#include <iostream>

using namespace boost::spirit;
using std::string;
using std::cout;
using std::stack;

namespace mapnik
{    
    template <typename FeatureT>
    struct push_integer
    {
	push_integer(stack<ref_ptr<expression<FeatureT> > >& exprs)
	    : exprs_(exprs) {}
	
	void operator() (int val) const
	{
	    cout << "push int("<<val<<")\n";
	    exprs_.push(ref_ptr<expression<FeatureT> >(new literal<FeatureT>(val)));
	}
	stack<ref_ptr<expression<FeatureT> > >& exprs_;
    };
   
    template <typename FeatureT>
    struct push_real
    {
	push_real(stack<ref_ptr<expression<FeatureT> > >& exprs)
	    : exprs_(exprs) {}
	void operator() (double val) const
	{
	    cout << "push real("<<val<<")\n";
	    exprs_.push(ref_ptr<expression<FeatureT> >(new literal<FeatureT>(val)));
	}
	stack<ref_ptr<expression<FeatureT> > >& exprs_;
    };
    
    template <typename FeatureT>
    struct push_string
    {
	push_string(stack<ref_ptr<expression<FeatureT> > >& exprs)
	    : exprs_(exprs) {}
	
	template <typename Iter>
	void operator() (Iter start,Iter end) const
	{
	    string str(start,end);
	    cout << "push string("<<str<<")\n";
	    exprs_.push(ref_ptr<expression<FeatureT> >(new literal<FeatureT>(str)));
	}
	stack<ref_ptr<expression<FeatureT> > >& exprs_;
    };
    
    template <typename FeatureT>
    struct push_property
    {
	push_property(stack<ref_ptr<expression<FeatureT> > >& exprs)
	    : exprs_(exprs) {}
	
	template <typename Iter>
	void operator() (Iter start,Iter end) const
	{
	    string str(start,end);
	    cout << "property name ["<<str<<"]\n";
	    exprs_.push(ref_ptr<expression<FeatureT> >(new property<FeatureT>(str)));
	}
	stack<ref_ptr<expression<FeatureT> > >& exprs_;
    };

    template <typename FeatureT,typename Op>
    struct compose_expression
    {
	compose_expression(stack<ref_ptr<expression<FeatureT> > >& exprs)
	    : exprs_(exprs) {}

	template <typename Iter>
	void operator() (Iter,Iter) const
	{
	    if (exprs_.size()>=2)
	    {
		ref_ptr<expression<FeatureT> > right = exprs_.top();
		exprs_.pop();
		ref_ptr<expression<FeatureT> > left = exprs_.top();
		exprs_.pop();
		if (left && right)
		{
		    exprs_.push(ref_ptr<expression<FeatureT> >(new math_expr_b<FeatureT,Op>(*left,*right)));
		}
	    }
	}
	stack<ref_ptr<expression<FeatureT> > >& exprs_;
    };
    
    template <typename FeatureT,typename Op>
    struct compose_filter
    {
	compose_filter(stack<ref_ptr<filter<FeatureT> > >& filters,
		       stack<ref_ptr<expression<FeatureT> > >& exprs)
	    : filters_(filters),exprs_(exprs) {}

	template <typename Iter>
	void operator() (Iter,Iter) const
	{
	    if (exprs_.size()>=2)
	    {
		ref_ptr<expression<FeatureT> > right = exprs_.top();
		exprs_.pop();
		ref_ptr<expression<FeatureT> > left = exprs_.top();
		exprs_.pop();
		if (left && right)
		{
		    filters_.push(ref_ptr<filter<FeatureT> >(new compare_filter<FeatureT,Op>(*left,*right)));
		}
	    }
	}
	stack<ref_ptr<filter<FeatureT> > >& filters_;
	stack<ref_ptr<expression<FeatureT> > >& exprs_;
    };
    
    template <typename FeatureT>
    struct compose_and_filter
    {
	compose_and_filter(stack<ref_ptr<filter<FeatureT> > >& filters)
	    : filters_(filters) {}

	template <typename Iter>
	void operator() (Iter,Iter) const
	{
	    if (filters_.size()>=2)
	    {
		ref_ptr<filter<FeatureT> > right = filters_.top();
		filters_.pop();
		ref_ptr<filter<FeatureT> > left = filters_.top();
		filters_.pop();
		if (left && right)
		{
		    filters_.push(ref_ptr<filter<FeatureT> >(new logical_and<FeatureT>(*left,*right)));
		}
	    }
	}
	stack<ref_ptr<filter<FeatureT> > >& filters_;
    };
    
    template <typename FeatureT>
    struct compose_or_filter
    {
	compose_or_filter(stack<ref_ptr<filter<FeatureT> > >& filters)
	    : filters_(filters) {}

	template <typename Iter>
	void operator() (Iter,Iter) const
	{
	    if (filters_.size()>=2)
	    {
		ref_ptr<filter<FeatureT> > right = filters_.top();
		filters_.pop();
		ref_ptr<filter<FeatureT> > left = filters_.top();
		filters_.pop();
		if (left && right)
		{
		    filters_.push(ref_ptr<filter<FeatureT> >(new logical_or<FeatureT>(*left,*right)));
		}
	    }
	}
	stack<ref_ptr<filter<FeatureT> > >& filters_;
    };
    
    template <typename FeatureT>
    struct compose_not_filter
    {
	compose_not_filter(stack<ref_ptr<filter<FeatureT> > >& filters)
	    : filters_(filters) {}

	template <typename Iter>
	void operator() (Iter,Iter) const
	{
	    if (filters_.size()>=1)
	    {
		ref_ptr<filter<FeatureT> > filter_ = filters_.top();
		filters_.pop();
		if (filter_)
		{
		    filters_.push(ref_ptr<filter<FeatureT> >(new logical_not<FeatureT>(*filter_)));
		}
	    }
	}
	stack<ref_ptr<filter<FeatureT> > >& filters_;
    };
    
    template <typename FeatureT>
    struct filter_grammar : public grammar<filter_grammar<FeatureT> >
    {
	filter_grammar(stack<ref_ptr<filter<FeatureT> > >& filters_,
		       stack<ref_ptr<expression<FeatureT> > >& exprs_)
	    : filters(filters_),exprs(exprs_) {}
	
	template <typename ScannerT>
	struct definition
	{
	    definition(filter_grammar const& self)
	    {	

		func1_op = "sqrt","sin","cos";
		func2_op = "min","max";
		spatial_op = "Equals","Disjoint","Touches","Within","Overlaps",
		    "Crosses","Intersects","Contains","DWithin","Beyond","BBOX";
		
		number = strict_real_p [push_real<FeatureT>(self.exprs)] 
		    | int_p [push_integer<FeatureT>(self.exprs)];
		string_ ='\''>> ( (alpha_p | '_')  >> 
				  * (alnum_p | '_' )) [push_string<FeatureT>(self.exprs)] >> '\'';
		property = '[' >> ( (alpha_p | '_') 
				    >> * (alnum_p | '_' ))[push_property<FeatureT>(self.exprs)]>>']';
		
		literal = number | string_ | property;
		
		function = literal | ( func1_op >> '('>> literal >> ')') | 
		    (func2_op >> '(' >> literal >>','>> literal >> ')');
		factor = function 
		    | '(' >> or_expr >> ')'
		    | ( '-' >> factor) 
		    ;
		term = factor
		    >> *(('*' >> factor) [compose_expression<FeatureT,mult<value> >(self.exprs)] 
			 | ('/' >> factor) [compose_expression<FeatureT,div<value> >(self.exprs)]);
		
		expression = term >> *(('+' >> term) [compose_expression<FeatureT,add<value> >(self.exprs)] 
				       | ('-' >> term) [compose_expression<FeatureT,sub<value> >(self.exprs)]);
		
		relation   = expression 
		    >> *((">=" >> expression) 
			 [compose_filter<FeatureT,greater_than_or_equal<value> >(self.filters,self.exprs)]
			 | ('>' >> expression)
			 [compose_filter<FeatureT,greater_than<value> >(self.filters,self.exprs)]
			 | ('<' >> expression)
			 [compose_filter<FeatureT,less_than<value> >(self.filters,self.exprs)]
			 | ("<=" >> expression)
			 [compose_filter<FeatureT,less_than_or_equal<value> >(self.filters,self.exprs)]);

		equation = relation >> *( ( '=' >> relation)
		     [compose_filter<FeatureT,equals<value> >(self.filters,self.exprs)]
		     | ( "<>" >> relation)
		     [compose_filter<FeatureT,not_equals<value> >(self.filters,self.exprs)]);
		not_expr = equation | *(str_p("not") >> equation)[compose_not_filter<FeatureT>(self.filters)];
		and_expr = not_expr >> *("and" >> not_expr)[compose_and_filter<FeatureT>(self.filters)];
		or_expr  = and_expr >> *("or" >> and_expr)[compose_or_filter<FeatureT>(self.filters)];
		filter_statement = or_expr;	
	    }
	    
	    boost::spirit::rule<ScannerT> const& start() const
	    {
		return filter_statement;
	    }
	    	    
	    boost::spirit::rule<ScannerT> factor; 
	    boost::spirit::rule<ScannerT> term;
	    boost::spirit::rule<ScannerT> expression;
	    boost::spirit::rule<ScannerT> relation;
	    boost::spirit::rule<ScannerT> equation;
	    boost::spirit::rule<ScannerT> not_expr;
	    boost::spirit::rule<ScannerT> and_expr;
	    boost::spirit::rule<ScannerT> or_expr;
	    
	    boost::spirit::rule<ScannerT> filter_statement;
	    boost::spirit::rule<ScannerT> literal,number,string_,property,function;
	    symbols<string> func1_op;
	    symbols<string> func2_op;
	    symbols<string> spatial_op;
	};
	stack<ref_ptr<filter<FeatureT> > >& filters;
	stack<ref_ptr<expression<FeatureT> > >& exprs;
    };
    
}

#endif //FILTER_PARSER_HH 
