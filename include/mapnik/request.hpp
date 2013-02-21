/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_REQUEST_HPP
#define MAPNIK_REQUEST_HPP

// mapnik
//#include <mapnik/well_known_srs.hpp>
#include <mapnik/config.hpp>
#include <mapnik/box2d.hpp>

// boost
#include <boost/optional/optional.hpp>

namespace mapnik
{

class MAPNIK_DECL request
{

private:
    static const unsigned MIN_MAPSIZE=16;
    static const unsigned MAX_MAPSIZE=MIN_MAPSIZE<<10;
    unsigned width_;
    unsigned height_;
    int buffer_size_;
    box2d<double> current_extent_;
    boost::optional<box2d<double> > maximum_extent_;

public:

    /*! \brief Constructor
     *  @param width Initial map width.
     *  @param height Initial map height.
     *  @param srs Initial map projection.
     */
    request(int width, int height);

    /*! \brief Get map width.
     */
    unsigned width() const;

    /*! \brief Get map height.
     */
    unsigned height() const;

    /*! \brief Set map width.
     */
    void set_width(unsigned width);

    /*! \brief Set map height.
     */
    void set_height(unsigned height);

    /*! \brief Resize the map.
     */
    void resize(unsigned width,unsigned height);

    /*! \brief Set buffer size
     *  @param buffer_size Buffer size in pixels.
     */

    void set_buffer_size(int buffer_size);

    /*! \brief Get the map buffer size
     *  @return Buffer size as int
     */
    int buffer_size() const;

    /*! \brief Set the map maximum extent.
     *  @param box The bounding box for the maximum extent.
     */
    void set_maximum_extent(box2d<double> const& box);

    /*! \brief Get the map maximum extent as box2d<double>
     */
    boost::optional<box2d<double> > const& maximum_extent() const;

    void reset_maximum_extent();

    /*! \brief Zoom the map to a bounding box.
     *
     *  Aspect is handled automatic if not fitting to width/height.
     *  @param box The bounding box where to zoom.
     */
    void zoom_to_box(const box2d<double>& box);

    /*! \brief Get current bounding box.
     *  @return The current bounding box.
     */
    const box2d<double>& get_current_extent() const;

    /*! \brief Get current buffered bounding box.
     *  @return The current buffered bounding box.
     */
    box2d<double> get_buffered_extent() const;

    /*!
     * @return The Map Scale.
     */
    double scale() const;

    ~request();
private:
};

}

#endif // MAPNIK_REQUEST_HPP
