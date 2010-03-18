#include <boost/python.hpp>
#include <boost/tuple/tuple.hpp>
#include <mapnik/glyph_symbolizer.hpp>

using mapnik::glyph_symbolizer;
using mapnik::position;
using namespace boost::python;

list get_displacement_list(const glyph_symbolizer& t)
{
    position pos = t.get_displacement();
    double dx = boost::get<0>(pos);
    double dy = boost::get<1>(pos);
    boost::python::list disp;
    disp.append(dx);
    disp.append(dy);
    return disp;
}

void export_glyph_symbolizer()
{
    class_<glyph_symbolizer>("GlyphSymbolizer",
                             init<std::string const&,std::string const&>())
        .add_property("face_name",
                    make_function(&glyph_symbolizer::get_face_name,return_value_policy<copy_const_reference>()),
                    &glyph_symbolizer::set_face_name)
        .add_property("char",
                    make_function(&glyph_symbolizer::get_char,return_value_policy<copy_const_reference>()),
                    &glyph_symbolizer::set_char)
        .add_property("angle_offset",
                    &glyph_symbolizer::get_angle_offset,
                    &glyph_symbolizer::set_angle_offset)
        .add_property("allow_overlap",
                    &glyph_symbolizer::get_allow_overlap,
                    &glyph_symbolizer::set_allow_overlap)
        .add_property("avoid_edges",
                    &glyph_symbolizer::get_avoid_edges,
                    &glyph_symbolizer::set_avoid_edges)
        .def("get_displacement", get_displacement_list)
        .def("set_displacement", &glyph_symbolizer::set_displacement)
        .add_property("halo_fill",
                      make_function(&glyph_symbolizer::get_halo_fill,
                                 return_value_policy<copy_const_reference>()),
                      &glyph_symbolizer::set_halo_fill)
        .add_property("halo_radius",
                      &glyph_symbolizer::get_halo_radius, 
                      &glyph_symbolizer::set_halo_radius)
        .add_property("value_attr", make_function(
                      &glyph_symbolizer::get_value_attr,
                      return_value_policy<copy_const_reference>()),
                      &glyph_symbolizer::set_value_attr)
        .add_property("azimuth_attr", make_function(
                      &glyph_symbolizer::get_azimuth_attr,
                      return_value_policy<copy_const_reference>()),
                      &glyph_symbolizer::set_azimuth_attr)
        .add_property("colorizer",
                      &glyph_symbolizer::get_colorizer,
                      &glyph_symbolizer::set_colorizer,
                      "Get/Set the RasterColorizer used to color the arrows.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import GlyphSymbolizer, RasterColorizer\n"
                      ">>> sym = GlyphSymbolizer()\n"
                      ">>> sym.colorizer = RasterColorizer()\n"
                      ">>> for value, color in [\n"
                      "...     (0, \"#000000\"),\n"
                      "...     (10, \"#ff0000\"),\n"
                      "...     (40, \"#00ff00\"),\n"
                      "... ]:\n"
                      "...      sym.colorizer.append_band(value, color)\n"
                      )
        ;    
        ;    
}
