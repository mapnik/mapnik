/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#include <mapnik/map.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/geometry_types.hpp>
#include <mapnik/geometry/geometry_type.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/util/from_u8string.hpp>
#include <mapnik/util/fs.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional_io.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
MAPNIK_DISABLE_WARNING_POP

#include <iostream>


namespace {

bool is_csv(std::string const& filename)
{
    return boost::iends_with(filename,".csv")
        || boost::iends_with(filename,".tsv");
}

void add_csv_files(std::string dir, std::vector<std::string> &csv_files)
{
    for (auto const& path : mapnik::util::list_directory(dir))
    {
        if (is_csv(path))
        {
            csv_files.emplace_back(path);
        }
    }
}

mapnik::datasource_ptr get_csv_ds(std::string const& file_name, bool strict = true, std::string const& base="")
{
    mapnik::parameters params;
    params["type"] = std::string("csv");
    params["file"] = file_name;
    if (!base.empty())
    {
        params["base"] = base;
    }
    params["strict"] = mapnik::value_bool(strict);
    auto ds = mapnik::datasource_cache::instance().create(params);
    // require a non-null pointer returned
    REQUIRE(ds != nullptr);
    return ds;
}

} // anonymous namespace

TEST_CASE("csv") {
    using mapnik::util::from_u8string;
    std::string csv_plugin("./plugins/input/csv.input");
    if (mapnik::util::exists(csv_plugin))
    {
        // make the tests silent since we intentionally test error conditions that are noisy
        auto const severity = mapnik::logger::instance().get_severity();
        mapnik::logger::instance().set_severity(mapnik::logger::none);

        // check the CSV datasource is loaded
        const std::vector<std::string> plugin_names =
            mapnik::datasource_cache::instance().plugin_names();
        const bool have_csv_plugin =
            std::find(plugin_names.begin(), plugin_names.end(), "csv") != plugin_names.end();

        SECTION("CSV I/O errors")
        {
            std::string filename = "does_not_exist.csv";
            for (auto create_index : { true, false })
            {
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    // index wont be created
                    CHECK(!mapnik::util::exists(filename + ".index"));
                }
                mapnik::parameters params;
                params["type"] = "csv";
                params["file"] = filename;
                REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
                params["base"] = "";
                REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
                params["base"] = "/";
                REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));
            }
        }

        SECTION("broken files")
        {
            for (auto create_index : { false, true })
            {
                if (have_csv_plugin)
                {
                    std::vector<std::string> broken;
                    add_csv_files("test/data/csv/fails", broken);
                    add_csv_files("test/data/csv/warns", broken);
                    broken.emplace_back("test/data/csv/fails/does_not_exist.csv");

                    for (auto const& path : broken)
                    {
                        bool require_fail = true;
                        if (create_index)
                        {
                            int ret = create_disk_index(path);
                            int ret_posix = (ret >> 8) & 0x000000ff;
                            INFO(ret);
                            INFO(ret_posix);
                            require_fail = (boost::iends_with(path,"feature_id_counting.csv")) ? false : true;
                            if (!require_fail)
                            {
                                REQUIRE(mapnik::util::exists(path + ".index"));
                            }
                        }
                        INFO(path);
                        if (require_fail)
                        {
                            REQUIRE_THROWS(get_csv_ds(path));
                        }
                        else
                        {
                            CHECK(bool(get_csv_ds(path)));
                        }
                        if (mapnik::util::exists(path + ".index"))
                        {
                            CHECK(mapnik::util::remove(path + ".index"));
                        }
                    }
                }
            }
        } // END SECTION

        SECTION("good files")
        {
            if (have_csv_plugin)
            {
                std::vector<std::string> good;
                add_csv_files("test/data/csv", good);
                add_csv_files("test/data/csv/warns", good);

                for (auto const& path : good)
                {
                    // cleanup in the case of a failed previous run
                    if (mapnik::util::exists(path + ".index"))
                    {
                        mapnik::util::remove(path + ".index");
                    }
                    for (auto create_index : { false, true })
                    {
                        if (create_index)
                        {
                            int ret = create_disk_index(path);
                            int ret_posix = (ret >> 8) & 0x000000ff;
                            INFO(ret);
                            INFO(ret_posix);
                            if (!boost::iends_with(path,"more_headers_than_column_values.csv")) // mapnik-index won't create *.index for 0 features
                            {
                                CHECK(mapnik::util::exists(path + ".index"));
                            }
                        }
                        auto ds = get_csv_ds(path, false);
                        // require a non-null pointer returned
                        REQUIRE(bool(ds));
                        if (mapnik::util::exists(path + ".index"))
                        {
                            CHECK(mapnik::util::remove(path + ".index"));
                        }
                    }
                }
            }
        } // END SECTION

        SECTION("lon/lat detection")
        {
            for (auto create_index : { false, true })
            {
                for (auto const& lon_name : {std::string("lon"), std::string("lng")})
                {
                    std::string filename = (boost::format("test/data/csv/%1%_lat.csv") % lon_name).str();
                    // cleanup in the case of a failed previous run
                    if (mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                    if (create_index)
                    {
                        int ret = create_disk_index(filename);
                        int ret_posix = (ret >> 8) & 0x000000ff;
                        INFO(ret);
                        INFO(ret_posix);
                        CHECK(mapnik::util::exists(filename + ".index"));
                    }
                    auto ds = get_csv_ds(filename);
                    auto fields = ds->get_descriptor().get_descriptors();
                    require_field_names(fields, {lon_name, "lat"});
                    require_field_types(fields, {mapnik::Integer, mapnik::Integer});

                    CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);

                    mapnik::query query(ds->envelope());
                    for (auto const &field : fields)
                    {
                        query.add_property_name(field.get_name());
                    }
                    auto features = ds->features(query);
                    auto feature = features->next();

                    REQUIRE_ATTRIBUTES(feature, {
                            attr { lon_name, mapnik::value_integer(0) },
                                attr { "lat", mapnik::value_integer(0) }
                        });
                    if (mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                }
            }
        } // END SECTION

        SECTION("type detection")
        {
            for (auto create_index : { false, true })
            {
                std::string base = "test/data/csv/";
                std::string filename = "nypd.csv";
                std::string filepath = base + filename;
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filepath + ".index"))
                {
                    mapnik::util::remove(filepath + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filepath);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filepath + ".index"));
                }
                auto ds = get_csv_ds(filename,true,base);
                CHECK(ds->type() == mapnik::datasource::datasource_t::Vector);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"Precinct", "Phone", "Address", "City", "geo_longitude", "geo_latitude", "geo_accuracy"});
                require_field_types(fields, {mapnik::String, mapnik::String, mapnik::String, mapnik::String, mapnik::Double, mapnik::Double, mapnik::String});

                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                CHECK(count_features(all_features(ds)) == 2);

                auto fs = all_features(ds);
                auto fs2 = ds->features_at_point(ds->envelope().center(),10000);
                REQUIRE(fs != nullptr);
                auto feature = fs->next();
                auto feature2 = fs2->next();
                REQUIRE(feature != nullptr);
                REQUIRE(feature2 != nullptr);
                auto expected_attr = {
                        attr { "City", mapnik::value_unicode_string("New York, NY") }
                        , attr { "geo_accuracy", mapnik::value_unicode_string("house") }
                        , attr { "Phone", mapnik::value_unicode_string("(212) 334-0711") }
                        , attr { "Address", mapnik::value_unicode_string("19 Elizabeth Street") }
                        , attr { "Precinct", mapnik::value_unicode_string("5th Precinct") }
                        , attr { "geo_longitude", mapnik::value_double(-70.0) }
                        , attr { "geo_latitude", mapnik::value_double(40.0) }
                    };
                REQUIRE_ATTRIBUTES(feature, expected_attr);
                REQUIRE_ATTRIBUTES(feature2, expected_attr);
                if (mapnik::util::exists(filepath + ".index"))
                {
                    mapnik::util::remove(filepath + ".index");
                }
            }
        } // END SECTION

        SECTION("skipping blank rows")
        {
            for (auto create_index : { false, true })
            {
                std::string filename = "test/data/csv/blank_rows.csv";
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "name"});
                require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String});
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                CHECK(count_features(all_features(ds)) == 2);
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("empty rows")
        {
            for (auto create_index : { false, true })
            {
                std::string filename = "test/data/csv/empty_rows.csv";
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);

                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "text", "date", "integer", "boolean", "float", "time", "datetime", "empty_column"});
                require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String, mapnik::String,
                            mapnik::Integer, mapnik::Boolean, mapnik::Double, mapnik::String, mapnik::String, mapnik::String});
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
                CHECK(count_features(all_features(ds)) == 4);

                auto featureset = all_features(ds);
                auto feature = featureset->next();
                REQUIRE_ATTRIBUTES(feature, {
                        attr { "x", mapnik::value_integer(0) }
                        , attr { "empty_column", mapnik::value_unicode_string("") }
                        , attr { "text", mapnik::value_unicode_string("a b") }
                        , attr { "float", mapnik::value_double(1.0) }
                        , attr { "datetime", mapnik::value_unicode_string("1971-01-01T04:14:00") }
                        , attr { "y", mapnik::value_integer(0) }
                        , attr { "boolean", mapnik::value_bool(true) }
                        , attr { "time", mapnik::value_unicode_string("04:14:00") }
                        , attr { "date", mapnik::value_unicode_string("1971-01-01") }
                        , attr { "integer", mapnik::value_integer(40) }
                    });

                while (bool(feature = featureset->next())) {
                    CHECK(feature->size() == 10);
                    CHECK(feature->get("empty_column") == mapnik::value_unicode_string(""));
                }
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("slashes")
        {
            for (auto create_index : { false, true })
            {
                std::string filename = "test/data/csv/has_attributes_with_slashes.csv";
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "name"});
                // NOTE: y column is integer, even though a double value is used below in the test?
                require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String});

                auto featureset = all_features(ds);
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 0}
                        , attr{"y", 0}
                        , attr{"name", mapnik::value_unicode_string("a/a") } });
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 1}
                        , attr{"y", 4}
                        , attr{"name", mapnik::value_unicode_string("b/b") } });
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 10}
                        , attr{"y", 2.5}
                        , attr{"name", mapnik::value_unicode_string("c/c") } });
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("wkt field")
        {
            for (auto create_index : { false, true })
            {
                std::string filename = "test/data/csv/wkt.csv";
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                using mapnik::geometry::geometry_types;
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"type"});
                require_field_types(fields, {mapnik::String});

                auto featureset = all_features(ds);
                require_geometry(featureset->next(), 1, geometry_types::Point);
                require_geometry(featureset->next(), 1, geometry_types::LineString);
                require_geometry(featureset->next(), 1, geometry_types::Polygon);
                require_geometry(featureset->next(), 1, geometry_types::Polygon);
                require_geometry(featureset->next(), 4, geometry_types::MultiPoint);
                require_geometry(featureset->next(), 2, geometry_types::MultiLineString);
                require_geometry(featureset->next(), 2, geometry_types::MultiPolygon);
                require_geometry(featureset->next(), 2, geometry_types::MultiPolygon);
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("handling of missing header")
        {
            for (auto create_index : { false, true })
            {
                std::string filename = "test/data/csv/missing_header.csv";
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                // TODO: does this mean 'missing_header.csv' should be in the warnings
                // subdirectory, since it doesn't work in strict mode?
                auto ds = get_csv_ds(filename, false);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"one", "two", "x", "y", "_4", "aftermissing"});
                auto feature = all_features(ds)->next();
                REQUIRE(feature);
                REQUIRE(feature->has_key("_4"));
                CHECK(feature->get("_4") == mapnik::value_unicode_string("missing"));
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("handling of headers that are numbers")
        {
            for (auto create_index : { false, true })
            {
                std::string filename = "test/data/csv/numbers_for_headers.csv";
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "1990", "1991", "1992"});
                auto feature = all_features(ds)->next();
                REQUIRE_ATTRIBUTES(feature, {
                        attr{"x", 0}
                        , attr{"y", 0}
                        , attr{"1990", 1}
                        , attr{"1991", 2}
                        , attr{"1992", 3}
                    });
                auto expression = mapnik::parse_expression("[1991]=2");
                REQUIRE(bool(expression));
                auto value = mapnik::util::apply_visitor(
                    mapnik::evaluate<mapnik::feature_impl, mapnik::value_type, mapnik::attributes>(
                        *feature, mapnik::attributes()), *expression);
                CHECK(value == true);
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("quoted numbers")
        {
            using ustring = mapnik::value_unicode_string;
            for (auto create_index : { false, true })
            {
                std::string filename = "test/data/csv/quoted_numbers.csv";
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "label"});
                auto featureset = all_features(ds);

                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 0}, attr{"y", 0}, attr{"label", ustring("0,0") } });
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 5}, attr{"y", 5}, attr{"label", ustring("5,5") } });
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 0}, attr{"y", 5}, attr{"label", ustring("0,5") } });
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 5}, attr{"y", 0}, attr{"label", ustring("5,0") } });
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 2.5}, attr{"y", 2.5}, attr{"label", ustring("2.5,2.5") } });
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("reading newlines")
        {
            for (auto create_index : { false, true })
            {
                for (auto const& platform : {std::string("windows"), std::string("mac")})
                {
                    std::string filename = (boost::format("test/data/csv/%1%_newlines.csv") % platform).str();
                    // cleanup in the case of a failed previous run
                    if (mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                    if (create_index)
                    {
                        int ret = create_disk_index(filename);
                        int ret_posix = (ret >> 8) & 0x000000ff;
                        INFO(ret);
                        INFO(ret_posix);
                        CHECK(mapnik::util::exists(filename + ".index"));
                    }
                    auto ds = get_csv_ds(filename);
                    auto fields = ds->get_descriptor().get_descriptors();
                    require_field_names(fields, {"x", "y", "z"});
                    REQUIRE_ATTRIBUTES(all_features(ds)->next(), {
                            attr{"x", 1}, attr{"y", 10}, attr{"z", 9999.9999} });
                    if (mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                }
            }
        } // END SECTION

        SECTION("mixed newlines")
        {
            using ustring = mapnik::value_unicode_string;
            for (auto create_index : { false, true })
            {
                for (auto const& filename : {
                        std::string("test/data/csv/mac_newlines_with_unix_inline.csv")
                            , std::string("test/data/csv/mac_newlines_with_unix_inline_escaped.csv")
                            , std::string("test/data/csv/windows_newlines_with_unix_inline.csv")
                            , std::string("test/data/csv/windows_newlines_with_unix_inline_escaped.csv")
                            })
                {
                    // cleanup in the case of a failed previous run
                    if (mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                    if (create_index)
                    {
                        int ret = create_disk_index(filename);
                        int ret_posix = (ret >> 8) & 0x000000ff;
                        INFO(ret);
                        INFO(ret_posix);
                        CHECK(mapnik::util::exists(filename + ".index"));
                    }
                    auto ds = get_csv_ds(filename);
                    auto fields = ds->get_descriptor().get_descriptors();
                    require_field_names(fields, {"x", "y", "line"});
                    REQUIRE_ATTRIBUTES(all_features(ds)->next(), {
                            attr{"x", 0}, attr{"y", 0}
                            , attr{"line", ustring("many\n  lines\n  of text\n  with unix newlines")} });
                    if (mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                }
            }
        } // END SECTION

        SECTION("tabs")
        {
            for (auto create_index : { false, true })
            {
                std::string filename = "test/data/csv/tabs_in_csv.csv";
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "z"});
                REQUIRE_ATTRIBUTES(all_features(ds)->next(), {
                        attr{"x", -122}, attr{"y", 48}, attr{"z", 0} });
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("separators")
        {
            using ustring = mapnik::value_unicode_string;
            for (auto const& filename : {
                    std::string("test/data/csv/pipe_delimiters.csv")
                        , std::string("test/data/csv/semicolon_delimiters.csv")
                        })
            {
                for (auto create_index : { false, true })
                {
                    // cleanup in the case of a failed previous run
                    if (mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                    if (create_index)
                    {
                        int ret = create_disk_index(filename);
                        int ret_posix = (ret >> 8) & 0x000000ff;
                        INFO(ret);
                        INFO(ret_posix);
                        CHECK(mapnik::util::exists(filename + ".index"));
                    }
                    auto ds = get_csv_ds(filename);
                    auto fields = ds->get_descriptor().get_descriptors();
                    require_field_names(fields, {"x", "y", "z"});
                    REQUIRE_ATTRIBUTES(all_features(ds)->next(), {
                            attr{"x", 0}, attr{"y", 0}, attr{"z", ustring("hello")} });
                    if (mapnik::util::exists(filename + ".index"))
                    {
                        mapnik::util::remove(filename + ".index");
                    }
                }
            }
        } // END SECTION

        SECTION("null and bool keywords are empty strings")
        {
            using ustring = mapnik::value_unicode_string;
            std::string filename = "test/data/csv/nulls_and_booleans_as_strings.csv";
            for (auto create_index : { false, true })
            {
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "null", "boolean"});
                require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String, mapnik::Boolean});

                auto featureset = all_features(ds);
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 0}, attr{"y", 0}, attr{"null", ustring("null")}, attr{"boolean", true}});
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 0}, attr{"y", 0}, attr{"null", ustring("")}, attr{"boolean", false}});

                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("nonexistent query fields throw")
        {
            std::string filename = "test/data/csv/lon_lat.csv";
            for (auto create_index : { false, true })
            {
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"lon", "lat"});
                require_field_types(fields, {mapnik::Integer, mapnik::Integer});

                mapnik::query query(ds->envelope());
                for (auto const &field : fields)
                {
                    query.add_property_name(field.get_name());
                }
                // also add an invalid one, triggering throw
                query.add_property_name("bogus");
                REQUIRE_THROWS(ds->features(query));
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("leading zeros mean strings")
        {
            using ustring = mapnik::value_unicode_string;
            std::string filename = "test/data/csv/leading_zeros.csv";
            for (auto create_index : { false, true })
            {
                // cleanup in the case of a failed previous run
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
                if (create_index)
                {
                    int ret = create_disk_index(filename);
                    int ret_posix = (ret >> 8) & 0x000000ff;
                    INFO(ret);
                    INFO(ret_posix);
                    CHECK(mapnik::util::exists(filename + ".index"));
                }
                auto ds = get_csv_ds(filename);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "fips"});
                require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String});

                auto featureset = all_features(ds);
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 0}, attr{"y", 0}, attr{"fips", ustring("001")}});
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 0}, attr{"y", 0}, attr{"fips", ustring("003")}});
                REQUIRE_ATTRIBUTES(featureset->next(), {
                        attr{"x", 0}, attr{"y", 0}, attr{"fips", ustring("005")}});
                if (mapnik::util::exists(filename + ".index"))
                {
                    mapnik::util::remove(filename + ".index");
                }
            }
        } // END SECTION

        SECTION("advanced geometry detection")
        {
            using row = std::pair<std::string, mapnik::datasource_geometry_t>;
            for (row r : {
                    row{"point", mapnik::datasource_geometry_t::Point}
                    , row{"poly", mapnik::datasource_geometry_t::Polygon}
                    , row{"multi_poly", mapnik::datasource_geometry_t::Polygon}
                    , row{"line", mapnik::datasource_geometry_t::LineString}
                }) {
                std::string file_name = (boost::format("test/data/csv/%1%_wkt.csv") % r.first).str();
                auto ds = get_csv_ds(file_name);
                CHECK(ds->get_geometry_type() == r.second);
            }
        } // END SECTION

        SECTION("creation of CSV from in-memory strings")
        {
            using ustring = mapnik::value_unicode_string;

            for (auto const &name : {std::string("Winthrop, WA"), from_u8string(u8"Qu\u00e9bec")}) {
                std::string csv_string =
                    (boost::format(
                        "wkt,Name\n"
                        "\"POINT (120.15 48.47)\",\"%1%\"\n"
                        ) % name).str();

                mapnik::parameters params;
                params["type"] = std::string("csv");
                params["inline"] = csv_string;
                auto ds = mapnik::datasource_cache::instance().create(params);
                REQUIRE(bool(ds));

                auto feature = all_features(ds)->next();
                REQUIRE(bool(feature));
                REQUIRE(feature->has_key("Name"));
                std::string utf8;
                mapnik::transcoder tr("utf-8");
                ustring expected_string = tr.transcode(name.c_str());
                mapnik::value val(expected_string);
                mapnik::to_utf8(expected_string,utf8);
                INFO(feature->get("Name"));
                INFO(utf8);
                INFO(val);
                CHECK(feature->get("Name") == val);
            }
        } // END SECTION

        SECTION("creation of CSV from in-memory strings with bogus headers")
        {
            mapnik::parameters params;
            params["type"] = std::string("csv");

            // should throw
            params["inline"] = "latitude, longtitude, Name\n" // misspellt (!)
                "120.15,48.47,Winhrop";
            REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));

            // should throw
            params["strict"] = true;
            params["inline"] = "latitude, longitude\n" // -- missing header
                "120.15,48.47,Winhrop";
            REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));

            // should not throw
            params["strict"] = false;
            params["inline"] = "latitude, longitude,Name\n"
                "0,0,Unknown, extra bogus field\n"
                "120.15,48.47,Winhrop\n";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(bool(ds));
            REQUIRE(ds->envelope() == mapnik::box2d<double>(48.47,120.15,48.47,120.15));
            auto feature = all_features(ds)->next();
            REQUIRE(bool(feature));
            REQUIRE(feature->has_key("Name"));

            // should throw
            params["strict"] = false;
            params["inline"] = "x, Name\n" // -- missing required *geometry* header
                "120.15,Winhrop";
            REQUIRE_THROWS(mapnik::datasource_cache::instance().create(params));

        } // END SECTION

        SECTION("geojson quoting") {
            using mapnik::geometry::geometry_types;

            for (auto const &file : {
                    std::string("test/data/csv/geojson_double_quote_escape.csv")
                        , std::string("test/data/csv/geojson_single_quote.csv")
                        , std::string("test/data/csv/geojson_2x_double_quote_filebakery_style.csv")
                        }) {
                auto ds = get_csv_ds(file);
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"type"});
                require_field_types(fields, {mapnik::String});

                auto featureset = all_features(ds);
                require_geometry(featureset->next(), 1, geometry_types::Point);
                require_geometry(featureset->next(), 1, geometry_types::LineString);
                require_geometry(featureset->next(), 1, geometry_types::Polygon);
                require_geometry(featureset->next(), 1, geometry_types::Polygon);
                require_geometry(featureset->next(), 4, geometry_types::MultiPoint);
                require_geometry(featureset->next(), 2, geometry_types::MultiLineString);
                require_geometry(featureset->next(), 2, geometry_types::MultiPolygon);
                require_geometry(featureset->next(), 2, geometry_types::MultiPolygon);
            }
        } // END SECTION

        SECTION("fewer headers than rows throws") {
            REQUIRE_THROWS(get_csv_ds("test/data/csv/more_column_values_than_headers.csv"));
        } // END SECTION

        SECTION("feature ID only incremented for valid rows") {
            auto ds = get_csv_ds("test/data/csv/warns/feature_id_counting.csv", false);
            auto fs = all_features(ds);

            // first
            auto feature = fs->next();
            REQUIRE(bool(feature));
            CHECK(feature->id() == 1);

            // second, should have skipped bogus one
            feature = fs->next();
            REQUIRE(bool(feature));
            CHECK(feature->id() == 2);

            feature = fs->next();
            CHECK(!feature);
        } // END SECTION

        SECTION("dynamically defining headers") {
            using ustring = mapnik::value_unicode_string;
            using row = std::pair<std::string, std::size_t>;

            for (auto const& r : {
                    row{"test/data/csv/fails/needs_headers_two_lines.csv", 2},
                        row{"test/data/csv/fails/needs_headers_one_line.csv", 1},
                            row{"test/data/csv/fails/needs_headers_one_line_no_newline.csv", 1}})
            {
                mapnik::parameters params;
                params["type"] = std::string("csv");
                params["file"] = r.first;
                params["headers"] = "x,y,name";
                auto ds = mapnik::datasource_cache::instance().create(params);
                REQUIRE(bool(ds));
                auto fields = ds->get_descriptor().get_descriptors();
                require_field_names(fields, {"x", "y", "name"});
                require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String});
                REQUIRE_ATTRIBUTES(all_features(ds)->next(), {
                        attr{"x", 0}, attr{"y", 0}, attr{"name", ustring("data_name")} });
                REQUIRE(count_features(all_features(ds)) == r.second);
                CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
            }
        } // END SECTION


MAPNIK_DISABLE_WARNING_PUSH
MAPNIK_DISABLE_LONG_LONG
        SECTION("64bit int fields work") {
            auto ds = get_csv_ds("test/data/csv/64bit_int.csv");
            auto fields = ds->get_descriptor().get_descriptors();
            require_field_names(fields, {"x", "y", "bigint"});
            require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::Integer});

            auto fs = all_features(ds);
            auto feature = fs->next();
            REQUIRE_ATTRIBUTES(feature, {
                    attr{"x", 0}, attr{"y", 0}, attr{"bigint", 2147483648} });

            feature = fs->next();
            REQUIRE_ATTRIBUTES(feature, {
                    attr{"x", 0}, attr{"y", 0}, attr{"bigint", 9223372036854775807ll} });
            REQUIRE_ATTRIBUTES(feature, {
                    attr{"x", 0}, attr{"y", 0}, attr{"bigint", 0x7FFFFFFFFFFFFFFFll} });
        } // END SECTION
MAPNIK_DISABLE_WARNING_POP

        SECTION("various number types") {
            auto ds = get_csv_ds("test/data/csv/number_types.csv");
            auto fields = ds->get_descriptor().get_descriptors();
            require_field_names(fields, {"x", "y", "floats"});
            require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::Double});
            auto fs = all_features(ds);
            for (double d : { .0, +.0, 1e-06, -1e-06, 0.000001, 1.234e+16, 1.234e+16 }) {
                auto feature = fs->next();
                REQUIRE(bool(feature));
                CHECK(feature->get("floats").get<mapnik::value_double>() == Approx(d));
            }
        } // END SECTION

        SECTION("manually supplied extent") {
            std::string csv_string("wkt,Name\n");
            mapnik::parameters params;
            params["type"] = std::string("csv");
            params["inline"] = csv_string;
            params["extent"] = "-180,-90,180,90";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(bool(ds));
            auto box = ds->envelope();
            CHECK(box.minx() == -180);
            CHECK(box.miny() ==  -90);
            CHECK(box.maxx() ==  180);
            CHECK(box.maxy() ==   90);
        } // END SECTION

        SECTION("inline geojson") {
            std::string csv_string = "geojson\n'{\"coordinates\":[-92.22568,38.59553],\"type\":\"Point\"}'";
            mapnik::parameters params;
            params["type"] = std::string("csv");
            params["inline"] = csv_string;
            params["quote"] = "'";
            auto ds = mapnik::datasource_cache::instance().create(params);
            REQUIRE(bool(ds));

            auto fields = ds->get_descriptor().get_descriptors();
            require_field_names(fields, {});

            // TODO: this originally had the following comment:
            //   - re-enable after https://github.com/mapnik/mapnik/issues/2319 is fixed
            // but that seems to have been merged and tested separately?
            auto fs = all_features(ds);
            auto feat = fs->next();
            CHECK(feature_count(feat->get_geometry()) == 1);
        } // END SECTION
        mapnik::logger::instance().set_severity(severity);
    }
} // END TEST CASE
