#include "catch.hpp"
#include "../../../plugins/input/postgis/postgis_featureset.hpp"
#include "../../../plugins/input/postgis/resultset.hpp"

namespace {
struct observed_result : ResultSet
{
    observed_result(PGresult* result, unsigned& closes)
        : ResultSet(result),
          closes_(closes)
    {}
    void close() override
    {
        ++closes_;
        ResultSet::close();
    }
    unsigned& closes_;
};

PGresult* make_result(bool empty, bool null_geometry)
{
    PGresult* result = PQmakeEmptyPGresult(nullptr, PGRES_TUPLES_OK);
    PGresAttDesc attrs[2]{};
    attrs[0].name = const_cast<char*>("geom");
    attrs[0].typid = 17;
    attrs[0].format = 1;
    attrs[0].typlen = -1;
    attrs[1].name = const_cast<char*>("name");
    attrs[1].typid = 25;
    attrs[1].format = 1;
    attrs[1].typlen = -1;
    REQUIRE(PQsetResultAttrs(result, 2, attrs));
    if (!empty)
    {
        unsigned char point[21]{};
        point[0] = point[1] = 1;
        point[11] = 0xf0;
        point[12] = 0x3f;
        point[20] = 0x40;
        REQUIRE(PQsetvalue(result,
                           0,
                           0,
                           null_geometry ? nullptr : reinterpret_cast<char*>(point),
                           null_geometry ? -1 : sizeof(point)));
        REQUIRE(PQsetvalue(result, 0, 1, const_cast<char*>("retained name"), 13));
    }
    return result;
}
} // namespace

TEST_CASE("PostGIS releases its result at EOF without invalidating decoded features")
{
    bool empty = false, null_geometry = false, abandon = false;
    SECTION("nonempty result") {}
    SECTION("empty result")
    {
        empty = true;
    }
    SECTION("all rows skipped")
    {
        null_geometry = true;
    }
    SECTION("destruction before EOF")
    {
        abandon = true;
    }
    unsigned closes = 0;
    auto result = std::make_shared<observed_result>(make_result(empty, null_geometry), closes);
    auto context = std::make_shared<mapnik::context_type>();
    context->push("name");
    mapnik::feature_ptr retained;
    {
        postgis_featureset features(result, context, "utf-8", false, false, false);
        if (!abandon)
        {
            retained = features.next();
            REQUIRE(bool(retained) == (!empty && !null_geometry));
            REQUIRE_FALSE(features.next());
            REQUIRE(closes == 1);
            REQUIRE_FALSE(features.next());
            REQUIRE_FALSE(features.next());
            REQUIRE(closes == 1);
        }
    }
    REQUIRE(closes == 1);
    if (retained)
    {
        REQUIRE(retained->get("name").to_string() == "retained name");
        auto const& point = mapnik::util::get<mapnik::geometry::point<double>>(retained->get_geometry());
        REQUIRE(point.x == 1);
        REQUIRE(point.y == 2);
    }
}
