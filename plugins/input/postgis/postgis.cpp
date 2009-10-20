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

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/ptree_helpers.hpp>

#ifdef MAPNIK_DEBUG
#include <mapnik/wall_clock_timer.hpp>
#endif

#include "connection_manager.hpp"
#include "postgis.hpp"

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

#ifndef MAPNIK_BIG_ENDIAN
#define WKB_ENCODING "NDR"
#else
#define WKB_ENCODING "XDR"
#endif

DATASOURCE_PLUGIN(postgis_datasource)

const std::string postgis_datasource::GEOMETRY_COLUMNS="geometry_columns";
const std::string postgis_datasource::SPATIAL_REF_SYS="spatial_ref_system";

using std::clog;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::shared_ptr;

using mapnik::PoolGuard;
using mapnik::attribute_descriptor;

postgis_datasource::postgis_datasource(parameters const& params)
   : datasource (params),
     table_(*params.get<std::string>("table","")),
     geometry_field_(*params.get<std::string>("geometry_field","")),
     cursor_fetch_size_(*params_.get<int>("cursor_size",0)),
     row_limit_(*params_.get<int>("row_limit",0)),
     type_(datasource::Vector),
     srid_(*params_.get<int>("srid",0)),
     extent_initialized_(false),
     desc_(*params.get<std::string>("type"),"utf-8"),
     creator_(params.get<std::string>("host"),
              params.get<std::string>("port"),
              params.get<std::string>("dbname"),
              params.get<std::string>("user"),
              params.get<std::string>("password")),
     bbox_token_("!bbox!"),
     persist_connection_(*params_.get<mapnik::boolean>("persist_connection",true))
{   

   if (table_.empty()) throw mapnik::datasource_exception("PostGIS: missing <table> parameter");

#ifdef MAPNIK_DEBUG
   if (persist_connection_)
   {
       clog << "PostGIS: persisting connection pool..." << endl;
   }
   else
   {
       clog << "PostGIS: not persisting connection..." << endl;   
   }
#endif

   boost::optional<int> initial_size = params_.get<int>("inital_size",1);
   boost::optional<int> max_size = params_.get<int>("max_size",10);

   multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);
   
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
            d[i] = boost::lexical_cast<double>(boost::trim_copy(*beg));
         }
         catch (boost::bad_lexical_cast & ex)
         {
            clog << *beg << " : " << ex.what() << "\nAre your coordinates each separated by commas?\n";
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

   ConnectionManager *mgr=ConnectionManager::instance();   
   mgr->registerPool(creator_, *initial_size, *max_size);
    
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {      
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {
         PoolGuard<shared_ptr<Connection>,
            shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
         
         desc_.set_encoding(conn->client_encoding());
         
         std::string table_name=table_from_sql(table_);
         std::string schema_name="";
         std::string::size_type idx=table_name.find_last_of('.');
         if (idx!=std::string::npos)
         {
            schema_name=table_name.substr(0,idx);
            table_name=table_name.substr(idx+1);
         }
         else
         {
            table_name=table_name.substr(0);
         }

         geometryColumn_ = geometry_field_;
         if (!geometryColumn_.length() > 0 || srid_ == 0)
         {
             std::ostringstream s;
             s << "SELECT f_geometry_column, srid FROM ";
             s << GEOMETRY_COLUMNS <<" WHERE f_table_name='" << table_name<<"'";
             
             if (schema_name.length() > 0) 
                s << " AND f_table_schema='" << schema_name << "'";
            
             if (geometry_field_.length() > 0)
                s << " AND f_geometry_column='" << geometry_field_ << "'";

#ifdef MAPNIK_DEBUG
             clog << s.str() << endl;
#endif
             shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
             if (rs->next())
             {
                geometryColumn_ = rs->getValue("f_geometry_column");
#ifdef MAPNIK_DEBUG
                clog << "setting geometry field to=" << geometryColumn_ << "\n";
#endif
                if (srid_ == 0)
                {
                    try 
                    {
                        srid_ = lexical_cast<int>(rs->getValue("srid"));
#ifdef MAPNIK_DEBUG
                        clog << "setting SRID to=" << srid_ << "\n";
#endif
                    }
                    catch (bad_lexical_cast &ex)
                    {
                       clog << "SRID: " << rs->getValue("srid") << ":" << ex.what() << endl;
                    }
                }
             }
             rs->close();
             
             if (geometryColumn_.length() == 0)
                throw mapnik::datasource_exception( "PostGIS Driver Error: Geometry column not specified or found in " + GEOMETRY_COLUMNS + " table: '" + table_name + "'. Try setting the 'geometry_field' parameter or adding a proper " + GEOMETRY_COLUMNS + " record");

             if (srid_ <= 0)
             {
                s.str("");
                s << "SELECT SRID(\"" << geometryColumn_ << "\") AS srid FROM ";
                if (schema_name.length() > 0)
                    s << schema_name << ".";
                s << table_name << " WHERE \"" << geometryColumn_ << "\" IS NOT NULL LIMIT 1;";

#ifdef MAPNIK_DEBUG
                clog << s.str() << endl;
#endif
                shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
                if (rs->next())
                {
                    try
                    {
                        srid_ = lexical_cast<int>(rs->getValue("srid"));
#ifdef MAPNIK_DEBUG
                        clog << "setting SRID to=" << srid_ << endl;
#endif
                    }
                    catch (bad_lexical_cast &ex)
                    {
                        clog << "SRID: " << rs->getValue("srid") << ":" << ex.what() << endl;
                    }
                }
                rs->close();
            }
         }
         
         if (srid_ == 0)
         {
            srid_ = -1;
            clog << "SRID: warning, using srid=-1" << endl;
         }
         
#ifdef MAPNIK_DEBUG
         clog << "using srid=" << srid_ << endl;
         clog << "using geometry_column=" << geometryColumn_ << endl;
#endif

         // collect attribute desc         
         std::ostringstream s;
         std::string table_with_bbox = populate_sql_bbox(table_,extent_);
         s << "select * from " << table_with_bbox << " limit 0";

         shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
         int count = rs->getNumFields();
         for (int i=0;i<count;++i)
         {
            std::string fld_name=rs->getFieldName(i);
            int type_oid = rs->getTypeOID(i);
            switch (type_oid)
            {
               case 21:    // int2
               case 23:    // int4
                  desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
                  break;
               case 700:   // float4 
               case 701:   // float8
               case 1700:  // numeric ??
                  desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
               case 1042:  // bpchar
               case 1043:  // varchar
               case 25:    // text
                  desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
                  break;
               default: // should not get here
#ifdef MAPNIK_DEBUG
                  std::ostringstream s_oid;
                  s_oid << "select oid, typname from pg_type where oid = " << type_oid;
                  shared_ptr<ResultSet> rs_oid=conn->executeQuery(s_oid.str());
                  if (rs_oid->next())
                  {
                      clog << "PostGIS: unknown type = " << rs_oid->getValue("typname") << " (oid:" << rs_oid->getValue("oid") << ")\n";
                  }
                  else
                  {
                      clog << "PostGIS: unknown oid type =" << type_oid << endl;              
                  }
                  rs_oid->close();
#endif
                  break;
            }
         }
         rs->close();
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

std::string postgis_datasource::populate_sql_bbox(const std::string& sql, Envelope<double> const& box) const
{
    std::string sql_with_bbox = sql;
    std::ostringstream b;
    if (srid_ > 0)
        b << "SetSRID(";
    b << "'BOX3D(";
    b << std::setprecision(16);
    b << box.minx() << " " << box.miny() << ",";
    b << box.maxx() << " " << box.maxy() << ")'::box3d";
    if (srid_ > 0)
        b << ", " << srid_ << ")";
    if ( boost::algorithm::icontains(sql,bbox_token_) )
    {
        boost::algorithm::replace_all(sql_with_bbox,bbox_token_,b.str());
        return sql_with_bbox;
    }
    else
    {
        std::ostringstream s;
        s << " WHERE \"" << geometryColumn_ << "\" && " << b.str();
        return sql_with_bbox + s.str();    
    }
}

std::string postgis_datasource::table_from_sql(const std::string& sql)
{
   std::string table_name = boost::algorithm::to_lower_copy(sql);
   boost::algorithm::replace_all(table_name,"\n"," ");
   
   std::string::size_type idx = table_name.rfind(" from ");
   if (idx!=std::string::npos)
   {
      
      idx=table_name.find_first_not_of(" ",idx+5);
      if (idx != std::string::npos)
      {
         table_name=table_name.substr(idx);
      }
      idx=table_name.find_first_of(" )");
      if (idx != std::string::npos)
      {
         table_name = table_name.substr(0,idx);
      }
   }
   return table_name;
}

boost::shared_ptr<IResultSet> postgis_datasource::get_resultset(boost::shared_ptr<Connection> const &conn, const std::string &sql) const
{
     if (cursor_fetch_size_ > 0) {
         // cursor
         std::ostringstream csql;
         std::string cursor_name = conn->new_cursor_name();

         csql << "DECLARE " << cursor_name << " BINARY INSENSITIVE NO SCROLL CURSOR WITH HOLD FOR " << sql << " FOR READ ONLY";

#ifdef MAPNIK_DEBUG
         clog << csql.str() << "\n";
#endif
         if (!conn->execute(csql.str())) {
            throw mapnik::datasource_exception( "PSQL Error: Creating cursor for data select." );
         }
         return shared_ptr<CursorResultSet>(new CursorResultSet(conn, cursor_name, cursor_fetch_size_));

     } else {
         // no cursor
#ifdef MAPNIK_DEBUG
         clog << sql << "\n";
#endif
         return conn->executeQuery(sql,1);
     }
}

featureset_ptr postgis_datasource::features(const query& q) const
{
#ifdef MAPNIK_DEBUG
    mapnik::wall_clock_progress_timer timer(clog, "end feature query: ");
#endif

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
         s << "SELECT AsBinary(\""<<geometryColumn_<<"\",'"<< WKB_ENCODING << "') AS geom";
         std::set<std::string> const& props=q.property_names();
         std::set<std::string>::const_iterator pos=props.begin();
         std::set<std::string>::const_iterator end=props.end();
         while (pos != end)
         {
            s <<",\""<<*pos<<"\"";
            ++pos;
         }	 

         std::string table_with_bbox = populate_sql_bbox(table_,box);

         s << " from " << table_with_bbox;

         if (row_limit_ > 0) {
             s << " LIMIT " << row_limit_;
         }
         
         boost::shared_ptr<IResultSet> rs = get_resultset(conn, s.str());
         return featureset_ptr(new postgis_featureset(rs,desc_.get_encoding(),multiple_geometries_,props.size()));
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
           
         s << "SELECT AsBinary(\"" << geometryColumn_ << "\",'"<< WKB_ENCODING << "') AS geom";
            
         std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
         std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
         unsigned size=0;
         while (itr != end)
         {
            s <<",\""<< itr->get_name() << "\"";
            ++itr;
            ++size;
         }

         Envelope<double> box(pt.x,pt.y,pt.x,pt.y);
         std::string table_with_bbox = populate_sql_bbox(table_,box);

         s << " from " << table_with_bbox;
         
         if (row_limit_ > 0) {
             s << " LIMIT " << row_limit_;
         }
         
         boost::shared_ptr<IResultSet> rs = get_resultset(conn, s.str());
         return featureset_ptr(new postgis_featureset(rs,desc_.get_encoding(),multiple_geometries_, size));
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
         PoolGuard<shared_ptr<Connection>,shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
         std::ostringstream s;
         std::string table_name = table_from_sql(table_);
         boost::optional<std::string> estimate_extent = params_.get<std::string>("estimate_extent");
         
         if (estimate_extent && *estimate_extent == "true")
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

postgis_datasource::~postgis_datasource() 
{
    if (!persist_connection_)
    {
        ConnectionManager *mgr=ConnectionManager::instance();
        shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
        if (pool)
        {
            shared_ptr<Connection> conn = pool->borrowObject();
            if (conn)
            {
                conn->close();
            }
        }
    }
}
