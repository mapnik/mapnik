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

#ifndef PTR_HH
#define PTR_HH

namespace mapnik
{

    template <typename T> struct DefaultDeletePolicy
    {
        static void destroy(T* p)
        {
            delete p;
        }
    };
    
    template <typename T,
	      template <typename T> class DeallocPolicy=DefaultDeletePolicy>
    class ref_ptr
    {
    private:
	T* ptr_;
	int* pCount_;
    public:
	T* operator->() {return ptr_;}
	const T* operator->() const {return ptr_;}
	T* get() {return ptr_;}
	const T* get() const {return ptr_;}
	const T& operator *() const {return *ptr_;}
	T& operator *() {return *ptr_;}
	explicit ref_ptr(T* ptr=0)
	    :ptr_(ptr),pCount_(new int(1)) {}
	ref_ptr(const ref_ptr& rhs)
	    :ptr_(rhs.ptr_),pCount_(rhs.pCount_)
	{
	    (*pCount_)++;
	}
	ref_ptr& operator=(const ref_ptr& rhs)
	{
	    if (ptr_==rhs.ptr_) return *this;
	    if (--(*pCount_)==0)
	    {
		DeallocPolicy<T>::destroy(ptr_);
		delete pCount_;
	    }
	    ptr_=rhs.ptr_;
	    pCount_=rhs.pCount_;
	    (*pCount_)++;
	    return *this;
	}
	bool operator !() const
	{
	    return ptr_==0;
	}
	operator bool () const
	{
	    return ptr_!=0;
	}
	inline friend bool operator==(const ref_ptr& lhs,
				      const T* rhs)
	{
	    return lhs.ptr_==rhs;
	}
	inline friend bool operator==(const T* lhs,
				      const ref_ptr& rhs)
	{
	    return lhs==rhs.ptr_;
	}
	inline friend bool operator!=(const ref_ptr& lhs,
				      const T* rhs)
	{
	    return lhs.ptr_!=rhs;
	}
	inline friend bool operator!=(const T* lhs,
				      const ref_ptr& rhs)
	{
	    return lhs!=rhs.ptr_;
	}
	~ref_ptr()
	{
	    if (--(*pCount_)==0)
	    {
		DeallocPolicy<T>::destroy(ptr_);
		delete pCount_;
	    }
	}
    };
}
#endif                                            //PTR_HH
