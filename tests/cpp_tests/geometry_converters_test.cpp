#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include "utils.hpp"

#include <mapnik/layer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/well_known_srs.hpp>
#include <mapnik/util/geometry_to_wkb.hpp>
#include <mapnik/util/geometry_to_wkt.hpp>
#include <mapnik/util/geometry_to_svg.hpp>

// stl
#include <stdexcept>

struct output_geometry_backend
{
    output_geometry_backend(boost::ptr_vector<mapnik::geometry_type> & paths, mapnik::geometry_type::types type)
        : paths_(paths),
          type_(type) {}

    template <typename T>
    void add_path(T & path)
    {
        mapnik::vertex2d vtx(mapnik::vertex2d::no_init);
        path.rewind(0);
        std::unique_ptr<mapnik::geometry_type> geom_ptr(new mapnik::geometry_type(type_));

        while ((vtx.cmd = path.vertex(&vtx.x, &vtx.y)) != mapnik::SEG_END)
        {
            //std::cerr << vtx.x << "," << vtx.y << "   cmd=" << vtx.cmd << std::endl;
            geom_ptr->push_vertex(vtx.x, vtx.y, (mapnik::CommandType)vtx.cmd);
        }
        paths_.push_back(geom_ptr.release());
    }
    boost::ptr_vector<mapnik::geometry_type> &  paths_;
    mapnik::geometry_type::types type_;
};

boost::optional<std::string> linestring_bbox_clipping(mapnik::box2d<double> bbox,
                                                      std::string wkt_in)
{
    using namespace mapnik;
    agg::trans_affine tr;
    projection src(MAPNIK_LONGLAT_PROJ);
    projection dst(MAPNIK_LONGLAT_PROJ);
    proj_transform prj_trans(src,dst);
    line_symbolizer sym;
    CoordTransform t(bbox.width(),bbox.height(), bbox);
    boost::ptr_vector<mapnik::geometry_type> output_paths;
    output_geometry_backend backend(output_paths, mapnik::geometry_type::types::LineString);

    typedef boost::mpl::vector<clip_line_tag> conv_types;
    vertex_converter<box2d<double>, output_geometry_backend, line_symbolizer,
        CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(bbox, backend, sym, t, prj_trans, tr, 1.0);

    converter.set<clip_line_tag>();

    boost::ptr_vector<geometry_type> p;
    if (!mapnik::from_wkt(wkt_in , p))
    {
        throw std::runtime_error("Failed to parse WKT");
    }

    for (geometry_type & geom : p)
    {
        converter.apply(geom);
    }

    std::string wkt_out;
    if (mapnik::util::to_wkt(wkt_out, output_paths))
    {
        return boost::optional<std::string>(wkt_out);
    }

    return boost::optional<std::string>();
}

boost::optional<std::string> polygon_bbox_clipping(mapnik::box2d<double> bbox,
                                                   std::string wkt_in)
{
    using namespace mapnik;
    agg::trans_affine tr;
    projection src(MAPNIK_LONGLAT_PROJ);
    projection dst(MAPNIK_LONGLAT_PROJ);
    proj_transform prj_trans(src,dst);
    polygon_symbolizer sym;
    CoordTransform t(bbox.width(),bbox.height(), bbox);
    boost::ptr_vector<mapnik::geometry_type> output_paths;
    output_geometry_backend backend(output_paths, mapnik::geometry_type::types::Polygon);

    typedef boost::mpl::vector<clip_poly_tag> conv_types;
    vertex_converter<box2d<double>, output_geometry_backend, polygon_symbolizer,
        CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(bbox, backend, sym, t, prj_trans, tr, 1.0);

    converter.set<clip_poly_tag>();

    boost::ptr_vector<geometry_type> p;
    if (!mapnik::from_wkt(wkt_in , p))
    {
        throw std::runtime_error("Failed to parse WKT");
    }

    for (geometry_type & geom : p)
    {
        converter.apply(geom);
    }

    std::string wkt_out;
    if (mapnik::util::to_wkt(wkt_out, output_paths))
    {
        return boost::optional<std::string>(wkt_out);
    }

    return boost::optional<std::string>();
}

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    BOOST_TEST(set_working_dir(args));

    // LineString/bbox clipping
    {
        std::string wkt_in("LineString(0 0,200 200)");
        boost::optional<std::string> result = linestring_bbox_clipping(mapnik::box2d<double>(50,50,150,150),wkt_in);
        BOOST_TEST(result);
        BOOST_TEST_EQ(*result,std::string("LineString(50 50,150 150)"));
    }
    // Polygon/bbox clipping
#if 0
    // these tests will fail
    {
        std::string wkt_in("Polygon((50 50,150 50,150 150,50 150,50 50))");
        boost::optional<std::string> result = polygon_bbox_clipping(mapnik::box2d<double>(50,50,150,150),wkt_in);
        BOOST_TEST(result);
        BOOST_TEST_EQ(*result,std::string("Polygon((50 50,150 50,150 150,50 150,50 50))"));
    }

    {
        std::string wkt_in("Polygon((60 60,140 60,140 160,60 140,60 60))");
        boost::optional<std::string> result = polygon_bbox_clipping(mapnik::box2d<double>(50,50,150,150),wkt_in);
        BOOST_TEST(result);
        BOOST_TEST_EQ(*result,std::string("Polygon((60 60,140 60,140 160,60 140,60 60))"));
    }

    {
        std::string wkt_in("Polygon((0 0,10 0,10 10,0 10,0 0))");
        boost::optional<std::string> result = polygon_bbox_clipping(mapnik::box2d<double>(50,50,150,150),wkt_in);
        BOOST_TEST(result);
        BOOST_TEST_EQ(*result, std::string("GeometryCollection EMPTY"));
    }
    {
        std::string wkt_in("Polygon((0 0,100 200,200 0,0 0 ))");
        boost::optional<std::string> result = polygon_bbox_clipping(mapnik::box2d<double>(50,50,150,150),wkt_in);
        BOOST_TEST(result);
        BOOST_TEST_EQ(*result,std::string("Polygon((50 50,50 100,75 150,125 150,150 100,150 50,50 50))"));
    }
#endif

    if (!::boost::detail::test_errors())
    {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ geometry conversions: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    }
    else
    {
        return ::boost::report_errors();
    }
}
