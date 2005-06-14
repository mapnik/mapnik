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

//$Id: image_data.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef IMAGE_DATA_HPP
#define IMAGE_DATA_HPP

namespace mapnik 
{
    template <class T> class ImageData
    {
    private:
	const unsigned width_;
	const unsigned height_;
	T *pData_;
	ImageData& operator=(const ImageData&);
    public:
	ImageData(unsigned width,unsigned height)
	    : width_(width),
	      height_(height),
	      pData_((width!=0 && height!=0)? static_cast<T*>(::operator new(sizeof(T)*width*height)):0)
	{
	    if (pData_) memset(pData_,0,sizeof(T)*width_*height_);
	}

	ImageData(const ImageData<T>& rhs)
	    :width_(rhs.width_),
	     height_(rhs.height_),
	     pData_((rhs.width_!=0 && rhs.height_!=0)? new T[rhs.width_*rhs.height_]:0)
	{
	    if (pData_) memcpy(pData_,rhs.pData_,sizeof(T)*rhs.width_* rhs.height_);
	}
	inline T& operator() (unsigned i,unsigned j)
	{
	    assert(i<width_ && j<height_);
	    return pData_[j*width_+i];
	}
	inline const T& operator() (unsigned i,unsigned j) const
	{
	    assert(i<width_ && j<height_);
	    return pData_[j*width_+i];
	}
	inline unsigned width() const
	{
	    return width_;
	}
	inline unsigned height() const
	{
	    return height_;
	}
	inline void set(const T& t)
	{
	    for (unsigned i=0;i<width_;++i)
	    {
		for (unsigned j=0;j<height_;++j)
		{
		    (*this)(i,j)=t;
		}
	    }
	}
	inline const T* getData() const
	{
	    return pData_;
	}

	inline T* getData()
	{
	    return pData_;
	}

	inline const unsigned char* getBytes() const
	{
	    return (unsigned char*)pData_;
	}
	
	inline unsigned char* getBytes()
	{
	    return (unsigned char*)pData_;
	}
	
	inline const T* getRow(unsigned row) const
	{
	    return pData_+row*width_;
	}
	inline void setRow(unsigned row,const T* buf,unsigned size)
	{
	    assert(row<height_);
	    assert(size<=(width_*sizeof(T)));
	    memcpy(pData_+row*width_,buf,size*sizeof(T));
	}
	inline void setRow(unsigned row,unsigned x0,unsigned x1,const T* buf)
	{
	    memcpy(pData_+row*width_+x0,buf,(x1-x0)*sizeof(T));
	}

	inline ~ImageData()
	{
	    ::operator delete(pData_),pData_=0;
	}
	
    };

    typedef ImageData<unsigned> ImageData32;
}

#endif //IMAGE_DATA_HPP
