// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_XML_PARSER_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_XML_PARSER_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/xml_parser_write.hpp>
#include <boost/property_tree/detail/xml_parser_error.hpp>
#include <boost/property_tree/detail/xml_parser_flags.hpp>

// Include proper parser
#ifdef BOOST_PROPERTY_TREE_XML_PARSER_TINYXML
#include <boost/property_tree/detail/xml_parser_read_tinyxml.hpp>
#else
#include <boost/property_tree/detail/xml_parser_read_spirit.hpp>
#endif

#include <fstream>
#include <string>
#include <locale>

namespace boost { namespace property_tree { namespace xml_parser
{

    // Read XML from stream
    template<class Ptree>
    void read_xml(std::basic_istream<typename Ptree::char_type> &stream,
                  Ptree &pt,
                  int flags = 0)
    {
        read_xml_internal(stream, pt, flags, std::string());
    }

    // Read XML from file
    template<class Ptree>
    void read_xml(const std::string &filename,
                  Ptree &pt,
                  int flags = 0,
                  const std::locale &loc = std::locale())
    {
        BOOST_ASSERT(validate_flags(flags));
        std::basic_ifstream<typename Ptree::char_type> stream(filename.c_str());
        if (!stream)
            throw xml_parser_error("cannot open file", filename, 0);
        stream.imbue(loc);
        read_xml_internal(stream, pt, flags, filename);
    }

    // Write XML to stream
    template<class Ptree>
    void write_xml(std::basic_ostream<typename Ptree::char_type> &stream, 
                   const Ptree &pt)
    {
        write_xml_internal(stream, pt, std::string());
    }

    // Write XML to file
    template<class Ptree>
    void write_xml(const std::string &filename,
                   const Ptree &pt,
                   const std::locale &loc = std::locale())
    {
        std::basic_ofstream<typename Ptree::char_type> stream(filename.c_str());
        if (!stream)
            throw xml_parser_error("cannot open file", filename, 0);
        stream.imbue(loc);
        write_xml_internal(stream, pt, filename);
    }

} } }

namespace boost { namespace property_tree
{
    using xml_parser::read_xml;
    using xml_parser::write_xml;
    using xml_parser::xml_parser_error;
} }

#endif
