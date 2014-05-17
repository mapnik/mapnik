#include "bench_framework.hpp"
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/datasource_cache.hpp>

class test : public benchmark::test_case
{
    std::string xml_;
    mapnik::box2d<double> extent_;
public:
    test(mapnik::parameters const& params,
         std::string const& xml,
         mapnik::box2d<double> const& extent)
     : test_case(params),
       xml_(xml),
       extent_(extent)
      {}
    bool validate() const
    {
        mapnik::Map m(256,256);
        mapnik::load_map(m,xml_);
        m.zoom_to_box(extent_);
        mapnik::image_32 im(m.width(),m.height());
        mapnik::agg_renderer<mapnik::image_32> ren(m,im);
        ren.apply();
        //mapnik::save_to_file(im,"test.png");
        return true;
    }
    void operator()() const
    {
        mapnik::Map m(256,256);
        mapnik::load_map(m,xml_);
        m.zoom_to_box(extent_);
        for (unsigned i=0;i<iterations_;++i)
        {
            mapnik::image_32 im(m.width(),m.height());
            mapnik::agg_renderer<mapnik::image_32> ren(m,im);
            ren.apply();
        }
    }
};


int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    mapnik::datasource_cache::instance().register_datasources("./plugins/input/");
    mapnik::box2d<double> z1(-20037508.3428,-8317435.0606,20037508.3428,18399242.7298);
    // bbox for 16/10491/22911.png
    mapnik::box2d<double> z16(-13622912.929097254,6026906.8062295765,-13621689.93664469,6028129.79868214);
    {
        test test_runner(params,
                          "benchmark/data/polygon_rendering_clip.xml",
                          z1);
        run(test_runner,"polygon clip render z1");        
    }
    {
        test test_runner(params,
                          "benchmark/data/polygon_rendering_no_clip.xml",
                          z1);
        run(test_runner,"polygon noclip render z1");        
    }
    {
        test test_runner(params,
                          "benchmark/data/polygon_rendering_clip.xml",
                          z16);
        run(test_runner,"polygon clip render z16");        
    }
    {
        test test_runner(params,
                          "benchmark/data/polygon_rendering_no_clip.xml",
                          z16);
        run(test_runner,"polygon noclip render z16");        
    }
    return 0;
}
