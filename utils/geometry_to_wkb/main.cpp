/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <string>

#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/util/geometry_to_wkb.hpp>

#include <boost/foreach.hpp>


int main (int argc, char ** argv )
{

    if ( argc !=2)
    {
        std::cerr << "Usage: " << argv[0] << " <path-to-shapefile>\n";
        return EXIT_SUCCESS;
    }

    std::cerr << "Geometry to WKB converter\n";

    mapnik::datasource_cache::instance().register_datasources("/opt/mapnik/lib/mapnik/input/");

    std::string filename(argv[1]);
    std::cerr << filename << std::endl;

    mapnik::parameters p;
    p["type"] = "shape";
    p["file"] = filename;

    mapnik::datasource_ptr ds;

    try
    {
        ds = mapnik::datasource_cache::instance().create(p);
    }
    catch ( ... )
    {
        std::cerr << "Can't create datasource!\n";
        return EXIT_FAILURE;
    }

    if (ds)
    {
        std::cerr << ds->envelope() << std::endl;

        mapnik::query q(ds->envelope());
        mapnik::layer_descriptor layer_desc = ds->get_descriptor();
        BOOST_FOREACH ( mapnik::attribute_descriptor const& attr_desc, layer_desc.get_descriptors())
        {
            q.add_property_name(attr_desc.get_name());
        }

        mapnik::featureset_ptr fs = ds->features(q);
        mapnik::feature_ptr f = fs->next();

        while(f)
        {
            std::cerr << *f << std::endl;
            boost::ptr_vector<mapnik::geometry_type> & paths = f->paths();
            BOOST_FOREACH ( mapnik::geometry_type const& geom, paths)
            {
                // NDR
                {
                    mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb(geom,mapnik::util::wkbNDR);
                    std::cerr << mapnik::util::to_hex(wkb->buffer(),wkb->size()) << std::endl;
                }
                // XDR
                {
                    mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb(geom,mapnik::util::wkbXDR);
                    std::cerr << mapnik::util::to_hex(wkb->buffer(),wkb->size()) << std::endl;
                }
            }

            f = fs->next();
        }
    }



    return EXIT_SUCCESS;
}
