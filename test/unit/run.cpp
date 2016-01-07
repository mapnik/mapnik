#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <string>
#include <mapnik/util/fs.hpp>
#include <mapnik/datasource_cache.hpp>
#include <boost/filesystem/convenience.hpp>

#include "cleanup.hpp" // run_cleanup()

std::string plugin_path;

inline void set_plugin_path(Catch::ConfigData&, std::string const& _plugin_path ) {
    plugin_path = _plugin_path;
}

std::string working_dir;

inline void set_working_dir(Catch::ConfigData&, std::string const& _working_dir ) {
    working_dir = _working_dir;
}


int main (int argc, char* const argv[])
{
    Catch::Session session;

    auto & cli = session.cli();

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
        result = session.run();
    }

    testing::run_cleanup();

    return result;

}
