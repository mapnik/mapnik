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

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "compare_images.hpp"

// stl
#include <sstream>
#include <iomanip>

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/agg_renderer.hpp>
#if defined(HAVE_CAIRO)
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_image_util.hpp>
#endif

// boost
#include <boost/filesystem.hpp>

namespace visual_tests
{

template <typename ImageType>
struct renderer_base
{
    using image_type = ImageType;

    unsigned compare(image_type const & actual, boost::filesystem::path const& reference) const
    {
        return compare_images(actual, reference.string());
    }

    void save(image_type const & image, boost::filesystem::path const& path) const
    {
        mapnik::save_to_file(image, path.string(), "png32");
    }
};

struct agg_renderer : renderer_base<mapnik::image_rgba8>
{
    static constexpr const char * name = "agg";

    image_type render(mapnik::Map const & map, double scale_factor) const
    {
        image_type image(map.width(), map.height());
        mapnik::agg_renderer<image_type> ren(map, image, scale_factor);
        ren.apply();
        return image;
    }
};

#if defined(HAVE_CAIRO)
struct cairo_renderer : renderer_base<mapnik::image_rgba8>
{
    static constexpr const char * name = "cairo";

    image_type render(mapnik::Map const & map, double scale_factor) const
    {
        mapnik::cairo_surface_ptr image_surface(
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map.width(), map.height()),
            mapnik::cairo_surface_closer());
        mapnik::cairo_ptr image_context(mapnik::create_context(image_surface));
        mapnik::cairo_renderer<mapnik::cairo_ptr> ren(map, image_context, scale_factor);
        ren.apply();
        image_type image(map.width(), map.height());
        mapnik::cairo_image_to_rgba8(image, image_surface);
        return image;
    }
};
#endif

struct grid_renderer : renderer_base<mapnik::image_gray8>
{
    static constexpr const char * name = "grid";

    image_type render(mapnik::Map const & map, double scale_factor) const
    {
        image_type image(map.width(), map.height());
        // TODO: Render grid here.
        return image;
    }
};

template <typename Renderer>
class renderer
{
public:
    renderer(boost::filesystem::path const & output_dir, boost::filesystem::path const & reference_dir, bool overwrite)
        : output_dir(output_dir), reference_dir(reference_dir), overwrite(overwrite)
    {
    }

    result test(std::string const & name, mapnik::Map const & map, double scale_factor) const
    {
        typename Renderer::image_type image(ren.render(map, scale_factor));
        boost::filesystem::path reference = reference_dir / image_file_name(name, map.width(), map.height(), scale_factor, true);
        bool reference_exists = boost::filesystem::exists(reference);
        result res;

        res.state = reference_exists ? STATE_OK : STATE_OVERWRITE;
        res.name = name;
        res.renderer_name = Renderer::name;
        res.scale_factor = scale_factor;
        res.size = map_size(map.width(), map.height());
        res.reference_image_path = reference;
        res.diff = reference_exists ? ren.compare(image, reference) : 0;

        if (res.diff)
        {
            boost::filesystem::create_directories(output_dir);
            boost::filesystem::path path = output_dir / image_file_name(name, map.width(), map.height(), scale_factor);
            res.actual_image_path = path;
            res.state = STATE_FAIL;
            ren.save(image, path);
        }

        if ((res.diff && overwrite) || !reference_exists)
        {
            ren.save(image, reference);
            res.state = STATE_OVERWRITE;
        }

        return res;
    }

private:
    std::string image_file_name(std::string const & test_name,
                                double width,
                                double height,
                                double scale_factor,
                                bool reference=false) const
    {
        std::stringstream s;
        s << test_name << '-' << width << '-' << height << '-'
          << std::fixed << std::setprecision(1) << scale_factor
          << '-' << Renderer::name << (reference ? "-reference" : "") << ".png";
        return s.str();
    }

    Renderer ren;
    boost::filesystem::path const & output_dir;
    boost::filesystem::path const & reference_dir;
    const bool overwrite;
};

}

#endif
