#ifndef __MAPNIK_COMPARE_IMAGES_HPP__
#define __MAPNIK_COMPARE_IMAGES_HPP__ 

#include <mapnik/graphics.hpp>
#include <mapnik/image_data.hpp>
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
        std::shared_ptr<image_32> image_ptr1 = std::make_shared<image_32>(reader1->width(),reader1->height());
        reader1->read(0,0,image_ptr1->data());

        std::unique_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(src_fn,"png"));
        if (!reader2.get())
        {
            throw mapnik::image_reader_exception("Failed to load: " + src_fn);
        }
        std::shared_ptr<image_32> image_ptr2 = std::make_shared<image_32>(reader2->width(),reader2->height());
        reader2->read(0,0,image_ptr2->data());

        image_data_32 const& dest = image_ptr1->data();
        image_data_32 const& src = image_ptr2->data();

        unsigned int width = src.width();
        unsigned int height = src.height();
        if ((width != dest.width()) || height != dest.height()) return false;
        for (unsigned int y = 0; y < height; ++y)
        {
            const unsigned int* row_from = src.getRow(y);
            const unsigned int* row_to = dest.getRow(y);
            for (unsigned int x = 0; x < width; ++x)
            {
               if (row_from[x] != row_to[x]) return false;
            }
        }
        return true;
    }

}

#endif // __MAPNIK_COMPARE_IMAGES_HPP__