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
#include <mapnik/util/fs.hpp>
#include <mapnik/quad_tree.hpp>
#include <mapnik/geometry/envelope.hpp>
#include "shapefile.hpp"
#include "shape_io.hpp"
#include "shape_index_featureset.hpp"
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

const int DEFAULT_DEPTH = 8;
const double DEFAULT_RATIO=0.55;

int main (int argc,char** argv)
{
    using namespace mapnik;
    namespace po = boost::program_options;

    bool verbose=false;
    bool index_parts = false;
    unsigned int depth = DEFAULT_DEPTH;
    double ratio = DEFAULT_RATIO;
    std::vector<std::string> shape_files;

    try
    {
        po::options_description desc("shapeindex utility");
        desc.add_options()
            ("help,h", "produce usage message")
            ("version,V","print version string")
            ("index-parts","index individual shape parts (default: no)")
            ("verbose,v","verbose output")
            ("depth,d", po::value<unsigned int>(), "max tree depth\n(default 8)")
            ("ratio,r",po::value<double>(),"split ratio (default 0.55)")
            ("shape_files",po::value<std::vector<std::string> >(),"shape files to index: file1 file2 ...fileN")
            ;

        po::positional_options_description p;
        p.add("shape_files",-1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("version"))
        {
            std::clog << "version 0.3.0" <<std::endl;
            return EXIT_FAILURE;
        }

        if (vm.count("help"))
        {
            std::clog << desc << std::endl;
            return EXIT_FAILURE;
        }
        if (vm.count("verbose"))
        {
            verbose = true;
        }
        if (vm.count("index-parts"))
        {
            index_parts = true;
        }
        if (vm.count("depth"))
        {
            depth = vm["depth"].as<unsigned int>();
        }
        if (vm.count("ratio"))
        {
            ratio = vm["ratio"].as<double>();
        }

        if (vm.count("shape_files"))
        {
            shape_files=vm["shape_files"].as< std::vector<std::string> >();
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::clog << "max tree depth:" << depth << std::endl;
    std::clog << "split ratio:" << ratio << std::endl;

    if (shape_files.size() == 0)
    {
        std::clog << "no shape files to index" << std::endl;
        return EXIT_FAILURE;
    }
    for (auto const& filename : shape_files)
    {
        std::clog << "processing " << filename << std::endl;
        std::string shapename (filename);
        boost::algorithm::ireplace_last(shapename,".shp","");
        std::string shapename_full (shapename + ".shp");
        std::string shxname(shapename + ".shx");
        if (! mapnik::util::exists (shapename_full))
        {
            std::clog << "Error : file " << shapename_full << " does not exist" << std::endl;
            continue;
        }
        if (! mapnik::util::exists(shxname))
        {
            std::clog << "Error : shapefile index file (*.shx) " << shxname << " does not exist" << std::endl;
            continue;
        }
        shape_file shp (shapename_full);

        if (! shp.is_open())
        {
            std::clog << "Error : cannot open " << shapename_full << std::endl;
            continue;
        }

        shape_file shx (shxname);
        if (!shx.is_open())
        {
            std::clog << "Error : cannot open " << shxname << std::endl;
            continue;
        }

        int code = shx.read_xdr_integer(); //file_code == 9994
        std::clog << code << std::endl;
        shx.skip(5*4);

        int file_length=shx.read_xdr_integer();
        int version=shx.read_ndr_integer();
        int shape_type=shx.read_ndr_integer();
        box2d<double> extent;
        shx.read_envelope(extent);


        std::clog << "length=" << file_length << std::endl;
        std::clog << "version=" << version << std::endl;
        std::clog << "type=" << shape_type << std::endl;
        std::clog << "extent:" << extent << std::endl;

        if (!extent.valid() || std::isnan(extent.width()) || std::isnan(extent.height()))
        {
            std::clog << "Invalid extent aborting..." << std::endl;
            return EXIT_FAILURE;
        }
        int pos = 50;
        shx.seek(pos * 2);
        mapnik::quad_tree<mapnik::detail::node> tree(extent, depth, ratio);
        int count = 0;

        if (shape_type != shape_io::shape_null)
        {
            while (shx.is_good() && pos <= file_length - 4)
            {
                int offset = shx.read_xdr_integer();
                int shx_content_length = shx.read_xdr_integer();
                pos += 4;
                box2d<double> item_ext;
                shp.seek(offset * 2);
                int record_number = shp.read_xdr_integer();
                int shp_content_length = shp.read_xdr_integer();
                if (shx_content_length != shp_content_length)
                {
                    std::clog << "Content length mismatch for record number " << record_number << std::endl;
                    continue;
                }
                shape_type = shp.read_ndr_integer();

                if (shape_type == shape_io::shape_null) continue;

                if (shape_type==shape_io::shape_point
                    || shape_type==shape_io::shape_pointm
                    || shape_type == shape_io::shape_pointz)
                {
                    double x=shp.read_double();
                    double y=shp.read_double();
                    item_ext=box2d<double>(x,y,x,y);
                }
                else if (index_parts &&
                         (shape_type == shape_io::shape_polygon || shape_type == shape_io::shape_polygonm || shape_type == shape_io::shape_polygonz
                          || shape_type == shape_io::shape_polyline || shape_type == shape_io::shape_polylinem || shape_type == shape_io::shape_polylinez))
                {
                    shp.read_envelope(item_ext);
                    int num_parts = shp.read_ndr_integer();
                    int num_points = shp.read_ndr_integer();
                    std::vector<int> parts;
                    parts.resize(num_parts);
                    std::for_each(parts.begin(), parts.end(), [&](int & part) { part = shp.read_ndr_integer();});
                    for (int k = 0; k < num_parts; ++k)
                    {
                        int start = parts[k];
                        int end;
                        if (k == num_parts - 1) end = num_points;
                        else end = parts[k + 1];

                        mapnik::geometry::linear_ring<double> ring;
                        ring.reserve(end - start);
                        for (int j = start; j < end; ++j)
                        {
                            double x = shp.read_double();
                            double y = shp.read_double();
                            ring.emplace_back(x, y);
                        }
                        item_ext = mapnik::geometry::envelope(ring);
                        if (item_ext.valid())
                        {
                            if (verbose)
                            {
                                std::clog << "record number " << record_number << " box=" << item_ext << std::endl;
                            }
                            tree.insert(mapnik::detail::node(offset * 2, start, end),item_ext);
                            ++count;
                        }
                    }
                    item_ext = mapnik::box2d<double>(); //invalid
                }
                else
                {
                    shp.read_envelope(item_ext);
                }

                if (item_ext.valid())
                {
                    if (verbose)
                    {
                        std::clog << "record number " << record_number << " box=" << item_ext << std::endl;
                    }
                    tree.insert(mapnik::detail::node(offset * 2,-1,0),item_ext);
                    ++count;
                }
            }
        }

        if (count > 0)
        {
            std::clog << " number shapes=" << count << std::endl;
            std::ofstream file((shapename+".index").c_str(), std::ios::trunc | std::ios::binary);
            if (!file)
            {
                std::clog << "cannot open index file for writing file \""
                          << (shapename+".index") << "\"" << std::endl;
            }
            else
            {
                tree.trim();
                std::clog << " number nodes=" << tree.count() << std::endl;
                file.exceptions(std::ios::failbit | std::ios::badbit);
                tree.write(file);
                file.flush();
                file.close();
            }
        }
        else
        {
            std::clog << "No non-empty geometries in shapefile" << std::endl;
        }
    }

    std::clog << "done!" << std::endl;
    return EXIT_SUCCESS;
}
