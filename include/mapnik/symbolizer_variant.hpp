#ifndef MAPNIK_SYMBOLIZER_VARIANT_HPP
#define MAPNIK_SYMBOLIZER_VARIANT_HPP

#include <mapnik/building_symbolizer.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/line_pattern_symbolizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/polygon_pattern_symbolizer.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/raster_symbolizer.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/markers_symbolizer.hpp>

#include <boost/variant.hpp>

namespace mapnik
{

class group_symbolizer;

typedef boost::variant<point_symbolizer,
                       line_symbolizer,
                       line_pattern_symbolizer,
                       polygon_symbolizer,
                       polygon_pattern_symbolizer,
                       raster_symbolizer,
                       shield_symbolizer,
                       text_symbolizer,
                       building_symbolizer,
                       markers_symbolizer,
                       boost::recursive_wrapper<group_symbolizer> > symbolizer;

}

#include <mapnik/group_symbolizer.hpp>

#endif /* MAPNIK_SYMBOLIZER_VARIANT_HPP */
