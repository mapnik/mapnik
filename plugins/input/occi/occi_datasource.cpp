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

#include "occi_datasource.hpp"
#include "occi_featureset.hpp"

// mapnik
#include <mapnik/ptree_helpers.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

// stl
#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include <iomanip>

using std::clog;
using std::endl;
using std::vector;

using boost::lexical_cast;
using boost::bad_lexical_cast;

using mapnik::datasource;
using mapnik::parameters;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;
using mapnik::Envelope;
using mapnik::coord2d;

using oracle::occi::Environment;
using oracle::occi::Connection;
using oracle::occi::Statement;
using oracle::occi::ResultSet;
using oracle::occi::MetaData;
using oracle::occi::SQLException;
using oracle::occi::Type;
using oracle::occi::StatelessConnectionPool;

DATASOURCE_PLUGIN(occi_datasource)


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

occi_datasource::occi_datasource(parameters const& params)
   : datasource (params),
     table_(*params.get<std::string>("table","")),
     geometry_field_(*params.get<std::string>("geometry_field","GEOLOC")),
     type_(datasource::Vector),
     extent_initialized_(false),
     row_limit_(*params_.get<int>("row_limit",0)),
     row_prefetch_(*params_.get<int>("row_prefetch",100)),
     desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8")),
     pool_(0)
{
   boost::optional<int> initial_size = params_.get<int>("inital_size",1);
   boost::optional<int> max_size = params_.get<int>("max_size",10);

   multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);
   use_spatial_index_ = *params_.get<mapnik::boolean>("use_spatial_index",true);

   boost::optional<std::string> ext = params_.get<std::string>("extent");
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

   // connect to environment
   try
   {
        Environment* env = occi_environment::get_environment();

        pool_ = env->createStatelessConnectionPool(
                    *params.get<std::string>("user"),
                    *params.get<std::string>("password"),
                    *params.get<std::string>("host"),
                    *max_size,
                    *initial_size,
                    1,
                    StatelessConnectionPool::HOMOGENEOUS);
   }
   catch (SQLException &ex)
   {
       throw datasource_exception(ex.getMessage());
   }

   std::string table_name = table_from_sql(table_);

   // get SRID from geometry metadata
   {
       occi_connection_ptr conn (pool_);

       std::ostringstream s;
       s << "select srid from " << SDO_GEOMETRY_METADATA_TABLE << " where";
       s << " lower(table_name) = lower('" << table_name << "') and";
       s << " lower(column_name) = lower('" << geometry_field_ << "')";

       try
       {
           ResultSet* rs = conn.execute_query (s.str());
           if (rs && rs->next ())
           {
               srid_ = rs->getInt(1);
           }
       }
       catch (SQLException &ex)
       {
           throw datasource_exception(ex.getMessage());
       }
   }

   // get columns description
   {
       std::string::size_type idx = table_.find(table_name);
       std::ostringstream s;
       s << "select * from (" << table_.substr(0,idx + table_name.length()) << ") where rownum < 1";

       occi_connection_ptr conn (pool_);

       try
       {
           ResultSet* rs = conn.execute_query (s.str());
           if (rs)
           {
               std::vector<MetaData> listOfColumns = rs->getColumnListMetaData();

               for (unsigned int i=0;i<listOfColumns.size();++i)
               {
                   MetaData columnObj = listOfColumns[i];

                   std::string fld_name = columnObj.getString(MetaData::ATTR_NAME);
                   int type_oid = columnObj.getInt(MetaData::ATTR_DATA_TYPE);

#if 0
                   int type_code = columnObj.getInt(MetaData::ATTR_TYPECODE);
                   if (type_code == OCCI_TYPECODE_OBJECT)
                   {
                       desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Object));
                       continue;
                   }
#endif

                   switch (type_oid)
                   {
                   case oracle::occi::OCCIINT:
                   case oracle::occi::OCCIUNSIGNED_INT:
                      desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
                      break;
                   case oracle::occi::OCCIFLOAT:
                   case oracle::occi::OCCIBFLOAT:
                   case oracle::occi::OCCIDOUBLE:
                   case oracle::occi::OCCIBDOUBLE:
                   case oracle::occi::OCCINUMBER:
                   case oracle::occi::OCCI_SQLT_NUM:
                      desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
                      break;
                   case oracle::occi::OCCICHAR:
                   case oracle::occi::OCCISTRING:
                   case oracle::occi::OCCI_SQLT_AFC:
                   case oracle::occi::OCCI_SQLT_AVC:
                   case oracle::occi::OCCI_SQLT_CHR:
                   case oracle::occi::OCCI_SQLT_LVC:
                   case oracle::occi::OCCI_SQLT_STR:
                   case oracle::occi::OCCI_SQLT_VCS:
                   case oracle::occi::OCCI_SQLT_VNU:
                   case oracle::occi::OCCI_SQLT_VBI:
                   case oracle::occi::OCCI_SQLT_VST:
                   case oracle::occi::OCCI_SQLT_RDD:
                      desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
                      break;
                   case oracle::occi::OCCIDATE:
                   case oracle::occi::OCCITIMESTAMP:
                   case oracle::occi::OCCI_SQLT_DAT:
                   case oracle::occi::OCCI_SQLT_TIMESTAMP:
                   case oracle::occi::OCCI_SQLT_TIMESTAMP_LTZ:
                   case oracle::occi::OCCI_SQLT_TIMESTAMP_TZ:
                   case oracle::occi::OCCIPOBJECT:
#ifdef MAPNIK_DEBUG
                      clog << "unsupported type_oid="<<type_oid<<endl;
#endif
                      break;
                   default: // shouldn't get here
#ifdef MAPNIK_DEBUG
                      clog << "unknown type_oid="<<type_oid<<endl;
#endif
                      break;
                   }	  
               }
           }
       }
       catch (SQLException &ex)
       {
           throw datasource_exception(ex.getMessage());
       }
   }        
}

occi_datasource::~occi_datasource()
{
    Environment* env = occi_environment::get_environment();
    env->terminateStatelessConnectionPool (pool_);
}

std::string const occi_datasource::name_="occi";

std::string occi_datasource::name()
{
    return name_;
}

int occi_datasource::type() const
{
    return type_;
}

Envelope<double> occi_datasource::envelope() const
{
    if (extent_initialized_) return extent_;

    double lox, loy, hix, hiy;
    occi_connection_ptr conn (pool_);

    boost::optional<mapnik::boolean> estimate_extent = params_.get<mapnik::boolean>("estimate_extent",false);
         
    if (estimate_extent && *estimate_extent)
    {
        std::ostringstream s;
        s << "select min(c.x), min(c.y), max(c.x), max(c.y) from ";
        s << " (select sdo_aggr_mbr(" << geometry_field_ << ") shape from " << table_ << ") a, ";
        s << " table (sdo_util.getvertices(a.shape)) c";

        try
        {
            ResultSet* rs = conn.execute_query (s.str());
            if (rs && rs->next ())
            {
                try 
                {
                    lox = lexical_cast<double>(rs->getDouble(1));
                    loy = lexical_cast<double>(rs->getDouble(2));
                    hix = lexical_cast<double>(rs->getDouble(3));
                    hiy = lexical_cast<double>(rs->getDouble(4));		    
                    extent_.init (lox,loy,hix,hiy);
                    extent_initialized_ = true;
                }
                catch (bad_lexical_cast &ex)
                {
                    clog << ex.what() << endl;
                }
            }
        }
        catch (SQLException &ex)
        {
            throw datasource_exception(ex.getMessage());
        }
    }
    else if (use_spatial_index_)
    {
        std::string table_name = table_from_sql(table_);

        {
            std::ostringstream s;
            s << "select dim.sdo_lb as lx, dim.sdo_ub as ux from ";
            s << SDO_GEOMETRY_METADATA_TABLE << " m, table(m.diminfo) dim ";
            s << " where lower(m.table_name) = '" << table_name << "' and dim.sdo_dimname = 'X'";
        
            try
            {
                ResultSet* rs = conn.execute_query (s.str());
                if (rs && rs->next ())
                {
                    try 
                    {
                        lox = lexical_cast<double>(rs->getDouble(1));
                        hix = lexical_cast<double>(rs->getDouble(2));
                    }
                    catch (bad_lexical_cast &ex)
                    {
                        clog << ex.what() << endl;
                    }
                }
            }
            catch (SQLException &ex)
            {
                throw datasource_exception(ex.getMessage());
            }
        }

        {
            std::ostringstream s;
            s << "select dim.sdo_lb as ly, dim.sdo_ub as uy from ";
            s << SDO_GEOMETRY_METADATA_TABLE << " m, table(m.diminfo) dim ";
            s << " where lower(m.table_name) = '" << table_name << "' and dim.sdo_dimname = 'Y'";
        
            try
            {
                ResultSet* rs = conn.execute_query (s.str());
                if (rs && rs->next ())
                {
                    try 
                    {
                        loy = lexical_cast<double>(rs->getDouble(1));
                        hiy = lexical_cast<double>(rs->getDouble(2));
                    }
                    catch (bad_lexical_cast &ex)
                    {
                        clog << ex.what() << endl;
                    }
                }
            }
            catch (SQLException &ex)
            {
                throw datasource_exception(ex.getMessage());
            }
        }

        extent_.init (lox,loy,hix,hiy);
        extent_initialized_ = true;
    }

    return extent_;
}

layer_descriptor occi_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr occi_datasource::features(query const& q) const
{
    if (pool_)
    {
        Envelope<double> const& box=q.get_bbox();
    
        std::ostringstream s;
        s << "select " << geometry_field_ << " as geom";
        std::set<std::string> const& props=q.property_names();
        std::set<std::string>::const_iterator pos=props.begin();
        std::set<std::string>::const_iterator end=props.end();
        while (pos != end)
        {
           s <<",\""<<*pos<<"\"";
           ++pos;
        }	 

        s << " from ";

        std::string query (table_); 
        std::string table_name = table_from_sql(query);

        if (use_spatial_index_)
        {
           std::ostringstream spatial_sql;
           spatial_sql << std::setprecision(16);
           spatial_sql << " where sdo_filter(" << geometry_field_ << ",";
           spatial_sql << " mdsys.sdo_geometry(" << SDO_GTYPE_2DPOLYGON << "," << srid_ << ",NULL,";
           spatial_sql << " mdsys.sdo_elem_info_array(1," << SDO_ETYPE_POLYGON << "," << SDO_INTERPRETATION_RECTANGLE << "),";
           spatial_sql << " mdsys.sdo_ordinate_array(";
           spatial_sql << box.minx() << "," << box.miny() << ", ";
           spatial_sql << box.maxx() << "," << box.maxy() << ")), 'querytype=WINDOW') = 'TRUE'";      

           if (boost::algorithm::ifind_first(query,"where"))
           {
              boost::algorithm::ireplace_first(query, "where", spatial_sql.str() + " and");
           }
           else if (boost::algorithm::find_first(query,table_name))  
           {
              boost::algorithm::ireplace_first(query, table_name , table_name + " " + spatial_sql.str());
           }
        }
        
        if (row_limit_ > 0)
        {
           std::string row_limit_string = "rownum < " + row_limit_;

           if (boost::algorithm::ifind_first(query,"where"))
           {
              boost::algorithm::ireplace_first(query, "where", row_limit_string + " and");
           }
           else if (boost::algorithm::find_first(query,table_name))  
           {
              boost::algorithm::ireplace_first(query, table_name , table_name + " " + row_limit_string);
           }
        }
        
        s << query;

#ifdef MAPNIK_DEBUG
        clog << s.str() << endl;
#endif
        
        return featureset_ptr (new occi_featureset (pool_,
                                                    s.str(),
                                                    desc_.get_encoding(),
                                                    multiple_geometries_,
                                                    row_prefetch_,
                                                    props.size()));
    }
    
    return featureset_ptr();
}

featureset_ptr occi_datasource::features_at_point(coord2d const& pt) const
{
    if (pool_)
    {
        std::ostringstream s;
        s << "select " << geometry_field_ << " as geom";
        std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
        std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
        unsigned size=0;
        while (itr != end)
        {
           s <<",\""<< itr->get_name() << "\"";
           ++itr;
           ++size;
        }

        s << " from ";

        std::string query (table_); 
        std::string table_name = table_from_sql(query);

        std::ostringstream spatial_sql;
        spatial_sql << std::setprecision(16);
        spatial_sql << " where sdo_filter(" << geometry_field_ << ",";
        spatial_sql << " mdsys.sdo_geometry(" << SDO_GTYPE_2DPOINT << "," << srid_ << ",NULL,";
        spatial_sql << " mdsys.sdo_elem_info_array(1," << SDO_ETYPE_POINT << "," << SDO_INTERPRETATION_POINT << "),";
        spatial_sql << " mdsys.sdo_ordinate_array(";
        spatial_sql << pt.x << "," << pt.y << ")), 'querytype=WINDOW') = 'TRUE'";      

        if (boost::algorithm::ifind_first(query,"where"))
        {
           boost::algorithm::ireplace_first(query, "where", spatial_sql.str() + " and");
        }
        else if (boost::algorithm::find_first(query,table_name))  
        {
           boost::algorithm::ireplace_first(query, table_name , table_name + " " + spatial_sql.str());
        }

        if (row_limit_ > 0)
        {
           std::string row_limit_string = "rownum < " + row_limit_;

           if (boost::algorithm::ifind_first(query,"where"))
           {
              boost::algorithm::ireplace_first(query, "where", row_limit_string + " and");
           }
           else if (boost::algorithm::find_first(query,table_name))  
           {
              boost::algorithm::ireplace_first(query, table_name , table_name + " " + row_limit_string);
           }
        }
        
        s << query;

#ifdef MAPNIK_DEBUG
        clog << s.str() << endl;
#endif
        
        return featureset_ptr (new occi_featureset (pool_,
                                                    s.str(),
                                                    desc_.get_encoding(),
                                                    multiple_geometries_,
                                                    row_prefetch_,
                                                    size));
    }

    return featureset_ptr();
}

