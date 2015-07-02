#ifndef __MAPNIK_COMPARE_IMAGES_HPP__
#define __MAPNIK_COMPARE_IMAGES_HPP__

#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>

using namespace mapnik;

namespace benchmark {

    bool compare_images(std::string const& src_fn,std::string const& dest_fn)
    {
        std::unique_ptr<mapnik::image_reader> reader1(mapnik::get_image_reader(dest_fn,"png"));
        if (!reader1.get())
        {
            throw mapnik::image_reader_exception("Failed to load: " + dest_fn);
        }

        std::unique_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(src_fn,"png"));
        if (!reader2.get())
        {
            throw mapnik::image_reader_exception("Failed to load: " + src_fn);
        }

        const image_any desc_any = reader1->read(0,0,reader1->width(), reader1->height());
        const image_any src_any = reader2->read(0,0,reader2->width(), reader2->height());

        image_rgba8 const& dest = util::get<image_rgba8>(desc_any);
        image_rgba8 const& src = util::get<image_rgba8>(src_any);

        unsigned int width = src.width();
        unsigned int height = src.height();
        if ((width != dest.width()) || height != dest.height()) return false;
        for (unsigned int y = 0; y < height; ++y)
        {
            const unsigned int* row_from = src.get_row(y);
            const unsigned int* row_to = dest.get_row(y);
            for (unsigned int x = 0; x < width; ++x)
            {
               if (row_from[x] != row_to[x]) return false;
            }
        }
        return true;
    }

}

#endif // __MAPNIK_COMPARE_IMAGES_HPP__
