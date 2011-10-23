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

#ifndef MAPNIK_CTRANS_HPP
#define MAPNIK_CTRANS_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/coord_array.hpp>
#include <mapnik/proj_transform.hpp>

// boost
#include <boost/math/constants/constants.hpp>

// stl
#include <algorithm>

const double pi = boost::math::constants::pi<double>();
const double pi_by_2 = pi/2.0;

namespace mapnik
{

typedef coord_array<coord2d> CoordinateArray;


template <typename Transform, typename Geometry>
struct MAPNIK_DECL coord_transform
{
    coord_transform(Transform const& t, Geometry& geom)
        : t_(t), geom_(geom) {}

    unsigned vertex(double *x, double *y) const
    {
        unsigned command = geom_.vertex(x, y);
        t_.forward(x, y);
        return command;
    }

    void rewind(unsigned pos)
    {
        geom_.rewind(pos);
    }

private:
    Transform const& t_;
    Geometry& geom_;
};


template <typename Transform, typename Geometry>
struct MAPNIK_DECL coord_transform2
{
    typedef std::size_t size_type;
    typedef typename Geometry::value_type value_type;

    coord_transform2(Transform const& t,
                     Geometry const& geom,
                     proj_transform const& prj_trans)
        : t_(t),
        geom_(geom),
        prj_trans_(prj_trans)  {}

    unsigned vertex(double *x, double *y) const
    {
        unsigned command = SEG_MOVETO;
        bool ok = false;
        bool skipped_points = false;
        double z = 0;
        while (!ok && command != SEG_END)
        {
            command = geom_.vertex(x, y);
            ok = prj_trans_.backward(*x, *y, z);
            if (!ok) {
                skipped_points = true;
            }
        }
        if (skipped_points && (command == SEG_LINETO))
        {
            command = SEG_MOVETO;
        }
        t_.forward(x, y);
        return command;
    }

    void rewind(unsigned pos)
    {
        geom_.rewind(pos);
    }

    Geometry const& geom() const
    {
        return geom_;
    }

private:
    Transform const& t_;
    Geometry const& geom_;
    proj_transform const& prj_trans_;
};


template <typename Transform, typename Geometry>
struct MAPNIK_DECL coord_transform3
{
    coord_transform3(Transform const& t,
                     Geometry const& geom,
                     proj_transform const& prj_trans,
                     int dx, int dy)
        : t_(t),
        geom_(geom),
        prj_trans_(prj_trans),
        dx_(dx), dy_(dy) {}

    unsigned vertex(double *x, double *y) const
    {
        unsigned command = geom_.vertex(x, y);
        double z = 0;
        prj_trans_.backward(*x, *y, z);
        t_.forward(x, y);
        *x += dx_;
        *y += dy_;
        return command;
    }

    void rewind(unsigned pos)
    {
        geom_.rewind(pos);
    }

private:
    Transform const& t_;
    Geometry const& geom_;
    proj_transform const& prj_trans_;
    int dx_;
    int dy_;
};


// TODO - expose this and make chainable
template <typename Transform,typename Geometry>
struct MAPNIK_DECL coord_transform_parallel
{
    typedef std::size_t size_type;
    typedef typename Geometry::value_type value_type;

    coord_transform_parallel(Transform const& t,
                     Geometry const& geom,
                     proj_transform const& prj_trans )
        : t_(t), 
          geom_(geom), 
          prj_trans_(prj_trans),
          offset_(0.0),
          threshold_(10),
          m_status(initial) {}
    
    enum status
    { 
        initial, 
        start,
        first, 
        process,
        last_vertex,
        angle_joint,
        end 
    };

    double get_offset() const
    {
        return offset_;
    }
    
    void set_offset(double offset)
    {
        offset_ = offset;
    }

    unsigned int get_threshold() const
    {
        return threshold_;
    }
    
    void set_threshold(unsigned int t)
    {
        threshold_ = t;
    }

    unsigned vertex(double * x , double  * y)
    {
        double z=0;
       
        if (offset_==0.0)
        {
            unsigned command = geom_.vertex(x,y);
            prj_trans_.backward(*x,*y,z);
            t_.forward(x,y);
            return command;
        }
        else
        {
            while(true){
                switch(m_status)
                {
                case end:
                    return SEG_END;
                    break;
                case initial:
                    m_pre_cmd = geom_.vertex(x,y);
                    prj_trans_.backward(*x,*y,z);
                    t_.forward(x,y);
                    m_pre_x = *x;
                    m_pre_y = *y;
                    //m_status = (m_pre_cmd!=SEG_END)?start:end; //
                case start:
                    m_cur_cmd = geom_.vertex(&m_cur_x, &m_cur_y);
                    prj_trans_.backward(m_cur_x,m_cur_y,z);
                    t_.forward(&m_cur_x,&m_cur_y);
                case first:
                    angle_a = atan2((m_pre_y-m_cur_y),(m_pre_x-m_cur_x));
                    dx_pre = cos(angle_a + pi_by_2);
                    dy_pre = sin(angle_a + pi_by_2);
                    #ifdef MAPNIK_DEBUG
                        std::clog << "offsetting line by: " << offset_ << "\n";
                        std::clog << "initial dx=" << (dx_pre * offset_) << " dy=" << (dy_pre * offset_) << "\n";
                    #endif
                    *x = m_pre_x + (dx_pre * offset_);
                    *y = m_pre_y + (dy_pre * offset_);
                    m_status = process;
                    return SEG_MOVETO;
                case process:
                    switch(m_cur_cmd)
                    {
                        case SEG_LINETO:
                            m_next_cmd = geom_.vertex(&m_next_x, &m_next_y);
                            prj_trans_.backward(m_next_x,m_next_y,z);
                            t_.forward(&m_next_x,&m_next_y);
                            switch(m_next_cmd)
                            {
                                case SEG_LINETO:
                                    m_status = angle_joint;
                                    break;
                                default:
                                    m_status = last_vertex;
                                    break;
                            }
                            break;
                        case SEG_END:
                            m_status = end;
                            return SEG_END;
                    }
                    break;
                case last_vertex:
                    dx_curr = cos(angle_a + pi_by_2);
                    dy_curr = sin(angle_a + pi_by_2);
                    *x = m_cur_x + (dx_curr * offset_);
                    *y = m_cur_y + (dy_curr * offset_);
                    m_status = end;
                    return m_cur_cmd;
                case angle_joint:
                    angle_b = atan2((m_cur_y-m_next_y),(m_cur_x-m_next_x));
                    h = tan((angle_b - angle_a)/2.0);

                    if (fabs(h) < threshold_)
                    {
                        dx_curr = cos(angle_a + pi_by_2);
                        dy_curr = sin(angle_a + pi_by_2);
                        *x = m_cur_x + (dx_curr * offset_) - h * (dy_curr * offset_);
                        *y = m_cur_y + (dy_curr * offset_) + h * (dx_curr * offset_);
                    }
                    else // skip sharp spikes
                    {
                        
                        #ifdef MAPNIK_DEBUG
                        dx_curr = cos(angle_a + pi_by_2);
                        dy_curr = sin(angle_a + pi_by_2);
                        sin_curve = dx_curr*dy_pre-dy_curr*dx_pre;
                        std::clog << "angle a: " << angle_a << "\n";
                        std::clog << "angle b: " << angle_b << "\n";
                        std::clog << "h: " << h << "\n";
                        std::clog << "sin_curve: " << sin_curve << "\n";
                        #endif
                        m_status = process;
                        break;
                    }

                    // alternate sharp spike fix, but suboptimal...
                   
                    /*
                    sin_curve = dx_curr*dy_pre-dy_curr*dx_pre;
                    cos_curve = -dx_pre*dx_curr-dy_pre*dy_curr;
                   
                    #ifdef MAPNIK_DEBUG
                        std::clog << "sin_curve value: " << sin_curve << "\n";
                    #endif
                    if(sin_curve > -0.3 && sin_curve < 0.3) {
                        angle_b = atan2((m_cur_y-m_next_y),(m_cur_x-m_next_x));
                        h = tan((angle_b - angle_a)/2.0);
                        *x = m_cur_x + (dx_curr * offset_) - h * (dy_curr * offset_);
                        *y = m_cur_y + (dy_curr * offset_) + h * (dx_curr * offset_);
                    } else {
                        if (angle_b - angle_a > 0)
                            h = -1.0*(1.0+cos_curve)/sin_curve;
                        else
                            h = (1.0+cos_curve)/sin_curve;
                        *x = m_cur_x + (dx_curr + base_shift*dy_curr)*offset_;
                        *y = m_cur_y + (dy_curr - base_shift*dx_curr)*offset_;
                    }
                    */
                   
                    m_pre_x = *x;
                    m_pre_x = *y;
                    m_cur_x = m_next_x;
                    m_cur_y = m_next_y;
                    angle_a = angle_b;
                    m_pre_cmd = m_cur_cmd;
                    m_cur_cmd = m_next_cmd;
                    m_status = process;
                    return m_pre_cmd;
                }
            }  
        }
    }

    void rewind (unsigned pos)
    {
        geom_.rewind(pos);
        m_status = initial;
    }
   
private:
    Transform const& t_;
    Geometry const& geom_;
    proj_transform const& prj_trans_;
    int           offset_;
    unsigned int  threshold_;
    status        m_status;
    double        dx_pre;
    double        dy_pre;
    double        dx_curr;
    double        dy_curr;
    double        sin_curve;
    double        cos_curve;
    double        angle_a;
    double        angle_b;
    double        h;
    unsigned      m_pre_cmd;
    double        m_pre_x;
    double        m_pre_y;
    unsigned      m_cur_cmd;
    double        m_cur_x;
    double        m_cur_y;
    unsigned      m_next_cmd;
    double        m_next_x;
    double        m_next_y;
};


class CoordTransform
{
private:
    int width_;
    int height_;
    double sx_;
    double sy_;
    box2d<double> extent_;
    double offset_x_;
    double offset_y_;

public:
    CoordTransform(int width, int height, const box2d<double>& extent,
                   double offset_x = 0, double offset_y = 0)
        : width_(width), height_(height), extent_(extent),
        offset_x_(offset_x), offset_y_(offset_y)
    {
        sx_ = static_cast<double>(width_) / extent_.width();
        sy_ = static_cast<double>(height_) / extent_.height();
    }

    inline int width() const
    {
        return width_;
    }

    inline int height() const
    {
        return height_;
    }

    inline double scale_x() const
    {
        return sx_;
    }

    inline double scale_y() const
    {
        return sy_;
    }

    inline void forward(double *x, double *y) const
    {
        *x = (*x - extent_.minx()) * sx_ - offset_x_;
        *y = (extent_.maxy() - *y) * sy_ - offset_y_;
    }

    inline void backward(double *x, double *y) const
    {
        *x = extent_.minx() + (*x + offset_x_) / sx_;
        *y = extent_.maxy() - (*y + offset_y_) / sy_;
    }

    inline coord2d& forward(coord2d& c) const
    {
        forward(&c.x, &c.y);
        return c;
    }

    inline coord2d& backward(coord2d& c) const
    {
        backward(&c.x, &c.y);
        return c;
    }

    inline box2d<double> forward(const box2d<double>& e,
                                 proj_transform const& prj_trans) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        double z = 0.0;
        prj_trans.backward(x0, y0, z);
        forward(&x0, &y0);
        prj_trans.backward(x1, y1, z);
        forward(&x1, &y1);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline box2d<double> forward(const box2d<double>& e) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        forward(&x0, &y0);
        forward(&x1, &y1);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline box2d<double> backward(const box2d<double>& e,
                                  proj_transform const& prj_trans) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        double z = 0.0;
        backward(&x0, &y0);
        prj_trans.forward(x0, y0, z);
        backward(&x1, &y1);
        prj_trans.forward(x1, y1, z);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline box2d<double> backward(const box2d<double>& e) const
    {
        double x0 = e.minx();
        double y0 = e.miny();
        double x1 = e.maxx();
        double y1 = e.maxy();
        backward(&x0, &y0);
        backward(&x1, &y1);
        return box2d<double>(x0, y0, x1, y1);
    }

    inline CoordinateArray& forward(CoordinateArray& coords) const
    {
        for (unsigned i = 0; i < coords.size(); ++i)
        {
            forward(coords[i]);
        }
        return coords;
    }

    inline CoordinateArray& backward(CoordinateArray& coords) const
    {
        for (unsigned i = 0; i < coords.size(); ++i)
        {
            backward(coords[i]);
        }
        return coords;
    }

    inline box2d<double> const& extent() const
    {
        return extent_;
    }
};
}

#endif // MAPNIK_CTRANS_HPP
