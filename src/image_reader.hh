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

//$Id$

#ifndef IMAGE_READER_HH
#define IMAGE_READER_HH

#include "mapnik.hh"
#include <stdexcept>

namespace mapnik {

    class ImageReaderException : public std::exception
    {
    private:
	std::string message_;
    public:
	ImageReaderException(const std::string& message) 
	    : message_(message) {}

	~ImageReaderException() throw() {}

	virtual const char* what() const throw()
	{
	    return message_.c_str();
	}
    };

    struct ImageReader
    {
	virtual unsigned width() const=0;
	virtual unsigned height() const=0;
	virtual void read(unsigned x,unsigned y,ImageData32& image)=0;
	virtual ~ImageReader() {}
    };
    
    typedef singleton<factory<ImageReader,std::string, 
			      ImageReader* (*)(const std::string&)> > ImageReaderFactory;
}

#endif                                            //IMAGE_READER_HH
