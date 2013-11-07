/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko, Jean-Francois Doyon
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
#ifndef MAPNIK_THREADS_HPP
#define MAPNIK_THREADS_HPP

#include <boost/thread/tss.hpp>         // for thread_specific_ptr
#include <Python.h>
 
namespace mapnik {
class python_thread
{
    /* Docs:
       http://docs.python.org/c-api/init.html#thread-state-and-the-global-interpreter-lock
    */
public:
    static void unblock()
    {
#ifdef MAPNIK_DEBUG
        if (state.get())
        {
            std::cerr << "ERROR: Python threads are already unblocked. "
                "Unblocking again will loose the current state and "
                "might crash later. Aborting!\n";
            abort(); //This is a serious error and can't be handled in any other sane way
        }
#endif
        PyThreadState *_save = 0; //Name defined by python
        Py_UNBLOCK_THREADS;
        state.reset(_save);
#ifdef MAPNIK_DEBUG
        if (!_save) {
            thread_support = false;
        }
#endif
    }

    static void block()
    {
#ifdef MAPNIK_DEBUG
        if (thread_support && !state.get())
        {
            std::cerr << "ERROR: Trying to restore python thread state, "
                "but no state is saved. Can't continue and also "
                "can't raise an exception because the python "
                "interpreter might be non-function. Aborting!\n";
            abort();
        }
#endif
        PyThreadState *_save = state.release(); //Name defined by python
        Py_BLOCK_THREADS;
    }

private:
    static boost::thread_specific_ptr<PyThreadState> state;
#ifdef MAPNIK_DEBUG
    static bool thread_support;
#endif
};

class python_block_auto_unblock
{
public:
    python_block_auto_unblock()
    {
        python_thread::block();
    }

    ~python_block_auto_unblock()
    {
        python_thread::unblock();
    }
};

class python_unblock_auto_block
{
public:
    python_unblock_auto_block()
    {
        python_thread::unblock();
    }

    ~python_unblock_auto_block()
    {
        python_thread::block();
    }
};

} //namespace

#endif // MAPNIK_THREADS_HPP
