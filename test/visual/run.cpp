/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#include "runner.hpp"
#include "config.hpp"

#include <mapnik/mapnik.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>

// boost
#include <boost/program_options.hpp>

#include "cleanup.hpp" // run_cleanup()

#if defined(_WIN32)
#include <windows.h>
#endif

#include <boost/format.hpp>
#include <random>

#ifdef MAPNIK_LOG
using log_levels_map = std::map<std::string, mapnik::logger::severity_type>;

log_levels_map log_levels{{"debug", mapnik::logger::severity_type::debug},
                          {"warn", mapnik::logger::severity_type::warn},
                          {"error", mapnik::logger::severity_type::error},
                          {"none", mapnik::logger::severity_type::none}};
#endif

using namespace visual_tests;
namespace po = boost::program_options;

namespace {

static std::random_device entropy;

std::string unique_name()
{
    std::mt19937 gen(entropy());
    std::uniform_int_distribution<> distrib(0, 65535);
    auto fmt = boost::format("%1$04x-%2$04x-%3$04x-%4$04x") % distrib(gen) % distrib(gen) % distrib(gen) % distrib(gen);
    return fmt.str();
}
} // namespace

runner::renderer_container
  create_renderers(po::variables_map const& args, mapnik::fs::path const& output_dir, bool force_append = false)
{
    mapnik::fs::path reference_dir(args["images-dir"].as<std::string>());
    bool overwrite = args.count("overwrite");
    runner::renderer_container renderers;

    if (force_append || args.count(agg_renderer::name))
    {
        renderers.emplace_back(renderer<agg_renderer>(output_dir, reference_dir, overwrite));
    }
#if defined(HAVE_CAIRO)
    if (force_append || args.count(cairo_renderer::name))
    {
        renderers.emplace_back(renderer<cairo_renderer>(output_dir, reference_dir, overwrite));
    }
#ifdef CAIRO_HAS_SVG_SURFACE
    if (args.count(cairo_svg_renderer::name))
    {
        renderers.emplace_back(renderer<cairo_svg_renderer>(output_dir, reference_dir, overwrite));
    }
#endif
#ifdef CAIRO_HAS_PS_SURFACE
    if (args.count(cairo_ps_renderer::name))
    {
        renderers.emplace_back(renderer<cairo_ps_renderer>(output_dir, reference_dir, overwrite));
    }
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
    if (args.count(cairo_pdf_renderer::name))
    {
        renderers.emplace_back(renderer<cairo_pdf_renderer>(output_dir, reference_dir, overwrite));
    }
#endif
#endif
#if defined(SVG_RENDERER)
    if (force_append || args.count(svg_renderer::name))
    {
        renderers.emplace_back(renderer<svg_renderer>(output_dir, reference_dir, overwrite));
    }
#endif
#if defined(GRID_RENDERER)
    if (force_append || args.count(grid_renderer::name))
    {
        renderers.emplace_back(renderer<grid_renderer>(output_dir, reference_dir, overwrite));
    }
#endif

    if (renderers.empty())
    {
        return create_renderers(args, output_dir, true);
    }

    return renderers;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    po::options_description desc("visual test runner");
    // clang-format off
    desc.add_options()
        ("help,h", "produce usage message")
        ("verbose,v", "verbose output")
        ("overwrite,o", "overwrite reference image")
        ("duration,d", "output rendering duration")
        ("iterations,i", po::value<std::size_t>()->default_value(1), "number of iterations for benchmarking")
        ("jobs,j", po::value<std::size_t>()->default_value(1), "number of parallel threads")
        ("limit,l", po::value<std::size_t>()->default_value(0), "limit number of failures")
        ("styles-dir", po::value<std::string>()->default_value("test/data-visual/styles"), "directory with styles")
        ("images-dir", po::value<std::string>()->default_value("test/data-visual/images"), "directory with reference images")
        ("output-dir", po::value<std::string>()->default_value("/tmp/mapnik-visual-images"), "directory for output files")
        ("unique-subdir,u", "write output files to subdirectory with unique name")
        ("styles", po::value<std::vector<std::string>>(), "selected styles to test")
        ("fonts", po::value<std::string>()->default_value("fonts"), "font search path")
        ("plugins", po::value<std::string>()->default_value("plugins/input"), "input plugins search path")
#ifdef MAPNIK_LOG
        ("log", po::value<std::string>()->default_value(std::find_if(log_levels.begin(), log_levels.end(),
             [](log_levels_map::value_type const & level) { return level.second == mapnik::logger::get_severity(); } )->first),
             "log level (debug, warn, error, none)")
#endif
        ("scale-factor,s", po::value<std::vector<double>>()->default_value({ 1.0, 2.0 }, "1.0, 2.0"), "scale factor")
        (agg_renderer::name, "render with AGG renderer")
#if defined(HAVE_CAIRO)
        (cairo_renderer::name, "render with Cairo renderer")
#ifdef CAIRO_HAS_SVG_SURFACE
        (cairo_svg_renderer::name, "render with Cairo SVG renderer")
#endif
#ifdef CAIRO_HAS_PS_SURFACE
        (cairo_ps_renderer::name, "render with Cairo PS renderer")
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
        (cairo_pdf_renderer::name, "render with Cairo PDF renderer")
#endif
#endif
#if defined(SVG_RENDERER)
        (svg_renderer::name, "render with SVG renderer")
#endif
#if defined(GRID_RENDERER)
        (grid_renderer::name, "render with Grid renderer")
#endif
        ;
    // clang-format on
    po::positional_options_description p;
    p.add("styles", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::clog << desc << std::endl;
        return 1;
    }

    mapnik::setup();
#ifdef MAPNIK_LOG
    std::string log_level(vm["log"].as<std::string>());
    log_levels_map::const_iterator level_iter = log_levels.find(log_level);
    if (level_iter == log_levels.end())
    {
        std::cerr << "Error: Unknown log level: " << log_level << std::endl;
        return 1;
    }
    else
    {
        mapnik::logger::set_severity(level_iter->second);
    }
#endif

    mapnik::freetype_engine::register_fonts(vm["fonts"].as<std::string>(), true);
    mapnik::datasource_cache::instance().register_datasources(vm["plugins"].as<std::string>());

    mapnik::fs::path output_dir(vm["output-dir"].as<std::string>());

    if (vm.count("unique-subdir"))
    {
        output_dir /= unique_name();
    }

    config defaults;
    defaults.scales = vm["scale-factor"].as<std::vector<double>>();

    runner run(vm["styles-dir"].as<std::string>(),
               defaults,
               vm["iterations"].as<std::size_t>(),
               vm["limit"].as<std::size_t>(),
               vm["jobs"].as<std::size_t>(),
               create_renderers(vm, output_dir));
    bool show_duration = vm.count("duration");
    report_type report(vm.count("verbose") ? report_type((console_report(show_duration)))
                                           : report_type((console_short_report(show_duration))));
    result_list results;

    try
    {
        if (vm.count("styles"))
        {
            results = run.test(vm["styles"].as<std::vector<std::string>>(), report);
        }
        else
        {
            results = run.test_all(report);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Error running tests: " << e.what() << std::endl;
        return 1;
    }

    unsigned failed_count = mapnik::util::apply_visitor(summary_visitor(results), report);

    if (failed_count)
    {
        html_summary(results, output_dir);
    }

    testing::run_cleanup();

    return failed_count;
}
