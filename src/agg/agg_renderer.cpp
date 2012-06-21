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
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/map.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_filter.hpp>
// agg
#define AGG_RENDERING_BUFFER row_ptr_cache<int8u>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"

// boost
#include <boost/utility.hpp>
#include <boost/make_shared.hpp>
#include <boost/math/special_functions/round.hpp>

// stl
#include <cmath>

namespace mapnik
{
class pattern_source : private boost::noncopyable
{
public:
    pattern_source(image_data_32 const& pattern)
        : pattern_(pattern) {}

    unsigned int width() const
    {
        return pattern_.width();
    }
    unsigned int height() const
    {
        return pattern_.height();
    }
    agg::rgba8 pixel(int x, int y) const
    {
        unsigned c = pattern_(x,y);
        return agg::rgba8(c & 0xff,
                          (c >> 8) & 0xff,
                          (c >> 16) & 0xff,
                          (c >> 24) & 0xff);
    }
private:
    image_data_32 const& pattern_;
};


template <typename T>
agg_renderer<T>::agg_renderer(Map const& m, T & pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      pixmap_(pixmap),
      internal_buffer_(),
      current_buffer_(&pixmap),      
      style_level_compositing_(false),
      width_(pixmap_.width()),
      height_(pixmap_.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(),
      font_manager_(font_engine_),
      detector_(boost::make_shared<label_collision_detector4>(box2d<double>(-m.buffer_size(), -m.buffer_size(), m.width() + m.buffer_size() ,m.height() + m.buffer_size()))),
      ras_ptr(new rasterizer)
{
    setup(m);
}

template <typename T>
agg_renderer<T>::agg_renderer(Map const& m, T & pixmap, boost::shared_ptr<label_collision_detector4> detector,
                              double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      pixmap_(pixmap),
      internal_buffer_(),
      current_buffer_(&pixmap),
      style_level_compositing_(false),
      width_(pixmap_.width()),
      height_(pixmap_.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(),
      font_manager_(font_engine_),
      detector_(detector),
      ras_ptr(new rasterizer)
{
    setup(m);
}

template <typename T>
void agg_renderer<T>::setup(Map const &m)
{
    boost::optional<color> const& bg = m.background();
    if (bg) pixmap_.set_background(*bg);

    boost::optional<std::string> const& image_filename = m.background_image();
    if (image_filename)
    {
        boost::optional<mapnik::marker_ptr> bg_marker = mapnik::marker_cache::instance()->find(*image_filename,true);
        if (bg_marker && (*bg_marker)->is_bitmap())
        {
            mapnik::image_ptr bg_image = *(*bg_marker)->get_bitmap_data();
            int w = bg_image->width();
            int h = bg_image->height();
            if ( w > 0 && h > 0)
            {
                // repeat background-image both vertically and horizontally
                unsigned x_steps = unsigned(std::ceil(width_/double(w)));
                unsigned y_steps = unsigned(std::ceil(height_/double(h)));
                for (unsigned x=0;x<x_steps;++x)
                {
                    for (unsigned y=0;y<y_steps;++y)
                    {
                        composite(pixmap_.data(),*bg_image, src_over, 1.0f, x*w, y*h, true);
                    }
                }
            }
        }
    }

    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Scale=" << m.scale();
}

template <typename T>
agg_renderer<T>::~agg_renderer() {}

template <typename T>
void agg_renderer<T>::start_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start map processing bbox=" << map.get_current_extent();

    ras_ptr->clip_box(0,0,width_,height_);
}

template <typename T>
void agg_renderer<T>::end_map_processing(Map const& )
{

    agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
    agg::pixfmt_rgba32 pixf(buf);
    pixf.demultiply();
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End map processing";
}

template <typename T>
void agg_renderer<T>::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start processing layer=" << lay.name();
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: -- datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: -- query_extent=" << query_extent;

    if (lay.clear_label_cache())
    {
        detector_->clear();
    }
    query_extent_ = query_extent;
}

template <typename T>
void agg_renderer<T>::end_layer_processing(layer const&)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End layer processing";
}

template <typename T>
void agg_renderer<T>::start_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start processing style";
    if (st.comp_op() || st.image_filters().size() > 0 || st.get_opacity() < 1)
    {
        style_level_compositing_ = true;
    }
    else
    {
        style_level_compositing_ = false;
    }
    
    if (style_level_compositing_)
    {
        if (!internal_buffer_)
        {
            internal_buffer_ = boost::make_shared<buffer_type>(pixmap_.width(),pixmap_.height()); 
        }
        else
        {
            internal_buffer_->set_background(color(0,0,0,0)); // fill with transparent colour        
        }
        current_buffer_ = internal_buffer_.get();
    }
    else
    {
        current_buffer_ = &pixmap_;
    }
}

template <typename T>
void agg_renderer<T>::end_style_processing(feature_type_style const& st)
{
    if (style_level_compositing_)
    {
        bool blend_from = false;
        if (st.image_filters().size() > 0)
        {
            blend_from = true;
            mapnik::filter::filter_visitor<image_32> visitor(*current_buffer_);
            BOOST_FOREACH(mapnik::filter::filter_type filter_tag, st.image_filters())
            {
                boost::apply_visitor(visitor, filter_tag);
            }
        }

        if (st.comp_op())
        {
            composite(pixmap_.data(),current_buffer_->data(), *st.comp_op(), st.get_opacity(), 0, 0, false);
        }
        else if (blend_from || st.get_opacity() < 1)
        {
            composite(pixmap_.data(),current_buffer_->data(), src_over, st.get_opacity(), 0, 0, false);
        }

        // apply any 'direct' image filters
        mapnik::filter::filter_visitor<image_32> visitor(pixmap_);
        BOOST_FOREACH(mapnik::filter::filter_type filter_tag, st.direct_image_filters())
        {
            boost::apply_visitor(visitor, filter_tag);
        }
    }
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End processing style";
}

template <typename T>
void agg_renderer<T>::render_marker(pixel_position const& pos, marker const& marker, agg::trans_affine const& tr, 
                                    double opacity, composite_mode_e comp_op)
{
    if (marker.is_vector())
    {
        typedef agg::rgba8 color_type;
        typedef agg::order_rgba order_type;
        typedef agg::pixel32_type pixel_type;
        typedef agg::comp_op_adaptor_rgba<color_type, order_type> blender_type; // comp blender
        typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
        typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
        typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;

        ras_ptr->reset();
        ras_ptr->gamma(agg::gamma_power());
        agg::scanline_u8 sl;
        agg::rendering_buffer buf(current_buffer_->raw_data(), width_, height_, width_ * 4);
        pixfmt_comp_type pixf(buf);
        pixf.comp_op(static_cast<agg::comp_op_e>(comp_op));
        renderer_base renb(pixf);
        
        box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box();
        coord<double,2> c = bbox.center();
        // center the svg marker on '0,0'
        agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
        // apply symbol transformation to get to map space
        mtx *= tr;
        mtx *= agg::trans_affine_scaling(scale_factor_);
        // render the marker at the center of the marker box
        mtx.translate(pos.x, pos.y);
        using namespace mapnik::svg;
        vertex_stl_adapter<svg_path_storage> stl_storage((*marker.get_vector_data())->source());
        svg_path_adapter svg_path(stl_storage);
        svg_renderer<svg_path_adapter,
            agg::pod_bvector<path_attributes>,
            renderer_type,
            agg::pixfmt_rgba32> svg_renderer(svg_path,
                                                   (*marker.get_vector_data())->attributes());
        
        svg_renderer.render(*ras_ptr, sl, renb, mtx, opacity, bbox);
    }
    else
    {        
        double cx = 0.5 * (*marker.get_bitmap_data())->width();
        double cy = 0.5 * (*marker.get_bitmap_data())->height();
        composite(current_buffer_->data(), **marker.get_bitmap_data(), 
                  comp_op, opacity,
                  boost::math::iround(pos.x - cx),
                  boost::math::iround(pos.y - cy),
                  false);
    }
}

template <typename T>
void agg_renderer<T>::painted(bool painted)
{
    pixmap_.painted(painted);
}

template <typename T>
void agg_renderer<T>::debug_draw_box(box2d<double> const& box,
                                     double x, double y, double angle)
{
    agg::rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_ * 4);
    debug_draw_box(buf, box, x, y, angle);
}

template <typename T> template <typename R>
void agg_renderer<T>::debug_draw_box(R& buf, box2d<double> const& box,
                                     double x, double y, double angle)
{
    typedef agg::pixfmt_rgba32 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;

    agg::scanline_p8 sl_line;
    pixfmt pixf(buf);
    renderer_base renb(pixf);
    renderer_type ren(renb);

    // compute tranformation matrix
    agg::trans_affine_rotation tr(angle);
    tr.translate(x, y);

    // prepare path
    agg::path_storage pbox;
    pbox.start_new_path();
    pbox.move_to(box.minx(), box.miny());
    pbox.line_to(box.maxx(), box.miny());
    pbox.line_to(box.maxx(), box.maxy());
    pbox.line_to(box.minx(), box.maxy());
    pbox.line_to(box.minx(), box.miny());

    // prepare stroke with applied transformation
    typedef agg::conv_transform<agg::path_storage> conv_transform;
    typedef agg::conv_stroke<conv_transform> conv_stroke;
    conv_transform tbox(pbox, tr);
    conv_stroke sbox(tbox);
    sbox.generator().width(1.0 * scale_factor_);

    // render the outline
    ras_ptr->reset();
    ras_ptr->add_path(sbox);
    ren.color(agg::rgba8(0x33, 0x33, 0xff, 0xcc)); // blue is fine
    agg::render_scanlines(*ras_ptr, sl_line, ren);
}

template class agg_renderer<image_32>;
template void agg_renderer<image_32>::debug_draw_box<agg::rendering_buffer>(
                agg::rendering_buffer& buf,
                box2d<double> const& box,
                double x, double y, double angle);
}
