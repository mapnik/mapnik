#include <boost/detail/lightweight_test.hpp>

#include <iostream>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#if defined(HAVE_CAIRO)
#include <mapnik/cairo_renderer.hpp>
#include <mapnik/cairo_context.hpp>
#endif
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/feature_style_processor.hpp>

#include <vector>
#include <algorithm>
#include "utils.hpp"

bool compare_images(std::string const& src_fn,std::string const& dest_fn)
{
    using namespace mapnik;
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

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();
    // TODO - re-enable if we can control the freetype/cairo versions used
    // https://github.com/mapnik/mapnik/issues/1868
    /*
    std::string expected("./tests/cpp_tests/support/map-request-marker-text-line-expected.png");
    std::string expected_cairo("./tests/cpp_tests/support/map-request-marker-text-line-expected-cairo.png");
    try {

        BOOST_TEST(set_working_dir(args));

        mapnik::datasource_cache::instance().register_datasources("./plugins/input/");
        mapnik::freetype_engine::register_fonts("./fonts", true );
        mapnik::Map m(256,256);
        mapnik::load_map(m,"./tests/data/good_maps/marker-text-line.xml",false);
        m.zoom_all();
        mapnik::image_32 im(m.width(),m.height());
        double scale_factor = 1.2;

        // render normally with apply() and just map and image
        mapnik::agg_renderer<mapnik::image_32> renderer1(m,im,scale_factor);
        renderer1.apply();
        std::string actual1("/tmp/map-request-marker-text-line-actual1.png");
        //mapnik::save_to_file(im,expected);
        mapnik::save_to_file(im,actual1);
        BOOST_TEST(compare_images(actual1,expected));

        // reset image
        im.clear();

        // set up a mapnik::request object
        mapnik::request req(m.width(),m.height(),m.get_current_extent());
        req.set_buffer_size(m.buffer_size());

        // render using apply() and mapnik::request
        mapnik::agg_renderer<mapnik::image_32> renderer2(m,req,im,scale_factor);
        renderer2.apply();
        std::string actual2("/tmp/map-request-marker-text-line-actual2.png");
        mapnik::save_to_file(im,actual2);
        BOOST_TEST(compare_images(actual2,expected));

        // reset image
        im.clear();

        // render with apply_to_layer api and mapnik::request params passed to apply_to_layer
        mapnik::agg_renderer<mapnik::image_32> renderer3(m,req,im,scale_factor);
        renderer3.start_map_processing(m);
        mapnik::projection map_proj(m.srs(),true);
        double scale_denom = mapnik::scale_denominator(req.scale(),map_proj.is_geographic());
        scale_denom *= scale_factor;
        for (mapnik::layer const& lyr : m.layers() )
        {
            if (lyr.visible(scale_denom))
            {
                std::set<std::string> names;
                renderer3.apply_to_layer(lyr,
                                         renderer3,
                                         map_proj,
                                         req.scale(),
                                         scale_denom,
                                         req.width(),
                                         req.height(),
                                         req.extent(),
                                         req.buffer_size(),
                                         names);

            }
        }
        renderer3.end_map_processing(m);
        std::string actual3("/tmp/map-request-marker-text-line-actual3.png");
        mapnik::save_to_file(im,actual3);
        BOOST_TEST(compare_images(actual3,expected));

        // also test cairo
#if defined(HAVE_CAIRO)
        mapnik::cairo_surface_ptr image_surface(
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32,req.width(),req.height()),
            mapnik::cairo_surface_closer());
        mapnik::cairo_ptr image_context = (mapnik::create_context(image_surface));
        mapnik::cairo_renderer<mapnik::cairo_ptr> png_render(m,req,image_context,scale_factor);
        png_render.apply();
        //cairo_surface_write_to_png(&*image_surface, expected_cairo.c_str());
        std::string actual_cairo("/tmp/map-request-marker-text-line-actual4.png");
        cairo_surface_write_to_png(&*image_surface, actual_cairo.c_str());
        BOOST_TEST(compare_images(actual_cairo,expected_cairo));
#endif
        // TODO - test grid_renderer

    } catch (std::exception const& ex) {
        std::clog << ex.what() << "\n";
    }
    */
    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ Map Request rendering hook: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    } else {
        return ::boost::report_errors();
    }
}
