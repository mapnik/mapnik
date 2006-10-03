// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_DETAIL_INFO_PARSER_WRITE_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_DETAIL_INFO_PARSER_WRITE_HPP_INCLUDED

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/detail/info_parser_utils.hpp"
#include <string>

namespace boost { namespace property_tree { namespace info_parser
{
    
    // Create necessary escape sequences from illegal characters
    template<class Ch>
    std::basic_string<Ch> create_escapes(const std::basic_string<Ch> &s)
    {
        std::basic_string<Ch> result;
        typename std::basic_string<Ch>::const_iterator b = s.begin();
        typename std::basic_string<Ch>::const_iterator e = s.end();
        while (b != e)
        {
            if (*b == Ch('\0')) result += Ch('\\'), result += Ch('0');
            else if (*b == Ch('\a')) result += Ch('\\'), result += Ch('a');
            else if (*b == Ch('\b')) result += Ch('\\'), result += Ch('b');
            else if (*b == Ch('\f')) result += Ch('\\'), result += Ch('f');
            else if (*b == Ch('\n')) result += Ch('\\'), result += Ch('n');
            else if (*b == Ch('\r')) result += Ch('\\'), result += Ch('r');
            else if (*b == Ch('\v')) result += Ch('\\'), result += Ch('v');
            else if (*b == Ch('"')) result += Ch('\\'), result += Ch('"');
            else if (*b == Ch('\\')) result += Ch('\\'), result += Ch('\\');
            else
                result += *b;
            ++b;
        }
        return result;
    }

    template<class Ch>
    bool is_simple_key(const std::basic_string<Ch> &key)
    {
        const static std::basic_string<Ch> chars = convert_chtype<Ch, char>(" \t{};\n\"");
        return !key.empty() && key.find_first_of(chars) == key.npos;
    }
    
    template<class Ch>
    bool is_simple_data(const std::basic_string<Ch> &data)
    {
        const static std::basic_string<Ch> chars = convert_chtype<Ch, char>(" \t{};\n\"");
        return !data.empty() && data.find_first_of(chars) == data.npos;
    }

    template<class Ptree>
    void write_info_helper(std::basic_ostream<typename Ptree::char_type> &stream, 
                           const Ptree &pt, 
                           int indent)
    {

        // Character type
        typedef typename Ptree::char_type Ch;
        
        // Write data
        if (indent >= 0)
        {
            if (!pt.data().empty())
            {
                std::basic_string<Ch> data = create_escapes(pt.template get_own<std::basic_string<Ch> >());
                if (is_simple_data(data))
                    stream << Ch(' ') << data << Ch('\n');
                else
                    stream << Ch(' ') << Ch('\"') << data << Ch('\"') << Ch('\n');
            }
            else if (pt.empty())
                stream << Ch(' ') << Ch('\"') << Ch('\"') << Ch('\n');
            else
                stream << Ch('\n');
        }
        
        // Write keys
        if (!pt.empty())
        {
            
            // Open brace
            if (indent >= 0) 
                stream << std::basic_string<Ch>(4 * indent, Ch(' ')) << Ch('{') << Ch('\n');
            
            // Write keys
            typename Ptree::const_iterator it = pt.begin();
            for (; it != pt.end(); ++it)
            {

                // Output key
                std::basic_string<Ch> key = create_escapes(it->first);
                stream << std::basic_string<Ch>(4 * (indent + 1), Ch(' '));
                if (is_simple_key(key))
                    stream << key;
                else
                    stream << Ch('\"') << key << Ch('\"');

                // Output data and children  
                write_info_helper(stream, it->second, indent + 1);

            }
            
            // Close brace
            if (indent >= 0) 
                stream << std::basic_string<Ch>(4 * indent, Ch(' ')) << Ch('}') << Ch('\n');

        }
    }

    // Write ptree to info stream
    template<class Ptree>
    void write_info_internal(std::basic_ostream<typename Ptree::char_type> &stream, 
                             const Ptree &pt,
                             const std::string &filename)
    {
        write_info_helper(stream, pt, -1);
        if (!stream.good())
            throw info_parser_error("write error", filename, 0);
    }

} } }

#endif
