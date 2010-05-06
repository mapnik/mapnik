
/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id$

#ifndef RASTER_COLORIZER_HPP
#define RASTER_COLORIZER_HPP

#include <mapnik/config.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/color.hpp>
#include <mapnik/feature.hpp>

#include <vector>

namespace mapnik
{
struct MAPNIK_DECL color_band
{
    float value_;
    float max_value_;
    color color_;
    unsigned midpoints_;
    bool is_interpolated_;
    color_band(float value, color c)
	: value_(value),
	max_value_(value),
	color_(c),
	midpoints_(0),
	is_interpolated_(false) {}
    color_band(float value, float max_value, color c)
	: value_(value),
	max_value_(max_value),
	color_(c),
	midpoints_(0),
	is_interpolated_(false) {}
    const bool is_interpolated() const
    {
	return is_interpolated_;
    }
    const unsigned get_midpoints() const
    {
	return midpoints_;
    }
    const float get_value() const
    {
	return value_;
    }
    const float get_max_value() const
    {
	return max_value_;
    }
    const color& get_color() const
    {
	return color_;
    }
    bool operator==(color_band const& other) const
    {
	return value_ == other.value_ && color_ == other.color_ && max_value_ == other.max_value_;
    }
    std::string to_string() const
    {
	std::stringstream ss;
	ss << color_.to_string() << " " << value_ << " " << max_value_;
	return ss.str();
    }
};

typedef std::vector<color_band> color_bands;

struct MAPNIK_DECL raster_colorizer
{
    explicit raster_colorizer()
	: colors_() {}

    raster_colorizer(const raster_colorizer &ps)
	: colors_(ps.colors_) {}

    raster_colorizer(color_bands &colors)
	: colors_(colors) {}

    const color_bands& get_color_bands() const
    {
	return colors_;
    }
    void append_band (color_band band)
    {
	if (colors_.size() > 0 && colors_.back().value_ > band.value_) {
#ifdef MAPNIK_DEBUG
	    std::clog << "prev.v=" << colors_.back().value_ << ". band.v=" << band.value_ << "\n";
#endif
	    throw config_error(
		"Bands must be appended in ascending value order"
		);
	}
	colors_.push_back(band);
	if (colors_.size() > 0 && colors_.back().value_ == colors_.back().max_value_)
        colors_.back().max_value_ = band.value_;
    }
    void append_band (color_band band, unsigned midpoints)
    {
	band.midpoints_ = midpoints;
	if (colors_.size() > 0 && midpoints > 0) {
	    color_band lo = colors_.back();
	    color_band const &hi = band;
	    int steps = midpoints+1;
	    float dv = (hi.value_ - lo.value_)/steps;
	    float da = (float(hi.color_.alpha()) - lo.color_.alpha())/steps;
	    float dr = (float(hi.color_.red()) - lo.color_.red())/steps;
	    float dg = (float(hi.color_.green()) - lo.color_.green())/steps;
	    float db = (float(hi.color_.blue()) - lo.color_.blue())/steps;

#ifdef MAPNIK_DEBUG
	    std::clog << "lo.v=" << lo.value_ << ", hi.v=" << hi.value_ << ", dv="<<dv<<"\n";
#endif
	    // interpolate intermediate values and colors
	    int j;
	    for (j=1; j<steps; j++) {
		color_band b(
		    lo.get_value() + dv*j,
		    color(int(float(lo.color_.red()) + dr*j),
			  int(float(lo.color_.green()) + dg*j),
			  int(float(lo.color_.blue()) + db*j),
			  int(float(lo.color_.alpha()) + da*j)
			)
		    );
		b.is_interpolated_ = true;
		append_band(b);
	    }
	}
	append_band(band);
    }

    void append_band (float value, color c)
    {
	append_band(color_band(value, c));
    }

    void append_band (float value, float max_value, color c)
    {
	append_band(color_band(value, max_value, c));
    }

    void append_band (float value, color c, unsigned midpoints)
    {
	append_band(color_band(value, c), midpoints);
    }

    void append_band (float value, float max_value, color c, unsigned midpoints)
    {
	append_band(color_band(value, max_value, c), midpoints);
    }


    /* rgba = 
     *   if cs[pos].value <= value < cs[pos+1].value: cs[pos].color
     *   otherwise: transparent
     *     where 0 <= pos < length(bands)-1
     *   Last band is special, its value represents the upper bound and its
     *   color will only be used if the value matches its value exactly.
     */
    color get_color(float value) const {
	int pos=-1, last=(int)colors_.size()-1, lo=0, hi=last;
	while (lo<=hi) {
	    pos = (lo+hi)/2;
	    if (colors_[pos].value_<value) {
		lo = pos+1;
	    } else if (colors_[pos].value_>value) {
		hi = pos-1;
	    } else {
		lo = pos+1;
		break;
	    }
	}
	lo--;
	if ((0 <= lo && lo < last) ||
	    (lo==last && (colors_[last].value_==value || value<colors_[last].max_value_)))
	    return colors_[lo].color_;
	else
	    return color(0,0,0,0);
    }

    void colorize(raster_ptr const& raster) const 
    {
	float *rasterData = reinterpret_cast<float*>(raster->data_.getBytes());
	unsigned *imageData = raster->data_.getData();
	unsigned i;
	for (i=0; i<raster->data_.width()*raster->data_.height(); i++)
	{
	    imageData[i] = get_color(rasterData[i]).rgba();
	}
    }
      
private:
    color_bands colors_;
};

typedef boost::shared_ptr<raster_colorizer> raster_colorizer_ptr;

} // mapnik namespace

#endif //RASTER_COLORIZER_HPP
