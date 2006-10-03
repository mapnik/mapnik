// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_DETAIL_XML_PARSER_WRITE_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_DETAIL_XML_PARSER_WRITE_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/xml_parser_utils.hpp>
#include <string>
#include <ostream>
#include <iomanip>

namespace boost { namespace property_tree { namespace xml_parser
{

    template<class Ch>
    void write_xml_comment(std::basic_ostream<Ch> &stream,
                           const std::basic_string<Ch> &s, 
                           int indent)
    {
        typedef typename std::basic_string<Ch> Str;
        stream << Str(4 * indent, Ch(' '));
        stream << Ch('<') << Ch('!') << Ch('-') << Ch('-');
        stream << s;
        stream << Ch('-') << Ch('-') << Ch('>') << std::endl;
    }
    
    template<class Ch>
    void write_xml_text(std::basic_ostream<Ch> &stream,
                        const std::basic_string<Ch> &s, 
                        int indent, 
                        bool separate_line)
    {
        typedef typename std::basic_string<Ch> Str;
        if (separate_line)    
            stream << Str(4 * indent, Ch(' '));
        stream << encode_char_entities(s);
        if (separate_line)    
            stream << Ch('\n');
    }

    template<class Ptree>
    void write_xml_element(std::basic_ostream<typename Ptree::char_type> &stream, 
                           const std::basic_string<typename Ptree::char_type> &key,
                           const Ptree &pt, 
                           int indent)
    {

        typedef typename Ptree::char_type Ch;
        typedef typename std::basic_string<Ch> Str;
        typedef typename Ptree::const_iterator It;

        // Find if elements present
        bool has_elements = false;
        for (It it = pt.begin(), end = pt.end(); it != end; ++it)
            if (it->first != xmlattr<Ch>() &&
                it->first != xmltext<Ch>())
            {
                has_elements = true;
                break;
            }
        
        // Write element
        if (pt.data().empty() && pt.empty())    // Empty key
        {
            if (indent >= 0)
                stream << Str(4 * indent, Ch(' ')) << Ch('<') << key << 
                          Ch('/') << Ch('>') << std::endl;
        }
        else    // Nonempty key
        {
        
            // Write opening tag, attributes and data
            if (indent >= 0)
            {
            
                // Write opening brace and key
                stream << Str(4 * indent, Ch(' '));
                stream << Ch('<') << key;

                // Write attributes
                if (optional<const Ptree &> attribs = pt.get_child_optional(xmlattr<Ch>()))
                    for (It it = attribs.get().begin(); it != attribs.get().end(); ++it)
                        stream << Ch(' ') << it->first << Ch('=') << 
                                  Ch('"') << it->second.template get_own<std::basic_string<Ch> >() << Ch('"');

                // Write closing brace
                stream << Ch('>');

                // Break line if needed
                if (has_elements)
                    stream << Ch('\n');

            }
            
            // Write data text, if present
            if (!pt.data().empty())
                write_xml_text(stream, pt.template get_own<std::basic_string<Ch> >(), indent + 1, has_elements);
            
            // Write elements, comments and texts
            for (It it = pt.begin(); it != pt.end(); ++it)
            {
                if (it->first == xmlattr<Ch>())
                    continue;
                else if (it->first == xmlcomment<Ch>())
                    write_xml_comment(stream, it->second.template get_own<std::basic_string<Ch> >(), indent + 1);
                else if (it->first == xmltext<Ch>())
                    write_xml_text(stream, it->second.template get_own<std::basic_string<Ch> >(), indent + 1, has_elements);
                else
                    write_xml_element(stream, it->first, it->second, indent + 1);
            }
            
            // Write closing tag
            if (indent >= 0)
            {
                if (has_elements)
                    stream << Str(4 * indent, Ch(' '));
                stream << Ch('<') << Ch('/') << key << Ch('>') << std::endl;
            }

        }
    }

    template<class Ptree>
    void write_xml_internal(std::basic_ostream<typename Ptree::char_type> &stream, 
                            const Ptree &pt,
                            const std::string &filename)
    {
        typedef typename Ptree::char_type Ch;
        typedef typename std::basic_string<Ch> Str;
        stream << detail::widen<Ch>("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
        write_xml_element(stream, Str(), pt, -1);
        if (!stream)
            throw xml_parser_error("write error", filename, 0);
    }

} } }

#endif
