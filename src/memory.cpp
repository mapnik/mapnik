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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/memory.hpp>

namespace mapnik
{
void* Object::operator new(size_t size)
{
    void* block=::operator new (size);
    return (char*) block;
}

void* Object::operator new(size_t size,MemoryManager* manager)
{
    assert(manager);
    size_t headerSize=MemoryUtils::alignPointerSize(sizeof(MemoryManager*));
    void* const block=manager->allocate(headerSize+size);
    *(MemoryManager**)block=manager;
    return (char*)block+headerSize;
}

void Object::operator delete(void* p)
{
    ::operator delete(p);
}

void Object::operator delete(void* , MemoryManager* )
{
    //MAPNIK_LOG_DEBUG(memory) << "operator delete with Manager " << std::hex << p << " " << manager;
}

inline size_t MemoryUtils::alignPointerSize(size_t ptrSize)
{
    size_t alignment=(sizeof(void*) >= sizeof(double)) ? sizeof(void*) : sizeof(double);
    size_t current=ptrSize % alignment;
    return (current==0)?ptrSize:(ptrSize+alignment-current);
}
}
