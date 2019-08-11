
#include "catch.hpp"

// mapnik
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/path.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

// stl
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

// agg
#include "agg_conv_clip_polyline.h"

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
                      mapnik::path_type const& path)
{
    using line_clipper = agg::conv_clip_polyline<mapnik::vertex_adapter>;
    mapnik::vertex_adapter va(path);
    line_clipper clipped(va);
    clipped.clip_box(bbox.minx(),bbox.miny(),bbox.maxx(),bbox.maxy());
    return dump_path(clipped);
}

void parse_geom(mapnik::path_type & path,
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
            path.push_vertex(x,y,(mapnik::CommandType)c);
        }
        else
        {
            throw std::runtime_error(std::string("could not parse geometry '") + geom_string + "'");
        }
    }
}

TEST_CASE("clipping") {

SECTION("lines") {

    try {

        std::string filename("test/unit/data/cases.txt");
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
            mapnik::path_type path;
            parse_geom(path, parts[1]);
            //std::clog << dump_path(path) << "\n";
            // third part is expected, clipped geometry
            REQUIRE(clip_line(bbox, path) == mapnik::util::trim_copy(parts[2]));
        }
        stream.close();
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
    }

}

}
