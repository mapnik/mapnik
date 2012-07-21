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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/markers_symbolizer.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_path_storage.h"
#include "agg_conv_clip_polyline.h"
#include "agg_conv_transform.h"

// boost
#include <boost/optional.hpp>

namespace mapnik {

template <typename BufferType, typename SvgRenderer, typename Rasterizer, typename Detector>
struct markers_rasterizer_dispatch
{
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;

    markers_rasterizer_dispatch(BufferType & image_buffer,
                                SvgRenderer & svg_renderer,
                                Rasterizer & ras,
                                box2d<double> const& bbox,
                                agg::trans_affine const& marker_trans,
                                markers_symbolizer const& sym,
                                Detector & detector,
                                double scale_factor)
        : buf_(image_buffer.raw_data(), image_buffer.width(), image_buffer.height(), image_buffer.width() * 4),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(svg_renderer),
        ras_(ras),
        bbox_(bbox),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(sym_.comp_op()));
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();

        if (placement_method == MARKER_POINT_PLACEMENT)
        {

            double x,y;
            path.rewind(0);
            label::interior_position(path, x, y);
            agg::trans_affine matrix = marker_trans_;
            matrix.translate(x,y);
            box2d<double> transformed_bbox = bbox_ * matrix;

            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {
                svg_renderer_.render(ras_, sl_, renb_, matrix, sym_.get_opacity(), bbox_);

                if (!sym_.get_ignore_placement())
                    detector_.insert(transformed_bbox);
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      sym_.get_spacing() * scale_factor_,
                                                                      sym_.get_max_error(),
                                                                      sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x, y);
                svg_renderer_.render(ras_, sl_, renb_, matrix, sym_.get_opacity(), bbox_);
            }
        }
    }

private:
    agg::scanline_u8 sl_;
    agg::rendering_buffer buf_;
    pixfmt_comp_type pixf_;
    renderer_base renb_;
    SvgRenderer & svg_renderer_;
    Rasterizer & ras_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
};


template <typename T>
void agg_renderer<T>::process(markers_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;

    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance()->find(filename, true);
        if (mark && *mark)
        {
            if (!(*mark)->is_vector())
            {
                MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: markers_symbolizer does not yet support non-SVG markers";
                return;
            }

            ras_ptr->reset();
            ras_ptr->gamma(agg::gamma_power());

            agg::trans_affine geom_tr;
            evaluate_transform(geom_tr, feature, sym.get_transform());

            boost::optional<path_ptr> marker = (*mark)->get_vector_data();
            box2d<double> const& bbox = (*marker)->bounding_box();

            agg::trans_affine tr;
            setup_label_transform(tr, bbox, feature, sym);
            tr = agg::trans_affine_scaling(scale_factor_) * tr;

            coord2d center = bbox.center();
            agg::trans_affine_translation recenter(-center.x, -center.y);
            agg::trans_affine marker_trans = recenter * tr;

            using namespace mapnik::svg;
            vertex_stl_adapter<svg_path_storage> stl_storage((*marker)->source());
            svg_path_adapter svg_path(stl_storage);

            agg::pod_bvector<path_attributes> attributes;
            bool result = push_explicit_style( (*marker)->attributes(), attributes, sym);

            typedef label_collision_detector4 detector_type;
            typedef svg_renderer<svg_path_adapter,
                                 agg::pod_bvector<path_attributes>,
                                 renderer_type,
                                 agg::pixfmt_rgba32 > svg_renderer_type;
            typedef markers_rasterizer_dispatch<buffer_type, svg_renderer_type, rasterizer, detector_type>  markers_rasterizer_dispatch_type;
            typedef boost::mpl::vector<clip_line_tag,transform_tag,smooth_tag> conv_types;

            svg_renderer_type svg_renderer(svg_path, result ? attributes : (*marker)->attributes());

            markers_rasterizer_dispatch_type rasterizer_dispatch(*current_buffer_,svg_renderer,*ras_ptr,
                                                                 bbox, marker_trans, sym, *detector_, scale_factor_);


            vertex_converter<box2d<double>, markers_rasterizer_dispatch_type, markers_symbolizer,
                             CoordTransform, proj_transform, agg::trans_affine, conv_types>
                converter(query_extent_* 1.1,rasterizer_dispatch, sym,t_,prj_trans,tr,scale_factor_);

            if (sym.clip()) converter.template set<clip_line_tag>(); //optional clip (default: true)
            converter.template set<transform_tag>(); //always transform
            if (sym.smooth() > 0.0) converter.template set<smooth_tag>(); // optional smooth converter

            BOOST_FOREACH(geometry_type & geom, feature.paths())
            {
                converter.apply(geom);
            }
        }
    }
}

template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
