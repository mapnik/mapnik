#include <boost/python.hpp>

#include <mapnik/glyph_symbolizer.hpp>
#include "mapnik_enumeration.hpp"
#include <boost/tuple/tuple.hpp>

using mapnik::glyph_symbolizer;
using mapnik::position;
using mapnik::enumeration_;
using mapnik::angle_mode_e;
using mapnik::AZIMUTH;
using mapnik::TRIGONOMETRIC;
using namespace boost::python;

namespace {
using namespace boost::python;

tuple get_displacement(const glyph_symbolizer& s)
{
    boost::tuple<double,double> pos = s.get_displacement();
    return boost::python::make_tuple(boost::get<0>(pos),boost::get<1>(pos));
}

void set_displacement(glyph_symbolizer & s, boost::python::tuple arg)
{
    s.set_displacement(extract<double>(arg[0]),extract<double>(arg[1]));
}

}

void export_glyph_symbolizer()
{
    enumeration_<angle_mode_e>("angle_mode")
        .value("AZIMUTH", AZIMUTH)
        .value("TRIGONOMETRIC", TRIGONOMETRIC)
        ;

    class_<glyph_symbolizer>("GlyphSymbolizer",
                             init<std::string,mapnik::expression_ptr>())
        .add_property("face_name",
                      make_function(&glyph_symbolizer::get_face_name,
                                    return_value_policy<copy_const_reference>()),
                      &glyph_symbolizer::set_face_name,
                      "Get/Set the name of the font face (eg:\"DejaVu Sans "
                      "Book\") which contains the glyph"
            )
        .add_property("char",
                      &glyph_symbolizer::get_char,
                      &glyph_symbolizer::set_char,
                      "Get/Set the char expression. The char is the unicode "
                      "character indexing the glyph in the font referred by "
                      "face_name."
            )
        .add_property("allow_overlap",
                      &glyph_symbolizer::get_allow_overlap,
                      &glyph_symbolizer::set_allow_overlap,
                      "Get/Set the flag which controls if glyphs should "
                      "overlap any symbols previously rendered"
            )
        .add_property("avoid_edges",
                      &glyph_symbolizer::get_avoid_edges,
                      &glyph_symbolizer::set_avoid_edges,
                      "Get/Set the flag which controls if glyphs should be "
                      "partially drawn beside the edge of a tile."
            )
        .add_property("displacement",
                      &get_displacement,
                      &set_displacement)

        .add_property("halo_fill",
                      make_function(&glyph_symbolizer::get_halo_fill,
                                    return_value_policy<copy_const_reference>()),
                      &glyph_symbolizer::set_halo_fill)

        .add_property("halo_radius",
                      &glyph_symbolizer::get_halo_radius, 
                      &glyph_symbolizer::set_halo_radius)

        .add_property("size",
                      &glyph_symbolizer::get_size,
                      &glyph_symbolizer::set_size,
                      "Get/Set the size expression used to size the glyph."
            )

        .add_property("angle",
                      &glyph_symbolizer::get_angle,
                      &glyph_symbolizer::set_angle,
                      "Get/Set the angle expression used to rotate the glyph "
                      "along its center."
            )
        .add_property("angle_mode",
                      &glyph_symbolizer::get_angle_mode,
                      &glyph_symbolizer::set_angle_mode,
                      "Get/Set the angle_mode property. This controls how the "
                      "angle is interpreted. Valid values are AZIMUTH and  "
                      "TRIGONOMETRIC."
            )
        .add_property("value",
                      &glyph_symbolizer::get_value,
                      &glyph_symbolizer::set_value,
                      "Get/set the value expression which will be used to "
                      "retrieve a a value for the colorizer to use to choose "
                      "a color."
            )
        .add_property("color",
                      &glyph_symbolizer::get_color,
                      &glyph_symbolizer::set_color,
                      "Get/Set the color expression used to color the glyph. "
                      "(See also the 'colorizer' attribute)"

            )
        .add_property("colorizer",
                      &glyph_symbolizer::get_colorizer,
                      &glyph_symbolizer::set_colorizer,
                      "Get/Set the RasterColorizer used to color the glyph "
                      "depending on the 'value' expression (which must be "
                      "defined).\n"
                      "Only needed if no explicit color is provided"
            )
        ;    
}
