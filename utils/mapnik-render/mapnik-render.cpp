#include <mapnik/mapnik.hpp>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/version.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/filesystem.hpp>
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/adapted/struct.hpp>
MAPNIK_DISABLE_WARNING_POP

#include <string>

BOOST_FUSION_ADAPT_STRUCT(mapnik::box2d<double>, (double, minx_)(double, miny_)(double, maxx_)(double, maxy_))

int main(int argc, char** argv)
{
    namespace po = boost::program_options;

    mapnik::setup();

    bool verbose = false;
    bool auto_open = false;
    int return_value = 0;
    std::string xml_file;
    std::string img_file;
    double scale_factor = 1;
    bool params_as_variables = false;
    mapnik::logger logger;
    logger.set_severity(mapnik::logger::error);
    int map_width = 600;
    int map_height = 400;
    try
    {
        po::options_description desc("mapnik-render utility");
        // clang-format off
        desc.add_options()
            ("help,h", "produce usage message")
            ("version,V","print version string")
            ("verbose,v","verbose output")
            ("open","automatically open the file after rendering")
            ("xml",po::value<std::string>(),"xml map to read")
            ("img",po::value<std::string>(),"image to render")
            ("scale-factor",po::value<double>(),"scale factor for rendering")
            ("map-width",po::value<int>(),"map width in pixels")
            ("map-height",po::value<int>(),"map height in pixels")
            ("variables","make map parameters available as render-time variables")
            ("bbox", po::value<std::string>(), "bounding box  e.g <minx,miny,maxx,maxy> in Map's SRS")
            ("geographic,g","bounding box is in WGS 84 lon/lat")
            ("plugins-dir", po::value<std::string>(), "directory containing input plug-ins (default: ./plugins/input)")
            ("fonts-dir", po::value<std::string>(), "directory containing fonts (default: relative to <plugins-dir> or ./fonts if no <plugins-dir> specified)");
        // clang-format on
        po::positional_options_description p;
        p.add("xml", 1);
        p.add("img", 1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("version"))
        {
            std::clog << "version " << MAPNIK_VERSION_STRING << std::endl;
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
            xml_file = vm["xml"].as<std::string>();
        }
        else
        {
            std::clog << "mapnik-render: no XML map specified" << std::endl;
            std::clog << "Try \"mapnik-render --help\" for more information" << std::endl;
            return -1;
        }

        if (vm.count("img"))
        {
            img_file = vm["img"].as<std::string>();
        }
        else
        {
            std::clog << "please provide an img as second argument!" << std::endl;
            return -1;
        }

        if (vm.count("scale-factor"))
        {
            scale_factor = vm["scale-factor"].as<double>();
        }

        if (vm.count("variables"))
        {
            params_as_variables = true;
        }

        if (vm.count("map-width"))
        {
            map_width = vm["map-width"].as<int>();
        }

        if (vm.count("map-height"))
        {
            map_height = vm["map-height"].as<int>();
        }

        if (vm.count("plugins-dir"))
        {
            mapnik::datasource_cache::instance().register_datasources(vm["plugins-dir"].as<std::string>());
            if (vm.count("fonts-dir"))
            {
                mapnik::freetype_engine::register_fonts(vm["fonts-dir"].as<std::string>(), true);
            }
            else
            {
                // relative to plugins-dir
                try
                {
                    mapnik::fs::path p(vm["plugins-dir"].as<std::string>());
                    p = p.parent_path() / "fonts";
                    mapnik::freetype_engine::register_fonts(p.string(), true);
                }
                catch (...)
                {}
            }
        }
        else
        {
            mapnik::datasource_cache::instance().register_datasources("./plugins/input");
            mapnik::freetype_engine::register_fonts("./fonts", true);
        }
        if (verbose)
        {
            auto plugin_names = mapnik::datasource_cache::instance().plugin_names();
            if (plugin_names.empty())
            {
                std::cerr << "*WARNING*: no datasource plug-ings registered" << std::endl;
            }
            else
            {
                std::cerr << "Registered datasource plug-ins:";
                for (auto const& name : plugin_names)
                {
                    std::cerr << name << " ";
                }
                std::cerr << std::endl;
            }
        }

        mapnik::Map map(map_width, map_height);
        mapnik::load_map(map, xml_file, true);

        if (vm.count("bbox"))
        {
            namespace x3 = boost::spirit::x3;

            mapnik::box2d<double> bbox;
            std::string str = vm["bbox"].as<std::string>();

            auto start = str.begin();
            auto end = str.end();
            if (!x3::phrase_parse(start,
                                  end,
                                  x3::double_ >> -x3::lit(',') >> x3::double_ >> -x3::lit(',') >> x3::double_ >>
                                    -x3::lit(',') >> x3::double_,
                                  x3::space,
                                  bbox))
            {
                std::cerr << "Failed to parse BBOX: " << str << std::endl;
                return -1;
            }
            if (!bbox.valid())
            {
                std::cerr << "Invalid BBOX: " << str << std::endl;
                return -1;
            }
            if (vm.count("geographic"))
            {
                mapnik::projection source("epsg:4326");
                mapnik::projection destination(map.srs());
                mapnik::proj_transform tr(source, destination);
                if (!tr.forward(bbox))
                {
                    std::cerr << "Failed to project input BBOX into " << map.srs() << std::endl;
                    return -1;
                }
            }
            std::cerr << "zoom to:" << bbox << std::endl;
            map.zoom_to_box(bbox);
        }
        else
        {
            map.zoom_all();
        }
        mapnik::image_rgba8 im(map.width(), map.height());
        mapnik::request req(map.width(), map.height(), map.get_current_extent());
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
        mapnik::agg_renderer<mapnik::image_rgba8> ren(map, req, vars, im, scale_factor, 0, 0);
        ren.apply();
        mapnik::save_to_file(im, img_file);
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
