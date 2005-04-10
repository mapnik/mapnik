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

#ifndef DATASOURCE_HH
#define DATASOURCE_HH

#include <map>
#include <string>
#include "ptr.hh"
#include "ctrans.hh"
#include "params.hh"
#include "feature.hh"
#include "query.hh"

namespace mapnik
{
    struct Featureset
    {
        virtual Feature* next()=0;
        virtual ~Featureset() {};
    };

    typedef ref_ptr<Featureset> FeaturesetPtr;

    class datasource_exception : public std::exception
    {
    private:
        const std::string message_;
    public:
	datasource_exception(const std::string& message=std::string())
	    :message_(message) {}

	~datasource_exception() throw() {}
	virtual const char* what() const throw()
	{
	    return message_.c_str();
	}
    };
    
    class datasource
    {
    public:
	enum {
	    Point=1,
	    Line,
	    Polygon,
	    Raster
	};
	virtual int type() const=0;
	virtual FeaturesetPtr features(const query& q) const=0;
	virtual const Envelope<double>& envelope() const=0;
	virtual ~datasource() {};
    };
    
    typedef std::string datasource_name();
    typedef datasource* create_ds(const Parameters& params);
    typedef void destroy_ds(datasource *ds);

    template <typename DATASOURCE>
    struct datasource_delete
    {
        static void destroy(DATASOURCE* ds)
        {
            destroy_ds(ds);
        }
    };

    typedef ref_ptr<datasource,datasource_delete> datasource_p;

    ///////////////////////////////////////////
    #define DATASOURCE_PLUGIN(classname) \
        extern "C" std::string datasource_name() \
        { \
        return classname::name();\
        }\
        extern "C"  datasource* create(const Parameters &params) \
        { \
        return new classname(params);\
        }\
        extern "C" void destroy(datasource *ds) \
        { \
        delete ds;\
        }\
        ///////////////////////////////////////////
}
#endif                                            //DATASOURCE_HH
