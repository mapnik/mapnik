
#ifndef PROJECTION_HPP
#define PROJECTION_HPP

#include <string>
#include <iostream>
#include <stdexcept>
#include <boost/utility.hpp>

#include <mapnik/envelope.hpp>
#include <proj_api.h>

namespace mapnik
{   
    class proj_init_error : public std::runtime_error
    {
    public:
        proj_init_error(std::string const& params)
            : std::runtime_error("failed to initialize projection with:" + params) {}
    };
    
    class projection
    {
        friend class proj_transform;
    public:
        explicit projection(std::string params = "+proj=latlong +ellps=WGS84")
            : params_(params)
        { 
            init(); //
        }
        
        projection(projection const& rhs)
            : params_(rhs.params_) 
        {
            init(); //
        }
        
        projection& operator=(projection const& rhs) 
        { 
            projection tmp(rhs);
            swap(tmp);
            return *this;
        }
        
        bool is_initialized() const
        {
            return proj_ ? true : false;
        }
        
        std::string const& params() const
        {
            return params_;
        }
        
        void forward(double & x, double &y ) const
        {
            projUV p;
            p.u = x * DEG_TO_RAD;
            p.v = y * DEG_TO_RAD;
            p = pj_fwd(p,proj_);
            x = p.u;
            y = p.v;
        }
        
        void inverse(double & x,double & y) const
        {
            projUV p;
            p.u = x;
            p.v = y;
            p = pj_inv(p,proj_);
            x = RAD_TO_DEG * p.u;
            y = RAD_TO_DEG * p.v;
        }
        
        ~projection() 
        {
            if (proj_) pj_free(proj_);
        }
    private:
        
        void init()
        {
            proj_=pj_init_plus(params_.c_str());
            if (!proj_) throw proj_init_error(params_);
        }
        
        void swap (projection& rhs)
        {
            std::swap(params_,rhs.params_);
            init ();
        } 
        
    private:
        std::string params_;
        projPJ proj_;
    };
    
    class proj_transform : private boost::noncopyable
    {
    public:
        proj_transform(projection const& source, 
                       projection const& dest)
            : source_(source),
              dest_(dest) 
        {
            is_source_latlong_ = pj_is_latlong(source_.proj_);
            is_dest_latlong_ = pj_is_latlong(dest_.proj_);
        }
        
        bool forward (double & x, double & y , double & z) const
        {
            if (is_source_latlong_)
            {
                x *= DEG_TO_RAD;
                y *= DEG_TO_RAD;
            }
            
            if (pj_transform( source_.proj_, dest_.proj_, 1, 
                              0, &x,&y,&z) != 0)
            {
                return false;
            }
            
            if (is_dest_latlong_)
            {
                x *= RAD_TO_DEG;
                y *= RAD_TO_DEG;
            }
            
            return true;
        } 
        
        bool forward (Envelope<double> & ext) const
        {
            if (is_source_latlong_)
            {
                ext = ext.intersect(Envelope<double>(-180,-90,180,90));
            }
            // TODO
            return true;
        }
        
        bool backward (double & x, double & y , double & z) const
        {
            if (is_dest_latlong_)
            {
                x *= DEG_TO_RAD;
                y *= DEG_TO_RAD;
            }
            
            if (pj_transform( dest_.proj_, source_.proj_, 1, 
                              0, &x,&y,&z) != 0)
            {
                return false;
            }
            
            if (is_source_latlong_)
            {
                x *= RAD_TO_DEG;
                y *= RAD_TO_DEG;
            }
            
            return true;
        } 
        
    private:
        projection const& source_;
        projection const& dest_;
        bool is_source_latlong_;
        bool is_dest_latlong_;
    };
}

#endif //PROJECTION_HPP
