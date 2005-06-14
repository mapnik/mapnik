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

//$Id: memory.cpp 17 2005-03-08 23:58:43Z pavlenko $

#include "memory.hpp"

//#define GC_THREADS
//#include "gc.h"

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

    void Object::operator delete(void* p, MemoryManager* manager)
    {
        std::cout <<"operator delete with Manager "<<std::hex<<p<<" "<<manager<<std::endl;
    }

    inline size_t MemoryUtils::alignPointerSize(size_t ptrSize)
    {
        size_t alignment=(sizeof(void*) >= sizeof(double)) ? sizeof(void*) : sizeof(double);
        size_t current=ptrSize % alignment;
        return (current==0)?ptrSize:(ptrSize+alignment-current);
    }
}
