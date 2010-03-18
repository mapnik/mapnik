#include <mapnik/glyph_symbolizer.hpp>
#include <mapnik/value.hpp>

#include <boost/lexical_cast.hpp>


namespace mapnik
{
    text_path_ptr glyph_symbolizer::get_text_path(face_set_ptr const& faces,
                                              Feature const& feature) const
    {
        // Calculate face rotation angle and box offset adjustments
        typedef std::pair<unsigned,unsigned> dimension_t;

        // Get string_info with arrow glyph
        string_info info(UnicodeString(get_char().c_str()));
        faces->get_string_info(info);
        if (info.num_characters() != 1)
        {
            throw config_error("'char' length must be exactly 1");
        }

        character_info ci = info.at(0);
        dimension_t cdim = faces->character_dimensions(ci.character);
        double cwidth = (static_cast<double>(cdim.first))/2.0;
        double cheight = (static_cast<double>(cdim.second))/2.0;
        double angle = get_angle(feature);
        double xoff = cwidth*cos(angle) - cheight*sin(angle);
        double yoff = cwidth*sin(angle) + cheight*cos(angle);
        
        text_path_ptr path_ptr = text_path_ptr(new text_path());
        path_ptr->add_node(ci.character, -xoff, -yoff, angle);
        return path_ptr;
    }

    unsigned int glyph_symbolizer::get_size(double value) const
    {
        double scale = (get_max_size()-get_min_size()) /
                       (get_max_value()-get_min_value());
        double size = get_min_size() + (value - get_min_value())*scale;
        if (size > get_max_size())
        {
            // size too large, clip it
            size = get_max_size();
        }
        return static_cast<unsigned>(floor(size));
    }

    double glyph_symbolizer::get_angle(Feature const& feature) const
    {
        // Returns the angle of rotation of the glyph in radians
        std::string key = get_azimuth_attr();
        double azimuth = feature[key].to_double();
        azimuth = std::fmod(azimuth, 360.0);
        if (azimuth<0) azimuth += 360.0;
        azimuth *= (M_PI/180.0);
        double angle = atan2(cos(azimuth), sin(azimuth));
        angle += get_angle_offset() * (M_PI/180.0);
        angle = std::fmod(angle, 2*M_PI);
        if (angle<0) angle += 2*M_PI;
        return angle;
    }

    double glyph_symbolizer::get_value(Feature const& feature) const
    {
        std::string key = get_value_attr();
        return feature[key].to_double();
    }
}
