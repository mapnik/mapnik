#ifndef ARROW_SYMBOLIZER_HPP
#define ARROW_SYMBOLIZER_HPP

#include  <mapnik/raster_colorizer.hpp>
#include  <mapnik/text_symbolizer.hpp>
#include  <mapnik/text_path.hpp>
#include  <mapnik/unicode.hpp>
#include  <mapnik/config_error.hpp>
#include <mapnik/font_engine_freetype.hpp>

namespace mapnik
{
    struct MAPNIK_DECL glyph_symbolizer
    {    
    glyph_symbolizer(std::string const& face_name, std::string const& c)
            : face_name_(face_name),
              char_(c),
              angle_offset_(0.0),
              min_size_(10),
              max_size_(100),
              min_value_(0),
              max_value_(1),
              colorizer_(),
              allow_overlap_(false),
              avoid_edges_(false),
              displacement_(0.0, 0.0),
              halo_fill_(color(255,255,255)),
              halo_radius_(0),
              azimuth_attr_("azimuth"),
              value_attr_("value")
    {
        if (get_min_size() > get_max_size())
            throw config_error("Invalid size range");
    }

    std::string const& get_face_name() const
    {
        return face_name_;
    }
   
    void set_face_name(std::string face_name)
    {
        face_name_ = face_name;
    }

    std::string const& get_char() const
    {
        return char_;
    }
   
    void set_char(std::string c)
    {
        char_ = c;
    }

    double get_angle_offset() const
    {
        return angle_offset_;
    }
    void set_angle_offset(double angle)
    {
        angle_offset_ = angle;
    }

    float get_min_value() const
    {
        return min_value_;
    }
    void set_min_value(float min_value)
    {
        min_value_ = min_value;
    }

    float get_max_value() const
    {
        return max_value_;
    }
    void set_max_value(float max_value)
    {
        max_value_ = max_value;
    }

    unsigned get_min_size() const
    {
        return min_size_;
    }
    void set_min_size(unsigned min_size)
    {
        min_size_ = min_size;
    }

    unsigned get_max_size() const
    {
        return max_size_;
    }
    void set_max_size(unsigned max_size)
    {
        max_size_ = max_size;
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
        
    std::string const& get_azimuth_attr() const
    {
        return azimuth_attr_;
    }
   
    void set_azimuth_attr(std::string azimuth_attr)
    {
        azimuth_attr_ = azimuth_attr;
    }

    std::string const& get_value_attr() const
    {
        return value_attr_;
    }

    raster_colorizer_ptr get_colorizer() const
    {
       return colorizer_;
    }
    void set_colorizer(raster_colorizer_ptr const& colorizer)
    {
        colorizer_ = colorizer;
    }
   
    void set_value_attr(std::string value_attr)
    {
        value_attr_ = value_attr;
    }

    /*
     * helpers
     */

    text_path_ptr get_text_path(face_set_ptr const& faces,
                                Feature const& feature) const;
    unsigned int get_size(double value) const;
    double get_angle(Feature const& feature) const;
    double get_value(Feature const& feature) const;



    private:
        std::string face_name_;
        std::string char_;
        double angle_offset_;
        unsigned min_size_;
        unsigned max_size_;
        float min_value_;
        float max_value_;
        raster_colorizer_ptr colorizer_;
        bool allow_overlap_;
        bool avoid_edges_;
        position displacement_;
        color halo_fill_;
        unsigned halo_radius_;
        std::string azimuth_attr_;
        std::string value_attr_;
    };
};

#endif
