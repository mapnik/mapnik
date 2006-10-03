// ----------------------------------------------------------------------------
// Copyright (C) 2002-2005 Marcin Kalicinski
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_INI_PARSER_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_INI_PARSER_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/ptree_utils.hpp>
#include <boost/property_tree/detail/file_parser_error.hpp>
#include <istream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <locale>

namespace boost { namespace property_tree { namespace ini_parser
{

    static const int skip_ini_validity_check = 1;     // Skip check if ptree is a valid ini

    inline bool validate_flags(int flags)
    {
        return (flags & ~skip_ini_validity_check) == 0;
    }

    //! Ini parser error
    class ini_parser_error: public file_parser_error
    {
    public:
        ini_parser_error(const std::string &message, 
                         const std::string &filename, 
                         unsigned long line): 
            file_parser_error(message, filename, line)
        { 
        }
    };

    //! Read ini from stream
    template<class Ptree>
    void read_ini(std::basic_istream<typename Ptree::char_type> &stream, 
                  Ptree &pt)
    {

        typedef typename Ptree::char_type Ch;
        typedef std::basic_string<Ch> Str;

        Ptree local;
        unsigned long line_no = 0;
        Ptree *section = 0;
        Str line;

        // For all lines
        while (stream.good())
        {

            // Get line from stream
            ++line_no;
            std::getline(stream, line);
            if (!stream.good() && !stream.eof())
                throw ini_parser_error("read error", "", line_no);

            // If line is non-empty
            line = detail::trim(line, stream.getloc());
            if (!line.empty())
            {
            
                // Comment, section or key?
                if (line[0] == Ch(';'))
                {
                    // Ignore comments
                }
                else if (line[0] == Ch('['))
                {
                    typename Str::size_type end = line.find(Ch(']'));
                    if (end == Str::npos)
                        throw ini_parser_error("unmatched '['", "", line_no);
                    Str key = detail::trim(line.substr(1, end - 1), stream.getloc());
                    if (local.find(key) != local.end())
                        throw ini_parser_error("duplicate section name", "", line_no);
                    section = &local.push_back(std::make_pair(key, Ptree()))->second;
                }
                else
                {
                    if (!section)
                        throw ini_parser_error("section expected", "", line_no);
                    typename Str::size_type eqpos = line.find(Ch('='));
                    if (eqpos == Str::npos)
                        throw ini_parser_error("'=' character not found in line", "", line_no);
                    if (eqpos == 0)
                        throw ini_parser_error("key expected", "", line_no);
                    Str key = detail::trim(line.substr(0, eqpos), stream.getloc());
                    Str data = detail::trim(line.substr(eqpos + 1, Str::npos), stream.getloc());
                    if (section->find(key) != section->end())
                        throw ini_parser_error("duplicate key name", "", line_no);
                    section->push_back(std::make_pair(key, Ptree(data)));
                }
            }
        }

        // Swap local ptree with result ptree
        pt.swap(local);

    }

    //! Read ini from file
    template<class Ptree>
    void read_ini(const std::string &filename, 
                  Ptree &pt,
                  const std::locale &loc = std::locale())
    {
        std::basic_ifstream<typename Ptree::char_type> stream(filename.c_str());
        if (!stream)
            throw ini_parser_error("cannot open file", filename, 0);
        stream.imbue(loc);
        try {
            read_ini(stream, pt);
        }
        catch (ini_parser_error &e) {
            throw ini_parser_error(e.message(), filename, e.line());
        }
    }

    //! Write ini to stream
    template<class Ptree>
    void write_ini(std::basic_ostream<typename Ptree::char_type> &stream, 
                   const Ptree &pt,
                   int flags = 0)
    {

        typedef typename Ptree::char_type Ch;
        typedef std::basic_string<Ch> Str;

        BOOST_ASSERT(validate_flags(flags));
        
        // Verify if ptree is not too rich to be saved as ini
        if (!(flags & skip_ini_validity_check))
            for (typename Ptree::const_iterator it = pt.begin(), end = pt.end(); it != end; ++it)
            {
                if (!it->second.data().empty())
                    throw ini_parser_error("ptree has data on root level keys", "", 0);
                if (pt.count(it->first) > 1)
                    throw ini_parser_error("duplicate section name", "", 0);
                for (typename Ptree::const_iterator it2 = it->second.begin(), end2 = it->second.end(); it2 != end2; ++it2)
                {
                    if (!it2->second.empty())
                        throw ini_parser_error("ptree is too deep", "", 0);
                    if (it->second.count(it2->first) > 1)
                        throw ini_parser_error("duplicate key name", "", 0);
                }
            }

        // Write ini
        for (typename Ptree::const_iterator it = pt.begin(), end = pt.end(); it != end; ++it)
        {
            stream << Ch('[') << it->first << Ch(']') << Ch('\n');
            for (typename Ptree::const_iterator it2 = it->second.begin(), end2 = it->second.end(); it2 != end2; ++it2)
                stream << it2->first << Ch('=') << it2->second.template get_own<std::basic_string<Ch> >() << Ch('\n');
        }

    }

    // Write ini to file
    template<class Ptree>
    void write_ini(const std::string &filename,
                   const Ptree &pt,
                   int flags = 0,
                   const std::locale &loc = std::locale())
    {
        std::basic_ofstream<typename Ptree::char_type> stream(filename.c_str());
        if (!stream)
            throw ini_parser_error("cannot open file", filename, 0);
        stream.imbue(loc);
        try {
            write_ini(stream, pt, flags);
        }
        catch (ini_parser_error &e) {
            throw ini_parser_error(e.message(), filename, e.line());
        }
    }

} } }

namespace boost { namespace property_tree
{
    using ini_parser::ini_parser_error;
    using ini_parser::read_ini;
    using ini_parser::write_ini;
} }

#endif
