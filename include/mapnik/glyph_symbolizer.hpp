#ifndef GLYPH_SYMBOLIZER_HPP
#define GLYPH_SYMBOLIZER_HPP

// mapnik
#include <mapnik/enumeration.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/color.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/symbolizer.hpp>

// boost
#include <boost/tuple/tuple.hpp>

namespace mapnik
{

typedef boost::tuple<double,double> position;

enum angle_mode_enum {
    AZIMUTH,
    TRIGONOMETRIC,
    angle_mode_enum_MAX
};

DEFINE_ENUM(angle_mode_e, angle_mode_enum);

struct MAPNIK_DECL glyph_symbolizer : public symbolizer_base
{    
    glyph_symbolizer(std::string face_name, expression_ptr c)
        : symbolizer_base(),
        face_name_(face_name),
        char_(c),
        angle_(),
        value_(),
        size_(),
        color_(),
        colorizer_(),
        allow_overlap_(false),
        avoid_edges_(false),
        displacement_(0.0, 0.0),
        halo_fill_(color(255,255,255)),
        halo_radius_(0),
        angle_mode_(TRIGONOMETRIC) {}

    std::string const& get_face_name() const
    {
        return face_name_;
    }
    void set_face_name(std::string face_name)
    {
        face_name_ = face_name;
    }

    expression_ptr get_char() const
    {
        return char_;
    }
    void set_char(expression_ptr c)
    {
        char_ = c;
    }

    bool get_allow_overlap() const
    {
        return allow_overlap_;
    }
    void set_allow_overlap(bool allow_overlap)
    {
        allow_overlap_ = allow_overlap;
    }

    bool get_avoid_edges() const
    {
        return avoid_edges_;
    }
    void set_avoid_edges(bool avoid_edges)
    {
        avoid_edges_ = avoid_edges;
    }

    void set_displacement(double x, double y)
    {
        displacement_ = boost::make_tuple(x,y);
    }
    position const& get_displacement() const
    {
        return displacement_;
    }

    void  set_halo_fill(color const& fill)
    {
        halo_fill_ = fill;
    }
    color const&  get_halo_fill() const
    {
        return halo_fill_;
    }

    void  set_halo_radius(unsigned radius)
    {
        halo_radius_ = radius;
    }

    unsigned  get_halo_radius() const
    {
        return halo_radius_;
    }
        
    expression_ptr get_angle() const
    {
        return angle_;
    }
    void set_angle(expression_ptr angle)
    {
        angle_ = angle;
    }

    expression_ptr get_value() const
    {
        return value_;
    }
    void set_value(expression_ptr value)
    {
        value_ = value;
    }

    expression_ptr get_size() const
    {
        return size_;
    }
    void set_size(expression_ptr size)
    {
        size_ = size;
    }

    expression_ptr get_color() const
    {
        return color_;
    }
    void set_color(expression_ptr color)
    {
        color_ = color;
    }

    raster_colorizer_ptr get_colorizer() const
    {
        return colorizer_;
    }
    void set_colorizer(raster_colorizer_ptr const& colorizer)
    {
        colorizer_ = colorizer;
    }
    void set_angle_mode(angle_mode_e angle_mode)
    {
        angle_mode_ = angle_mode;
    }
    angle_mode_e get_angle_mode() const
    {
        return angle_mode_;
    }


    text_path_ptr get_text_path(face_set_ptr const& faces,
                                Feature const& feature) const;
    UnicodeString eval_char(Feature const& feature) const;
    double eval_angle(Feature const& feature) const;
    unsigned eval_size(Feature const& feature) const;
    color eval_color(Feature const& feature) const;


private:
    std::string face_name_;
    expression_ptr char_;
    expression_ptr angle_;
    expression_ptr value_;
    expression_ptr size_;
    expression_ptr color_;
    raster_colorizer_ptr colorizer_;
    bool allow_overlap_;
    bool avoid_edges_;
    position displacement_;
    color halo_fill_;
    unsigned halo_radius_;
    angle_mode_e angle_mode_;
};

}  // end mapnik namespace

#endif
