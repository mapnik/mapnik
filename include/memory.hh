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

#ifndef MEMORY_HH
#define MEMORY_HH

#include <iostream>
#include <cassert>

namespace mapnik
{
    class MemoryUtils
    {
    public:
	static size_t alignPointerSize(size_t ptrSize);
    private:
	MemoryUtils();
	MemoryUtils(const MemoryUtils&);
	MemoryUtils& operator=(const MemoryUtils&);
    };

    class MemoryManager
    {
    public:
	virtual void* allocate(size_t size)=0;
	virtual void deallocate(void* p)=0;
	virtual ~MemoryManager();
    protected:
	MemoryManager();                      // {}
    private:
	MemoryManager(const MemoryManager&);
	MemoryManager& operator=(const MemoryManager&);
    };

    class Object
    {
    public:
	void* operator new(size_t size);
	void* operator new(size_t size, MemoryManager* manager);
	void operator delete(void* p);
	void operator delete(void* p, MemoryManager* manager);
    protected:
	virtual ~Object() {}
	Object() {}
	Object(const Object&) {}
    protected:
	Object& operator=(const Object&)
	{
	    return *this;
	}
    };

    template <typename Geometry>
    class geometry_pool
    {
    public:
	void* allocate()
	{
	    return ::operator new(sizeof(Geometry));
	}
	void deallocate(void* p)
	{
	    ::operator delete(p);
	}
    };

}
#endif                                            //MEMORY_HH
