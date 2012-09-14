
/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

/** \brief Raster Colouriser
 *
 * This class allows GDAL raster bands to be colourised. It only works with single
 * band GDAL rasters, not greyscale, alpha, or rgb  (due to the GDAL featureset loading
 * single channel GDAL rasters as FLOAT32, and the others as BYTE, and not having a method
 * of figuring out which).
 *
 * Every input value is translated to an output value. The output value is determined by
 * what 'stop' the input value is in. Each stop covers the range of input values from its
 * 'value' parameter, up to the 'value' parameter of the next stop.
 *
 */

#ifndef MAPNIK_RASTER_COLORIZER_HPP
#define MAPNIK_RASTER_COLORIZER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/color.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/enumeration.hpp>

// boost
#include <boost/shared_ptr.hpp>

// stl
#include <vector>

namespace mapnik
{

//! \brief Enumerates the modes of interpolation
enum colorizer_mode_enum
{
    COLORIZER_INHERIT = 0,    //!< The stop inherits the mode from the colorizer
    COLORIZER_LINEAR = 1,     //!< Linear interpolation between colors
    COLORIZER_DISCRETE = 2,   //!< Single color for stop
    COLORIZER_EXACT = 3,      //!< Only the exact value specified for the stop gets translated, others use the default
    colorizer_mode_enum_MAX
};

DEFINE_ENUM( colorizer_mode, colorizer_mode_enum );

//! \brief Structure to represent a stop position.
class MAPNIK_DECL colorizer_stop {
public:

    //! \brief Constructor
    //!
    //! \param[in] value The stop value
    //! \param[in] mode The stop mode
    //! \param[in] color The stop color
    colorizer_stop(float value = 0,
                   colorizer_mode mode = COLORIZER_INHERIT,
                   color const& _color = color(0,0,0,0),
                   std::string const& label="");

    //! \brief Copy constructor
    colorizer_stop(colorizer_stop const& stop);

    //! \brief Destructor
    ~colorizer_stop();


    //! \brief Set the stop value
    //! \param[in] value The stop value
    inline void set_value(float value) { value_ = value; };

    //! \brief Get the stop value
    //! \return The stop value
    inline float get_value() const {return value_; };


    //! \brief Set the stop mode
    //! \param[in] mode The stop mode
    inline void set_mode(colorizer_mode mode) { mode_ = mode; };
    inline void set_mode_enum(colorizer_mode_enum mode) { set_mode(mode); };

    //! \brief Get the stop mode
    //! \return The stop mode
    inline colorizer_mode get_mode() const { return mode_; };
    inline colorizer_mode_enum get_mode_enum() const { return get_mode(); };


    //! \brief set the stop color
    //! \param[in] the stop color
    inline void set_color(color const& _color) { color_ = _color; };

    //! \brief get the stop color
    //! \return The stop color
    inline color const& get_color() const {return color_; };

    //! \brief set the stop label
    //! \param[in] the stop label
    inline void set_label(std::string const& label) { label_ = label; };

    //! \brief get the stop label
    //! \return The stop label
    inline std::string const& get_label() const {return label_; };


    //! \brief Equality operator
    //! \return True if equal, false otherwise
    bool operator==(colorizer_stop const& other) const;

    //! \brief Print the stop to a string
    //! \return A string representing this stop.
    std::string to_string() const;

private:
    float value_;   //!< The stop value
    colorizer_mode mode_; //!< The stop mode
    color color_;   //!< The stop color
    std::string label_; //!< The stop label for use in legends
};


typedef std::vector<colorizer_stop> colorizer_stops;


//! \brief Class representing the raster colorizer
class MAPNIK_DECL raster_colorizer
{
public:
    //! \brief Constructor
    raster_colorizer(colorizer_mode mode = COLORIZER_LINEAR, color const& _color = color(0,0,0,0));

    //! \brief Destructor
    ~raster_colorizer();


    //! \brief Set the default mode
    //!
    //! This can not be set as INHERIT, if you do, LINEAR will be used instead.
    //! \param[in] mode The default mode

    void set_default_mode(colorizer_mode mode)
    {
        default_mode_ = (mode == COLORIZER_INHERIT) ? COLORIZER_LINEAR:(colorizer_mode_enum)mode;
    };

    void set_default_mode_enum(colorizer_mode_enum mode) { set_default_mode(mode); };

    //! \brief Get the default mode
    //! \return The default mode
    colorizer_mode get_default_mode() const {return default_mode_; };
    colorizer_mode_enum get_default_mode_enum() const {return get_default_mode(); };

    //! \brief Set the default color
    //! \param[in] color The default color
    void set_default_color(color const& color) { default_color_ = color; };

    //! \brief Get the default color
    //! \return The default color
    color const& get_default_color() const {return default_color_; };


    //! \brief Add a stop
    //!
    //! \param[in] stop The stop to add
    //! \return True if added, false if error
    bool add_stop(colorizer_stop const& stop);

    //! \brief Set the list of stops
    //! \param[in] stops The list of stops
    void set_stops(colorizer_stops const& stops) { stops_ = stops; };

    //! \brief Get the list of stops
    //! \return The list of stops
    colorizer_stops const& get_stops() const {return stops_; };


    //! \brief Colorize a raster
    //!
    //! \param[in, out] raster A raster stored in float32 single channel format, which gets colorized in place.
    //! \param[in] feature used to find 'NODATA' information if available
    void colorize(raster_ptr const& raster, Feature const& f) const;


    //! \brief Perform the translation of input to output
    //!
    //! \param[in] value Input value
    //! \return color associated with the value
    color get_color(float value) const;


    //! \brief Set the epsilon value for exact mode
    //! \param[in] e The epsilon value
    inline void set_epsilon(const float e) { if(e > 0) epsilon_ = e; };

    //! \brief Get the epsilon value for exact mode
    //! \return The epsilon value
    inline float get_epsilon() const { return epsilon_; };

private:
    colorizer_stops stops_;         //!< The vector of stops

    colorizer_mode default_mode_;   //!< The default mode inherited by stops
    color default_color_;           //!< The default color
    float epsilon_;                 //!< The epsilon value for exact mode
};


typedef boost::shared_ptr<raster_colorizer> raster_colorizer_ptr;


} // mapnik namespace

#endif // MAPNIK_RASTER_COLORIZER_HPP
