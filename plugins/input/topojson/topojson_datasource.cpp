/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#include "topojson_datasource.hpp"
#include "topojson_featureset.hpp"

// boost
#include <boost/algorithm/string.hpp>

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/json/topojson_grammar_x3.hpp>
#include <mapnik/json/topojson_utils.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/util/file_io.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN_IMPL(topojson_datasource_plugin, topojson_datasource);
DATASOURCE_PLUGIN_EXPORT(topojson_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(topojson_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(topojson_datasource_plugin);

struct attr_value_converter
{
    mapnik::eAttributeType operator()(mapnik::value_integer /*val*/) const { return mapnik::Integer; }

    mapnik::eAttributeType operator()(double /*val*/) const { return mapnik::Double; }

    mapnik::eAttributeType operator()(float /*val*/) const { return mapnik::Double; }

    mapnik::eAttributeType operator()(bool /*val*/) const { return mapnik::Boolean; }
    // string, object, array
    template<typename T>
    mapnik::eAttributeType operator()(T const& /*val*/) const
    {
        return mapnik::String;
    }
};

struct geometry_type_visitor
{
    int operator()(mapnik::topojson::point const&) const
    {
        return static_cast<int>(mapnik::datasource_geometry_t::Point);
    }
    int operator()(mapnik::topojson::multi_point const&) const
    {
        return static_cast<int>(mapnik::datasource_geometry_t::Point);
    }
    int operator()(mapnik::topojson::linestring const&) const
    {
        return static_cast<int>(mapnik::datasource_geometry_t::LineString);
    }
    int operator()(mapnik::topojson::multi_linestring const&) const
    {
        return static_cast<int>(mapnik::datasource_geometry_t::LineString);
    }
    int operator()(mapnik::topojson::polygon const&) const
    {
        return static_cast<int>(mapnik::datasource_geometry_t::Polygon);
    }
    int operator()(mapnik::topojson::multi_polygon const&) const
    {
        return static_cast<int>(mapnik::datasource_geometry_t::Polygon);
    }
    template<typename T>
    int operator()(T const&) const
    {
        return 0;
    }
};

struct collect_attributes_visitor
{
    mapnik::layer_descriptor& desc_;
    collect_attributes_visitor(mapnik::layer_descriptor& desc)
        : desc_(desc)
    {}

    // no-op
    void operator()(mapnik::topojson::empty) {}
    //
    template<typename GeomType>
    void operator()(GeomType const& g)
    {
        if (g.props)
        {
            for (auto const& p : *g.props)
            {
                desc_.add_descriptor(
                  mapnik::attribute_descriptor(std::get<0>(p),
                                               mapnik::util::apply_visitor(attr_value_converter(), std::get<1>(p))));
            }
        }
    }
};

topojson_datasource::topojson_datasource(parameters const& params)
    : datasource(params),
      type_(datasource::Vector),
      desc_(topojson_datasource::name(), *params.get<std::string>("encoding", "utf-8")),
      filename_(),
      inline_string_(),
      extent_(),
      tr_(new mapnik::transcoder(*params.get<std::string>("encoding", "utf-8"))),
      tree_(nullptr)
{
    auto const inline_string = params.get<std::string>("inline");
    if (inline_string)
    {
        inline_string_ = *inline_string;
    }
    else
    {
        auto const file = params.get<std::string>("file");
        if (!file)
            throw mapnik::datasource_exception("TopoJSON Plugin: missing <file> parameter");

        auto const base = params.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;
    }
    if (!inline_string_.empty())
    {
        parse_topojson(inline_string_.c_str());
    }
    else
    {
        mapnik::util::file file(filename_);
        if (!file)
        {
            throw mapnik::datasource_exception("TopoJSON Plugin: could not open: '" + filename_ + "'");
        }
        std::string file_buffer;
        file_buffer.resize(file.size());
        auto count = std::fread(&file_buffer[0], file.size(), 1, file.get());
        if (count == 1)
            parse_topojson(file_buffer.c_str());
    }
}

template<typename T>
void topojson_datasource::parse_topojson(T const& buffer)
{
    auto itr = buffer;
    auto end = buffer + std::strlen(buffer);
    using space_type = boost::spirit::x3::standard::space_type;
    try
    {
        boost::spirit::x3::phrase_parse(itr, end, mapnik::json::grammar::topology, space_type(), topo_);
    }
    catch (boost::spirit::x3::expectation_failure<char const*> const& ex)
    {
        std::clog << "failed to parse TopoJSON..." << std::endl;
        std::clog << ex.what() << std::endl;
        std::clog << "Expected: " << ex.which();
        std::clog << " Got: \"" << std::string(ex.where(), ex.where() + 200) << "...\"" << std::endl;
        throw mapnik::datasource_exception("topojson_datasource: Failed parse TopoJSON file '" + filename_ + "'");
    }

    using values_container = std::vector<std::pair<box_type, std::size_t>>;
    values_container values;
    values.reserve(topo_.geometries.size());

    std::size_t geometry_index = 0;
    bool first = true;
    for (auto const& geom : topo_.geometries)
    {
        mapnik::box2d<double> box = mapnik::util::apply_visitor(mapnik::topojson::bounding_box_visitor(topo_), geom);
        if (box.valid())
        {
            if (first)
            {
                first = false;
                extent_ = box;
                collect_attributes_visitor assessor(desc_);
                mapnik::util::apply_visitor(std::ref(assessor), geom);
            }
            else
            {
                extent_.expand_to_include(box);
            }
            values.emplace_back(box, geometry_index);
        }
        ++geometry_index;
    }

    // packing algorithm
    tree_ = std::make_unique<spatial_index_type>(values);
}

topojson_datasource::~topojson_datasource() {}

char const* topojson_datasource::name()
{
    return "topojson";
}

std::optional<mapnik::datasource_geometry_t> topojson_datasource::get_geometry_type() const
{
    std::optional<mapnik::datasource_geometry_t> result;
    int multi_type = 0;
    std::size_t num_features = topo_.geometries.size();
    for (std::size_t i = 0; i < num_features && i < 5; ++i)
    {
        mapnik::topojson::geometry const& geom = topo_.geometries[i];
        int type = mapnik::util::apply_visitor(geometry_type_visitor(), geom);
        if (type > 0)
        {
            if (multi_type > 0 && multi_type != type)
            {
                result = mapnik::datasource_geometry_t::Collection;
                return result;
            }
            else
            {
                result = static_cast<mapnik::datasource_geometry_t>(type);
            }
            multi_type = type;
        }
    }
    return result;
}

mapnik::datasource::datasource_t topojson_datasource::type() const
{
    return type_;
}

mapnik::box2d<double> topojson_datasource::envelope() const
{
    return extent_;
}

mapnik::layer_descriptor topojson_datasource::get_descriptor() const
{
    return desc_;
}

mapnik::featureset_ptr topojson_datasource::features(mapnik::query const& q) const
{
    // if the query box intersects our world extent then query for features
    mapnik::box2d<double> const& box = q.get_bbox();
    if (extent_.intersects(box))
    {
        topojson_featureset::array_type index_array;
        if (tree_)
        {
            tree_->query(boost::geometry::index::intersects(box), std::back_inserter(index_array));
            return std::make_shared<topojson_featureset>(topo_, *tr_, std::move(index_array));
        }
    }
    // otherwise return an empty featureset pointer
    return mapnik::make_empty_featureset();
}

mapnik::featureset_ptr topojson_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    mapnik::box2d<double> query_bbox(pt, pt);
    query_bbox.pad(tol);
    mapnik::query q(query_bbox);
    for (auto const& attr_info : desc_.get_descriptors())
    {
        q.add_property_name(attr_info.get_name());
    }
    return features(q);
}
