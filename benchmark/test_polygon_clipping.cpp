#include "bench_framework.hpp"
#include "compare_images.hpp"
#include <mapnik/vertex.hpp>
#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <mapnik/geometry_envelope.hpp>
#include <mapnik/geometry_correct.hpp>
#include <mapnik/geometry_is_empty.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/color.hpp>
// boost geometry
#include <boost/geometry.hpp>
// agg
#include "agg_conv_clip_polygon.h"
// clipper
#include "agg_path_storage.h"
// rendering
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"

// stl
#include <fstream>
#include <iostream>
#include <cstdlib>

void render(mapnik::geometry::multi_polygon<double> const& geom,
            mapnik::box2d<double> const& extent,
            std::string const& name)
{
    using path_type = mapnik::transform_path_adapter<mapnik::view_transform,mapnik::geometry::polygon_vertex_adapter<double>>;
    using ren_base = agg::renderer_base<agg::pixfmt_rgba32_plain>;
    using renderer = agg::renderer_scanline_aa_solid<ren_base>;
    mapnik::image_rgba8 im(256,256);
    mapnik::fill(im, mapnik::color("white"));
    mapnik::box2d<double> padded_extent(155,134,665,466);//extent;
    padded_extent.pad(10);
    mapnik::view_transform tr(im.width(),im.height(),padded_extent,0,0);
    agg::rendering_buffer buf(im.bytes(),im.width(),im.height(), im.row_size());
    agg::pixfmt_rgba32_plain pixf(buf);
    ren_base renb(pixf);
    renderer ren(renb);
    mapnik::proj_transform prj_trans(mapnik::projection("+init=epsg:4326"),mapnik::projection("+init=epsg:4326"));
    ren.color(agg::rgba8(127,127,127,255));
    agg::rasterizer_scanline_aa<> ras;
    for (auto const& poly : geom)
    {
        mapnik::geometry::polygon_vertex_adapter<double> va(poly);
        path_type path(tr,va,prj_trans);
        ras.add_path(path);
    }
    agg::scanline_u8 sl;
    agg::render_scanlines(ras, sl, ren);
    mapnik::save_to_file(im,name);
}

class test1 : public benchmark::test_case
{
    std::string wkt_in_;
    mapnik::box2d<double> extent_;
    std::string expected_;
public:
    using conv_clip = agg::conv_clip_polygon<mapnik::geometry::polygon_vertex_adapter<double>>;
    test1(mapnik::parameters const& params,
          std::string const& wkt_in,
          mapnik::box2d<double> const& extent)
     : test_case(params),
       wkt_in_(wkt_in),
       extent_(extent),
       expected_("./benchmark/data/polygon_clipping_agg") {}
    bool validate() const
    {
        mapnik::geometry::geometry<double> geom;
        if (!mapnik::from_wkt(wkt_in_, geom))
        {
            throw std::runtime_error("Failed to parse WKT");
        }
        if (mapnik::geometry::is_empty(geom))
        {
            std::clog << "empty geom!\n";
            return false;
        }
        if (!geom.is<mapnik::geometry::polygon<double>>())
        {
            std::clog << "not a polygon!\n";
            return false;
        }
        mapnik::geometry::polygon<double> const& poly = mapnik::util::get<mapnik::geometry::polygon<double>>(geom);
        mapnik::geometry::polygon_vertex_adapter<double> va(poly);


        conv_clip clipped(va);
        clipped.clip_box(
                    extent_.minx(),
                    extent_.miny(),
                    extent_.maxx(),
                    extent_.maxy());


        clipped.rewind(0);
        mapnik::geometry::polygon<double> poly2;
        mapnik::geometry::linear_ring<double> ring;
        // exterior ring
        unsigned cmd;
        double x, y, x0, y0;
        while ((cmd = clipped.vertex(&x, &y)) != mapnik::SEG_END)
        {
            if (cmd == mapnik::SEG_MOVETO)
            {
                x0 = x; y0 = y;
            }

            if (cmd == mapnik::SEG_CLOSE)
            {
                ring.add_coord(x0, y0);
                break;
            }
            ring.add_coord(x,y);
        }
        poly2.set_exterior_ring(std::move(ring));
        // interior rings
        ring.clear();
        while ((cmd = clipped.vertex(&x, &y)) != mapnik::SEG_END)
        {
            if (cmd == mapnik::SEG_MOVETO)
            {
                x0 = x; y0 = y;
            }
            else if (cmd == mapnik::SEG_CLOSE)
            {
                ring.add_coord(x0,y0);
                poly2.add_hole(std::move(ring));
                ring.clear();
                continue;
            }
            ring.add_coord(x,y);
        }

        std::string expect = expected_+".png";
        std::string actual = expected_+"_actual.png";
        mapnik::geometry::multi_polygon<double> mp;
        mp.emplace_back(poly2);
        auto env = mapnik::geometry::envelope(mp);
        if (!mapnik::util::exists(expect) || (std::getenv("UPDATE") != nullptr))
        {
            std::clog << "generating expected image: " << expect << "\n";
            render(mp,env,expect);
        }
        render(mp,env,actual);
        return benchmark::compare_images(actual,expect);
    }
    bool operator()() const
    {
        mapnik::geometry::geometry<double> geom;
        if (!mapnik::from_wkt(wkt_in_, geom))
        {
            throw std::runtime_error("Failed to parse WKT");
        }
        if (mapnik::geometry::is_empty(geom))
        {
            std::clog << "empty geom!\n";
            return false;
        }
        if (!geom.is<mapnik::geometry::polygon<double>>())
        {
            std::clog << "not a polygon!\n";
            return false;
        }
        bool valid = true;
        for (unsigned i=0;i<iterations_;++i)
        {
            unsigned count = 0;
            mapnik::geometry::polygon<double> const& poly = mapnik::util::get<mapnik::geometry::polygon<double>>(geom);
            mapnik::geometry::polygon_vertex_adapter<double> va(poly);
            conv_clip clipped(va);
            clipped.clip_box(
                        extent_.minx(),
                        extent_.miny(),
                        extent_.maxx(),
                        extent_.maxy());
            unsigned cmd;
            double x,y;
            // NOTE: this rewind is critical otherwise
            // agg_conv_adapter_vpgen will give garbage
            // values for the first vertex
            clipped.rewind(0);
            while ((cmd = clipped.vertex(&x, &y)) != mapnik::SEG_END) {
                count++;
            }
            unsigned expected_count = 30;
            if (count != expected_count) {
                std::clog << "test1: clipping failed: processed " << count << " verticies but expected " << expected_count << "\n";
                valid = false;
            }
        }
        return valid;
    }
};


class test3 : public benchmark::test_case
{
    std::string wkt_in_;
    mapnik::box2d<double> extent_;
    std::string expected_;
public:
    test3(mapnik::parameters const& params,
          std::string const& wkt_in,
          mapnik::box2d<double> const& extent)
     : test_case(params),
       wkt_in_(wkt_in),
       extent_(extent),
       expected_("./benchmark/data/polygon_clipping_boost") {}
    bool validate() const
    {
        mapnik::geometry::geometry<double> geom;
        if (!mapnik::from_wkt(wkt_in_, geom))
        {
            throw std::runtime_error("Failed to parse WKT");
        }
        if (mapnik::geometry::is_empty(geom))
        {
            std::clog << "empty geom!\n";
            return false;
        }
        if (!geom.is<mapnik::geometry::polygon<double> >())
        {
            std::clog << "not a polygon!\n";
            return false;
        }
        mapnik::geometry::polygon<double> & poly = mapnik::util::get<mapnik::geometry::polygon<double> >(geom);
        mapnik::geometry::correct(poly);

        mapnik::geometry::linear_ring<double> bbox;
        bbox.add_coord(extent_.minx(), extent_.miny());
        bbox.add_coord(extent_.minx(), extent_.maxy());
        bbox.add_coord(extent_.maxx(), extent_.maxy());
        bbox.add_coord(extent_.maxx(), extent_.miny());
        bbox.add_coord(extent_.minx(), extent_.miny());

        std::deque<mapnik::geometry::polygon<double> > result;
        boost::geometry::intersection(bbox, poly, result);

        std::string expect = expected_+".png";
        std::string actual = expected_+"_actual.png";
        mapnik::geometry::multi_polygon<double> mp;
        for (auto const& _geom: result)
        {
            //std::clog << boost::geometry::dsv(geom) << "\n";
            mp.emplace_back(_geom);
        }
        mapnik::geometry::geometry<double> geom2(mp);
        auto env = mapnik::geometry::envelope(geom2);
        if (!mapnik::util::exists(expect) || (std::getenv("UPDATE") != nullptr))
        {
            std::clog << "generating expected image: " << expect << "\n";
            render(mp,env,expect);
        }
        render(mp,env,actual);
        return benchmark::compare_images(actual,expect);
    }
    bool operator()() const
    {
        mapnik::geometry::geometry<double> geom;
        if (!mapnik::from_wkt(wkt_in_, geom))
        {
            throw std::runtime_error("Failed to parse WKT");
        }
        if (mapnik::geometry::is_empty(geom))
        {
            std::clog << "empty geom!\n";
            return false;
        }
        if (!geom.is<mapnik::geometry::polygon<double> >())
        {
            std::clog << "not a polygon!\n";
            return false;
        }
        mapnik::geometry::polygon<double> & poly = mapnik::util::get<mapnik::geometry::polygon<double> >(geom);
        mapnik::geometry::correct(poly);

        mapnik::geometry::linear_ring<double> bbox;
        bbox.add_coord(extent_.minx(), extent_.miny());
        bbox.add_coord(extent_.minx(), extent_.maxy());
        bbox.add_coord(extent_.maxx(), extent_.maxy());
        bbox.add_coord(extent_.maxx(), extent_.miny());
        bbox.add_coord(extent_.minx(), extent_.miny());

        bool valid = true;
        for (unsigned i=0;i<iterations_;++i)
        {
            std::deque<mapnik::geometry::polygon<double> > result;
            boost::geometry::intersection(bbox, poly, result);
            unsigned count = 0;
            for (auto const& _geom : result)
            {
                mapnik::geometry::polygon_vertex_adapter<double> va(_geom);
                unsigned cmd;
                double x,y;
                while ((cmd = va.vertex(&x, &y)) != mapnik::SEG_END) {
                    ++count;
                }
                unsigned expected_count = 29;
                if (count != expected_count) {
                    std::clog << "test3: clipping failed: processed " << count << " verticies but expected " << expected_count << "\n";
                    valid = false;
                }
            }
        }
        return valid;
    }
};

/*
Commented out section because clipper moved out of mapnik core.
inline void process_polynode_branch(ClipperLib::PolyNode* polynode,
                             mapnik::geometry::multi_polygon<double> & mp)
{
    mapnik::geometry::polygon<double> polygon;
    mapnik::geometry::linear_ring<double> outer;
    for (auto const& pt : polynode->Contour)
    {
        outer.emplace_back(static_cast<double>(pt.x),static_cast<double>(pt.y));
    }
    if (outer.front() != outer.back())
    {
        outer.emplace_back(outer.front().x, outer.front().y);
    }
    polygon.set_exterior_ring(std::move(outer));
    for (auto * ring : polynode->Childs)
    {
        mapnik::geometry::linear_ring<double> inner;
        for (auto const& pt : ring->Contour)
        {
            inner.emplace_back(static_cast<double>(pt.x),static_cast<double>(pt.y));
        }
        if (inner.front() != inner.back())
        {
            inner.emplace_back(inner.front().x, inner.front().y);
        }
        polygon.add_hole(std::move(inner));
    }
    mp.emplace_back(std::move(polygon));
    for (auto * ring : polynode->Childs)
    {
        for (auto * sub_ring : ring->Childs)
        {
            process_polynode_branch(sub_ring, mp);
        }
    }
}

class test4 : public benchmark::test_case
{
    std::string wkt_in_;
    mapnik::box2d<double> extent_;
    std::string expected_;
public:
    test4(mapnik::parameters const& params,
          std::string const& wkt_in,
          mapnik::box2d<double> const& extent)
     : test_case(params),
       wkt_in_(wkt_in),
       extent_(extent),
       expected_("./benchmark/data/polygon_clipping_clipper") {}
    bool validate() const
    {
        mapnik::geometry::geometry<double> geom;
        if (!mapnik::from_wkt(wkt_in_, geom))
        {
            throw std::runtime_error("Failed to parse WKT");
        }
        if (mapnik::geometry::is_empty(geom))
        {
            std::clog << "empty geom!\n";
            return false;
        }
        if (!geom.is<mapnik::geometry::polygon<double> >())
        {
            std::clog << "not a polygon!\n";
            return false;
        }
        mapnik::geometry::polygon<double> & poly = mapnik::util::get<mapnik::geometry::polygon<double> >(geom);
        mapnik::geometry::correct(poly);
        ClipperLib::Clipper clipper;

        mapnik::geometry::line_string<std::int64_t> path;
        for (auto const& pt : poly.exterior_ring)
        {
            double x = pt.x;
            double y = pt.y;
            path.emplace_back(static_cast<ClipperLib::cInt>(x),static_cast<ClipperLib::cInt>(y));
        }
        double area = ClipperLib::Area(path);
        if (area > 0)
        {
            std::reverse(path.begin(), path.end());
        }
        if (!clipper.AddPath(path, ClipperLib::ptSubject, true))
        {
            std::clog << "ptSubject ext failed!\n";
        }
        for (auto const& ring : poly.interior_rings)
        {
            path.clear();
            for (auto const& pt : ring)
            {
                double x = pt.x;
                double y = pt.y;
                path.emplace_back(static_cast<ClipperLib::cInt>(x),static_cast<ClipperLib::cInt>(y));
            }
            area = ClipperLib::Area(path);
            if (area < 0)
            {
                std::reverse(path.begin(), path.end());
            }
            if (!clipper.AddPath(path, ClipperLib::ptSubject, true))
            {
                std::clog << "ptSubject ext failed!\n";
            }
        }
        std::cerr << "path size=" << path.size() << std::endl;
        mapnik::geometry::line_string<std::int64_t> clip_box;
        clip_box.emplace_back(static_cast<ClipperLib::cInt>(extent_.minx()),static_cast<ClipperLib::cInt>(extent_.miny()));
        clip_box.emplace_back(static_cast<ClipperLib::cInt>(extent_.maxx()),static_cast<ClipperLib::cInt>(extent_.miny()));
        clip_box.emplace_back(static_cast<ClipperLib::cInt>(extent_.maxx()),static_cast<ClipperLib::cInt>(extent_.maxy()));
        clip_box.emplace_back(static_cast<ClipperLib::cInt>(extent_.minx()),static_cast<ClipperLib::cInt>(extent_.maxy()));
        clip_box.emplace_back(static_cast<ClipperLib::cInt>(extent_.minx()),static_cast<ClipperLib::cInt>(extent_.miny()));

        if (!clipper.AddPath( clip_box, ClipperLib::ptClip, true ))
        {
            std::clog << "ptClip failed!\n";
        }

        ClipperLib::PolyTree polygons;
        clipper.Execute(ClipperLib::ctIntersection, polygons);// ClipperLib::pftNonZero);
        clipper.Clear();
        mapnik::geometry::multi_polygon<double> mp;
        for (auto * polynode : polygons.Childs)
        {
             process_polynode_branch(polynode, mp);
        }
        std::string expect = expected_+".png";
        std::string actual = expected_+"_actual.png";
        //mapnik::geometry::geometry<double> geom2(mp);
        auto env = mapnik::geometry::envelope(mp);
        if (!mapnik::util::exists(expect) || (std::getenv("UPDATE") != nullptr))
        {
            std::clog << "generating expected image: " << expect << "\n";
            render(mp,env,expect);
        }
        render(mp,env,actual);
        return benchmark::compare_images(actual,expect);
    }
    bool operator()() const
    {
        mapnik::geometry::geometry<double> geom;
        if (!mapnik::from_wkt(wkt_in_, geom))
        {
            throw std::runtime_error("Failed to parse WKT");
        }
        if (mapnik::geometry::is_empty(geom))
        {
            std::clog << "empty geom!\n";
            return false;
        }
        if (!geom.is<mapnik::geometry::polygon<double> >())
        {
            std::clog << "not a polygon!\n";
            return false;
        }
        mapnik::geometry::polygon<double> & poly = mapnik::util::get<mapnik::geometry::polygon<double> >(geom);
        mapnik::geometry::correct(poly);

        bool valid = true;
        for (unsigned i=0;i<iterations_;++i)
        {
            std::deque<mapnik::geometry::polygon<double> > result;
            boost::geometry::intersection(extent_,poly,result);

            unsigned count = 0;
            for (auto const& geom : result)
            {
                mapnik::geometry::polygon_vertex_adapter<double> va(geom);
                unsigned cmd;
                double x,y;
                while ((cmd = va.vertex(&x, &y)) != mapnik::SEG_END) {
                    ++count;
                }
                unsigned expected_count = 29;
                if (count != expected_count) {
                    std::clog << "test3: clipping failed: processed " << count << " verticies but expected " << expected_count << "\n";
                    valid = false;
                }
            }
        }
        return valid;
    }
};
*/

int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);

    // polygon/rect clipping
    // IN : POLYGON ((155 203, 233 454, 315 340, 421 446, 463 324, 559 466, 665 253, 528 178, 394 229, 329 138, 212 134, 183 228, 200 264, 155 203),(313 190, 440 256, 470 248, 510 305, 533 237, 613 263, 553 397, 455 262, 405 378, 343 287, 249 334, 229 191, 313 190))
    // RECT : POLYGON ((181 106, 181 470, 631 470, 631 106, 181 106))
    // OUT (expected)
    // POLYGON ((181 286.6666666666667, 233 454, 315 340, 421 446, 463 324, 559 466, 631 321.3207547169811, 631 234.38686131386862, 528 178, 394 229, 329 138, 212 134, 183 228, 200 264, 181 238.24444444444444, 181 286.6666666666667),(313 190, 440 256, 470 248, 510 305, 533 237, 613 263, 553 397, 455 262, 405 378, 343 287, 249 334, 229 191, 313 190))

    mapnik::box2d<double> clipping_box(181,106,631,470);
    std::string filename_("./benchmark/data/polygon.wkt");
    std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
    if (!in.is_open())
        throw std::runtime_error("could not open: '" + filename_ + "'");
    std::string wkt_in( (std::istreambuf_iterator<char>(in) ),
               (std::istreambuf_iterator<char>()) );
    {
        test1 test_runner(params,wkt_in,clipping_box);
        run(test_runner,"clipping polygon with agg");
    }
    {
        test3 test_runner(params,wkt_in,clipping_box);
        run(test_runner,"clipping polygon with boost");
    }
    /*
    {
        test4 test_runner(params,wkt_in,clipping_box);
        run(test_runner,"clipping polygon with clipper_tree");
    }
    */
    return 0;
}
