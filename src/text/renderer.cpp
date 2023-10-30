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

// mapnik
#include <mapnik/text/renderer.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/agg_rasterizer.hpp>

namespace mapnik {

text_renderer::text_renderer(halo_rasterizer_e rasterizer,
                             composite_mode_e comp_op,
                             composite_mode_e halo_comp_op,
                             double scale_factor,
                             stroker_ptr stroker)
    : rasterizer_(rasterizer)
    , comp_op_(comp_op)
    , halo_comp_op_(halo_comp_op)
    , scale_factor_(scale_factor)
    , glyphs_()
    , stroker_(stroker)
    , transform_()
    , halo_transform_()
{}

void text_renderer::set_transform(agg::trans_affine const& transform)
{
    transform_ = transform;
}

void text_renderer::set_halo_transform(agg::trans_affine const& halo_transform)
{
    halo_transform_ = halo_transform;
}

void text_renderer::prepare_glyphs(glyph_positions const& positions)
{
    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error error;

    glyphs_.clear();
    glyphs_.reserve(positions.size());

    for (auto const& glyph_pos : positions)
    {
        glyph_info const& glyph = glyph_pos.glyph;
        FT_Int32 load_flags = FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING;

        FT_Face face = glyph.face->get_face();
        if (glyph.face->is_color())
        {
            load_flags |= FT_LOAD_COLOR;
            if (face->num_fixed_sizes > 0)
            {
                int scaled_size = static_cast<int>(glyph.format->text_size * scale_factor_);
                int best_match = 0;
                int diff = std::abs(scaled_size - face->available_sizes[0].width);
                for (int i = 1; i < face->num_fixed_sizes; ++i)
                {
                    int ndiff = std::abs(scaled_size - face->available_sizes[i].height);
                    if (ndiff < diff)
                    {
                        best_match = i;
                        diff = ndiff;
                    }
                }
                error = FT_Select_Size(face, best_match);
            }
        }
        else
        {
            glyph.face->set_character_sizes(glyph.format->text_size * scale_factor_);
        }

        double size = glyph.format->text_size * scale_factor_;
        matrix.xx = static_cast<FT_Fixed>(glyph_pos.rot.cos * 0x10000L);
        matrix.xy = static_cast<FT_Fixed>(-glyph_pos.rot.sin * 0x10000L);
        matrix.yx = static_cast<FT_Fixed>(glyph_pos.rot.sin * 0x10000L);
        matrix.yy = static_cast<FT_Fixed>(glyph_pos.rot.cos * 0x10000L);

        pixel_position pos = glyph_pos.pos + glyph.offset.rotate(glyph_pos.rot);
        pen.x = static_cast<FT_Pos>(pos.x * 64);
        pen.y = static_cast<FT_Pos>(pos.y * 64);

        FT_Set_Transform(face, &matrix, &pen);
        error = FT_Load_Glyph(face, glyph.glyph_index, load_flags);
        if (error)
            continue;
        FT_Glyph image;
        error = FT_Get_Glyph(face->glyph, &image);
        if (error)
            continue;
        box2d<double> bbox(0, glyph_pos.glyph.ymin(), glyph_pos.glyph.advance(), glyph_pos.glyph.ymax());
        glyphs_.emplace_back(image, *glyph.format, pos, glyph_pos.rot, size, bbox);
    }
}

template<typename T>
void composite_bitmap(T& pixmap,
                      FT_Bitmap* bitmap,
                      unsigned rgba,
                      int x,
                      int y,
                      double opacity,
                      composite_mode_e comp_op)
{
    int x_max = x + bitmap->width;
    int y_max = y + bitmap->rows;

    for (int i = x, p = 0; i < x_max; ++i, ++p)
    {
        for (int j = y, q = 0; j < y_max; ++j, ++q)
        {
            unsigned gray = bitmap->buffer[q * bitmap->width + p];
            if (gray)
            {
                mapnik::composite_pixel(pixmap, comp_op, i, j, rgba, gray, opacity);
            }
        }
    }
}

template<typename T>
agg_text_renderer<T>::agg_text_renderer(pixmap_type& pixmap,
                                        halo_rasterizer_e rasterizer,
                                        composite_mode_e comp_op,
                                        composite_mode_e halo_comp_op,
                                        double scale_factor,
                                        stroker_ptr stroker)
    : text_renderer(rasterizer, comp_op, halo_comp_op, scale_factor, stroker)
    , pixmap_(pixmap)
{}

template<typename T>
void agg_text_renderer<T>::render(glyph_positions const& pos)
{
    prepare_glyphs(pos);
    FT_Error error;
    FT_Vector start;
    FT_Vector start_halo;
    int height = pixmap_.height();
    pixel_position const& base_point = pos.get_base_point();

    start.x = static_cast<FT_Pos>(base_point.x * (1 << 6));
    start.y = static_cast<FT_Pos>((height - base_point.y) * (1 << 6));
    start_halo = start;
    start.x += transform_.tx * 64;
    start.y += transform_.ty * 64;
    start_halo.x += halo_transform_.tx * 64;
    start_halo.y += halo_transform_.ty * 64;

    FT_Matrix halo_matrix;
    halo_matrix.xx = halo_transform_.sx * 0x10000L;
    halo_matrix.xy = halo_transform_.shx * 0x10000L;
    halo_matrix.yy = halo_transform_.sy * 0x10000L;
    halo_matrix.yx = halo_transform_.shy * 0x10000L;

    FT_Matrix matrix;
    matrix.xx = transform_.sx * 0x10000L;
    matrix.xy = transform_.shx * 0x10000L;
    matrix.yy = transform_.sy * 0x10000L;
    matrix.yx = transform_.shy * 0x10000L;

    // default formatting
    double halo_radius = 0;
    color black(0, 0, 0);
    unsigned fill = black.rgba();
    unsigned halo_fill = black.rgba();
    double text_opacity = 1.0;
    double halo_opacity = 1.0;

    for (auto const& glyph : glyphs_)
    {
        halo_fill = glyph.properties.halo_fill.rgba();
        halo_opacity = glyph.properties.halo_opacity;
        halo_radius = glyph.properties.halo_radius * scale_factor_;
        // make sure we've got reasonable values.
        if (halo_radius <= 0.0 || halo_radius > 1024.0)
            continue;
        FT_Glyph g;
        error = FT_Glyph_Copy(glyph.image, &g);
        if (!error)
        {
            FT_Glyph_Transform(g, &halo_matrix, &start_halo);
            if (rasterizer_ == halo_rasterizer_enum::HALO_RASTERIZER_FULL)
            {
                stroker_->init(halo_radius);
                FT_Glyph_Stroke(&g, stroker_->get(), 1);
                error = FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, 0, 1);
                if (!error)
                {
                    FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(g);
                    if (bit->bitmap.pixel_mode != FT_PIXEL_MODE_BGRA)
                    {
                        composite_bitmap(pixmap_,
                                         &bit->bitmap,
                                         halo_fill,
                                         bit->left,
                                         height - bit->top,
                                         halo_opacity,
                                         halo_comp_op_);
                    }
                }
            }
            else
            {
                error = FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, 0, 1);
                if (error)
                {
                    continue;
                }
                FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(g);
                if (bit->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
                {
                    pixel_position render_pos(base_point);
                    image_rgba8 glyph_image(render_glyph_image(glyph, bit->bitmap, transform_, render_pos));
                    const constexpr std::size_t pixel_size = sizeof(image_rgba8::pixel_type);
                    render_halo<pixel_size>(glyph_image.bytes(),
                                            glyph_image.width(),
                                            glyph_image.height(),
                                            halo_fill,
                                            render_pos.x,
                                            render_pos.y,
                                            halo_radius,
                                            halo_opacity,
                                            halo_comp_op_);
                }
                else
                {
                    render_halo<1>(bit->bitmap.buffer,
                                   bit->bitmap.width,
                                   bit->bitmap.rows,
                                   halo_fill,
                                   bit->left,
                                   height - bit->top,
                                   halo_radius,
                                   halo_opacity,
                                   halo_comp_op_);
                }
            }
        }
        FT_Done_Glyph(g);
    }

    // render actual text
    for (auto& glyph : glyphs_)
    {
        fill = glyph.properties.fill.rgba();
        text_opacity = glyph.properties.text_opacity;

        FT_Glyph_Transform(glyph.image, &matrix, &start);
        error = 0;
        if (glyph.image->format != FT_GLYPH_FORMAT_BITMAP)
        {
            error = FT_Glyph_To_Bitmap(&glyph.image, FT_RENDER_MODE_NORMAL, 0, 1);
        }
        if (error == 0)
        {
            FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(glyph.image);
            int pixel_mode = bit->bitmap.pixel_mode;
            if (pixel_mode == FT_PIXEL_MODE_BGRA)
            {
                int x = base_point.x + glyph.pos.x;
                int y = base_point.y - glyph.pos.y;
                agg::trans_affine transform(
                  glyph_transform(transform_, bit->bitmap.rows, x, y, -glyph.rot.angle(), glyph.bbox));
                composite_color_glyph(pixmap_, bit->bitmap, transform, text_opacity, comp_op_);
            }
            else
            {
                composite_bitmap(pixmap_, &bit->bitmap, fill, bit->left, height - bit->top, text_opacity, comp_op_);
            }
        }
        FT_Done_Glyph(glyph.image);
    }
}

template<typename T>
void grid_text_renderer<T>::render(glyph_positions const& pos, value_integer feature_id)
{
    prepare_glyphs(pos);
    FT_Error error;
    FT_Vector start;
    unsigned height = pixmap_.height();
    pixel_position const& base_point = pos.get_base_point();
    start.x = static_cast<FT_Pos>(base_point.x * (1 << 6));
    start.y = static_cast<FT_Pos>((height - base_point.y) * (1 << 6));
    start.x += transform_.tx * 64;
    start.y += transform_.ty * 64;

    // now render transformed glyphs
    double halo_radius = 0.0;
    FT_Matrix halo_matrix;
    halo_matrix.xx = halo_transform_.sx * 0x10000L;
    halo_matrix.xy = halo_transform_.shx * 0x10000L;
    halo_matrix.yy = halo_transform_.sy * 0x10000L;
    halo_matrix.yx = halo_transform_.shy * 0x10000L;
    for (auto& glyph : glyphs_)
    {
        halo_radius = glyph.properties.halo_radius * scale_factor_;
        FT_Glyph_Transform(glyph.image, &halo_matrix, &start);
        error = FT_Glyph_To_Bitmap(&glyph.image, FT_RENDER_MODE_NORMAL, 0, 1);
        if (!error)
        {
            FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(glyph.image);
            if (bit->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
            {
                pixel_position render_pos(base_point);
                image_rgba8 glyph_image(render_glyph_image(glyph, bit->bitmap, transform_, render_pos));
                const constexpr std::size_t pixel_size = sizeof(image_rgba8::pixel_type);
                render_halo_id<pixel_size>(glyph_image.bytes(),
                                           glyph_image.width(),
                                           glyph_image.height(),
                                           feature_id,
                                           render_pos.x,
                                           render_pos.y,
                                           static_cast<int>(halo_radius));
            }
            else
            {
                render_halo_id<1>(bit->bitmap.buffer,
                                  bit->bitmap.width,
                                  bit->bitmap.rows,
                                  feature_id,
                                  bit->left,
                                  height - bit->top,
                                  static_cast<int>(halo_radius));
            }
        }
        FT_Done_Glyph(glyph.image);
    }
}

template<typename T>
template<std::size_t PixelWidth>
void agg_text_renderer<T>::render_halo(unsigned char* buffer,
                                       unsigned width,
                                       unsigned height,
                                       unsigned rgba,
                                       int x1,
                                       int y1,
                                       double halo_radius,
                                       double opacity,
                                       composite_mode_e comp_op)
{
    if (halo_radius < 1.0)
    {
        for (unsigned x = 0; x < width; ++x)
        {
            for (unsigned y = 0; y < height; ++y)
            {
                int gray = buffer[(y * width + x) * PixelWidth + PixelWidth - 1];
                if (gray)
                {
                    mapnik::composite_pixel(pixmap_,
                                            comp_op,
                                            x + x1 - 1,
                                            y + y1 - 1,
                                            rgba,
                                            gray * halo_radius * halo_radius,
                                            opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x + x1, y + y1 - 1, rgba, gray * halo_radius, opacity);
                    mapnik::composite_pixel(pixmap_,
                                            comp_op,
                                            x + x1 + 1,
                                            y + y1 - 1,
                                            rgba,
                                            gray * halo_radius * halo_radius,
                                            opacity);

                    mapnik::composite_pixel(pixmap_, comp_op, x + x1 - 1, y + y1, rgba, gray * halo_radius, opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x + x1, y + y1, rgba, gray, opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x + x1 + 1, y + y1, rgba, gray * halo_radius, opacity);

                    mapnik::composite_pixel(pixmap_,
                                            comp_op,
                                            x + x1 - 1,
                                            y + y1 + 1,
                                            rgba,
                                            gray * halo_radius * halo_radius,
                                            opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x + x1, y + y1 + 1, rgba, gray * halo_radius, opacity);
                    mapnik::composite_pixel(pixmap_,
                                            comp_op,
                                            x + x1 + 1,
                                            y + y1 + 1,
                                            rgba,
                                            gray * halo_radius * halo_radius,
                                            opacity);
                }
            }
        }
    }
    else
    {
        for (unsigned x = 0; x < width; ++x)
        {
            for (unsigned y = 0; y < height; ++y)
            {
                int gray = buffer[(y * width + x) * PixelWidth + PixelWidth - 1];
                if (gray)
                {
                    for (int n = -halo_radius; n <= halo_radius; ++n)
                        for (int m = -halo_radius; m <= halo_radius; ++m)
                            mapnik::composite_pixel(pixmap_, comp_op, x + x1 + m, y + y1 + n, rgba, gray, opacity);
                }
            }
        }
    }
}

template<typename T>
template<std::size_t PixelWidth>
void grid_text_renderer<T>::render_halo_id(unsigned char* buffer,
                                           unsigned width,
                                           unsigned height,
                                           mapnik::value_integer feature_id,
                                           int x1,
                                           int y1,
                                           int halo_radius)
{
    for (unsigned x = 0; x < width; ++x)
    {
        for (unsigned y = 0; y < height; ++y)
        {
            int gray = buffer[(y * width + x) * PixelWidth + PixelWidth - 1];
            if (gray)
            {
                for (int n = -halo_radius; n <= halo_radius; ++n)
                    for (int m = -halo_radius; m <= halo_radius; ++m)
                        pixmap_.setPixel(x + x1 + m, y + y1 + n, feature_id);
            }
        }
    }
}

template<typename T>
grid_text_renderer<T>::grid_text_renderer(pixmap_type& pixmap, composite_mode_e comp_op, double scale_factor)
    : text_renderer(halo_rasterizer_enum::HALO_RASTERIZER_FAST, comp_op, src_over, scale_factor)
    , pixmap_(pixmap)
{}

template class agg_text_renderer<image_rgba8>;
template class grid_text_renderer<grid>;

} // namespace mapnik
