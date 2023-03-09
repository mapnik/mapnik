#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <string>
#include <mapnik/mapnik.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/datasource_cache.hpp>
#if __cplusplus >= 201703L && !defined(USE_BOOST_FILESYSTEM)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <boost/filesystem/convenience.hpp>
namespace fs = boost::filesystem;
#endif

#include "cleanup.hpp" // run_cleanup()

int main(int argc, char** argv)
{
    Catch::Session session;
    std::string plugin_path;
    std::string working_dir;
    auto cli =
      session.cli() | Catch::clara::Opt(plugin_path, "plugins")["-p"]["--plugins"]("path to mapnik plugins") |
      Catch::clara::Opt(working_dir, "working directory")["-C"]["--working-directory"]("change working directory");

    session.cli(cli);
    int result = session.applyCommandLine(argc, argv);

    mapnik::setup();
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
        fs::current_path(working_dir);
    }

    if (result == 0)
    {
        result = session.run();
    }

    testing::run_cleanup();

    return result;
}
