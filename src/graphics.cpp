/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: graphics.cpp 17 2005-03-08 23:58:43Z pavlenko $

#include <cassert>
#include <string.h>
#include <stack>
#include <iostream>
#include "graphics.hpp"
#include "image_util.hpp"

namespace mapnik
{
    Image32::Image32(int width,int height)
        :width_(width),
        height_(height),
        data_(width,height) {}

    Image32::Image32(const Image32& rhs)
        :width_(rhs.width_),
        height_(rhs.height_),
        data_(rhs.data_) {}

    Image32::~Image32() {}

    gamma Image32::gammaTable_;

    const ImageData32& Image32::data() const
    {
        return data_;
    }

    void Image32::setBackground(const Color& background)
    {
        background_=background;
        data_.set(background_.rgba());
    }

    const Color& Image32::getBackground() const
    {
        return background_;
    }
    
    void Image32::saveToFile(const std::string& file,const std::string& format) 
    {
	//TODO: image writer factory
	ImageUtils::save_to_file(file,format,*this);
    }
}
