/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Daniel Patterson
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
#include <mapnik/png_io.hpp>
#include <mapnik/image_view.hpp>

extern "C"
{
#include <libimagequant.h>
}

namespace mapnik
{

  // Note: This only works with rgba8_t image types
  template <typename T1, typename T2>
  void save_as_png8_libimagequant(T1 & file,
                                  T2 const& image,
                                  png_options const& opts)
  {
      unsigned width = image.width();
      unsigned height = image.height();

      uint32_t const* buf[height];

      // TODO: this won't work on big-endian architectures, liq expects data in RGBA byte order
      for (size_t y = 0; y < height; ++y)
      {
          buf[y] = image.get_row(y);
      }

      // Set up libimagequant
      liq_attr *attr = liq_attr_create();
      liq_set_speed(attr, opts.iq_speed);
      liq_image *liq_image = liq_image_create_rgba_rows(attr, (void **)buf, width, height, 0);

      // Do the quantization
      liq_result *res = liq_quantize_image(attr, liq_image);

      if (opts.iq_dither != -1)
      {
          liq_set_dithering_level(res, opts.iq_dither);
      }
      if (opts.gamma > 0)
      {
          liq_set_output_gamma(res, opts.gamma);
      }

      // Extract the generated 8-bit imag
      image_gray8 quantized_image(width, height);
      liq_write_remapped_image(res, liq_image, (void *)quantized_image.data(), width*height*sizeof(gray8_t));

      // Extract the generated palette and alpha vector from libimagequant
      std::vector<mapnik::rgb> palette;
      std::vector<unsigned> alpha;
      const liq_palette *liq_pal = liq_get_palette(res);
      for (unsigned i = 0; i < liq_pal->count; ++i)
      {
          palette.push_back({ liq_pal->entries[i].r, liq_pal->entries[i].g, liq_pal->entries[i].b });
          alpha.push_back(liq_pal->entries[i].a);
      }

      // Encode the image, using a bit-depth depending on the number of colors in the generated
      // palette
      if (palette.size() > 16)  // 8-bit encoding
      {
          save_as_png(file, palette, quantized_image, width, height, 8, alpha, opts);
      }
      else if (palette.size() > 1) // up to 16 colors, 4-bit encoding
      {
          unsigned image_width  = ((width + 7) >> 1) & ~3U; // 4-bit image, round up to 32-bit boundary
          unsigned image_height = height;
          image_gray8 reduced_image(image_width, image_height);
          for (unsigned y = 0; y < height; ++y)
          {
              mapnik::image_gray8::pixel_type const * row = quantized_image.get_row(y);
              mapnik::image_gray8::pixel_type  * row_out = reduced_image.get_row(y);
              std::uint8_t index = 0;
              for (unsigned x = 0; x < width; ++x)
              {
                  index = row[x];
                  if (x%2 == 0)
                  {
                      index = index<<4;
                  }
                  row_out[x>>1] |= index;
              }
          }
          save_as_png(file, palette, reduced_image, width, height, 4, alpha, opts);
      }
      else // 1 color, 1-bit encoding
      {
          unsigned image_width  = ((width + 15) >> 3) & ~1U; // 1-bit image, round up to 16-bit boundary
          unsigned image_height = height;
          image_gray8 reduced_image(image_width, image_height);
          reduced_image.set(0);
          save_as_png(file, palette, reduced_image, width, height, 1, alpha, opts);
      }

      // Cleanup libimagequant
      liq_attr_destroy(attr);
      liq_image_destroy(liq_image);
      liq_result_destroy(res);
  }

  // Both image_view and image expose a get_row(), so we can support both parameter types
  template void save_as_png8_libimagequant(std::ostream &, mapnik::image_view<mapnik::image<mapnik::rgba8_t>> const&, png_options const&);
  template void save_as_png8_libimagequant(std::ostream &, mapnik::image<mapnik::rgba8_t> const&, png_options const&);

}
