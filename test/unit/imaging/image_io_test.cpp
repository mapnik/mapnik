#include "catch.hpp"

#include <cstring>
#include <mapnik/color.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_util_jpeg.hpp>
#include <mapnik/util/fs.hpp>
#if defined(HAVE_CAIRO)
#include <mapnik/cairo/cairo_context.hpp>
#include <mapnik/cairo/cairo_image_util.hpp>
#endif

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/optional/optional_io.hpp>
MAPNIK_DISABLE_WARNING_POP
#include <mapnik/util/mapped_memory_file.hpp>

inline void make_directory(std::string const& dir)
{
    boost::filesystem::create_directories(dir);
}

namespace {
template<typename T>
void check_tiny_png_image_quantising(T const& im)
{
    std::ostringstream ss(std::ios::binary);
    mapnik::save_to_stream(im, ss, "png8");
    ss.flush();
    std::string str = ss.str();
    std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(str.data(), str.size()));
    auto w = reader->width();
    auto h = reader->height();
    CHECK(w > 0);
    CHECK(h > 0);
    auto im2 = mapnik::util::get<mapnik::image_rgba8>(reader->read(0, 0, w, h));
    for (std::size_t i = 0; i < w; ++i)
    {
        for (std::size_t j = 0; j < h; ++j)
        {
            REQUIRE(im2(i, j) == im(i, j));
        }
    }
}

} // namespace

TEST_CASE("image io")
{
    SECTION("readers")
    {
        std::string should_throw;
        boost::optional<std::string> type;
        try
        {
            mapnik::image_rgba8 im_og;
            auto im_size = mapnik::image_rgba8::pixel_size * im_og.width() * im_og.height();
            mapnik::detail::buffer buf(im_og.bytes(), im_size);
            mapnik::image_rgba8 im2(im_og.width(), im_og.height(), buf.data());
            CHECK(im2.bytes() == im_og.bytes());
#if defined(HAVE_JPEG)
            should_throw = "./test/data/images/blank.jpg";
            REQUIRE(mapnik::util::exists(should_throw));
            type = mapnik::type_from_filename(should_throw);
            REQUIRE(type);
            REQUIRE_THROWS(mapnik::get_image_reader(should_throw, *type));

            // actually a png so jpeg reader should throw
            should_throw = "./test/data/images/landusepattern.jpg";
            REQUIRE(mapnik::util::exists(should_throw));
            type = mapnik::type_from_filename(should_throw);
            REQUIRE(type);
            try
            {
                std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw, *type));
                REQUIRE(false);
            }
            catch (std::exception const& ex)
            {
                REQUIRE(
                  std::string(ex.what()) ==
                  std::string("JPEG Reader: libjpeg could not read image: Not a JPEG file: starts with 0x89 0x50"));
            }

#endif

            REQUIRE_THROWS(mapnik::image_rgba8(-10, -10)); // should throw rather than overflow

#if defined(HAVE_CAIRO)
            mapnik::cairo_surface_ptr image_surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 257),
                                                    mapnik::cairo_surface_closer());
            mapnik::image_rgba8 im_data(cairo_image_surface_get_width(&*image_surface),
                                        cairo_image_surface_get_height(&*image_surface));
            im_data.set(1);
            REQUIRE((unsigned)im_data(0, 0) == unsigned(1));
            // Should set back to fully transparent
            mapnik::cairo_image_to_rgba8(im_data, image_surface);
            REQUIRE((unsigned)im_data(0, 0) == unsigned(0));
#endif

#if defined(HAVE_PNG)
            should_throw = "./test/data/images/blank.png";
            REQUIRE(mapnik::util::exists(should_throw));
            type = mapnik::type_from_filename(should_throw);
            REQUIRE(type);
            REQUIRE_THROWS(mapnik::get_image_reader(should_throw, *type));

            should_throw = "./test/data/images/xcode-CgBI.png";
            REQUIRE(mapnik::util::exists(should_throw));
            type = mapnik::type_from_filename(should_throw);
            REQUIRE(type);
            REQUIRE_THROWS(mapnik::get_image_reader(should_throw, *type));
#endif

#if defined(HAVE_TIFF)
            should_throw = "./test/data/images/blank.tiff";
            REQUIRE(mapnik::util::exists(should_throw));
            type = mapnik::type_from_filename(should_throw);
            REQUIRE(type);
            REQUIRE_THROWS(mapnik::get_image_reader(should_throw, *type));
#endif

#if defined(HAVE_WEBP)
            should_throw = "./test/data/images/blank.webp";
            REQUIRE(mapnik::util::exists(should_throw));
            type = mapnik::type_from_filename(should_throw);
            REQUIRE(type);
            REQUIRE_THROWS(mapnik::get_image_reader(should_throw, *type));
#endif
        }
        catch (std::exception const& ex)
        {
            std::clog << ex.what() << "\n";
            REQUIRE(false);
        }

    } // END SECTION

    SECTION("writers options")
    {
#if defined(HAVE_JPEG)
        // test we can parse both jpegXX and quality=XX options
        REQUIRE_THROWS(mapnik::detail::parse_jpeg_quality("jpegXX"));
        REQUIRE_THROWS(mapnik::detail::parse_jpeg_quality("jpeg:quality=XX"));
        int q0 = mapnik::detail::parse_jpeg_quality("jpeg50");
        int q1 = mapnik::detail::parse_jpeg_quality("jpeg:quality=50");
        REQUIRE(q0 == q1);
#endif
    } // END SECTION

    SECTION("image_util : save_to_file/save_to_stream/save_to_string")
    {
        mapnik::image_rgba8 im(256, 256);
        std::string named_color = "lightblue";
        mapnik::fill(im, mapnik::color(named_color).rgba());
        ////////////////////////////////////////////////////
        std::vector<std::tuple<std::string, std::string>> supported_types;
#if defined(HAVE_PNG)
        supported_types.push_back(std::make_tuple("png", "png"));
        supported_types.push_back(std::make_tuple("png", "png24"));
        supported_types.push_back(std::make_tuple("png", "png32"));
        supported_types.push_back(std::make_tuple("png", "png8"));
        supported_types.push_back(std::make_tuple("png", "png256"));
#endif
#if defined(HAVE_JPEG)
        supported_types.push_back(std::make_tuple("jpeg", "jpeg"));
        supported_types.push_back(std::make_tuple("jpeg", "jpeg80"));
        supported_types.push_back(std::make_tuple("jpeg", "jpeg90"));
#endif
#if defined(HAVE_TIFF)
        supported_types.push_back(std::make_tuple("tiff", "tiff"));
#endif
#if defined(HAVE_WEBP)
        supported_types.push_back(std::make_tuple("webp", "webp"));
#endif

        std::string directory_name("/tmp/mapnik-tests/");
        make_directory(directory_name);
        REQUIRE(mapnik::util::exists(directory_name));

        for (auto const& info : supported_types)
        {
            std::string extension;
            std::string format;
            std::tie(extension, format) = info;
            std::string filename = (boost::format(directory_name + "mapnik-%1%.%2%") % named_color % extension).str();
            mapnik::save_to_file(im, filename);
            std::string str = mapnik::save_to_string(im, format);
            std::ostringstream ss;
            mapnik::save_to_stream(im, ss, format);
            CHECK(str.length() == ss.str().length());
            // wrap reader in scope to ensure the file handle is
            // released before we try to remove the file
            {
                std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename, extension));
                unsigned w = reader->width();
                unsigned h = reader->height();
                auto im2 = reader->read(0, 0, w, h);
                CHECK(im2.size() == im.size());
                if (extension == "png" || extension == "tiff")
                {
                    CHECK(0 == std::memcmp(im2.bytes(), im.bytes(), im.width() * im.height()));
                }
            }
            mapnik::util::mapped_memory_file::deleteFile(filename);
        }
    }

    SECTION("Quantising small (less than 3 pixel images preserve original colours")
    {
#if defined(HAVE_PNG)
        {                                 // 1x1
            mapnik::image_rgba8 im(1, 1); // 1 pixel
            im(0, 0) = mapnik::color("green").rgba();
            check_tiny_png_image_quantising(im);
        }
        {                                 // 1x2
            mapnik::image_rgba8 im(1, 2); // 2 pixels
            mapnik::fill(im, mapnik::color("red").rgba());
            im(0, 0) = mapnik::color("green").rgba();
            check_tiny_png_image_quantising(im);
        }
        {                                 // 2x1
            mapnik::image_rgba8 im(2, 1); // 2 pixels
            mapnik::fill(im, mapnik::color("red").rgba());
            im(0, 0) = mapnik::color("green").rgba();
            check_tiny_png_image_quantising(im);
        }
#endif
    } // END SECTION

} // END TEST_CASE
