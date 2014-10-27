#include "bench_framework.hpp"
#include <mapnik/map.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/datasource_cache.hpp>
#include <stdexcept>

template <typename Renderer> void process_layers(Renderer & ren,
                                            mapnik::request const& m_req,
                                            mapnik::projection const& map_proj,
                                            std::vector<mapnik::layer> const& layers,
                                            double scale_denom)
{
    unsigned layers_size = layers.size();
    for (unsigned i=0; i < layers_size; ++i)
    {
        mapnik::layer const& lyr = layers[i];
        if (lyr.visible(scale_denom))
        {
            std::set<std::string> names;
            mapnik::parameters p;
            p["type"]="csv";
            p["file"]="benchmark/data/roads.csv";
            mapnik::datasource_ptr ds = mapnik::datasource_cache::instance().create(p);
            mapnik::layer l(lyr);
            l.set_datasource(ds);
            l.add_style("labels");
            ren.apply_to_layer(l,
                               ren,
                               map_proj,
                               m_req.scale(),
                               scale_denom,
                               m_req.width(),
                               m_req.height(),
                               m_req.extent(),
                               m_req.buffer_size(),
                               names);
        }
    }
}

class test : public benchmark::test_case
{
    std::string xml_;
    mapnik::box2d<double> extent_;
    mapnik::value_integer width_;
    mapnik::value_integer height_;
    std::shared_ptr<mapnik::Map> m_;
    double scale_factor_;
    std::string preview_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       xml_(),
       extent_(),
       width_(*params.get<mapnik::value_integer>("width",256)),
       height_(*params.get<mapnik::value_integer>("height",256)),
       m_(new mapnik::Map(width_,height_)),
       scale_factor_(*params.get<mapnik::value_double>("scale_factor",2.0)),
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
        mapnik::load_map(*m_,xml_,true);
    }

    bool validate() const
    {
        mapnik::request m_req(width_,height_,extent_);
        mapnik::image_32 im(m_->width(),m_->height());
        mapnik::attributes variables;
        m_req.set_buffer_size(m_->buffer_size());
        mapnik::projection map_proj(m_->srs(),true);
        double scale_denom = mapnik::scale_denominator(m_req.scale(),map_proj.is_geographic());
        scale_denom *= scale_factor_;
        mapnik::agg_renderer<mapnik::image_32> ren(*m_,m_req,variables,im,scale_factor_);
        ren.start_map_processing(*m_);
        std::vector<mapnik::layer> const& layers = m_->layers();
        process_layers(ren,m_req,map_proj,layers,scale_denom);
        ren.end_map_processing(*m_);
        if (!preview_.empty()) {
            std::clog << "preview available at " << preview_ << "\n";
            mapnik::save_to_file(im,preview_);
        }
        return true;
    }

    void operator()() const
    {
        if (preview_.empty()) {
            for (unsigned i=0;i<iterations_;++i)
            {
                mapnik::request m_req(width_,height_,extent_);
                mapnik::image_32 im(m_->width(),m_->height());
                mapnik::attributes variables;
                m_req.set_buffer_size(m_->buffer_size());
                mapnik::projection map_proj(m_->srs(),true);
                double scale_denom = mapnik::scale_denominator(m_req.scale(),map_proj.is_geographic());
                scale_denom *= scale_factor_;
                mapnik::agg_renderer<mapnik::image_32> ren(*m_,m_req,variables,im,scale_factor_);
                ren.start_map_processing(*m_);
                std::vector<mapnik::layer> const& layers = m_->layers();
                process_layers(ren,m_req,map_proj,layers,scale_denom);
                ren.end_map_processing(*m_);
            }            
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
        mapnik::freetype_engine::register_fonts("./fonts/",true);
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
