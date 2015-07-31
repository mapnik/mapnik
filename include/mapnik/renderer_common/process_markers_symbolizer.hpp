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

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_MARKERS_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_MARKERS_SYMBOLIZER_HPP

#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/geometry_type.hpp>

namespace mapnik {

template <typename VD, typename RD, typename RendererType, typename ContextType>
struct render_marker_symbolizer_visitor
{
    using vector_dispatch_type = VD;
    using raster_dispatch_type = RD;
    using buffer_type = typename std::tuple_element<0,ContextType>::type;

    using vertex_converter_type = vertex_converter<clip_line_tag,
                                                   clip_poly_tag,
                                                   transform_tag,
                                                   affine_transform_tag,
                                                   simplify_tag,
                                                   smooth_tag,
                                                   offset_transform_tag>;

    render_marker_symbolizer_visitor(std::string const& filename,
                                     markers_symbolizer const& sym,
                                     mapnik::feature_impl & feature,
                                     proj_transform const& prj_trans,
                                     RendererType const& common,
                                     box2d<double> const& clip_box,
                                     ContextType const& renderer_context)
        : filename_(filename),
          sym_(sym),
          feature_(feature),
          prj_trans_(prj_trans),
          common_(common),
          clip_box_(clip_box),
          renderer_context_(renderer_context) {}

    void operator() (marker_null const&) {}

    void operator() (marker_svg const& mark)
    {
        using namespace mapnik::svg;
        bool clip = get<value_bool, keys::clip>(sym_, feature_, common_.vars_);
        double offset = get<value_double, keys::offset>(sym_, feature_, common_.vars_);
        double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym_, feature_, common_.vars_);
        double smooth = get<value_double, keys::smooth>(sym_, feature_, common_.vars_);

        // https://github.com/mapnik/mapnik/issues/1316
        bool snap_to_pixels = !mapnik::marker_cache::instance().is_uri(filename_);

        agg::trans_affine geom_tr;
        auto transform = get_optional<transform_type>(sym_, keys::geometry_transform);
        if (transform) evaluate_transform(geom_tr, feature_, common_.vars_, *transform, common_.scale_factor_);
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);

        boost::optional<svg_path_ptr> const& stock_vector_marker = mark.get_data();

        // special case for simple ellipse markers
        // to allow for full control over rx/ry dimensions
        if (filename_ == "shape://ellipse"
           && (has_key(sym_,keys::width) || has_key(sym_,keys::height)))
        {
            svg_path_ptr marker_ellipse = std::make_shared<svg_storage_type>();
            vertex_stl_adapter<svg_path_storage> stl_storage(marker_ellipse->source());
            svg_path_adapter svg_path(stl_storage);
            build_ellipse(sym_, feature_, common_.vars_, *marker_ellipse, svg_path);
            svg_attribute_type attributes;
            bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym_, feature_, common_.vars_);
            auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
            if (image_transform) evaluate_transform(image_tr, feature_, common_.vars_, *image_transform);
            vector_dispatch_type rasterizer_dispatch(marker_ellipse,
                                                     svg_path,
                                                     result ? attributes : (*stock_vector_marker)->attributes(),
                                                     image_tr,
                                                     sym_,
                                                     *common_.detector_,
                                                     common_.scale_factor_,
                                                     feature_,
                                                     common_.vars_,
                                                     snap_to_pixels,
                                                     renderer_context_);

            vertex_converter_type converter(clip_box_,
                                            sym_,
                                            common_.t_,
                                            prj_trans_,
                                            geom_tr,
                                            feature_,
                                            common_.vars_,
                                            common_.scale_factor_);
            if (clip) // optional clip (default: true)
            {
                geometry::geometry_types type = geometry::geometry_type(feature_.get_geometry());
                if (type == geometry::geometry_types::Polygon || type == geometry::geometry_types::MultiPolygon)
                    converter.template set<clip_poly_tag>();
                else if (type == geometry::geometry_types::LineString || type == geometry::geometry_types::MultiLineString)
                    converter.template set<clip_line_tag>();
            }

            converter.template set<transform_tag>(); //always transform
            if (std::fabs(offset) > 0.0) converter.template set<offset_transform_tag>(); // parallel offset
            converter.template set<affine_transform_tag>(); // optional affine transform
            if (simplify_tolerance > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
            if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
            apply_markers_multi(feature_, common_.vars_, converter, rasterizer_dispatch, sym_);
        }
        else
        {
            box2d<double> const& bbox = mark.bounding_box();
            setup_transform_scaling(image_tr, bbox.width(), bbox.height(), feature_, common_.vars_, sym_);
            auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
            if (image_transform) evaluate_transform(image_tr, feature_, common_.vars_, *image_transform);
            vertex_stl_adapter<svg_path_storage> stl_storage((*stock_vector_marker)->source());
            svg_path_adapter svg_path(stl_storage);
            svg_attribute_type attributes;
            bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym_, feature_, common_.vars_);
            vector_dispatch_type rasterizer_dispatch(*stock_vector_marker,
                                                     svg_path,
                                                     result ? attributes : (*stock_vector_marker)->attributes(),
                                                     image_tr,
                                                     sym_,
                                                     *common_.detector_,
                                                     common_.scale_factor_,
                                                     feature_,
                                                     common_.vars_,
                                                     snap_to_pixels,
                                                     renderer_context_);

            vertex_converter_type converter(clip_box_,
                                            sym_,
                                            common_.t_,
                                            prj_trans_,
                                            geom_tr,
                                            feature_,
                                            common_.vars_,
                                            common_.scale_factor_);
            if (clip) // optional clip (default: true)
            {
                geometry::geometry_types type = geometry::geometry_type(feature_.get_geometry());
                if (type == geometry::geometry_types::Polygon || type == geometry::geometry_types::MultiPolygon)
                    converter.template set<clip_poly_tag>();
                else if (type == geometry::geometry_types::LineString || type == geometry::geometry_types::MultiLineString)
                    converter.template set<clip_line_tag>();
            }

            converter.template set<transform_tag>(); //always transform
            if (std::fabs(offset) > 0.0) converter.template set<offset_transform_tag>(); // parallel offset
            converter.template set<affine_transform_tag>(); // optional affine transform
            if (simplify_tolerance > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
            if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
            apply_markers_multi(feature_, common_.vars_, converter, rasterizer_dispatch,  sym_);
        }
    }

    void operator() (marker_rgba8 const& mark)
    {
        using namespace mapnik::svg;
        bool clip = get<value_bool, keys::clip>(sym_, feature_, common_.vars_);
        double offset = get<value_double, keys::offset>(sym_, feature_, common_.vars_);
        double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym_, feature_, common_.vars_);
        double smooth = get<value_double, keys::smooth>(sym_, feature_, common_.vars_);

        agg::trans_affine geom_tr;
        auto transform = get_optional<transform_type>(sym_, keys::geometry_transform);
        if (transform) evaluate_transform(geom_tr, feature_, common_.vars_, *transform, common_.scale_factor_);
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);

        setup_transform_scaling(image_tr, mark.width(), mark.height(), feature_, common_.vars_, sym_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform) evaluate_transform(image_tr, feature_, common_.vars_, *image_transform);
        box2d<double> const& bbox = mark.bounding_box();
        mapnik::image_rgba8 const& marker = mark.get_data();
        // - clamp sizes to > 4 pixels of interactivity
        coord2d center = bbox.center();
        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine marker_trans = recenter * image_tr;
        raster_dispatch_type rasterizer_dispatch(marker,
                                                 marker_trans,
                                                 sym_,
                                                 *common_.detector_,
                                                 common_.scale_factor_,
                                                 feature_,
                                                 common_.vars_,
                                                 renderer_context_);


        vertex_converter_type converter(clip_box_,
                                        sym_,
                                        common_.t_,
                                        prj_trans_,
                                        geom_tr,
                                        feature_,
                                        common_.vars_,
                                        common_.scale_factor_);

        if (clip) // optional clip (default: true)
        {
            geometry::geometry_types type = geometry::geometry_type(feature_.get_geometry());
            if (type == geometry::geometry_types::Polygon || type == geometry::geometry_types::MultiPolygon)
                converter.template set<clip_poly_tag>();
            else if (type == geometry::geometry_types::LineString || type == geometry::geometry_types::MultiLineString)
                converter.template set<clip_line_tag>();
        }
        converter.template set<transform_tag>(); //always transform
        if (std::fabs(offset) > 0.0) converter.template set<offset_transform_tag>(); // parallel offset
        converter.template set<affine_transform_tag>(); // optional affine transform
        if (simplify_tolerance > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
        if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
        apply_markers_multi(feature_, common_.vars_, converter, rasterizer_dispatch, sym_);
    }

  private:
    std::string const& filename_;
    markers_symbolizer const& sym_;
    mapnik::feature_impl & feature_;
    proj_transform const& prj_trans_;
    RendererType const& common_;
    box2d<double> const& clip_box_;
    ContextType const& renderer_context_;
};

template <typename VD, typename RD, typename RendererType, typename ContextType>
void render_markers_symbolizer(markers_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans,
                               RendererType const& common,
                               box2d<double> const& clip_box,
                               ContextType const& renderer_context)
{
    using namespace mapnik::svg;
    std::string filename = get<std::string>(sym, keys::file, feature, common.vars_, "shape://ellipse");
    if (!filename.empty())
    {
        std::shared_ptr<mapnik::marker const> mark = mapnik::marker_cache::instance().find(filename, true);
        render_marker_symbolizer_visitor<VD,RD,RendererType,ContextType> visitor(filename,
                                                                                 sym,
                                                                                 feature,
                                                                                 prj_trans,
                                                                                 common,
                                                                                 clip_box,
                                                                                 renderer_context);
        util::apply_visitor(visitor, *mark);
    }
}

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_MARKERS_SYMBOLIZER_HPP
