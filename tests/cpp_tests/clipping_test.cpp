
// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>

// boost
#include <boost/detail/lightweight_test.hpp>
#include <boost/algorithm/string.hpp>

// stl
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

// agg
#include "agg_conv_clip_polygon.h"
#include "agg_conv_clip_polyline.h"
//#include "agg_path_storage.h"
//#include "agg_conv_clipper.h"

#include "utils.hpp"

template <typename T>
std::string dump_path(T & path)
{
    unsigned cmd = 1;
    double x = 0;
    double y = 0;
    unsigned idx = 0;
    std::ostringstream s;
    path.rewind(0);
    while ((cmd = path.vertex(&x, &y)) != mapnik::SEG_END)
    {
        if (idx > 0) s << ",";
        s << x << " " << y << " " << cmd;
        idx++;
    }
    return s.str();
}

std::string clip_line(mapnik::box2d<double> const& bbox,
                      mapnik::geometry_type & geom)
{
    typedef agg::conv_clip_polyline<mapnik::geometry_type> line_clipper;
    line_clipper clipped(geom);
    clipped.clip_box(bbox.minx(),bbox.miny(),bbox.maxx(),bbox.maxy());
    return dump_path(clipped);
}

void parse_geom(mapnik::geometry_type & geom,
                std::string const& geom_string) {
    std::vector<std::string> vertices;
    boost::split(vertices, geom_string, boost::is_any_of(","));
    for (std::string const& vert : vertices)
    {
        std::vector<std::string> commands;
        boost::split(commands, vert, boost::is_any_of(" "));
        if (commands.size() != 3)
        {
            throw std::runtime_error(std::string("could not parse geometry '") + geom_string + "'");
        }
        double x = 0;
        double y = 0;
        int c = 0;
        if (mapnik::util::string2double(commands[0],x)
            && mapnik::util::string2double(commands[1],y)
            && mapnik::util::string2int(commands[2],c))
        {
            geom.push_vertex(x,y,(mapnik::CommandType)c);
        }
        else
        {
            throw std::runtime_error(std::string("could not parse geometry '") + geom_string + "'");
        }
    }
}

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    try {

        BOOST_TEST(set_working_dir(args));

        std::string filename("tests/cpp_tests/data/cases.txt");
        std::ifstream stream(filename.c_str(),std::ios_base::in | std::ios_base::binary);
        if (!stream.is_open())
            throw std::runtime_error("could not open: '" + filename + "'");

        std::string csv_line;
        while(std::getline(stream,csv_line,'\n'))
        {
            if (csv_line.empty() || csv_line[0] == '#') continue;
            std::vector<std::string> parts;
            boost::split(parts, csv_line, boost::is_any_of(";"));
            // first part is clipping box
            mapnik::box2d<double> bbox;
            if (!bbox.from_string(parts[0])) {
                throw std::runtime_error(std::string("could not parse bbox '") + parts[0] + "'");
            }
            // second part is input geometry
            mapnik::geometry_type geom;
            parse_geom(geom,parts[1]);
            //std::clog << dump_path(geom) << "\n";
            // third part is expected, clipped geometry
            BOOST_TEST_EQ(clip_line(bbox,geom),mapnik::util::trim_copy(parts[2]));
        }
        stream.close();
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
    }

    if (!::boost::detail::test_errors())
    {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ clipping: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    }
    else
    {
        return ::boost::report_errors();
    }
}
