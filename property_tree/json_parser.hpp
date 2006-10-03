// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_JSON_PARSER_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_JSON_PARSER_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/json_parser_read.hpp>
#include <boost/property_tree/detail/json_parser_write.hpp>
#include <boost/property_tree/detail/json_parser_error.hpp>

#include <fstream>
#include <string>
#include <locale>

namespace boost { namespace property_tree { namespace json_parser
{

    // Read json from stream
    template<class Ptree>
    void read_json(std::basic_istream<typename Ptree::char_type> &stream,
                   Ptree &pt)
    {
        read_json_internal(stream, pt, std::string());
    }

    // Read json from file
    template<class Ptree>
    void read_json(const std::string &filename,
                   Ptree &pt,
                   const std::locale &loc = std::locale())
    {
        std::basic_ifstream<typename Ptree::char_type> stream(filename.c_str());
        if (!stream)
            throw json_parser_error("cannot open file", filename, 0);
        stream.imbue(loc);
        read_json_internal(stream, pt, filename);
    }

    // Write json to stream
    template<class Ptree>
    void write_json(std::basic_ostream<typename Ptree::char_type> &stream, 
                    const Ptree &pt)
    {
        write_json_internal(stream, pt, std::string());
    }

    // Write json to file
    template<class Ptree>
    void write_json(const std::string &filename,
                    const Ptree &pt,
                    const std::locale &loc = std::locale())
    {
        std::basic_ofstream<typename Ptree::char_type> stream(filename.c_str());
        if (!stream)
            throw json_parser_error("cannot open file", filename, 0);
        stream.imbue(loc);
        write_json_internal(stream, pt, filename);
    }

} } }

namespace boost { namespace property_tree
{
    using json_parser::read_json;
    using json_parser::write_json;
    using json_parser::json_parser_error;
} }

#endif
