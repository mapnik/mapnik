

/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifdef HAVE_LIBXML2
#error HAVE_LIBXML2 defined but compiling rapidxml_loader.cpp!
#endif

// mapnik
#include <mapnik/xml_loader.hpp>
#include <boost/property_tree/detail/xml_parser_read_rapidxml.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/algorithm/string/trim.hpp>

// stl
#include <iostream>
#include <fstream>

using namespace std;
namespace rapidxml = boost::property_tree::detail::rapidxml;
namespace mapnik
{
class rapidxml_loader : boost::noncopyable
{
public:
    rapidxml_loader(const char *encoding = NULL) :
        filename_()
    {

    }

    ~rapidxml_loader()
    {
    }

    void load(std::string const& filename, xml_node &node)
    {
        filename_ = filename;
        std::basic_ifstream<char> stream(filename.c_str());
        if (!stream)
        {
            throw config_error("Could not load map file", 0, filename);
        }
//        TODO: stream.imbue(loc);
        load(stream, node);
    }

    void load(std::basic_istream<char> &stream, xml_node &node)
    {
        stream.unsetf(std::ios::skipws);
        std::vector<char> v(std::istreambuf_iterator<char>(stream.rdbuf()),
                            std::istreambuf_iterator<char>());
        if (!stream.good())
        {
            throw config_error("Could not load map file", 0, filename_);
        }
        v.push_back(0); // zero-terminate
        try
        {
            // Parse using appropriate flags
            const int f_tws = rapidxml::parse_normalize_whitespace
                | rapidxml::parse_trim_whitespace;
            rapidxml::xml_document<> doc;
            doc.parse<f_tws>(&v.front());

            for (rapidxml::xml_node<char> *child = doc.first_node();
                 child; child = child->next_sibling())
            {
                populate_tree(child, node);
            }
        }
        catch (rapidxml::parse_error &e)
        {
            long line = static_cast<long>(
                std::count(&v.front(), e.where<char>(), '\n') + 1);
            throw config_error(e.what(), line, filename_);
        }
    }

    void load_string(std::string const& buffer, xml_node &node, std::string const & base_path )
    {

//        if (!base_path.empty())
//        {
//            boost::filesystem::path path(base_path);
//            if (!boost::filesystem::exists(path)) {
//                throw config_error(string("Could not locate base_path '") +
//                                   base_path + "': file or directory does not exist");
//            }
//        }


        load(buffer, node);
    }
private:
    void populate_tree(rapidxml::xml_node<char> *cur_node, xml_node &node)
    {
        switch (cur_node->type())
        {
        case rapidxml::node_element:
        {
            xml_node &new_node = node.add_child((char *)cur_node->name(), 0, false);
            // Copy attributes
            for (rapidxml::xml_attribute<char> *attr = cur_node->first_attribute();
                 attr; attr = attr->next_attribute())
            {
                new_node.add_attribute(attr->name(), attr->value());
            }

            // Copy children
            for (rapidxml::xml_node<char> *child = cur_node->first_node();
                 child; child = child->next_sibling())
            {
                populate_tree(child, new_node);
            }
        }
        break;

        // Data nodes
        case rapidxml::node_data:
        case rapidxml::node_cdata:
        {
            std::string trimmed(cur_node->value());
            boost::trim(trimmed);
            if (trimmed.empty()) break; //Don't add empty text nodes
            node.add_child(trimmed, 0, true);
        }
        break;
        default:
            break;
        }
    }
private:
    std::string filename_;
};

void read_xml(std::string const & filename, xml_node &node)
{
    rapidxml_loader loader;
    loader.load(filename, node);
}
void read_xml_string(std::string const & str, xml_node &node, std::string const & base_path)
{
    rapidxml_loader loader;
    loader.load_string(str, node, base_path);
}

} // end of namespace mapnik
