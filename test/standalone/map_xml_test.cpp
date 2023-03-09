#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mapnik/mapnik.hpp>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/debug.hpp>

#if __cplusplus >= 201703L && !defined(USE_BOOST_FILESYSTEM)
#include <filesystem>
#include <cstdio>
namespace fs = std::filesystem;
using error_code = std::error_code;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
using error_code = boost::system::error_code;
#endif

#include <boost/format.hpp>
#include <boost/range/iterator_range_core.hpp>

#include <iostream>
#include <random>

namespace {

static std::random_device entropy;

std::string unique_mapnik_name()
{
    std::mt19937 gen(entropy());
    std::uniform_int_distribution<> distrib(0, 65535);
    auto fmt = boost::format("mapnik-test-%1$04x-%2$04x-%3$04x-%4$04x") % distrib(gen) % distrib(gen) % distrib(gen) %
               distrib(gen);
    return fmt.str();
}

class tmp_dir
{
  private:
    fs::path m_path;

  public:
    tmp_dir()
        : m_path(fs::temp_directory_path() / unique_mapnik_name())
    {
        fs::create_directories(m_path);
    }

    ~tmp_dir()
    {
        // files might be deleted by other things while the test is
        // running, which isn't necessarily an error as far as this
        // code is concerned - it just wants to delete everything
        // underneath the temporary directory.
        error_code err;

        // catch all errors - we don't want to throw in the destructor
        try
        {
            // but loop while the path exists and the errors are
            // ignorable.
            while (fs::exists(m_path))
            {
                fs::remove_all(m_path, err);

                // for any non-ignorable error, there's not much we can
                // do from the destructor. it's in /tmp anyway, so it'll
                // get reclaimed next boot.
                if (err && (err != std::errc::no_such_file_or_directory))
                {
                    break;
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception caught while trying to remove "
                      << "temporary directory " << m_path << ": " << e.what() << "\n";
        }
        catch (...)
        {
            std::cerr << "Unknown exception caught while trying to "
                      << "remove temporary directory " << m_path << "\n";
        }
    }

    fs::path path() const { return m_path; }
};

void compare_map(fs::path xml)
{
    tmp_dir dir;
    mapnik::Map m(256, 256);
    REQUIRE(m.register_fonts("fonts", true));
    fs::path abs_base = xml.parent_path();

    // first, load the XML into a map object and save it. this
    // is a normalisation step to ensure that the file is in
    // whatever the current version of mapnik uses as the
    // standard indentation, quote style, etc...
    REQUIRE_NOTHROW(mapnik::load_map(m, xml.generic_string(), false, abs_base.generic_string()));
    fs::path test_map1 = dir.path() / "mapnik-temp-map1.xml";
    REQUIRE_NOTHROW(mapnik::save_map(m, test_map1.generic_string()));

    // create a new map, load the one saved in the previous
    // step, and write it out again.
    mapnik::Map new_map(256, 256);
    REQUIRE(new_map.register_fonts("fonts", true));
    REQUIRE_NOTHROW(mapnik::load_map(new_map, test_map1.generic_string(), false, abs_base.generic_string()));
    fs::path test_map2 = dir.path() / "mapnik-temp-map2.xml";
    REQUIRE_NOTHROW(mapnik::save_map(new_map, test_map2.generic_string()));

    // if all the information survived the load/save round-trip
    // then the two files ought to be identical.
    REQUIRE(fs::is_regular_file(test_map1));
    REQUIRE(fs::is_regular_file(test_map2));
    REQUIRE(fs::file_size(test_map1) == fs::file_size(test_map2));
    std::ifstream in_map1(test_map1.native()), in_map2(test_map2.native());
    REQUIRE(std::equal(std::istream_iterator<char>(in_map1),
                       std::istream_iterator<char>(),
                       std::istream_iterator<char>(in_map2)));
}

void add_xml_files(fs::path dir, std::vector<fs::path>& xml_files)
{
    for (auto const& entry : boost::make_iterator_range(fs::directory_iterator(dir), fs::directory_iterator()))
    {
        auto path = entry.path();
        if (path.extension().generic_string() == ".xml")
        {
            xml_files.emplace_back(path);
        }
    }
}

void load_map(mapnik::Map& m, fs::path const& path)
{
    try
    {
        mapnik::load_map(m, path.generic_string());
    }
    catch (std::exception const& ex)
    {
        // errors which come from the datasource not being loaded or
        // database not being set up aren't really useful - they're
        // more likely to be spurious than meaningful, so ignore them.
        std::string what = ex.what();
        if ((what.find("Could not create datasource") == std::string::npos) &&
            (what.find("Postgis Plugin: could not connect to server") == std::string::npos))
        {
            throw;
        }
    }
}

} // anonymous namespace
#ifndef MAPNIK_STATIC_PLUGINS
const bool registered =
  mapnik::datasource_cache::instance().register_datasources((fs::path("plugins") / "input").generic_string());
#endif
TEST_CASE("map xml I/O")
{
    mapnik::setup();
#ifndef MAPNIK_STATIC_PLUGINS
    // make sure plugins are loaded
    REQUIRE(registered);
#endif

    // make the tests silent since we intentially test error conditions that are noisy
    auto const severity = mapnik::logger::instance().get_severity();
    mapnik::logger::instance().set_severity(mapnik::logger::none);

    SECTION("good maps")
    {
        std::vector<fs::path> good_maps;
        add_xml_files(fs::path("test") / "data" / "good_maps", good_maps);

        for (auto const& path : good_maps)
        {
            CAPTURE(path.generic_string());

            // check that it can load
            mapnik::Map m(256, 256);
            REQUIRE(m.register_fonts("fonts", true));
            REQUIRE_NOTHROW(load_map(m, path));

            // and, if it can, that it saves the same way
            compare_map(path);
        }
    } // END SECTION

    SECTION("duplicate styles only throw in strict mode")
    {
        std::string duplicate_stylename(
          (fs::path("test") / "data" / "broken_maps" / "duplicate_stylename.xml").generic_string());
        CAPTURE(duplicate_stylename);
        mapnik::Map m(256, 256);
        REQUIRE(m.register_fonts("fonts", true));
        REQUIRE_NOTHROW(mapnik::load_map(m, duplicate_stylename, false));
        mapnik::Map m2(256, 256);
        REQUIRE(m2.register_fonts("fonts", true));
        REQUIRE_THROWS(mapnik::load_map(m2, duplicate_stylename, true));
    } // END SECTION

    SECTION("broken maps")
    {
        std::vector<fs::path> broken_maps;
        add_xml_files(fs::path("test") / "data" / "broken_maps", broken_maps);
        broken_maps.emplace_back(fs::path("test") / "data" / "broken_maps" / "does_not_exist.xml");

        for (auto const& path : broken_maps)
        {
            CAPTURE(path.generic_string());

            mapnik::Map m(256, 256);
            REQUIRE(m.register_fonts("fonts", true));
            REQUIRE_THROWS(mapnik::load_map(m, path.generic_string(), true));
        }
    } // END SECTION

    mapnik::logger::instance().set_severity(severity);
} // END TEST CASE
