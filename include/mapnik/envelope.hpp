/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: envelope.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef ENVELOPE_HPP
#define ENVELOPE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/coord.hpp>
// boost
#include <boost/operators.hpp>
// stl
#include <iomanip>

namespace mapnik {
  
   /*!
    * A spatial envelope (i.e. bounding box) which also defines some basic operators.
    */
   template <typename T> class MAPNIK_DECL Envelope 
    : boost::addable<Envelope<T>, 
      boost::subtractable<Envelope<T>, 
      boost::dividable2<Envelope<T>, T,
      boost::multipliable2<Envelope<T>, T > > > >
    {
    public:
        typedef Envelope<T> EnvelopeType;
    private:
        T minx_;
        T miny_;
        T maxx_;
        T maxy_;
    public:
        Envelope();
        Envelope(T minx,T miny,T maxx,T maxy);
        Envelope(const coord<T,2>& c0,const coord<T,2>& c1);
        Envelope(const EnvelopeType& rhs);
        T minx() const;
        T miny() const;
        T maxx() const;
        T maxy() const;
        T width() const;
        T height() const;
        void width(T w);
        void height(T h);
        coord<T,2> center() const;
        void expand_to_include(T x,T y);
        void expand_to_include(const coord<T,2>& c);
        void expand_to_include(const EnvelopeType& other);
        bool contains(const coord<T,2> &c) const;
        bool contains(T x,T y) const;
        bool contains(const EnvelopeType &other) const;
        bool intersects(const coord<T,2> &c) const;
        bool intersects(T x,T y) const;
        bool intersects(const EnvelopeType &other) const;
        EnvelopeType intersect(const EnvelopeType& other) const;
        bool operator==(const EnvelopeType &other) const;
        void re_center(T cx,T cy);
        void init(T x0,T y0,T x1,T y1);
        
        // define some operators 
        EnvelopeType& operator+=(EnvelopeType const& other);
        EnvelopeType& operator-=(EnvelopeType const& other);
        EnvelopeType& operator*=(T);
        EnvelopeType& operator/=(T);    
    };
    
    template <class charT,class traits,class T>
    inline std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& out,
                 const Envelope<T>& e)
    {
        std::basic_ostringstream<charT,traits> s;
        s.copyfmt(out);
        s.width(0);
        s <<"Envelope(" << std::setprecision(16) 
          << e.minx() << "," << e.miny() <<"," 
          << e.maxx() << "," << e.maxy() <<")";
        out << s.str();
        return out;
    }
}

#endif // ENVELOPE_HPP
