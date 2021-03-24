#include "catch.hpp"

#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/geometry/correct.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>


TEST_CASE("transform_path_adapter") {

#ifdef MAPNIK_USE_PROJ
SECTION("polygon closing - epsg 2330") {
    mapnik::geometry::polygon<double> g;
    g.emplace_back();
    auto & exterior = g.back();

    exterior.emplace_back(88.1844308217992, 69.3553916041731);
    exterior.emplace_back(88.1846166524913, 69.3552821191223);
    exterior.emplace_back(88.1845090893871, 69.3553454342903);
    exterior.emplace_back(88.1844308217992, 69.3553916041731);

    using va_type = mapnik::geometry::polygon_vertex_adapter<double>;
    using path_type = mapnik::transform_path_adapter<mapnik::view_transform, va_type>;

    va_type va(g);
    mapnik::box2d<double> extent(16310607, 7704513, 16310621, 7704527);
    mapnik::view_transform tr(512, 512, extent);
    mapnik::projection proj1("epsg:2330");
    mapnik::projection proj2("epsg:4326");
    mapnik::proj_transform prj_trans(proj1, proj2);
    path_type path(tr, va, prj_trans);

    double x,y;
    unsigned cmd;

    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_MOVETO );
    CHECK( x == Approx(110.4328050613) );
    CHECK( y == Approx(20.2204537392) );

    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_LINETO );
    CHECK( x == Approx(342.1220560074) );
    CHECK( y == Approx(486.732225963) );

    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_LINETO );
    CHECK( x == Approx(207.9962329183) );
    CHECK( y == Approx(216.9376912798) );

    // close
    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_CLOSE );
    CHECK( x == 0 );
    CHECK( y == 0 );

    // end
    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_END );
    CHECK( x == 0 );
    CHECK( y == 0 );
}

SECTION("polygon closing - epsg 32633") {
    mapnik::geometry::polygon<double> g;
    g.emplace_back();
    auto & exterior = g.back();

    exterior.emplace_back(13, 13);
    exterior.emplace_back(14, 13);
    exterior.emplace_back(14, 14);
    exterior.emplace_back(14, 14);

    using va_type = mapnik::geometry::polygon_vertex_adapter<double>;
    using path_type = mapnik::transform_path_adapter<mapnik::view_transform, va_type>;

    va_type va(g);
    mapnik::box2d<double> extent(166022, 0, 833978, 9329005);
    mapnik::view_transform tr(512, 512, extent);
    mapnik::projection proj1("epsg:32633");
    mapnik::projection proj2("epsg:4326");
    mapnik::proj_transform prj_trans(proj1, proj2);
    path_type path(tr, va, prj_trans);

    double x,y;
    unsigned cmd;

    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_MOVETO );
    CHECK( x == Approx(89.7250280748) );
    CHECK( y == Approx(433.0795069885) );

    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_LINETO );
    CHECK( x == Approx(172.873973465) );
    CHECK( y == Approx(433.1145779929) );

    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_LINETO );
    CHECK( x == Approx(173.2194366775) );
    CHECK( y == Approx(427.0442504759) );

    // close
    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_CLOSE );
    CHECK( x == 0 );
    CHECK( y == 0 );

    // end
    cmd = path.vertex(&x, &y);
    CHECK( cmd == mapnik::SEG_END );
    CHECK( x == 0 );
    CHECK( y == 0 );
}

#endif //MAPNIK_USE_PROJ
}
