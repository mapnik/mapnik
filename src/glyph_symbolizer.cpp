#include <mapnik/glyph_symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>

namespace mapnik
{

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
    std::pair<unsigned,unsigned> cdim = faces->character_dimensions(ci.character);
    double cwidth = static_cast<double>(cdim.first)/2.0;
    double cheight = static_cast<double>(cdim.second)/2.0;
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
        angle = result.to_double();
        // normalize to first rotation in case an expression has made it go past
        angle = std::fmod(angle, 360);
        angle *= (M_PI/180); // convert to radians
        if (true) {  //TODO: if (get_mode()==AZIMUTH)
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
    return static_cast<unsigned>(result.to_int());
}

color glyph_symbolizer::eval_color(Feature const& feature) const
{
    raster_colorizer_ptr colorizer = get_colorizer();
    if (colorizer) {
        expression_ptr value_expr = get_value();
        if (!value_expr) {
            throw config_error(
                "Must define a 'value' expression to use a colorizer"
                );
        }
        value_type value_result = boost::apply_visitor(
            evaluate<Feature,value_type>(feature),
            *value_expr
            );
        return colorizer->get_color((float)value_result.to_double());
    } else {
        expression_ptr color_expr = get_color();
        if (color_expr) {
            value_type color_result = boost::apply_visitor(
                evaluate<Feature,value_type>(feature),
                *color_expr
                );
            return color(color_result.to_string());
        } else {
            return color("black");
        }
    }
}

} // end mapnik namespace
