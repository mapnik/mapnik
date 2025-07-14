#ifndef MAPNIK_COMPARE_IMAGES_HPP
#define MAPNIK_COMPARE_IMAGES_HPP

#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>

namespace benchmark {

bool compare_images(std::string const& src_fn, std::string const& dest_fn)
{
    std::unique_ptr<mapnik::image_reader> reader1(mapnik::get_image_reader(dest_fn, "png"));
    if (!reader1.get())
    {
        throw mapnik::image_reader_exception("Failed to load: " + dest_fn);
    }

    std::unique_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(src_fn, "png"));
    if (!reader2.get())
    {
        throw mapnik::image_reader_exception("Failed to load: " + src_fn);
    }

    mapnik::image_any const desc_any = reader1->read(0, 0, reader1->width(), reader1->height());
    mapnik::image_any const src_any = reader2->read(0, 0, reader2->width(), reader2->height());

    mapnik::image_rgba8 const& dest = mapnik::util::get<mapnik::image_rgba8>(desc_any);
    mapnik::image_rgba8 const& src = mapnik::util::get<mapnik::image_rgba8>(src_any);

    return compare(dest, src, 0, true) == 0;
}

} // namespace benchmark

#endif // MAPNIK_COMPARE_IMAGES_HPP
