/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#include "catch.hpp"
#include "ds_test_util.hpp"

#include <mapnik/util/fs.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/json/topology.hpp>
#include <mapnik/json/topojson_grammar_x3.hpp>
#include <mapnik/json/topojson_utils.hpp>

namespace {

bool parse_topology(std::string const& filename, mapnik::topojson::topology & topo)
{
    mapnik::util::file file(filename);
    std::string buffer;
    buffer.resize(file.size());
    std::fread(&buffer[0], buffer.size(), 1, file.get());
    if (!file) return false;
    using space_type = boost::spirit::x3::standard::space_type;
    char const* itr = buffer.c_str();
    char const* end = itr + buffer.length();
    try
    {
        boost::spirit::x3::phrase_parse(itr, end, mapnik::json::topojson_grammar(), space_type() , topo);
    }
    catch (boost::spirit::x3::expectation_failure<char const*> const& ex)
    {
        std::cerr << "failed to parse TopoJSON..." << std::endl;
        std::cerr << ex.what() << std::endl;
        std::cerr << "Expected: " << ex.which();
        std::cerr << " Got: \"" << std::string(ex.where(), ex.where() + 200) << "...\"" << std::endl;
        return false;
    }
    return (itr == end);
}

}

TEST_CASE("topojson")
{
    SECTION("geometry parsing")
    {
        mapnik::value_integer feature_id = 0;
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        mapnik::transcoder tr("utf8");
        for (auto const& path : mapnik::util::list_directory("test/data/topojson/"))
        {
            mapnik::topojson::topology topo;
            REQUIRE(parse_topology(path, topo));
            for (auto const& geom : topo.geometries)
            {
                mapnik::box2d<double> bbox = mapnik::util::apply_visitor(mapnik::topojson::bounding_box_visitor(topo), geom);
                CHECK(bbox.valid());
                mapnik::topojson::feature_generator<mapnik::context_ptr> visitor(ctx, tr, topo, feature_id++);
                mapnik::feature_ptr feature = mapnik::util::apply_visitor(visitor, geom);
                CHECK(feature);
                CHECK(feature->envelope() == bbox);
            }
        }
    }

    SECTION("TopoJSON properties are properly expressed")
    {
        std::string filename("./test/data/topojson/escaped.topojson");
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        mapnik::transcoder tr("utf8");
        mapnik::topojson::topology topo;
        REQUIRE(parse_topology(filename, topo));
        mapnik::value_integer feature_id = 0;
        for (auto const& geom : topo.geometries)
        {
            mapnik::box2d<double> bbox = mapnik::util::apply_visitor(mapnik::topojson::bounding_box_visitor(topo), geom);
            CHECK(bbox.valid());
            mapnik::topojson::feature_generator<mapnik::context_ptr> visitor(ctx, tr, topo, feature_id);
            mapnik::feature_ptr feature = mapnik::util::apply_visitor(visitor, geom);
            CHECK(feature);
            CHECK(feature->envelope() == bbox);
            std::initializer_list<attr> attrs = {
                attr{"name", tr.transcode("Test")},
                attr{"NOM_FR", tr.transcode("Québec")},
                attr{"boolean", mapnik::value_bool(true)},
                attr{"description", tr.transcode("Test: \u005C")},
                attr{"double", mapnik::value_double(1.1)},
                attr{"int", mapnik::value_integer(1)},
                attr{"object", tr.transcode("{\"name\":\"waka\",\"spaces\":\"value with spaces\",\"int\":1,\"double\":1.1,\"boolean\":false"
                                                            ",\"NOM_FR\":\"Québec\",\"array\":[\"string\",\"value with spaces\",3,1.1,null,true"
                                                            ",\"Québec\"],\"another_object\":{\"name\":\"nested object\"}}")},
                attr{"spaces", tr.transcode("this has spaces")},
                attr{"array", tr.transcode("[\"string\",\"value with spaces\",3,1.1,null,true,"
                                                           "\"Québec\",{\"name\":\"object within an array\"},"
                                                           "[\"array\",\"within\",\"an\",\"array\"]]")},
                attr{"empty_array", tr.transcode("[]")},
                attr{"empty_object", tr.transcode("{}")},
            };
            REQUIRE_ATTRIBUTES(feature, attrs);
        }
    }

}
