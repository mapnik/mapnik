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

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include <mapnik/util/fs.hpp>
#include <mapnik/quad_tree.hpp>

#include "process_csv_file.hpp"
#include "process_geojson_file.hpp"

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

const int DEFAULT_DEPTH = 8;
const double DEFAULT_RATIO = 0.55;

namespace mapnik { namespace detail {

bool is_csv(std::string const& filename)
{
    return boost::iends_with(filename,".csv")
        || boost::iends_with(filename,".tsv");
}

bool is_geojson(std::string const& filename)
{
    return boost::iends_with(filename,".geojson")
        || boost::iends_with(filename,".json");
}

}}

int main (int argc, char** argv)
{
    //using namespace mapnik;
    namespace po = boost::program_options;
    bool verbose = false;
    bool validate_features = false;
    unsigned int depth = DEFAULT_DEPTH;
    double ratio = DEFAULT_RATIO;
    std::vector<std::string> files;
    char separator = 0;
    char quote = 0;
    std::string manual_headers;
    try
    {
        po::options_description desc("Mapnik CSV/GeoJSON index utility");
        desc.add_options()
            ("help,h", "produce usage message")
            ("version,V","print version string")
            ("verbose,v","verbose output")
            ("depth,d", po::value<unsigned int>(), "max tree depth\n(default 8)")
            ("ratio,r",po::value<double>(),"split ratio (default 0.55)")
            ("separator,s", po::value<char>(), "CSV columns separator")
            ("quote,q", po::value<char>(), "CSV columns quote")
            ("manual-headers,H", po::value<std::string>(), "CSV manual headers string")
            ("files",po::value<std::vector<std::string> >(),"Files to index: file1 file2 ...fileN")
            ("validate-features", "Validate GeoJSON features")
            ;

        po::positional_options_description p;
        p.add("files",-1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("version"))
        {
            std::clog << "version 1.0.0" << std::endl;
            return 1;
        }
        if (vm.count("help"))
        {
            std::clog << desc << std::endl;
            return 1;
        }
        if (vm.count("verbose"))
        {
            verbose = true;
        }
        if (vm.count("validate-features"))
        {
            validate_features = true;
        }
        if (vm.count("depth"))
        {
            depth = vm["depth"].as<unsigned int>();
        }
        if (vm.count("ratio"))
        {
            ratio = vm["ratio"].as<double>();
        }
        if (vm.count("separator"))
        {
            separator = vm["separator"].as<char>();
        }
        if (vm.count("quote"))
        {
            quote = vm["quote"].as<char>();
        }
        if (vm.count("manual-headers"))
        {
            manual_headers = vm["manual-headers"].as<std::string>();
        }
        if (vm.count("files"))
        {
            files=vm["files"].as<std::vector<std::string> >();
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> files_to_process;

    for (auto const& filename : files)
    {
        if (!mapnik::util::exists(filename))
        {
            continue;
        }

        if (mapnik::detail::is_csv(filename) || mapnik::detail::is_geojson(filename))
        {
            files_to_process.push_back(filename);
        }
    }

    if (files_to_process.size() == 0)
    {
        std::clog << "no files to index" << std::endl;
        return EXIT_FAILURE;
    }

    std::clog << "max tree depth:" << depth << std::endl;
    std::clog << "split ratio:" << ratio << std::endl;

    using box_type = mapnik::box2d<float>;
    using item_type = std::pair<box_type, std::pair<std::size_t, std::size_t>>;

    for (auto const& filename : files_to_process)
    {
        if (!mapnik::util::exists(filename))
        {
            std::clog << "Error : file " << filename << " does not exist" << std::endl;
            continue;
        }

        std::vector<item_type> boxes;
        box_type extent;
        if (mapnik::detail::is_csv(filename))
        {
            std::clog << "processing '" << filename << "' as CSV\n";
            auto result = mapnik::detail::process_csv_file(boxes, filename, manual_headers, separator, quote);
            if (!result.first)
            {
                std::clog << "Error: failed to process " << filename << std::endl;
                return EXIT_FAILURE;
            }
            extent = result.second;
        }
        else if (mapnik::detail::is_geojson(filename))
        {
            std::clog << "processing '" << filename << "' as GeoJSON\n";
            auto result = mapnik::detail::process_geojson_file(boxes, filename, validate_features, verbose);
            if (!result.first)
            {
                std::clog << "Error: failed to process " << filename << std::endl;
                return EXIT_FAILURE;
            }
            extent = result.second;
        }

        if (extent.valid())
        {
            std::clog << extent << std::endl;
            mapnik::box2d<double> extent_d(extent.minx(), extent.miny(), extent.maxx(), extent.maxy());
            mapnik::quad_tree<std::pair<std::size_t, std::size_t>> tree(extent_d, depth, ratio);
            for (auto const& item : boxes)
            {
                auto ext_f = std::get<0>(item);
                tree.insert(std::get<1>(item), mapnik::box2d<double>(ext_f.minx(), ext_f.miny(), ext_f.maxx(), ext_f.maxy()));
            }

            std::fstream file((filename + ".index").c_str(),
                              std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
            if (!file)
            {
                std::clog << "cannot open index file for writing file \""
                          << (filename + ".index") << "\"" << std::endl;
            }
            else
            {
                tree.trim();
                std::clog <<  "number nodes=" << tree.count() << std::endl;
                std::clog <<  "number element=" << tree.count_items() << std::endl;
                file.exceptions(std::ios::failbit | std::ios::badbit);
                tree.write(file);
                file.flush();
                file.close();
            }
        }
        else
        {
            std::clog << "Invalid extent " << extent << std::endl;
            return EXIT_FAILURE;
        }
    }
    std::clog << "done!" << std::endl;
    return EXIT_SUCCESS;
}
