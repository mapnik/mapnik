#include <mapnik/glyph_symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>

namespace mapnik
{

static const char * angle_mode_strings[] = {
    "azimuth",
    "trigonometric",
    ""
};

IMPLEMENT_ENUM( angle_mode_e, angle_mode_strings )

text_path_ptr glyph_symbolizer::get_text_path(face_set_ptr const& faces,
                                              Feature const& feature) const
{
    // Try to evaulate expressions against feature
    UnicodeString char_ = eval_char(feature);
    double angle = eval_angle(feature);

    // calculate displacement so glyph is rotated along center (default pivot is
    // lowerbottom corner)
    string_info info(char_);
    faces->get_string_info(info);

    // XXX: Perhaps this limitation can be overcomed?
    if (info.num_characters() != 1)
    {
        throw config_error("'char' length must be exactly 1");
    }

    character_info ci = info.at(0);
    font_face_set::dimension_t cdim = faces->character_dimensions(ci.character);
    double cwidth = static_cast<double>(cdim.width)/2.0;
    double cheight = static_cast<double>(cdim.height)/2.0;
    double xoff = cwidth*cos(angle) - cheight*sin(angle);
    double yoff = cwidth*sin(angle) + cheight*cos(angle);
    
    // Create text path and add character with displacement and angle
    text_path_ptr path_ptr = text_path_ptr(new text_path());
    path_ptr->add_node(ci.character, -xoff, -yoff, angle);
    return path_ptr;
}



UnicodeString glyph_symbolizer::eval_char(Feature const& feature) const
{
    expression_ptr expr = get_char();
    if (!expr)
        throw config_error("No 'char' expression");
    value_type result = boost::apply_visitor(
        evaluate<Feature,value_type>(feature),
        *expr
        );
#ifdef MAPNIK_DEBUG
    std::clog << "char_result=" << result.to_string() << "\n";
#endif
    return result.to_unicode();
}

double glyph_symbolizer::eval_angle(Feature const& feature) const
{
    double angle = 0.0;
    expression_ptr expr = get_angle();
    if (expr) {
        value_type result = boost::apply_visitor(
            evaluate<Feature,value_type>(feature),
            *expr
            );
#ifdef MAPNIK_DEBUG
        std::clog << "angle_result=" << result.to_string() << "\n";
#endif
        angle = result.to_double();
        // normalize to first rotation in case an expression has made it go past
        angle = std::fmod(angle, 360);
        angle *= (M_PI/180); // convert to radians
        if (get_angle_mode()==AZIMUTH) {
            // angle is an azimuth, convert into trigonometric angle
            angle = std::atan2(std::cos(angle), std::sin(angle));
        }
        if (angle<0)
            angle += 2*M_PI;
    }
    return angle;
}

unsigned glyph_symbolizer::eval_size(Feature const& feature) const
{
    expression_ptr expr = get_size();
    if (!expr) throw config_error("No 'size' expression");
    value_type result = boost::apply_visitor(
        evaluate<Feature,value_type>(feature),
        *expr
        );
#ifdef MAPNIK_DEBUG
    std::clog << "size_result=" << result.to_string() << "\n";
#endif
    unsigned size = static_cast<unsigned>(result.to_int());
#ifdef MAPNIK_DEBUG
    std::clog << "size=" << size << "\n";
#endif
    return size;
}

color glyph_symbolizer::eval_color(Feature const& feature) const
{
    raster_colorizer_ptr colorizer = get_colorizer();
    if (colorizer) 
    {
        expression_ptr value_expr = get_value();
        if (!value_expr) 
        {
            throw config_error(
                "Must define a 'value' expression to use a colorizer"
                );
        }
        value_type value_result = boost::apply_visitor(
            evaluate<Feature,value_type>(feature),
            *value_expr
            );
#ifdef MAPNIK_DEBUG
        std::clog << "value_result=" << value_result.to_string() << "\n";
#endif
        return colorizer->get_color((float)value_result.to_double());
    } 
    else 
    {
        expression_ptr color_expr = get_color();
        if (color_expr) 
        {
            value_type color_result = boost::apply_visitor(
                evaluate<Feature,value_type>(feature),
                *color_expr
                );
#ifdef MAPNIK_DEBUG
            std::clog << "color_result=" << color_result.to_string() << "\n";
#endif
            return color(color_result.to_string());
        } 
        else 
        {
            return color(0,0,0); // black
        }
    }
}

} // end mapnik namespace
