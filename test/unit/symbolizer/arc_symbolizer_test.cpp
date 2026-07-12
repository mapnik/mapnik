#include "catch.hpp"

#include <mapnik/map.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/symbolizer_utils.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/color.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/expression_string.hpp>

#include <string>

using namespace mapnik;

namespace {

// A style with a single ArcSymbolizer carrying a known set of properties.
// No <Layer> is needed: symbolizers live in the <Style>, so the round-trip
// exercises load/save without pulling in any datasource plugin.
std::string const arc_xml = R"xml(<?xml version="1.0" encoding="utf-8"?>
<Map>
  <Style name="arcs">
    <Rule>
      <ArcSymbolizer radius="20" start-angle="45" end-angle="135"
                     fill="rgb(255,0,0)" fill-opacity="0.5"
                     stroke="rgb(0,0,255)" stroke-width="2" stroke-opacity="0.7"
                     stroke-dasharray="5,5" stroke-dashoffset="0.5"
                     arc-stroke="rgb(0,255,0)" arc-stroke-width="3" arc-stroke-opacity="0.8"
                     arc-stroke-dasharray="6,6" arc-stroke-dashoffset="0.6"
                     radius-stroke="rgb(255,0,0)" radius-stroke-width="4" radius-stroke-opacity="0.9"
                     radius-stroke-dasharray="7,7" radius-stroke-dashoffset="0.7"
                     />
    </Rule>
  </Style>
</Map>
)xml";

arc_symbolizer const& first_arc(Map const& m)
{
    auto style = m.find_style("arcs");
    REQUIRE(bool(style));
    REQUIRE(style->get_rules().size() == 1);
    auto const& sym = style->get_rules().front().get_symbolizers().front();
    // exercises symbolizer_traits<arc_symbolizer>
    REQUIRE(symbolizer_name(sym) == "ArcSymbolizer");
    return util::get<arc_symbolizer>(sym);
}

// Dash arrays are stored as parsed dash_array values (pairs of doubles),
// not as their source string, so compare against the parsed form.
void check_dash(arc_symbolizer const& sym, keys key, double dash, double gap)
{
    dash_array const dashes = get<dash_array>(sym, key);
    REQUIRE(dashes.size() == 1);
    REQUIRE(dashes.front().first == Approx(dash));
    REQUIRE(dashes.front().second == Approx(gap));
}

void check_arc(arc_symbolizer const& sym)
{
    // basic attribute tests
    REQUIRE(get<double>(sym, keys::radius) == Approx(20.0));
    REQUIRE(get<double>(sym, keys::start_angle) == Approx(45.0));
    REQUIRE(get<double>(sym, keys::end_angle) == Approx(135.0));

    // fill test
    color const fill = get<mapnik::color>(sym, keys::fill);
    REQUIRE(fill.red() == 255);
    REQUIRE(fill.green() == 0);
    REQUIRE(fill.blue() == 0);
    REQUIRE(get<double>(sym, keys::fill_opacity) == Approx(0.5));

    // default stroke test
    color const stroke = get<mapnik::color>(sym, keys::stroke);
    REQUIRE(stroke.red() == 0);
    REQUIRE(stroke.green() == 0);
    REQUIRE(stroke.blue() == 255);
    REQUIRE(get<double>(sym, keys::stroke_width) == Approx(2.0));
    REQUIRE(get<double>(sym, keys::stroke_opacity) == Approx(0.7));
    check_dash(sym, keys::stroke_dasharray, 5.0, 5.0);
    REQUIRE(get<double>(sym, keys::stroke_dashoffset) == Approx(0.5));

    // arc stroke test
    color const arc_stroke = get<mapnik::color>(sym, keys::arc_stroke);
    REQUIRE(arc_stroke.red() == 0);
    REQUIRE(arc_stroke.green() == 255);
    REQUIRE(arc_stroke.blue() == 0);
    REQUIRE(get<double>(sym, keys::arc_stroke_width) == Approx(3.0));
    REQUIRE(get<double>(sym, keys::arc_stroke_opacity) == Approx(0.8));
    check_dash(sym, keys::arc_stroke_dasharray, 6.0, 6.0);
    REQUIRE(get<double>(sym, keys::arc_stroke_dashoffset) == Approx(0.6));

    // radius stroke test
    color const radius_stroke = get<mapnik::color>(sym, keys::radius_stroke);
    REQUIRE(radius_stroke.red() == 255);
    REQUIRE(radius_stroke.green() == 0);
    REQUIRE(radius_stroke.blue() == 0);
    REQUIRE(get<double>(sym, keys::radius_stroke_width) == Approx(4.0));
    REQUIRE(get<double>(sym, keys::radius_stroke_opacity) == Approx(0.9));
    check_dash(sym, keys::radius_stroke_dasharray, 7.0, 7.0);
    REQUIRE(get<double>(sym, keys::radius_stroke_dashoffset) == Approx(0.7));
}

} // namespace

TEST_CASE("arc_symbolizer")
{
    SECTION("XML load/save round-trip")
    {
        // load
        Map m(256, 256);
        REQUIRE_NOTHROW(load_map_string(m, arc_xml));
        check_arc(first_arc(m));

        // save and reload: properties must survive the serialization cycle
        std::string const saved = save_map_to_string(m);

        Map m2(256, 256);
        REQUIRE_NOTHROW(load_map_string(m2, saved));
        check_arc(first_arc(m2));
    }
}
