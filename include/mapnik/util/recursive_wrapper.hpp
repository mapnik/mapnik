/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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


#ifndef MAPNIK_UTIL_VARIANT_RECURSIVE_WRAPPER_HPP
#define MAPNIK_UTIL_VARIANT_RECURSIVE_WRAPPER_HPP

#include <utility>

namespace mapnik { namespace util {

template <typename T>
class recursive_wrapper
{
public:
    using type = T;
private:

    T* p_;

public:

    ~recursive_wrapper();
    recursive_wrapper();

    recursive_wrapper(recursive_wrapper const& operand);
    recursive_wrapper(T const& operand);
    recursive_wrapper(recursive_wrapper&& operand);
    recursive_wrapper(T&& operand);

private:

    void assign(const T& rhs);

public:

    inline recursive_wrapper& operator=(recursive_wrapper const& rhs)
    {
        assign( rhs.get() );
        return *this;
    }

    inline recursive_wrapper& operator=(T const& rhs)
    {
        assign( rhs );
        return *this;
    }

    inline void swap(recursive_wrapper& operand) noexcept
    {
        T* temp = operand.p_;
        operand.p_ = p_;
        p_ = temp;
    }


    recursive_wrapper& operator=(recursive_wrapper&& rhs) noexcept
    {
        swap(rhs);
        return *this;
    }

    recursive_wrapper& operator=(T&& rhs)
    {
        get() = std::move(rhs);
        return *this;
    }


public:

    T& get() { return *get_pointer(); }
    const T& get() const { return *get_pointer(); }
    T* get_pointer() { return p_; }
    const T* get_pointer() const { return p_; }
    operator T const&() const { return this->get(); }
    operator T&() { return this->get(); }
};

template <typename T>
recursive_wrapper<T>::~recursive_wrapper()
{
    delete p_;
}

template <typename T>
recursive_wrapper<T>::recursive_wrapper()
    : p_(new T)
{
}

template <typename T>
recursive_wrapper<T>::recursive_wrapper(recursive_wrapper const& operand)
    : p_(new T( operand.get() ))
{
}

template <typename T>
recursive_wrapper<T>::recursive_wrapper(T const& operand)
    : p_(new T(operand))
{
}

template <typename T>
recursive_wrapper<T>::recursive_wrapper(recursive_wrapper&& operand)
    : p_(operand.p_)
{
    operand.p_ = nullptr;
}

template <typename T>
recursive_wrapper<T>::recursive_wrapper(T&& operand)
    : p_(new T( std::move(operand) ))
{
}

template <typename T>
void recursive_wrapper<T>::assign(const T& rhs)
{
    this->get() = rhs;
}

template <typename T>
inline void swap(recursive_wrapper<T>& lhs, recursive_wrapper<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

}}

#endif // MAPNIK_UTIL_VARIANT_RECURSIVE_WRAPPER_HPP
