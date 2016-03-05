#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "catch_tmp.hpp"

#include <string>
#include <mapnik/debug.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/datasource_cache.hpp>
#include <boost/filesystem/convenience.hpp>

#include "cleanup.hpp" // run_cleanup()

boost::filesystem::path catch_temporary_path::base_dir =
    boost::filesystem::temp_directory_path() / "mapnik-test-unit";
bool catch_temporary_path::keep_temporary_files = false;

static std::string plugin_path;
static std::string working_dir;

static void set_keep_temporary_files(Catch::ConfigData&)
{
    catch_temporary_path::keep_temporary_files = true;
}

static void set_log_severity(Catch::ConfigData&, std::string const& severity)
{
    if (severity == "debug")
        mapnik::logger::set_severity(mapnik::logger::debug);
    else if (severity == "warn")
        mapnik::logger::set_severity(mapnik::logger::warn);
    else if (severity == "error")
        mapnik::logger::set_severity(mapnik::logger::error);
    else if (severity == "none")
        mapnik::logger::set_severity(mapnik::logger::none);
    else
        std::clog << "unknonw log severity '" << severity << "'\n";
}

static void set_plugin_path(Catch::ConfigData&, std::string const& _plugin_path)
{
    plugin_path = _plugin_path;
}

static void set_working_dir(Catch::ConfigData&, std::string const& _working_dir)
{
    working_dir = _working_dir;
}

int main (int argc, char* const argv[])
{
    Catch::Session session;

    auto & cli = session.cli();

    cli["--keep-temporary-files"]
        .describe("tests should't delete temporary files they create")
        .bind(&set_keep_temporary_files);

    cli["--log"]
        .describe("mapnik log severity")
        .bind(&set_log_severity, "debug|warn|error|none");

    cli["-p"]["--plugins"]
        .describe("path to mapnik plugins")
        .bind(&set_plugin_path, "plugins");

    cli["-C"]["--working-directory"]
        .describe("change working directory")
        .bind(&set_working_dir, "working directory");

    int result = session.applyCommandLine(argc, argv);

    if (!plugin_path.empty())
    {
        if (!mapnik::util::exists(plugin_path))
        {
            std::clog << "Could not find " << plugin_path << "\n";
            return -1;
        }
        mapnik::datasource_cache::instance().register_datasources(plugin_path);
    }
    else
    {
        mapnik::datasource_cache::instance().register_datasources("plugins/input/");
    }

    if (!working_dir.empty())
    {
        if (!mapnik::util::exists(working_dir))
        {
            std::clog << "Could not find " << working_dir << "\n";
            return -1;
        }
        boost::filesystem::current_path(working_dir);
    }

    if (result == 0)
    {
        auto tmp_base_dir = catch_temporary_path::create_dir("");
        result = session.run();
    }

    testing::run_cleanup();

    return result;

}
