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


#include <iostream>
#include <vector>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <mapnik/datasource.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/feature_layer_desc.hpp>

#include "../shapeindex/quadtree.hpp"

#include "ogr_converter.cpp"
#include "ogr_datasource.cpp"
#include "ogr_featureset.cpp"
#include "ogr_index_featureset.cpp"

using mapnik::datasource_exception;

const int MAXDEPTH = 64;
const int DEFAULT_DEPTH = 8;
const double MINRATIO=0.5;
const double MAXRATIO=0.8;
const double DEFAULT_RATIO=0.55;

int main (int argc,char** argv)
{
    using namespace mapnik;
    namespace po = boost::program_options;
    using std::string;
    using std::vector;

    bool verbose=false;
    unsigned int depth=DEFAULT_DEPTH;
    double ratio=DEFAULT_RATIO;
    vector<string> ogr_files;

    try
    {
        po::options_description desc("ogrindex utility");
        desc.add_options()
            ("help,h",     "produce usage message")
            ("version,V",  "print version string")
            ("verbose,v",  "verbose output")
            ("depth,d",    po::value<unsigned int>(), "max tree depth\n(default 8)")
            ("ratio,r",    po::value<double>(), "split ratio (default 0.55)")
            ("ogr_files",  po::value<vector<string> >(), "ogr supported files to index: file1 file2 ...fileN")
            ;

        po::positional_options_description p;
        p.add("ogr_files",-1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("version"))
        {
            std::clog<<"version 0.1.0" <<std::endl;
            return 1;
        }
        if (vm.count("help"))
        {
            std::clog << desc << std::endl;
            return 1;
        }
        if (vm.count("depth"))
        {
            depth = vm["depth"].as<unsigned int>();
        }
        if (vm.count("ratio"))
        {
            ratio = vm["ratio"].as<double>();
        }
        if (vm.count("ogr_files"))
        {
            ogr_files=vm["ogr_files"].as< vector<string> >();
        }
    }
    catch (...)
    {
        std::clog << "Exception of unknown type!" << std::endl;
        return -1;
    }

    std::clog << "max tree depth:" << depth << std::endl;
    std::clog << "split ratio:" << ratio << std::endl;

    vector<string>::const_iterator itr = ogr_files.begin();
    if (itr == ogr_files.end())
    {
        std::clog << "no ogr files to index" << std::endl;
        return 0;
    }
    while (itr != ogr_files.end())
    {
        std::clog << "processing " << *itr << std::endl;

        std::string ogrname (*itr++);

        if (! mapnik::util::exists (ogrname))
        {
            std::clog << "error : file " << ogrname << " doesn't exists" << std::endl;
            continue;
        }

        // TODO - layer names don't match dataset name, so this will break for
        // any layer types of ogr than shapefiles, etc
        size_t breakpoint = ogrname.find_last_of (".");
        if (breakpoint == string::npos) breakpoint = ogrname.length();
        std::string ogrlayername (ogrname.substr(0, breakpoint));

        if (mapnik::util::exists (ogrlayername + ".ogrindex"))
        {
            std::clog << "error : " << ogrlayername << ".ogrindex file already exists for " << ogrname << std::endl;
            continue;
        }

        mapnik::parameters params;
        params["type"] = "ogr";
        params["file"] = ogrname;
        //unsigned first = 0;
        params["layer_by_index"] = 0;//ogrlayername;

        try
        {
            ogr_datasource ogr (params);


            box2d<double> extent = ogr.envelope();
            quadtree<int> tree (extent, depth, ratio);
            int count=0;

            std::clog << "file:" << ogrname << std::endl;
            std::clog << "layer:" << ogrlayername << std::endl;
            std::clog << "extent:" << extent << std::endl;

            mapnik::query q (extent, 1.0);
            mapnik::featureset_ptr itr = ogr.features (q);

            while (true)
            {
                mapnik::feature_ptr fp = itr->next();
                if (!fp)
                {
                    break;
                }

                box2d<double> item_ext = fp->envelope();

                tree.insert (count, item_ext);
                if (verbose) {
                    std::clog << "record number " << (count + 1) << " box=" << item_ext << std::endl;
                }

                ++count;
            }

            std::clog << " number shapes=" << count << std::endl;

            std::fstream file((ogrlayername+".ogrindex").c_str(),
                              std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
            if (!file) {
                std::clog << "cannot open ogrindex file for writing file \""
                          << (ogrlayername+".ogrindex") << "\"" << std::endl;
            } else {
                tree.trim();
                std::clog<<" number nodes="<<tree.count()<<std::endl;
                file.exceptions(std::ios::failbit | std::ios::badbit);
                tree.write(file);
                file.flush();
                file.close();
            }
        }
        // catch problem at the datasource creation
        catch (const mapnik::datasource_exception & ex )
        {
            std::clog << ex.what() << "\n";
            return -1;
        }

        catch (...)
        {
            std::clog << "unknown exception..." << "\n";
            return -1;
        }

    }

    std::clog << "done!" << std::endl;
    return 0;
}
