#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/version.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

#include <string>

int main (int argc,char** argv)
{
    namespace po = boost::program_options;

    bool verbose = false;
    bool auto_open = true;
    int return_value = 0;
    std::string xml_file;
    std::string img_file;
    double scale_factor = 1;
    bool params_as_variables = false;
    mapnik::logger logger;
    logger.set_severity(mapnik::logger::error);

    try
    {
        po::options_description desc("nik2img utility");
        desc.add_options()
            ("help,h", "produce usage message")
            ("version,V","print version string")
            ("verbose,v","verbose output")
            ("open","automatically open the file after rendering (os x only)")
            ("xml",po::value<std::string>(),"xml map to read")
            ("img",po::value<std::string>(),"image to render")
            ("scale-factor",po::value<double>(),"scale factor for rendering")
            ("variables","make map parameters available as render-time variables")
            ;

        po::positional_options_description p;
        p.add("xml",1);
        p.add("img",1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("version"))
        {
            std::clog <<"version " << MAPNIK_VERSION_STRING << std::endl;
            return 1;
        }

        if (vm.count("help"))
        {
            std::clog << desc << std::endl;
            return 1;
        }

        if (vm.count("verbose"))
        {
            verbose = true;
        }

        if (vm.count("open"))
        {
            auto_open = true;
        }

        if (vm.count("xml"))
        {
            xml_file=vm["xml"].as<std::string>();
        }
        else
        {
            std::clog << "please provide an xml map as first argument!" << std::endl;
            return -1;
        }

        if (vm.count("img"))
        {
            img_file=vm["img"].as<std::string>();
        }
        else
        {
            std::clog << "please provide an img as second argument!" << std::endl;
            return -1;
        }

        if (vm.count("scale-factor"))
        {
            scale_factor=vm["scale-factor"].as<double>();
        }

        if (vm.count("variables"))
        {
            params_as_variables = true;
        }

        mapnik::datasource_cache::instance().register_datasources("./plugins/input/");
        mapnik::freetype_engine::register_fonts("./fonts",true);
        mapnik::Map map(600,400);
        mapnik::load_map(map,xml_file,true);
        map.zoom_all();
        mapnik::image_rgba8 im(map.width(),map.height());
        mapnik::request req(map.width(),map.height(),map.get_current_extent());
        req.set_buffer_size(map.buffer_size());
        mapnik::attributes vars;
        if (params_as_variables)
        {
            mapnik::transcoder tr("utf-8");
            for (auto const& param : map.get_extra_parameters())
            {
                std::string const& name = param.first.substr(1);
                if (!name.empty())
                {
                    if (param.second.is<mapnik::value_integer>())
                    {
                        vars[name] = param.second.get<mapnik::value_integer>();
                    }
                    else if (param.second.is<mapnik::value_double>())
                    {
                        vars[name] = param.second.get<mapnik::value_double>();
                    }
                    else if (param.second.is<std::string>())
                    {
                        vars[name] = tr.transcode(param.second.get<std::string>().c_str());
                    }
                }
            }
        }
        mapnik::agg_renderer<mapnik::image_rgba8> ren(map,req,vars,im,scale_factor,0,0);
        ren.apply();
        mapnik::save_to_file(im,img_file);
        if (auto_open)
        {
            std::ostringstream s;
#ifdef __APPLE__
            s << "open ";
#elif _WIN32
            s << "start ";
#else
            s << "xdg-open ";
#endif
            s << img_file;
            int ret = system(s.str().c_str());
            if (ret != 0)
                return_value = ret;
        }
        else
        {
            std::clog << "rendered to: " << img_file << "\n";
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << "Error " << ex.what() << std::endl;
        return -1;
    }
    return return_value;
}
