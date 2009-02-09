/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
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
// $Id$

#include "sqlite_datasource.hpp"
#include "sqlite_featureset.hpp"

// mapnik
#include <mapnik/ptree_helpers.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

using std::clog;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(sqlite_datasource)

using mapnik::Envelope;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;


sqlite_datasource::sqlite_datasource(parameters const& params)
   : datasource(params),
     extent_(),
     extent_initialized_(false),
     type_(datasource::Vector),
     table_(*params.get<std::string>("table","")),
     geometry_field_(*params.get<std::string>("geometry_field","geom")),
     geocatalog_(*params.get<std::string>("geometry_catalog","geocatalog")),
     desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8"))
{
    boost::optional<std::string> file = params.get<std::string>("file");
    if (!file) throw datasource_exception("missing <file> paramater");

    multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);

    dataset_ = new sqlite_connection (*file);

    boost::optional<std::string> ext  = params_.get<std::string>("extent");
    if (ext)
    {
        boost::char_separator<char> sep(",");
        boost::tokenizer<boost::char_separator<char> > tok(*ext,sep);
        unsigned i = 0;
        bool success = false;
        double d[4];
        for (boost::tokenizer<boost::char_separator<char> >::iterator beg=tok.begin(); 
             beg!=tok.end();++beg)
        {
            try 
            {
                d[i] = boost::lexical_cast<double>(*beg);
            }
            catch (boost::bad_lexical_cast & ex)
            {
                std::clog << ex.what() << "\n";
                break;
            }
            if (i==3) 
            {
                success = true;
                break;
            }
            ++i;
        }

        if (success)
        {
            extent_.init(d[0],d[1],d[2],d[3]);
            extent_initialized_ = true;
        }
    } 

#if 0
    {
        std::ostringstream s;
        s << "select minx, miny, maxx, maxy from " << geocatalog_;
        s << " where lower(table_name) = lower('" << table_ << "')";
        boost::shared_ptr<sqlite_resultset> rs = dataset_->execute_query (s.str());
        if (rs->is_valid () && rs->step_next())
        {
            double minx = rs->column_double (0);
            double miny = rs->column_double (1);
            double maxx = rs->column_double (2);
            double maxy = rs->column_double (3);
            
            extent_.init (minx,miny,maxx,maxy);
        }
    }
#endif

    {
        std::ostringstream s;
        s << "select * from " << table_ << " limit 1";
        boost::shared_ptr<sqlite_resultset> rs = dataset_->execute_query (s.str());
        if (rs->is_valid () && rs->step_next())
        {
            /*
                XXX - This is problematic, if we don't have at least a row,
                      we cannot determine the right columns types and names 
                      as all column_type are SQLITE_NULL
            */
    
            for (int i = 0; i < rs->column_count (); ++i)
            {
               const int type_oid = rs->column_type (i);
               const char* fld_name = rs->column_name (i);
               switch (type_oid)
               {
                  case SQLITE_INTEGER:
#ifdef MAPNIK_DEBUG
                     clog << fld_name << " integer" << endl;
#endif
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
                     break;
                     
                  case SQLITE_FLOAT:
#ifdef MAPNIK_DEBUG
                     clog << fld_name << " double" << endl;
#endif
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
                     break;
                     
                  case SQLITE_TEXT:
#ifdef MAPNIK_DEBUG
                     clog << fld_name << " text" << endl;
#endif
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
                     break;
                     
                  case SQLITE_NULL:
                  case SQLITE_BLOB:
                     break;
                     
                  default:
#ifdef MAPNIK_DEBUG
                     clog << "unknown type_oid="<<type_oid<<endl;
#endif
                     break;
                }
            }    
        }
    }
}

sqlite_datasource::~sqlite_datasource()
{
    delete dataset_;
}

std::string const sqlite_datasource::name_="sqlite";

std::string sqlite_datasource::name()
{
   return name_;
}

int sqlite_datasource::type() const
{
   return type_;
}

Envelope<double> sqlite_datasource::envelope() const
{
   return extent_;
}

layer_descriptor sqlite_datasource::get_descriptor() const
{
   return desc_;
}

featureset_ptr sqlite_datasource::features(query const& q) const
{
   if (dataset_)
   {

        mapnik::Envelope<double> const& e = q.get_bbox();
#if 0
        layer_->SetSpatialFilterRect (query_extent.minx(),
                                      query_extent.miny(),
                                      query_extent.maxx(),
                                      query_extent.maxy());
#endif

        std::ostringstream s;
        s << "select " << geometry_field_ << ",PK_UID";
        std::set<std::string> const& props = q.property_names();
        std::set<std::string>::const_iterator pos = props.begin();
        std::set<std::string>::const_iterator end = props.end();
        while (pos != end)
        {
           s << "," << *pos << "";
           ++pos;
        }	 
        s << " from " << table_ << "," <<  "idx_" << table_ << "_" <<  geometry_field_;
        s << " where " << table_<<".PK_UID="<<  "idx_" << table_ << "_" << geometry_field_ << ".pkid" ;
        s << " and xmax>=" << e.minx() << " and xmin<=" << e.maxx() ;
        s << " and ymax>=" << e.miny() << " and ymin<=" << e.maxy() ;
        
        std::cerr << s.str() << "\n";
        boost::shared_ptr<sqlite_resultset> rs = dataset_->execute_query (s.str());

        return featureset_ptr (new sqlite_featureset(rs, desc_.get_encoding(), multiple_geometries_));
   }

   return featureset_ptr();
}

featureset_ptr sqlite_datasource::features_at_point(coord2d const& pt) const
{
#if 0
   if (dataset_ && layer_)
   {
        OGRPoint point;
        point.setX (pt.x);
        point.setY (pt.y);
        
        layer_->SetSpatialFilter (&point);
        
        return featureset_ptr(new ogr_featureset(*dataset_, *layer_, desc_.get_encoding(), multiple_geometries_));
   }
#endif

   return featureset_ptr();
}

