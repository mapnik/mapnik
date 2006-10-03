// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_INFO_PARSER_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_INFO_PARSER_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/info_parser_error.hpp>
#include <boost/property_tree/detail/info_parser_read.hpp>
#include <boost/property_tree/detail/info_parser_write.hpp>
#include <istream>

namespace boost { namespace property_tree { namespace info_parser
{

    // Read info from stream
    template<class Ptree>
    void read_info(std::basic_istream<typename Ptree::char_type> &stream, 
                   Ptree &pt)
    {
        Ptree local;
        read_info_internal(stream, local, std::string(), 0);
        pt.swap(local);
    }

    // Read info from file
    template<class Ptree>
    void read_info(const std::string &filename,
                   Ptree &pt,
                   const std::locale &loc = std::locale())
    {
        std::basic_ifstream<typename Ptree::char_type> stream(filename.c_str());
        if (!stream)
            throw info_parser_error("cannot open file for reading", filename, 0);
        stream.imbue(loc);
        Ptree local;
        read_info_internal(stream, local, filename, 0);
        pt.swap(local);
    }

    // Write info to stream
    template<class Ptree>
    void write_info(std::basic_ostream<typename Ptree::char_type> &stream, 
                    const Ptree &pt)
    {
        write_info_internal(stream, pt, std::string());
    }

    // Write info to file
    template<class Ptree>
    void write_info(const std::string &filename,
                    const Ptree &pt,
                    const std::locale &loc = std::locale())
    {
        std::basic_ofstream<typename Ptree::char_type> stream(filename.c_str());
        if (!stream)
            throw info_parser_error("cannot open file for writing", filename, 0);
        stream.imbue(loc);
        write_info_internal(stream, pt, filename);
    }

} } }

namespace boost { namespace property_tree
{
    using info_parser::info_parser_error;
    using info_parser::read_info;
    using info_parser::write_info;
} }

#endif
