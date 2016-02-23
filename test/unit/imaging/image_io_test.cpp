#include "catch.hpp"
#include "catch_tmp.hpp"

#include <iostream>
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

#include <boost/format.hpp>

struct file_format_info
{
    std::string extension;
    std::string format;
    std::string fff; // filename-friendly format

    file_format_info(std::string const& ext, std::string const& fmt)
        : extension(ext), format(fmt), fff(fmt)
    {
        // colons are commonly used as list separators on Linux,
        // forbidden in filenames on Windows
        std::replace(fff.begin(), fff.end(), ':', '~');
        // cosmetic, so that filename doesn't look like assignment
        std::replace(fff.begin(), fff.end(), '=', '+');
    }
};

// feeds filename-friendly file-format and extension to boost::format object
static boost::format & operator%(boost::format & fmt, file_format_info const& info)
{
    return fmt % info.fff % info.extension;
}

static std::vector<file_format_info> const supported_types
{{
#if defined(HAVE_PNG)
    file_format_info("png", "png"),
    file_format_info("png", "png24"),
    file_format_info("png", "png32"),
    file_format_info("png", "png8"),
    file_format_info("png", "png256"),
#endif
#if defined(HAVE_JPEG)
    file_format_info("jpeg", "jpeg"),
    file_format_info("jpeg", "jpeg80"),
    file_format_info("jpeg", "jpeg90"),
#endif
#if defined(HAVE_TIFF)
    file_format_info("tiff", "tiff"),
#endif
#if defined(HAVE_WEBP)
    file_format_info("webp", "webp"),
    file_format_info("webp", "webp:lossless=1"),
#endif
}};

TEST_CASE("image io") {

SECTION("readers") {

    std::string should_throw;
    boost::optional<std::string> type;
    try
    {
        mapnik::image_rgba8 im_og;
        auto im_size = mapnik::image_rgba8::pixel_size * im_og.width() * im_og.height();
        mapnik::detail::buffer buf(im_og.bytes(), im_size);
        mapnik::image_rgba8 im2(im_og.width(), im_og.height(), buf.data());
        CHECK( im2.bytes() == im_og.bytes() );
#if defined(HAVE_JPEG)
        should_throw = "./test/data/images/blank.jpg";
        REQUIRE( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        REQUIRE( type );
        REQUIRE_THROWS(std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type)));

        // actually a png so jpeg reader should throw
        should_throw = "./test/data/images/landusepattern.jpg";
        REQUIRE( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        REQUIRE( type );
        try
        {
            std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            REQUIRE(false);
        }
        catch (std::exception const& ex)
        {
            REQUIRE( std::string(ex.what()) == std::string("JPEG Reader: libjpeg could not read image: Not a JPEG file: starts with 0x89 0x50") );
        }

#endif

        REQUIRE_THROWS(mapnik::image_rgba8 im(-10,-10)); // should throw rather than overflow

#if defined(HAVE_CAIRO)
        mapnik::cairo_surface_ptr image_surface(
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32,256,257),
            mapnik::cairo_surface_closer());
        mapnik::image_rgba8 im_data(cairo_image_surface_get_width(&*image_surface), cairo_image_surface_get_height(&*image_surface));
        im_data.set(1);
        REQUIRE( (unsigned)im_data(0,0) == unsigned(1) );
        // Should set back to fully transparent
        mapnik::cairo_image_to_rgba8(im_data, image_surface);
        REQUIRE( (unsigned)im_data(0,0) == unsigned(0) );
#endif

#if defined(HAVE_PNG)
        should_throw = "./test/data/images/blank.png";
        REQUIRE( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        REQUIRE( type );
        REQUIRE_THROWS(std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type)));

        should_throw = "./test/data/images/xcode-CgBI.png";
        REQUIRE( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        REQUIRE( type );
        REQUIRE_THROWS(std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type)));
#endif

#if defined(HAVE_TIFF)
        should_throw = "./test/data/images/blank.tiff";
        REQUIRE( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        REQUIRE( type );
        REQUIRE_THROWS(std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type)));
#endif

#if defined(HAVE_WEBP)
        should_throw = "./test/data/images/blank.webp";
        REQUIRE( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        REQUIRE( type );
        REQUIRE_THROWS(std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type)));
#endif
    }
    catch (std::exception const & ex)
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
    mapnik::image_rgba8 im(256,256);
    std::string named_color = "lightblue";
    mapnik::fill(im, mapnik::color(named_color).rgba());

    for (auto const& info : supported_types)
    {
        CAPTURE(info.format);

        boost::format FNFMT("image_io-%1%.%2%");
        catch_temporary_path filename = (FNFMT % named_color % info.extension).str();
        mapnik::save_to_file(im, filename);
        std::string str = mapnik::save_to_string(im, info.format);
        std::ostringstream ss;
        mapnik::save_to_stream(im, ss, info.format);
        CHECK(str.length() == ss.str().length());
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename, info.extension));
        unsigned w = reader->width();
        unsigned h = reader->height();
        auto im2 = reader->read(0, 0, w, h);
        CHECK(im2.size() == im.size());
        if (info.extension == "png" || info.extension == "tiff")
        {
            CHECK(0 == std::memcmp(im2.bytes(), im.bytes(), im.width() * im.height()));
        }
    }
}
} // END TEST_CASE
