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

#ifndef FILTER_PARSER_AST_HPP
#define FILTER_PARSER_AST_HPP
// stl
#include <iostream>
// boost
#include <boost/spirit/core.hpp>
#include <boost/spirit/tree/ast.hpp>

using namespace std;
using namespace boost::spirit;

namespace mapnik
{
    
    struct filter_grammar_ast : public grammar<filter_grammar_ast>
    {
	
        static const int integerID = 1;
        static const int realID = 2;
        static const int stringID = 3;
        static const int propertyID = 4;
        static const int factorID = 5;
        static const int termID = 6;
        static const int expressionID = 7;
        static const int relationID = 8;
        static const int equationID = 9;
        static const int and_exprID = 10;
        static const int or_exprID = 11;
	
        template <typename ScannerT>
        struct definition
        {
	    
            definition(filter_grammar_ast const& /*self*/)
            {			
                real = leaf_node_d[strict_real_p];
                integer    = leaf_node_d[int_p];
                number = real | integer;
		
                string_ = inner_node_d['\''>> leaf_node_d[( (alpha_p | '_')  >> 
                                                            * (alnum_p | '_' ))] >>  '\''];
		
                property = inner_node_d['[' >> leaf_node_d[ ( (alpha_p | '_') >> * (alnum_p | '_' )) ] >> ']'];
		
                literal = number | string_ | property;
		
                factor = literal 
                    | (root_node_d[str_p("not")] >> literal) 
                    | inner_node_d[ch_p('(') >> or_expr >> ch_p(')') ]
                    | (root_node_d[ch_p('-')] >> factor)
                    ;
		
                term = factor
                    >> *((root_node_d[ch_p('*')] >> factor) | (root_node_d[ch_p('/')] >> factor));
		
                expression = term >> *((root_node_d[ch_p('+')] >> term) | (root_node_d[ch_p('-')] >> term));
                relation   = expression >> *((root_node_d[str_p(">=")] >> expression) 
                                             | (root_node_d[ch_p('>')] >> expression)
                                             | (root_node_d[ch_p('<')] >> expression)
                                             | (root_node_d[str_p("<=")] >> expression));
		
                equation = relation >> *( (root_node_d[ch_p('=')] >> relation)
                                          | (root_node_d[str_p("<>")] >> relation));
                and_expr = equation >> *(root_node_d[str_p("and")] >> equation);
                or_expr  = and_expr >> *(root_node_d[str_p("or")] >> and_expr);
		
                //spatial_op = str_p("Equals") | "Disjoint" | "Touches" | "Within" 
                //   | "Overlaps" | "Crosses" | "Intersects" | "Contains" | "DWithin" | "Beyond" | "BBOX";

                filter_statement = or_expr;
            }
	    
            rule<ScannerT> const& start() const
            {
                return filter_statement;
            }
	    	    
            rule<ScannerT,parser_context<>, parser_tag<factorID> > factor; 
            rule<ScannerT,parser_context<>, parser_tag<termID> > term;
            rule<ScannerT,parser_context<>, parser_tag<expressionID> > expression;
            rule<ScannerT,parser_context<>, parser_tag<relationID> > relation;
            rule<ScannerT,parser_context<>, parser_tag<equationID> > equation;
	    
            rule<ScannerT,parser_context<>, parser_tag<and_exprID> > and_expr;
            rule<ScannerT,parser_context<>, parser_tag<or_exprID> > or_expr;
	    
            rule<ScannerT> filter_statement;
            rule<ScannerT> literal,number;
	    
            rule<ScannerT,parser_context<>, parser_tag<integerID> > integer;
            rule<ScannerT,parser_context<>, parser_tag<realID> > real;
            rule<ScannerT,parser_context<>, parser_tag<stringID> > string_;
            rule<ScannerT,parser_context<>, parser_tag<propertyID> > property;
	    

            //rule<ScannerT> spatial_op;

        };
	
    }; 

    class node_data
    {
    public:
        enum  {
            Unknown=0,
            Integer=1,
            Real   =2,
            String =3,
            Property=4
        };
        node_data()
            : type_(Unknown) {}

        node_data(int type)
            : type_(type) {}
    
        node_data(node_data const& other)
            : type_(other.type_) {}
    
        node_data& operator=(node_data const& other)
        {
            if (this==&other) 
                return *this;
            type_=other.type_;
            return *this;
        }
        ~node_data() {}
    private:
        int type_;    
    };

    typedef char const* iterator_t;
    typedef node_val_data_factory<node_data> factory_t;
    typedef tree_match<iterator_t,factory_t>::tree_iterator iter_t;

    void process_node(iter_t const&,string&);
    
    void walk_ast_tree(tree_parse_info<iterator_t,factory_t> info,string& text)
    {
        process_node(info.trees.begin(),text);
    }
    
    void process_node(iter_t const& i,string& text)
    {
        //clog << "In eval_expression. i->value = " <<
        //   string(i->value.begin(), i->value.end()) <<
        //   " i->children.size() = " << i->children.size() << endl;
        //std::clog<<typeid(*i).name()<<"\n";

        if (i->value.id() == filter_grammar_ast::integerID)
        {	
            assert(i->children.size()==0);
            string integer(i->value.begin(), i->value.end());	
            text+= integer;
        }
        else if (i->value.id() == filter_grammar_ast::realID)
        {	
            assert(i->children.size()==0);
            string real(i->value.begin(), i->value.end());
            text += real;
        }
        else if (i->value.id() == filter_grammar_ast::stringID)
        {	
            assert(i->children.size()==0);
            string str(i->value.begin(), i->value.end());
            text += str;
        }
        else if (i->value.id() == filter_grammar_ast::propertyID)
        {
            assert(i->children.size()==0);
            string property_name(i->value.begin(), i->value.end());
            text += property_name;
        }
        else if (i->value.id() == filter_grammar_ast::expressionID)
        {
            assert(i->children.size() == 2);
            assert(!i->children.begin()->value.is_root());
            process_node(i->children.begin(),text);	     
            text += string(i->value.begin(), i->value.end());
            process_node(i->children.begin()+1,text);
	    
            text +="\n";
        }
        else if (i->value.id() == filter_grammar_ast::termID)
        {
            assert(i->children.size() == 2);
            assert(!i->children.begin()->value.is_root());
            process_node(i->children.begin(),text);
            text +=  string(i->value.begin(), i->value.end());
            process_node(i->children.begin()+1,text);
	    
            text +="\n";
	
        }
        else if (i->value.id() == filter_grammar_ast::relationID)
        {
            assert(i->children.size() == 2);
            assert(!i->children.begin()->value.is_root());
            process_node(i->children.begin(),text);
            text += string(i->value.begin(), i->value.end());
            process_node(i->children.begin()+1,text);
	    
            text +="\n";

        }
        else if (i->value.id() == filter_grammar_ast::equationID)
        {
            assert(i->children.size() == 2);
            assert(!i->children.begin()->value.is_root());
            process_node(i->children.begin(),text);
            text += string(i->value.begin(), i->value.end());
            process_node(i->children.begin()+1,text);
	    
            text +="\n";
        }
        else if (i->value.id() == filter_grammar_ast::and_exprID)
        {
            assert(i->children.size() == 2);
            assert(!i->children.begin()->value.is_root());
            process_node(i->children.begin(),text);
            text += string(i->value.begin(), i->value.end());
            process_node(i->children.begin()+1,text);
	    
            text +="\n";
        }
        else if (i->value.id() == filter_grammar_ast::or_exprID)
        {
            assert(i->children.size() == 2);
            assert(!i->children.begin()->value.is_root());
            
            process_node(i->children.begin(),text);
            text += string(i->value.begin(), i->value.end());
            process_node(i->children.begin()+1,text);
	    
            text +="\n";

        }
    }   
}

#endif //FILTER_PARSER_AST_HPP 
