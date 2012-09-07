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

// stl
#include <iostream>
#include <cmath>

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/projection.hpp>

int main(int argc,char *argv[])
{
    if (argc < 6)
    {
        std::cerr << "Usage: render XMLfile w s e n [OSMfile]" << std::endl;
        exit(0);
    }

    mapnik::datasource_cache::instance().register_datasources("/usr/local/lib/mapnik/input");
    mapnik::freetype_engine::register_font("/usr/local/lib/mapnik/fonts/DejaVuSans.ttf");

    mapnik::Map m(800, 800);
    mapnik::load_map(m, argv[1]);

    if (argc > 6)
    {
        mapnik::parameters p;
        p["type"] = "osm";
        p["file"] = argv[6];
        for (int count = 0; count < m.layer_count(); count++)
        {
            mapnik::parameters q = m.getLayer(count).datasource()->params();
            m.getLayer(count).set_datasource(mapnik::datasource_cache::instance().create(p));
        }
    }

    mapnik::box2d<double> bbox (atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]));

    m.zoom_to_box(bbox);

    mapnik::image_32 buf (m.width(), m.height());
    mapnik::agg_renderer<mapnik::image_32> r(m, buf);
    r.apply();

    mapnik::save_to_file<mapnik::image_data_32>(buf.data(), "blah.png", "png");

    return 0;
}
