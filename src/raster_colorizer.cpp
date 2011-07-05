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

//$Id:  $

#include <mapnik/raster_colorizer.hpp>
#include <limits>

namespace mapnik
{

//! \brief Strings for the colorizer_mode enumeration
static const char *colorizer_mode_strings[] = {
    "inherit",
    "linear",
    "discrete",
    "exact",
    ""
};

IMPLEMENT_ENUM( colorizer_mode, colorizer_mode_strings )


colorizer_stop::colorizer_stop(const float value/* = 0*/, const colorizer_mode mode/* = COLORIZER_INHERIT*/, const color& _color/* = color(0,0,0,0)*/ )
    : value_(value)
    , mode_(mode)
    , color_(_color)
{
    
}

colorizer_stop::colorizer_stop(const colorizer_stop& stop)
    : value_(stop.value_)
    , mode_(stop.mode_)
    , color_(stop.color_)
{
}


colorizer_stop::~colorizer_stop()
{
    
}


bool colorizer_stop::operator==(colorizer_stop const& other) const
{
    return  (value_ == other.value_) && 
            (color_ == other.color_) &&
            (mode_ == other.mode_);
}


std::string colorizer_stop::to_string() const
{
    std::stringstream ss;
    ss << color_.to_string() << " " << value_ << " " << mode_.as_string();
    return ss.str();
}    





raster_colorizer::raster_colorizer(colorizer_mode mode/* = COLORIZER_LINEAR*/, const color& _color/* = color(0,0,0,0)*/)
    : default_mode_(mode)
    , default_color_(_color)
    , epsilon_(std::numeric_limits<float>::epsilon())
{
    
}

raster_colorizer::~raster_colorizer()
{
}

bool raster_colorizer::add_stop(const colorizer_stop & stop) {
    //make sure stops are added in order of value
    if(stops_.size()) {
        if(stop.get_value() <= stops_.back().get_value()) {
            return false;
        }
    }
    
    stops_.push_back(stop);

    return true;
}

void raster_colorizer::colorize(raster_ptr const& raster,const std::map<std::string,value> &Props) const
{
    unsigned *imageData = raster->data_.getData();
    
    int len = raster->data_.width() * raster->data_.height();
    
    bool hasNoData = false;
    float noDataValue = 0;

    if (Props.count("NODATA")>0)
    {
        hasNoData = true;
        noDataValue = Props.at("NODATA").to_double();
    }

    for (int i=0; i<len; ++i)
    {
        // the GDAL plugin reads single bands as floats
        float value = *reinterpret_cast<float *> (&imageData[i]);
        if (hasNoData && noDataValue == value)
            imageData[i] = color(0,0,0,0).rgba();
        else
            imageData[i] = get_color(value).rgba();
    }
}

inline float interpolate(float start,float end, float fraction)
{
    return fraction * (end - start) + start;
}

color raster_colorizer::get_color(float value) const {
    int stopCount = stops_.size();
    
    //use default color if no stops
    if(stopCount == 0) {
        return default_color_;
    }
    
    //1 - Find the stop that the value is in
    int stopIdx = -1;
    bool foundStopIdx = false;
    
    for(int i=0; i<stopCount; i++) {
        if(value < stops_[i].get_value()) {
            stopIdx = i-1;
            foundStopIdx = true;
            break;
        }
    }
    if(!foundStopIdx) {
        stopIdx = stopCount-1;
    }
    
    //2 - Find the next stop
    int nextStopIdx = stopIdx + 1;
    if(nextStopIdx >= stopCount) {      //there is no next stop
        nextStopIdx = stopCount - 1;
    }
    
    //3 - Work out the mode
    colorizer_mode stopMode;
    if( stopIdx == -1 ) {       //before the first stop
        stopMode = default_mode_;
    }
    else {
        stopMode = stops_[stopIdx].get_mode();
        if(stopMode == COLORIZER_INHERIT) {
            stopMode = default_mode_;
        }
    }
    
    //4 - Calculate the colour
    color stopColor;
    color nextStopColor;
    float stopValue = 0;
    float nextStopValue = 0;
    color outputColor = get_default_color();
    if(stopIdx == -1) {
        stopColor = default_color_;
        nextStopColor = stops_[nextStopIdx].get_color();
        stopValue = value;
        nextStopValue = stops_[nextStopIdx].get_value();
    }
    else {
        stopColor = stops_[stopIdx].get_color();
        nextStopColor = stops_[nextStopIdx].get_color();
        stopValue = stops_[stopIdx].get_value();
        nextStopValue = stops_[nextStopIdx].get_value();
    }
    
    switch(stopMode) {
    case COLORIZER_LINEAR:
        {
            //deal with this separately so we don't have to worry about div0
            if(nextStopValue == stopValue) {
                outputColor = stopColor;
            }
            else {
                float fraction = (value - stopValue) / (nextStopValue - stopValue);
                
                float r = interpolate(stopColor.red(), nextStopColor.red(),fraction);
                float g = interpolate(stopColor.green(), nextStopColor.green(),fraction);
                float b = interpolate(stopColor.blue(), nextStopColor.blue(),fraction);
                float a = interpolate(stopColor.alpha(), nextStopColor.alpha(),fraction);
                
                outputColor.set_red(r);
                outputColor.set_green(g);
                outputColor.set_blue(b);
                outputColor.set_alpha(a);
            }
            
        }
        break;
    case COLORIZER_DISCRETE:
        outputColor = stopColor;
        break;
    case COLORIZER_EXACT:
    default:
        //approximately equal (within epsilon)
        if(fabs(value - stopValue) < epsilon_) {
            outputColor = stopColor;
        }
        else {
            outputColor = default_color_;
        }
        break;
    }
    

    /*
    std::clog << "get_color: " << value << "\n";
    std::clog << "\tstopIdx: " << stopIdx << "\n";
    std::clog << "\tnextStopIdx: " << nextStopIdx << "\n";
    std::clog << "\tstopValue: " << stopValue << "\n";
    std::clog << "\tnextStopValue: " << nextStopValue << "\n";
    std::clog << "\tstopColor: " << stopColor.to_string() << "\n";
    std::clog << "\tnextStopColor: " << nextStopColor.to_string() << "\n";
    std::clog << "\tstopMode: " << stopMode.as_string() << "\n";
    std::clog << "\toutputColor: " << outputColor.to_string() << "\n";
*/

    return outputColor;
}


}

