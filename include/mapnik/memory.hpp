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

#ifndef MAPNIK_MEMORY_HPP
#define MAPNIK_MEMORY_HPP

#include <new>

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
#endif // MAPNIK_MEMORY_HPP
