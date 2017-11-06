#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/debug.hpp>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range_core.hpp>

#include <iostream>

namespace bfs = boost::filesystem;

namespace {

class tmp_dir {
private:
    bfs::path m_path;

public:
    tmp_dir()
        : m_path(bfs::temp_directory_path() / bfs::unique_path("mapnik-test-%%%%-%%%%-%%%%-%%%%")) {
        bfs::create_directories(m_path);
    }

    ~tmp_dir() {
        // files might be deleted by other things while the test is
        // running, which isn't necessarily an error as far as this
        // code is concerned - it just wants to delete everything
        // underneath the temporary directory.
        boost::system::error_code err;

        // catch all errors - we don't want to throw in the destructor
        try {
            // but loop while the path exists and the errors are
            // ignorable.
            while (bfs::exists(m_path)) {
                bfs::remove_all(m_path, err);

                // for any non-ignorable error, there's not much we can
                // do from the destructor. it's in /tmp anyway, so it'll
                // get reclaimed next boot.
                if (err && (err != boost::system::errc::no_such_file_or_directory)) {
                    break;
                }
            }

        } catch (const std::exception &e) {
            std::cerr << "Exception caught while trying to remove "
                      << "temporary directory " << m_path
                      << ": " << e.what() << "\n";

        } catch (...) {
            std::cerr << "Unknown exception caught while trying to "
                      << "remove temporary directory " << m_path
                      << "\n";
        }
    }

    bfs::path path() const {
        return m_path;
    }
};

void compare_map(bfs::path xml) {
    tmp_dir dir;
    mapnik::Map m(256, 256);
    REQUIRE(m.register_fonts("fonts", true));
    bfs::path abs_base = xml.parent_path();

    // first, load the XML into a map object and save it. this
    // is a normalisation step to ensure that the file is in
    // whatever the current version of mapnik uses as the
    // standard indentation, quote style, etc...
    REQUIRE_NOTHROW(mapnik::load_map(m, xml.native(), false, abs_base.native()));
    bfs::path test_map1 = dir.path() / "mapnik-temp-map1.xml";
    REQUIRE_NOTHROW(mapnik::save_map(m, test_map1.native()));

    // create a new map, load the one saved in the previous
    // step, and write it out again.
    mapnik::Map new_map(256, 256);
    REQUIRE(new_map.register_fonts("fonts", true));
    REQUIRE_NOTHROW(mapnik::load_map(new_map, test_map1.native(), false, abs_base.native()));
    bfs::path test_map2 = dir.path() / "mapnik-temp-map2.xml";
    REQUIRE_NOTHROW(mapnik::save_map(new_map, test_map2.native()));

    // if all the information survived the load/save round-trip
    // then the two files ought to be identical.
    REQUIRE(bfs::is_regular_file(test_map1));
    REQUIRE(bfs::is_regular_file(test_map2));
    REQUIRE(bfs::file_size(test_map1) == bfs::file_size(test_map2));
    std::ifstream in_map1(test_map1.native()), in_map2(test_map2.native());
    REQUIRE(std::equal(std::istream_iterator<char>(in_map1), std::istream_iterator<char>(),
                       std::istream_iterator<char>(in_map2)));
}

void add_xml_files(bfs::path dir, std::vector<bfs::path> &xml_files) {
    for (auto const &entry : boost::make_iterator_range(
             bfs::directory_iterator(dir), bfs::directory_iterator())) {
        auto path = entry.path();
        if (path.extension().native() == ".xml") {
            xml_files.emplace_back(path);
        }
    }
}

void load_map(mapnik::Map &m, bfs::path const &path) {

    try
    {
        mapnik::load_map(m, path.native());
    }
    catch (std::exception const &ex)
    {
        // errors which come from the datasource not being loaded or
        // database not being set up aren't really useful - they're
        // more likely to be spurious than meaningful, so ignore them.
        std::string what = ex.what();
        if ((what.find("Could not create datasource") == std::string::npos) &&
            (what.find("Postgis Plugin: could not connect to server") == std::string::npos)) {
            throw;
        }
    }
}

} // anonymous namespace

const bool registered = mapnik::datasource_cache::instance().register_datasources("./plugins/input/");

TEST_CASE("map xml I/O") {
    // make sure plugins are loaded
    REQUIRE(registered);

    // make the tests silent since we intentially test error conditions that are noisy
    auto const severity = mapnik::logger::instance().get_severity();
    mapnik::logger::instance().set_severity(mapnik::logger::none);

    SECTION("good maps") {
        std::vector<bfs::path> good_maps;
        add_xml_files("test/data/good_maps", good_maps);

        for (auto const &path : good_maps) {
            CAPTURE(path.native());

            // check that it can load
            mapnik::Map m(256, 256);
            REQUIRE(m.register_fonts("fonts", true));
            REQUIRE_NOTHROW(load_map(m, path));

            // and, if it can, that it saves the same way
            compare_map(path);
        }
    } // END SECTION

    SECTION("broken maps") {
        std::vector<bfs::path> broken_maps;
        add_xml_files("test/data/broken_maps", broken_maps);
        broken_maps.emplace_back("test/data/broken_maps/does_not_exist.xml");

        for (auto const &path : broken_maps) {
            CAPTURE(path.native());

            mapnik::Map m(256, 256);
            REQUIRE(m.register_fonts("fonts", true));
            REQUIRE_THROWS(mapnik::load_map(m, path.native(), true));
        }
    } // END SECTION

    mapnik::logger::instance().set_severity(severity);
} // END TEST CASE
