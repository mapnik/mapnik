// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_DETAIL_XML_PARSER_READ_TINYXML_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_DETAIL_XML_PARSER_READ_TINYXML_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/xml_parser_error.hpp>
#include <boost/property_tree/detail/xml_parser_flags.hpp>
#include <boost/property_tree/detail/xml_parser_utils.hpp>

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include <tinyxml.h>

namespace boost { namespace property_tree { namespace xml_parser
{

    template<class Ptree>
    void read_xml_node(TiXmlNode *node, Ptree &pt, int flags)
    {

        typedef typename Ptree::char_type Ch;

        if (TiXmlElement *elem = node->ToElement())
        {
            Ptree &tmp = pt.push_back(std::make_pair(elem->Value(), Ptree()))->second;
            for (TiXmlAttribute *attr = elem->FirstAttribute(); attr; attr = attr->Next())
                tmp.put(Ch('/'), xmlattr<Ch>() + "/" + attr->Name(), attr->Value());
            for (TiXmlNode *child = node->FirstChild(); child; child = child->NextSibling())
                read_xml_node(child, tmp, flags);
        }
        else if (TiXmlText *text = node->ToText())
        {
            if (flags & no_concat_text)
                pt.push_back(std::make_pair(xmltext<Ch>(), Ptree(text->Value())));
            else
                pt.data() += text->Value();
        }
        else if (TiXmlComment *comment = node->ToComment())
        {
            if (!(flags & no_comments))
                pt.push_back(std::make_pair(xmlcomment<Ch>(), Ptree(comment->Value())));
        }
    }

    template<class Ptree>
    void read_xml_internal(std::basic_istream<typename Ptree::char_type> &stream,
                           Ptree &pt,
                           int flags,
                           const std::string &filename)
    {

        // Create and load document from stream
        TiXmlDocument doc;
        stream >> doc;
        if (!stream.good())
            throw xml_parser_error("read error", filename, 0);
        if (doc.Error())
            throw xml_parser_error(doc.ErrorDesc(), filename, doc.ErrorRow());

        // Create ptree from nodes
        Ptree local;
        for (TiXmlNode *child = doc.FirstChild(); child; child = child->NextSibling())
            read_xml_node(child, local, flags);

        // Swap local and result ptrees
        pt.swap(local);

    }

} } }

#endif
