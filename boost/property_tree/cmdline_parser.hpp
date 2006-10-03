// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_CMDLINE_PARSER_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_CMDLINE_PARSER_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/ptree_utils.hpp>

namespace boost { namespace property_tree { namespace cmdline_parser
{

    template<class Ptree>
    void read_cmdline(int argc, 
                      typename Ptree::char_type *argv[], 
                      const std::basic_string<typename Ptree::char_type> &metachars,
                      Ptree &pt)
    {

        typedef typename Ptree::char_type Ch;
        typedef std::basic_string<Ch> Str;

        Ptree local;
        
        // For all arguments
        for (int i = 0; i < argc; ++i)
        {
            Str text = detail::trim<Ch>(argv[i]);
            if (!text.empty())
                if (metachars.find(text[0]) != Str::npos)
                {
                    if (text.size() == 1)
                    {
                        Ptree &child = local.put(text, Str());
                        Str key; 
                        if (child.size() < 10) 
                            key.push_back(typename Ptree::char_type('0' + child.size()));
                        child.push_back(std::make_pair(key, Ptree(child.data())));
                    }
                    else if (text.size() == 2)
                    {
                        Ptree &child = local.put(text.substr(1, 1), Str());
                        Str key; 
                        if (child.size() < 10) 
                            key.push_back(typename Ptree::char_type('0' + child.size()));
                        child.push_back(std::make_pair(key, Ptree(child.data())));
                    }
                    else
                    {
                        Ptree &child = local.put(text.substr(1, 1), detail::trim<Ch>(text.substr(2, Str::npos)));
                        Str key; 
                        if (child.size() < 10) 
                            key.push_back(typename Ptree::char_type('0' + child.size()));
                        child.push_back(std::make_pair(key, Ptree(child.data())));
                    }
                }
                else
                {
                    Ptree &child = local.put(Str(), detail::trim<Ch>(text));
                    Str key; 
                    if (child.size() < 10) 
                        key.push_back(typename Ptree::char_type('0' + child.size()));
                    child.push_back(std::make_pair(key, Ptree(child.data())));
                }
        }

        // Swap local and pt
        pt.swap(local);

    }

} } }

namespace boost { namespace property_tree
{
    using cmdline_parser::read_cmdline;
} }

#endif
