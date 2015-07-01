#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mapnik/map.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/util/fs.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional_io.hpp>

#include <iostream>

namespace bfs = boost::filesystem;

namespace {
void add_csv_files(bfs::path dir, std::vector<bfs::path> &csv_files) {
  for (auto const &entry : boost::make_iterator_range(
         bfs::directory_iterator(dir), bfs::directory_iterator())) {
    auto path = entry.path();
    if (path.extension().native() == ".csv") {
      csv_files.emplace_back(path);
    }
  }
}

mapnik::datasource_ptr get_csv_ds(std::string const &file_name, bool strict = true) {
  mapnik::parameters params;
  params["type"] = std::string("csv");
  params["file"] = file_name;
  params["strict"] = mapnik::value_bool(strict);
  auto ds = mapnik::datasource_cache::instance().create(params);
  // require a non-null pointer returned
  REQUIRE(bool(ds));
  return ds;
}

void require_field_names(std::vector<mapnik::attribute_descriptor> const &fields,
                         std::initializer_list<std::string> const &names) {
  REQUIRE(fields.size() == names.size());
  auto itr_a = fields.begin();
  auto const end_a = fields.end();
  auto itr_b = names.begin();
  for (; itr_a != end_a; ++itr_a, ++itr_b) {
    CHECK(itr_a->get_name() == *itr_b);
  }
}

void require_field_types(std::vector<mapnik::attribute_descriptor> const &fields,
                         std::initializer_list<mapnik::eAttributeType> const &types) {
  REQUIRE(fields.size() == types.size());
  auto itr_a = fields.begin();
  auto const end_a = fields.end();
  auto itr_b = types.begin();
  for (; itr_a != end_a; ++itr_a, ++itr_b) {
    CHECK(itr_a->get_type() == *itr_b);
  }
}

mapnik::featureset_ptr all_features(mapnik::datasource_ptr ds) {
  auto fields = ds->get_descriptor().get_descriptors();
  mapnik::query query(ds->envelope());
  for (auto const &field : fields) {
    query.add_property_name(field.get_name());
  }
  return ds->features(query);
}

std::size_t count_features(mapnik::featureset_ptr features) {
  std::size_t count = 0;
  while (features->next()) {
    ++count;
  }
  return count;
}

using attr = std::tuple<std::string, mapnik::value>;
void require_attributes(mapnik::feature_ptr feature,
                        std::initializer_list<attr> const &attrs) {
  REQUIRE(bool(feature));
  for (auto const &kv : attrs) {
    REQUIRE(feature->has_key(std::get<0>(kv)));
    CHECK(feature->get(std::get<0>(kv)) == std::get<1>(kv));
  }
}

namespace detail {
struct feature_count {
  template <typename T>
  std::size_t operator()(T const &geom) const {
    return mapnik::util::apply_visitor(*this, geom);
  }

  std::size_t operator()(mapnik::geometry::geometry_empty const &) const {
    return 0;
  }

  template <typename T>
  std::size_t operator()(mapnik::geometry::point<T> const &) const {
    return 1;
  }

  template <typename T>
  std::size_t operator()(mapnik::geometry::line_string<T> const &) const {
    return 1;
  }

  template <typename T>
  std::size_t operator()(mapnik::geometry::polygon<T> const &) const {
    return 1;
  }

  template <typename T>
  std::size_t operator()(mapnik::geometry::multi_point<T> const &mp) const {
    return mp.size();
  }

  template <typename T>
  std::size_t operator()(mapnik::geometry::multi_line_string<T> const &mls) const {
    return mls.size();
  }

  template <typename T>
  std::size_t operator()(mapnik::geometry::multi_polygon<T> const &mp) const {
    return mp.size();
  }

  template <typename T>
  std::size_t operator()(mapnik::geometry::geometry_collection<T> const &col) const {
    std::size_t sum = 0;
    for (auto const &geom : col) {
      sum += operator()(geom);
    }
    return sum;
  }
};
} // namespace detail

template <typename T>
std::size_t feature_count(mapnik::geometry::geometry<T> const &g) {
  return detail::feature_count()(g);
}

void require_geometry(mapnik::feature_ptr feature,
                      std::size_t num_parts,
                      mapnik::geometry::geometry_types type) {
  REQUIRE(bool(feature));
  CHECK(mapnik::geometry::geometry_type(feature->get_geometry()) == type);
  CHECK(feature_count(feature->get_geometry()) == num_parts);
}
} // anonymous namespace

static const std::string csv_plugin("./plugins/input/csv.input");

const bool registered = mapnik::datasource_cache::instance().register_datasources(csv_plugin);

TEST_CASE("csv") {

  if (mapnik::util::exists(csv_plugin))
  {

      REQUIRE(registered);

      // make the tests silent since we intentially test error conditions that are noisy
      auto const severity = mapnik::logger::instance().get_severity();
      mapnik::logger::instance().set_severity(mapnik::logger::none);

      // check the CSV datasource is loaded
      const std::vector<std::string> plugin_names =
        mapnik::datasource_cache::instance().plugin_names();
      const bool have_csv_plugin =
        std::find(plugin_names.begin(), plugin_names.end(), "csv") != plugin_names.end();

      SECTION("broken files") {
        if (have_csv_plugin) {
          std::vector<bfs::path> broken;
          add_csv_files("test/data/csv/fails", broken);
          add_csv_files("test/data/csv/warns", broken);
          broken.emplace_back("test/data/csv/fails/does_not_exist.csv");

          for (auto const &path : broken) {
            REQUIRE_THROWS(get_csv_ds(path.native()));
          }
        }
      } // END SECTION

      SECTION("good files") {
        if (have_csv_plugin) {
          std::vector<bfs::path> good;
          add_csv_files("test/data/csv", good);
          add_csv_files("test/data/csv/warns", good);

          for (auto const &path : good) {
            auto ds = get_csv_ds(path.native(), false);
            // require a non-null pointer returned
            REQUIRE(bool(ds));
          }
        }
      } // END SECTION

      SECTION("lon/lat detection") {
        for (auto const &lon_name : {std::string("lon"), std::string("lng")}) {
          auto ds = get_csv_ds((boost::format("test/data/csv/%1%_lat.csv") % lon_name).str());
          auto fields = ds->get_descriptor().get_descriptors();
          require_field_names(fields, {lon_name, "lat"});
          require_field_types(fields, {mapnik::Integer, mapnik::Integer});

          CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);

          mapnik::query query(ds->envelope());
          for (auto const &field : fields) {
            query.add_property_name(field.get_name());
          }
          auto features = ds->features(query);
          auto feature = features->next();

          require_attributes(feature, {
              attr { lon_name, mapnik::value_integer(0) },
              attr { "lat", mapnik::value_integer(0) }
            });
        }
      } // END SECTION

      SECTION("type detection") {
        auto ds = get_csv_ds("test/data/csv/nypd.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"Precinct", "Phone", "Address", "City", "geo_longitude", "geo_latitude", "geo_accuracy"});
        require_field_types(fields, {mapnik::String, mapnik::String, mapnik::String, mapnik::String, mapnik::Double, mapnik::Double, mapnik::String});

        CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
        CHECK(count_features(all_features(ds)) == 2);

        auto feature = all_features(ds)->next();
        require_attributes(feature, {
              attr { "City", mapnik::value_unicode_string("New York, NY") }
            , attr { "geo_accuracy", mapnik::value_unicode_string("house") }
            , attr { "Phone", mapnik::value_unicode_string("(212) 334-0711") }
            , attr { "Address", mapnik::value_unicode_string("19 Elizabeth Street") }
            , attr { "Precinct", mapnik::value_unicode_string("5th Precinct") }
            , attr { "geo_longitude", mapnik::value_integer(-70) }
            , attr { "geo_latitude", mapnik::value_integer(40) }
          });
      } // END SECTION

      SECTION("skipping blank rows") {
        auto ds = get_csv_ds("test/data/csv/blank_rows.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "name"});
        require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String});

        CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
        CHECK(count_features(all_features(ds)) == 2);
      } // END SECTION

      SECTION("empty rows") {
        auto ds = get_csv_ds("test/data/csv/empty_rows.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "text", "date", "integer", "boolean", "float", "time", "datetime", "empty_column"});
        require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String, mapnik::String, mapnik::Integer, mapnik::Boolean, mapnik::Double, mapnik::String, mapnik::String, mapnik::String});

        CHECK(ds->get_geometry_type() == mapnik::datasource_geometry_t::Point);
        CHECK(count_features(all_features(ds)) == 4);

        auto featureset = all_features(ds);
        auto feature = featureset->next();
        require_attributes(feature, {
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
      } // END SECTION

      SECTION("slashes") {
        auto ds = get_csv_ds("test/data/csv/has_attributes_with_slashes.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "name"});
        // NOTE: y column is integer, even though a double value is used below in the test?
        require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String});
        
        auto featureset = all_features(ds);
        require_attributes(featureset->next(), {
              attr{"x", 0}
            , attr{"y", 0}
            , attr{"name", mapnik::value_unicode_string("a/a") } });
        require_attributes(featureset->next(), {
              attr{"x", 1}
            , attr{"y", 4}
            , attr{"name", mapnik::value_unicode_string("b/b") } });
        require_attributes(featureset->next(), {
              attr{"x", 10}
            , attr{"y", 2.5}
            , attr{"name", mapnik::value_unicode_string("c/c") } });
      } // END SECTION

      SECTION("wkt field") {
        using mapnik::geometry::geometry_types;

        auto ds = get_csv_ds("test/data/csv/wkt.csv");
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
      } // END SECTION

      SECTION("handling of missing header") {
        // TODO: does this mean 'missing_header.csv' should be in the warnings
        // subdirectory, since it doesn't work in strict mode?
        auto ds = get_csv_ds("test/data/csv/missing_header.csv", false);
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"one", "two", "x", "y", "_4", "aftermissing"});
        auto feature = all_features(ds)->next();
        REQUIRE(feature);
        REQUIRE(feature->has_key("_4"));
        CHECK(feature->get("_4") == mapnik::value_unicode_string("missing"));
      } // END SECTION

      SECTION("handling of headers that are numbers") {
        auto ds = get_csv_ds("test/data/csv/numbers_for_headers.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "1990", "1991", "1992"});
        auto feature = all_features(ds)->next();
        require_attributes(feature, {
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
      } // END SECTION

      SECTION("quoted numbers") {
        using ustring = mapnik::value_unicode_string;

        auto ds = get_csv_ds("test/data/csv/quoted_numbers.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "label"});
        auto featureset = all_features(ds);

        require_attributes(featureset->next(), {
            attr{"x", 0}, attr{"y", 0}, attr{"label", ustring("0,0") } });
        require_attributes(featureset->next(), {
            attr{"x", 5}, attr{"y", 5}, attr{"label", ustring("5,5") } });
        require_attributes(featureset->next(), {
            attr{"x", 0}, attr{"y", 5}, attr{"label", ustring("0,5") } });
        require_attributes(featureset->next(), {
            attr{"x", 5}, attr{"y", 0}, attr{"label", ustring("5,0") } });
        require_attributes(featureset->next(), {
            attr{"x", 2.5}, attr{"y", 2.5}, attr{"label", ustring("2.5,2.5") } });

      } // END SECTION

      SECTION("reading newlines") {
        for (auto const &platform : {std::string("windows"), std::string("mac")}) {
          std::string file_name = (boost::format("test/data/csv/%1%_newlines.csv") % platform).str();
          auto ds = get_csv_ds(file_name);
          auto fields = ds->get_descriptor().get_descriptors();
          require_field_names(fields, {"x", "y", "z"});
          require_attributes(all_features(ds)->next(), {
              attr{"x", 1}, attr{"y", 10}, attr{"z", 9999.9999} });
        }
      } // END SECTION

      SECTION("mixed newlines") {
        using ustring = mapnik::value_unicode_string;

        for (auto const &file : {
                std::string("test/data/csv/mac_newlines_with_unix_inline.csv")
              , std::string("test/data/csv/mac_newlines_with_unix_inline_escaped.csv")
              , std::string("test/data/csv/windows_newlines_with_unix_inline.csv")
              , std::string("test/data/csv/windows_newlines_with_unix_inline_escaped.csv")
              }) {
          auto ds = get_csv_ds(file);
          auto fields = ds->get_descriptor().get_descriptors();
          require_field_names(fields, {"x", "y", "line"});
          require_attributes(all_features(ds)->next(), {
                attr{"x", 0}, attr{"y", 0}
              , attr{"line", ustring("many\n  lines\n  of text\n  with unix newlines")} });
        }
      } // END SECTION

      SECTION("tabs") {
        auto ds = get_csv_ds("test/data/csv/tabs_in_csv.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "z"});
        require_attributes(all_features(ds)->next(), {
            attr{"x", -122}, attr{"y", 48}, attr{"z", 0} });
      } // END SECTION

      SECTION("separators") {
        using ustring = mapnik::value_unicode_string;

        for (auto const &file : {
                std::string("test/data/csv/pipe_delimiters.csv")
              , std::string("test/data/csv/semicolon_delimiters.csv")
                  }) {
          auto ds = get_csv_ds(file);
          auto fields = ds->get_descriptor().get_descriptors();
          require_field_names(fields, {"x", "y", "z"});
          require_attributes(all_features(ds)->next(), {
              attr{"x", 0}, attr{"y", 0}, attr{"z", ustring("hello")} });
        }
      } // END SECTION

      SECTION("null and bool keywords are empty strings") {
        using ustring = mapnik::value_unicode_string;

        auto ds = get_csv_ds("test/data/csv/nulls_and_booleans_as_strings.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "null", "boolean"});
        require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String, mapnik::Boolean});

        auto featureset = all_features(ds);
        require_attributes(featureset->next(), {
            attr{"x", 0}, attr{"y", 0}, attr{"null", ustring("null")}, attr{"boolean", true}});
        require_attributes(featureset->next(), {
            attr{"x", 0}, attr{"y", 0}, attr{"null", ustring("")}, attr{"boolean", false}});
      } // END SECTION

      SECTION("nonexistent query fields throw") {
        auto ds = get_csv_ds("test/data/csv/lon_lat.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"lon", "lat"});
        require_field_types(fields, {mapnik::Integer, mapnik::Integer});

        mapnik::query query(ds->envelope());
        for (auto const &field : fields) {
          query.add_property_name(field.get_name());
        }
        // also add an invalid one, triggering throw
        query.add_property_name("bogus");

        REQUIRE_THROWS(ds->features(query));
      } // END SECTION

      SECTION("leading zeros mean strings") {
        using ustring = mapnik::value_unicode_string;

        auto ds = get_csv_ds("test/data/csv/leading_zeros.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "fips"});
        require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String});

        auto featureset = all_features(ds);
        require_attributes(featureset->next(), {
            attr{"x", 0}, attr{"y", 0}, attr{"fips", ustring("001")}});
        require_attributes(featureset->next(), {
            attr{"x", 0}, attr{"y", 0}, attr{"fips", ustring("003")}});
        require_attributes(featureset->next(), {
            attr{"x", 0}, attr{"y", 0}, attr{"fips", ustring("005")}});
      } // END SECTION

      SECTION("advanced geometry detection") {
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

      SECTION("creation of CSV from in-memory strings") {
        using ustring = mapnik::value_unicode_string;

        for (auto const &name : {std::string("Winthrop, WA"), std::string(u8"Qu\u00e9bec")}) {
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
          CHECK(feature->get("Name") == ustring(name.c_str()));
        }
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

      SECTION("blank undelimited rows are still parsed") {
        using ustring = mapnik::value_unicode_string;

        // TODO: does this mean this CSV file should be in the warnings
        // subdirectory, since it doesn't work in strict mode?
        auto ds = get_csv_ds("test/data/csv/more_headers_than_column_values.csv", false);
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "one", "two", "three"});
        require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String, mapnik::String, mapnik::String});

        require_attributes(all_features(ds)->next(), {
            attr{"x", 0}, attr{"y", 0}, attr{"one", ustring("")}, attr{"two", ustring("")}, attr{"three", ustring("")} });
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

        for (auto const &r : {
              row{"test/data/csv/fails/needs_headers_two_lines.csv", 2}
            , row{"test/data/csv/fails/needs_headers_one_line.csv", 1}
            , row{"test/data/csv/fails/needs_headers_one_line_no_newline.csv", 1}
          }) {
          mapnik::parameters params;
          params["type"] = std::string("csv");
          params["file"] = r.first;
          params["headers"] = "x,y,name";
          auto ds = mapnik::datasource_cache::instance().create(params);
          REQUIRE(bool(ds));

          auto fields = ds->get_descriptor().get_descriptors();
          require_field_names(fields, {"x", "y", "name"});
          require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::String});
          require_attributes(all_features(ds)->next(), {
              attr{"x", 0}, attr{"y", 0}, attr{"name", ustring("data_name")} });
          REQUIRE(count_features(all_features(ds)) == r.second);
        }
      } // END SECTION

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wlong-long"
      SECTION("64bit int fields work") {
        auto ds = get_csv_ds("test/data/csv/64bit_int.csv");
        auto fields = ds->get_descriptor().get_descriptors();
        require_field_names(fields, {"x", "y", "bigint"});
        require_field_types(fields, {mapnik::Integer, mapnik::Integer, mapnik::Integer});

        auto fs = all_features(ds);
        auto feature = fs->next();
        require_attributes(feature, {
            attr{"x", 0}, attr{"y", 0}, attr{"bigint", 2147483648} });

        feature = fs->next();
        require_attributes(feature, {
            attr{"x", 0}, attr{"y", 0}, attr{"bigint", 9223372036854775807ll} });
        require_attributes(feature, {
            attr{"x", 0}, attr{"y", 0}, attr{"bigint", 0x7FFFFFFFFFFFFFFFll} });
      } // END SECTION
    #pragma GCC diagnostic pop

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
