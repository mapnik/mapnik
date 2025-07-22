#ifndef MAPNIK_CREATE_IMAGE_READER_HPP
#define MAPNIK_CREATE_IMAGE_READER_HPP

namespace mapnik {
#ifdef HAVE_JPEG
void register_jpeg_reader();
#endif
#ifdef HAVE_PNG
void register_png_reader();
#endif
#ifdef HAVE_TIFF
void register_tiff_reader();
#endif
#ifdef HAVE_WEBP
void register_webp_reader();
#endif
#ifdef HAVE_AVIF
void register_avif_reader();
#endif
} // namespace mapnik

#endif
