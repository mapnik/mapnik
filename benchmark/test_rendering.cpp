#include "bench_framework.hpp"
#include <mapnik/map.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <stdexcept>

class test : public benchmark::test_case
{
    std::string xml_;
    mapnik::box2d<double> extent_;
    mapnik::value_integer width_;
    mapnik::value_integer height_;
    std::string preview_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       xml_(),
       extent_(),
       width_(*params.get<mapnik::value_integer>("width",256)),
       height_(*params.get<mapnik::value_integer>("height",256)),
       preview_(*params.get<std::string>("preview",""))
      {
        boost::optional<std::string> map = params.get<std::string>("map");
        if (!map)
        {
            throw std::runtime_error("please provide a --map=<path to xml> arg");
        }
        xml_ = *map;

        boost::optional<std::string> ext = params.get<std::string>("extent");
        if (ext && !ext->empty())
        {
            if (!extent_.from_string(*ext))
                throw std::runtime_error("could not parse `extent` string" + *ext);
        }
        else
        {
            throw std::runtime_error("please provide a --extent=<minx,miny,maxx,maxy> arg");
        }

      }
    bool validate() const
    {
        mapnik::Map m(width_,height_);
        mapnik::load_map(m,xml_,true);
        m.zoom_to_box(extent_);
        mapnik::image_32 im(m.width(),m.height());
        mapnik::agg_renderer<mapnik::image_32> ren(m,im);
        ren.apply();
        if (!preview_.empty()) mapnik::save_to_file(im,preview_);
        return true;
    }
    void operator()() const
    {
        mapnik::Map m(width_,height_);
        mapnik::load_map(m,xml_,true);
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
    try
    {
        mapnik::parameters params;
        benchmark::handle_args(argc,argv,params);
        boost::optional<std::string> name = params.get<std::string>("name");
        if (!name)
        {
            std::clog << "please provide a name for this test\n";
            return -1;
        }
        mapnik::datasource_cache::instance().register_datasources("./plugins/input/");
        {
            test test_runner(params);
            run(test_runner,*name);        
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << ex.what() << "\n";
        return -1;
    }
    return 0;
}
