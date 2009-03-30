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
#include <boost/filesystem/operations.hpp>

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


std::string table_from_sql(std::string const& sql)
{
   std::string table_name = boost::algorithm::to_lower_copy(sql);
   boost::algorithm::replace_all(table_name,"\n"," ");
   
   std::string::size_type idx = table_name.rfind("from");
   if (idx!=std::string::npos)
   {
      
      idx=table_name.find_first_not_of(" ",idx+4);
      if (idx != std::string::npos)
      {
         table_name=table_name.substr(idx);
      }
      idx=table_name.find_first_of(" ),");
      if (idx != std::string::npos)
      {
         table_name = table_name.substr(0,idx);
      }
   }
   return table_name;
}

sqlite_datasource::sqlite_datasource(parameters const& params)
   : datasource(params),
     extent_(),
     extent_initialized_(false),
     type_(datasource::Vector),
     table_(*params.get<std::string>("table","")),
     metadata_(*params.get<std::string>("metadata","")),
     geometry_field_(*params.get<std::string>("geometry_field","the_geom")),
     key_field_(*params.get<std::string>("key_field","OGC_FID")),
     row_offset_(*params_.get<int>("row_offset",0)),
     row_limit_(*params_.get<int>("row_limit",0)),
     desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8")),
     format_(mapnik::wkbGeneric)
{
    boost::optional<std::string> file = params.get<std::string>("file");
    if (!file) throw datasource_exception("missing <file> parameter");

    boost::optional<std::string> wkb = params.get<std::string>("wkb_format");
    if (wkb)
    {
        if (*wkb == "spatialite")
            format_ = mapnik::wkbSpatiaLite;  
    }

    multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);
    use_spatial_index_ = *params_.get<mapnik::boolean>("use_spatial_index",true);

    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
        dataset_name_ = *base + "/" + *file;
    else
        dataset_name_ = *file;

    if (!boost::filesystem::exists(dataset_name_)) throw datasource_exception(dataset_name_ + " does not exist");
          
    dataset_ = new sqlite_connection (dataset_name_);

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
    
    std::string table_name = table_from_sql(table_);
    
    if (metadata_ != "" && ! extent_initialized_)
    {
        std::ostringstream s;
        s << "select xmin, ymin, xmax, ymax from " << metadata_;
        s << " where lower(f_table_name) = lower('" << table_name << "')";
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
        s << " where lower(name) = lower('idx_" << table_name << "_" << geometry_field_ << "')";
        boost::scoped_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));
        if (rs->is_valid () && rs->step_next())
        {
            use_spatial_index_ = rs->column_integer (0) == 1;
        }

#ifdef MAPNIK_DEBUG
        if (! use_spatial_index_)
           clog << "cannot use the spatial index " << endl;
#endif
    }
    
    {
        /*
            XXX - This is problematic, if we do not have at least a row,
                  we cannot determine the right columns types and names 
                  as all column_type are SQLITE_NULL
        */

        std::string::size_type idx = table_.find(table_name);
        std::ostringstream s;
        s << "select * from (" << table_.substr(0,idx + table_name.length()) << ") limit 1";
        
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
        
        s << " from "; 
        
        std::string query (table_); 
        
        if (use_spatial_index_)
        {
           std::string table_name = table_from_sql(query);
           std::ostringstream spatial_sql;
           spatial_sql << std::setprecision(16);
           spatial_sql << " where rowid in (select pkid from idx_" << table_name << "_" << geometry_field_;
           spatial_sql << " where xmax>=" << e.minx() << " and xmin<=" << e.maxx() ;
           spatial_sql << " and ymax>=" << e.miny() << " and ymin<=" << e.maxy() << ")";
           if (boost::algorithm::ifind_first(query,"where"))
           {
              boost::algorithm::ireplace_first(query, "where", spatial_sql.str() + " and");
           }
           else if (boost::algorithm::find_first(query,table_name))  
           {
              boost::algorithm::ireplace_first(query, table_name , table_name + " " + spatial_sql.str());
           }
        }
        
        s << query ;
        
        if (row_limit_ > 0) {
            s << " limit " << row_limit_;
        }

        if (row_offset_ > 0) {
            s << " offset " << row_offset_;
        }

#ifdef MAPNIK_DEBUG
        std::cerr << s.str() << "\n";
#endif

        boost::shared_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));

        return featureset_ptr (new sqlite_featureset(rs, desc_.get_encoding(), format_, multiple_geometries_));
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

