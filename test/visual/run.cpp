/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>

// boost
#include <boost/program_options.hpp>

int main(int argc, char** argv)
{
    using namespace visual_tests;
    namespace po = boost::program_options;

    po::options_description desc("visual test runner");
    desc.add_options()
        ("help,h", "produce usage message")
        ("verbose,v", "verbose output")
        ("overwrite,o", "overwrite reference image")
        ("jobs,j", po::value<std::size_t>()->default_value(1), "number of parallel threads")
        ("styles-dir", po::value<std::string>()->default_value("test/data-visual/styles"), "directory with styles")
        ("images-dir", po::value<std::string>()->default_value("test/data-visual/images"), "directory with reference images")
        ("output-dir", po::value<std::string>()->default_value("/tmp/mapnik-visual-images"), "directory for output files")
        ("unique-subdir,u", "write output files to subdirectory with unique name")
        ("styles", po::value<std::vector<std::string>>(), "selected styles to test")
        ("fonts", po::value<std::string>()->default_value("fonts"), "font search path")
        ("plugins", po::value<std::string>()->default_value("plugins/input"), "input plugins search path")
        ;

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

    mapnik::freetype_engine::register_fonts(vm["fonts"].as<std::string>(), true);
    mapnik::datasource_cache::instance().register_datasources(vm["plugins"].as<std::string>());

    boost::filesystem::path output_dir(vm["output-dir"].as<std::string>());

    if (vm.count("unique-subdir"))
    {
        output_dir /= boost::filesystem::unique_path();
    }

    runner run(vm["styles-dir"].as<std::string>(),
               output_dir,
               vm["images-dir"].as<std::string>(),
               vm.count("overwrite"),
               vm["jobs"].as<std::size_t>());
    report_type report = vm.count("verbose") ? report_type((console_report())) : report_type((console_short_report()));
    result_list results;

    if (vm.count("styles"))
    {
        results = run.test(vm["styles"].as<std::vector<std::string>>(), report);
    }
    else
    {
        results = run.test_all(report);
    }

    unsigned failed_count = mapnik::util::apply_visitor(summary_visitor(results), report);

    if (failed_count)
    {
        html_summary(results, output_dir);
    }

    return failed_count;
}
