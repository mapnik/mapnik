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

//$Id: postgis.cc 44 2005-04-22 18:53:54Z pavlenko $

#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include "connection_manager.hpp"
#include "postgis.hpp"

DATASOURCE_PLUGIN(postgis_datasource)

   const std::string postgis_datasource::GEOMETRY_COLUMNS="geometry_columns";
const std::string postgis_datasource::SPATIAL_REF_SYS="spatial_ref_system";

using std::clog;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::shared_ptr;

postgis_datasource::postgis_datasource(parameters const& params)
   : datasource (params),
     table_(params.get("table")),
     type_(datasource::Vector),
     extent_initialized_(false),
     desc_(params.get("type"),"utf-8"),
     creator_(params.get("host"),
              params.get("port"),
              params.get("dbname"),
              params.get("user"),
              params.get("password"))    
{   

   unsigned initial_size;
   unsigned max_size;
    
   try 
   {
      initial_size = boost::lexical_cast<unsigned>(params_.get("initial_size")); 
   }
   catch (bad_lexical_cast& )
   {
      initial_size = 1;
   }
    
   try 
   {
      max_size = boost::lexical_cast<unsigned>(params_.get("max_size")); 
   }
   catch (bad_lexical_cast&)
   {
      max_size = 10;
   }
    
   ConnectionManager *mgr=ConnectionManager::instance();   
   mgr->registerPool(creator_, initial_size, max_size);
    
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {
         PoolGuard<shared_ptr<Connection>,
            shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
           
         //std::cout << "encoding = " << conn->client_encoding()<< "\n";
         //desc_.set_encoding(conn->client_encoding());
         
         std::string table_name=table_from_sql(table_);
         std::ostringstream s;
         s << "select f_geometry_column,srid,type from ";
         s << GEOMETRY_COLUMNS <<" where f_table_name='" << table_name<<"'";
            
         shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
         if (rs->next())
         {
            try 
            {
               srid_ = lexical_cast<int>(rs->getValue("srid"));
            }
            catch (bad_lexical_cast &ex)
            {
               clog << ex.what() << endl;
            }
            geometryColumn_=rs->getValue("f_geometry_column");
            std::string postgisType=rs->getValue("type");
         }
         rs->close();
            
         // collect attribute desc
         s.str("");
         s << "select * from " << table_ << " limit 1";
         rs=conn->executeQuery(s.str());
         if (rs->next())
         {
            int count = rs->getNumFields();
            for (int i=0;i<count;++i)
            {
               std::string fld_name=rs->getFieldName(i);
               int length = rs->getFieldLength(i);
		    
               int type_oid = rs->getTypeOID(i);
               switch (type_oid)
               {
                  case 21:    // int2
                  case 23:    // int4
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer,false,length));
                     break;
                  case 700:   // float4 
                  case 701:   // float8
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double,false,length));
                  case 1042:  // bpchar
                  case 1043:  // varchar
                  case 25:    // text
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
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
   }
}

std::string const postgis_datasource::name_="postgis";

std::string postgis_datasource::name()
{
   return name_;
}

int postgis_datasource::type() const
{
   return type_;
}

layer_descriptor postgis_datasource::get_descriptor() const
{
   return desc_;
}

std::string postgis_datasource::table_from_sql(const std::string& sql)
{
   std::string table_name(sql);
   std::transform(table_name.begin(),table_name.end(),table_name.begin(),tolower);
   std::string::size_type idx=table_name.rfind("from");
   if (idx!=std::string::npos)
   {
      idx=table_name.find_first_not_of(" ",idx+4);
      table_name=table_name.substr(idx);
      idx=table_name.find_first_of(" )");
      return table_name.substr(0,idx);
   }
   return table_name;
}

featureset_ptr postgis_datasource::features(const query& q) const
{
   Envelope<double> const& box=q.get_bbox();
   ConnectionManager *mgr=ConnectionManager::instance();
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {       
         PoolGuard<shared_ptr<Connection>,shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
         std::ostringstream s;
            
         s << "select asbinary("<<geometryColumn_<<") as geom";
         std::set<std::string> const& props=q.property_names();
         std::set<std::string>::const_iterator pos=props.begin();
         std::set<std::string>::const_iterator end=props.end();
         while (pos != end)
         {
            s <<",\""<<*pos<<"\"";
            ++pos;
         }	 
         s << " from " << table_<<" where "<<geometryColumn_<<" && setSRID('BOX3D(";
         s << std::setprecision(16);
         s << box.minx() << " " << box.miny() << ",";
         s << box.maxx() << " " << box.maxy() << ")'::box3d,"<<srid_<<")";
            
#ifdef MAPNIK_DEBUG
         std::clog << s.str() << "\n";
#endif           
         shared_ptr<ResultSet> rs=conn->executeQuery(s.str(),1);
         return featureset_ptr(new postgis_featureset(rs,props.size()));
      }
   }
   return featureset_ptr();
}

featureset_ptr postgis_datasource::features_at_point(coord2d const& pt) const
{
   ConnectionManager *mgr=ConnectionManager::instance();
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {       
         PoolGuard<shared_ptr<Connection>,shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
         std::ostringstream s;
            
         s << "select asbinary(" << geometryColumn_ << ") as geom";
            
         std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
         std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
         unsigned size=0;
         while (itr != end)
         {
            s <<",\""<< itr->get_name() << "\"";
            ++itr;
            ++size;
         }
            
         s << " from " << table_<<" where "<<geometryColumn_<<" && setSRID('BOX3D(";
         s << std::setprecision(16);
         s << pt.x << " " << pt.y << ",";
         s << pt.x << " " << pt.y << ")'::box3d,"<<srid_<<")";
            
#ifdef MAPNIK_DEBUG
         std::clog << s.str() << "\n";
#endif           
         shared_ptr<ResultSet> rs=conn->executeQuery(s.str(),1);
         return featureset_ptr(new postgis_featureset(rs, size));
      }
   }
   return featureset_ptr();
}

Envelope<double> postgis_datasource::envelope() const
{
   if (extent_initialized_) return extent_;
    
   ConnectionManager *mgr=ConnectionManager::instance();
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {
         std::ostringstream s;
         std::string table_name = table_from_sql(table_);
         if (params_.get("estimate_extent") == "true")
         {
            s << "select xmin(ext),ymin(ext),xmax(ext),ymax(ext)"
              << " from (select estimated_extent('" 
              << table_name <<"','" 
              << geometryColumn_ << "') as ext) as tmp";
         }
         else 
         {
            s << "select xmin(ext),ymin(ext),xmax(ext),ymax(ext)"
              << " from (select extent(" <<geometryColumn_<< ") as ext from " 
              << table_name << ") as tmp";
         }
            
         shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
         if (rs->next())
         {
            try 
            {
               double lox=lexical_cast<double>(rs->getValue(0));
               double loy=lexical_cast<double>(rs->getValue(1));
               double hix=lexical_cast<double>(rs->getValue(2));
               double hiy=lexical_cast<double>(rs->getValue(3));		    
               extent_.init(lox,loy,hix,hiy);
               extent_initialized_ = true;
            }
            catch (bad_lexical_cast &ex)
            {
               clog << ex.what() << endl;
            }
         }
         rs->close();
      }
   }
   return extent_;
}

postgis_datasource::~postgis_datasource() {}
