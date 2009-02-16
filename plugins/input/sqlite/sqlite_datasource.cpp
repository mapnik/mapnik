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
     metadata_(*params.get<std::string>("metadata","")),
     geometry_field_(*params.get<std::string>("geometry_field","geom")),
     key_field_(*params.get<std::string>("key_field","PK_UID")),
     desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8"))
{
    boost::optional<std::string> file = params.get<std::string>("file");
    if (!file) throw datasource_exception("missing <file> paramater");

    multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);
    use_spatial_index_ = *params_.get<mapnik::boolean>("use_spatial_index",true);

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

    if (metadata_ != "" && ! extent_initialized_)
    {
        std::ostringstream s;
        s << "select xmin, ymin, xmax, ymax from " << metadata_;
        s << " where lower(f_table_name) = lower('" << table_ << "')";
        boost::scoped_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));
        if (rs->is_valid () && rs->step_next())
        {
            double xmin = rs->column_double (0);
            double ymin = rs->column_double (1);
            double xmax = rs->column_double (2);
            double ymax = rs->column_double (3);
            
            extent_.init (xmin,ymin,xmax,ymax);
            extent_initialized_ = true;
        }
    }

    if (use_spatial_index_)
    {
        std::ostringstream s;
        s << "select count (*) from sqlite_master";
        s << " where name = 'idx_" << table_ << "_" << geometry_field_ << "'";
        boost::scoped_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));
        if (rs->is_valid () && rs->step_next())
        {
            if (rs->column_integer (0) == 0)
            {
#ifdef MAPNIK_DEBUG
                clog << "cannot use the spatial index " << endl;
#endif
                use_spatial_index_ = false;
            }
        }
    }

    {
        /*
            XXX - This is problematic, if we don't have at least a row,
                  we cannot determine the right columns types and names 
                  as all column_type are SQLITE_NULL
        */
        std::ostringstream s;
        s << "select * from " << table_ << " limit 1";
        boost::scoped_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));
        if (rs->is_valid () && rs->step_next())
        {
            for (int i = 0; i < rs->column_count (); ++i)
            {
               const int type_oid = rs->column_type (i);
               const char* fld_name = rs->column_name (i);
               switch (type_oid)
               {
                  case SQLITE_INTEGER:
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
                     break;
                     
                  case SQLITE_FLOAT:
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
                     break;
                     
                  case SQLITE_TEXT:
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

        std::ostringstream s;
        s << "select " << geometry_field_ << "," << key_field_;
        std::set<std::string> const& props = q.property_names();
        std::set<std::string>::const_iterator pos = props.begin();
        std::set<std::string>::const_iterator end = props.end();
        while (pos != end)
        {
           s << "," << *pos << "";
           ++pos;
        }	 
        s << " from " << table_;

        if (use_spatial_index_)
        {
            s << std::setprecision(16);
            s << " where rowid in (select pkid from idx_" << table_ << "_" << geometry_field_;
            s << " where xmax>=" << e.minx() << " and xmin<=" << e.maxx() ;
            s << " and ymax>=" << e.miny() << " and ymin<=" << e.maxy() << ")";
        }

#ifdef MAPNIK_DEBUG
        std::cerr << "executing sql: " << s.str() << "\n";
#endif

        boost::shared_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));

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

