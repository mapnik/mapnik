#include "bench_framework.hpp"
#include <mapnik/map.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <stdexcept>

class test : public benchmark::test_case
{
    std::string xml_;
    mapnik::box2d<double> extent_;
    mapnik::value_integer width_;
    mapnik::value_integer height_;
    double scale_factor_;
    std::string preview_;

  public:
    test(mapnik::parameters const& params)
        : test_case(params),
          xml_(),
          extent_(),
          width_(*params.get<mapnik::value_integer>("width", 256)),
          height_(*params.get<mapnik::value_integer>("height", 256)),
          scale_factor_(*params.get<mapnik::value_double>("scale_factor", 1.0)),
          preview_(*params.get<std::string>("preview", ""))
    {
        const auto map = params.get<std::string>("map");
        if (!map)
        {
            throw std::runtime_error("please provide a --map <path to xml> arg");
        }
        xml_ = *map;

        const auto ext = params.get<std::string>("extent");
        if (ext && !ext->empty())
        {
            if (!extent_.from_string(*ext))
                throw std::runtime_error("could not parse `extent` string" + *ext);
        }
        /*
        else
        {
            throw std::runtime_error("please provide a --extent=<minx,miny,maxx,maxy> arg");
        }*/
    }
    bool validate() const
    {
        mapnik::Map m(width_, height_);
        mapnik::load_map(m, xml_, true);
        if (extent_.valid())
        {
            m.zoom_to_box(extent_);
        }
        else
        {
            m.zoom_all();
        }
        mapnik::image_rgba8 im(m.width(), m.height());
        mapnik::agg_renderer<mapnik::image_rgba8> ren(m, im, scale_factor_);
        ren.apply();
        if (!preview_.empty())
        {
            std::clog << "preview available at " << preview_ << "\n";
            mapnik::save_to_file(im, preview_);
        }
        return true;
    }
    bool operator()() const
    {
        if (!preview_.empty())
        {
            return false;
        }
        mapnik::Map m(width_, height_);
        mapnik::load_map(m, xml_);
        if (extent_.valid())
        {
            m.zoom_to_box(extent_);
        }
        else
        {
            m.zoom_all();
        }
        for (unsigned i = 0; i < iterations_; ++i)
        {
            mapnik::image_rgba8 im(m.width(), m.height());
            mapnik::agg_renderer<mapnik::image_rgba8> ren(m, im, scale_factor_);
            ren.apply();
        }
        return true;
    }
};

int main(int argc, char** argv)
{
    mapnik::setup();
    int return_value = 0;
    try
    {
        mapnik::parameters params;
        benchmark::handle_args(argc, argv, params);
        const auto name = params.get<std::string>("name");
        if (!name)
        {
            std::clog << "please provide a name for this test\n";
            return -1;
        }
        mapnik::freetype_engine::register_fonts("./fonts/", true);
        mapnik::datasource_cache::instance().register_datasources("./plugins/input/");
        {
            test test_runner(params);
            return_value = run(test_runner, *name);
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << ex.what() << "\n";
        return -1;
    }
    return return_value;
}
