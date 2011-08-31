/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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
//$Id$

extern "C"
{
#include <png.h>
}

// boost
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/make_shared.hpp>

// mapnik
#include <mapnik/graphics.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/png_io.hpp>
#include <mapnik/image_reader.hpp>
#include <sstream>

// agg
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"

// jpeg
#if defined(HAVE_JPEG)
#include <mapnik/jpeg_io.hpp>
#endif

// cairo
#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
#include <cairomm/surface.h>
#include <pycairo.h>
#endif

using mapnik::image_32;
using mapnik::image_reader;
using mapnik::get_image_reader;
using mapnik::type_from_filename;
using namespace boost::python;
using mapnik::save_to_file;

// output 'raw' pixels
PyObject* tostring1( image_32 const& im)
{
    int size = im.width() * im.height() * 4;
    return
#if PY_VERSION_HEX >= 0x03000000 
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        ((const char*)im.raw_data(),size);
}

// encode (png,jpeg)
PyObject* tostring2(image_32 const & im, std::string const& format)
{
    std::string s = save_to_string(im, format);
    return
#if PY_VERSION_HEX >= 0x03000000 
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
    (s.data(),s.size());
}

PyObject* tostring3(image_32 const & im, std::string const& format, mapnik::rgba_palette& pal)
{
    std::string s = save_to_string(im, format, pal);
    return
#if PY_VERSION_HEX >= 0x03000000 
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
    (s.data(),s.size());
}

void save_to_file1(mapnik::image_32 const& im, std::string const& filename, std::string const& type)
{
    boost::shared_ptr<mapnik::rgba_palette> palette_ptr;
    mapnik::save_to_file(im,filename,type,*palette_ptr);
}

void save_to_file2(mapnik::image_32 const& im, std::string const& filename)
{
    mapnik::save_to_file(im,filename);
}


boost::shared_ptr<image_32> open_from_file(std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        std::auto_ptr<image_reader> reader(get_image_reader(filename,*type));
        if (reader.get())
        {
            
            boost::shared_ptr<image_32> image_ptr = boost::make_shared<image_32>(reader->width(),reader->height());
            reader->read(0,0,image_ptr->data());
            return image_ptr;
        }
        throw mapnik::image_reader_exception("Failed to load: " + filename);  
    }
    throw mapnik::image_reader_exception("Unsupported image format:" + filename);
}
    
void blend (image_32 & im, unsigned x, unsigned y, image_32 const& im2, float opacity)
{
    im.set_rectangle_alpha2(im2.data(),x,y,opacity);
}

// Compositing modes 
// http://www.w3.org/TR/2009/WD-SVGCompositing-20090430/

enum composite_mode_e
{
    clear = 1,
    src,
    dst,
    src_over,
    dst_over,
    src_in,
    dst_in,
    src_out,
    dst_out,
    src_atop,
    dst_atop,
    _xor,
    plus,
    minus,
    multiply,
    screen,
    overlay,
    darken,
    lighten,
    color_dodge,
    color_burn,
    hard_light,
    soft_light,
    difference,
    exclusion,
    contrast,
    invert,
    invert_rgb
};

void composite(image_32 & im, image_32 & im2, composite_mode_e mode)
{
    typedef agg::rgba8 color;
    typedef agg::order_bgra order;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba<color, order> blender_type;
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_type;
    typedef agg::renderer_base<pixfmt_type> renderer_type;
    typedef agg::comp_op_adaptor_rgba<color, order> blender_type;
    typedef agg::renderer_base<pixfmt_type> renderer_type;
    
    agg::rendering_buffer source(im.raw_data(),im.width(),im.height(),im.width() * 4);
    agg::rendering_buffer mask(im2.raw_data(),im2.width(),im2.height(),im2.width() * 4);

    agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixf(source);
    agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixf_mask(mask);
    
    switch(mode)
    {
    case clear :
        pixf.comp_op(agg::comp_op_clear);
        break;
    case src:
        pixf.comp_op(agg::comp_op_src);
        break;
    case dst:
        pixf.comp_op(agg::comp_op_dst);
        break;
    case src_over:
        pixf.comp_op(agg::comp_op_src_over);
        break;
    case dst_over:
        pixf.comp_op(agg::comp_op_dst_over);
        break;
    case src_in:
        pixf.comp_op(agg::comp_op_src_in);
        break;
    case dst_in:
        pixf.comp_op(agg::comp_op_dst_in);
        break;
    case src_out:
        pixf.comp_op(agg::comp_op_src_out);
        break;
    case dst_out:
        pixf.comp_op(agg::comp_op_dst_out);
        break;
    case src_atop:
        pixf.comp_op(agg::comp_op_src_atop);
        break;
    case dst_atop:
        pixf.comp_op(agg::comp_op_dst_atop);
        break;
    case _xor:
        pixf.comp_op(agg::comp_op_xor);
        break;
    case plus:
        pixf.comp_op(agg::comp_op_plus);
        break;
    case minus:
        pixf.comp_op(agg::comp_op_minus);
        break;
    case multiply:
        pixf.comp_op(agg::comp_op_multiply);
        break;     
    case screen:
        pixf.comp_op(agg::comp_op_screen);
        break;
    case overlay:
        pixf.comp_op(agg::comp_op_overlay);
        break;
    case darken:
        pixf.comp_op(agg::comp_op_darken);
        break;    
    case lighten:
        pixf.comp_op(agg::comp_op_lighten);
        break;
    case color_dodge:
        pixf.comp_op(agg::comp_op_color_dodge);
        break;
    case color_burn:
        pixf.comp_op(agg::comp_op_color_burn);
        break;
    case hard_light:
        pixf.comp_op(agg::comp_op_hard_light);
        break;
    case soft_light:
        pixf.comp_op(agg::comp_op_soft_light);
        break;
    case difference:
        pixf.comp_op(agg::comp_op_difference);
        break;
    case exclusion:
        pixf.comp_op(agg::comp_op_exclusion);
        break;
    case contrast:
        pixf.comp_op(agg::comp_op_contrast);
        break;
    case invert:
        pixf.comp_op(agg::comp_op_invert);
        break;
    case invert_rgb:
        pixf.comp_op(agg::comp_op_invert_rgb);
        break;
    default:
        break;
    
    }
    renderer_type ren(pixf);
    agg::renderer_base<pixfmt_type> rb(pixf);
    rb.blend_from(pixf_mask,0,0,0,255);
}

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
boost::shared_ptr<image_32> from_cairo(PycairoSurface* surface)
{
    Cairo::RefPtr<Cairo::ImageSurface> s(new Cairo::ImageSurface(surface->surface));
    boost::shared_ptr<image_32> image_ptr = boost::make_shared<image_32>(s);
    return image_ptr;
}
#endif

void export_image()
{
    using namespace boost::python;
    enum_<composite_mode_e>("CompositeOp")
        .value("clear",clear)
        .value("src",src)
        .value("dst",dst)
        .value("src_over",src_over)
        .value("dst_over",dst_over)        
        .value("src_in",src_in)
        .value("dst_in",dst_in)
        .value("src_out",src_out)
        .value("dst_out",dst_out)
        .value("src_atop",src_atop)
        .value("dst_atop",dst_atop)
        .value("xor",_xor)
        .value("plus",plus)
        .value("minus",minus)
        .value("multiply",multiply)
        .value("screen",screen)
        .value("overlay",overlay)
        .value("darken",darken)
        .value("lighten",lighten)
        .value("color_dodge",color_dodge)
        .value("color_burn",color_burn)
        .value("hard_light",hard_light)
        .value("soft_light",soft_light)
        .value("difference",difference)
        .value("exclusion",exclusion)
        .value("contrast",contrast)
        .value("invert",invert)
        .value("invert_rgb",invert_rgb)
        ;
    
    class_<image_32,boost::shared_ptr<image_32> >("Image","This class represents a 32 bit RGBA image.",init<int,int>())
        .def("width",&image_32::width)
        .def("height",&image_32::height)
        .def("view",&image_32::get_view)
        .add_property("background",make_function
                      (&image_32::get_background,return_value_policy<copy_const_reference>()),
                      &image_32::set_background, "The background color of the image.")
        .def("set_grayscale_to_alpha",&image_32::set_grayscale_to_alpha, "Set the grayscale values to the alpha channel of the Image")
        .def("set_color_to_alpha",&image_32::set_color_to_alpha, "Set a given color to the alpha channel of the Image")
        .def("set_alpha",&image_32::set_alpha, "Set the overall alpha channel of the Image")
        .def("blend",&blend)
        .def("composite",&composite)
        //TODO(haoyu) The method name 'tostring' might be confusing since they actually return bytes in Python 3
        .def("tostring",&tostring1)
        .def("tostring",&tostring2)
        .def("tostring",&tostring3)
        .def("save", &save_to_file1)
        .def("save", &save_to_file2)
        .def("open",open_from_file)
        .staticmethod("open")
#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
        .def("from_cairo",&from_cairo)
        .staticmethod("from_cairo")
#endif
        ;    
    
}
