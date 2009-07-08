/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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
//$Id$

#include "sqlite.hpp"

#include <mapnik/datasource.hpp>
#include <mapnik/wkb.hpp>

#include "connection_manager.hpp"
#include "cursorresultset.hpp"
// boost
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

//stl
#include <iostream>
#include <fstream>

namespace mapnik {

   std::string numeric2string(const char* buf)
   {
      int16_t ndigits = int2net(buf);
      int16_t weight  = int2net(buf+2);
      int16_t sign    = int2net(buf+4);
      int16_t dscale  = int2net(buf+6);
      
      boost::scoped_array<int16_t> digits(new int16_t[ndigits]); 
      for (int n=0; n < ndigits ;++n)
      {
         digits[n] = int2net(buf+8+n*2);
      }
      
      std::ostringstream ss;
      
      if (sign == 0x4000) ss << "-";
      
      int i = std::max(weight,int16_t(0));
      int d = 0;
      while ( i >= 0)
      {
         if (i <= weight && d < ndigits)
            ss <<  digits[d++];
         else
            ss <<  '0';
         i--;
      }
      if (dscale > 0)
      {
         ss << '.';
         while ( i >= -dscale)
         {
            if (i <= weight && d < ndigits)
               ss <<  digits[d++];
            i--;
         }
      }
      return ss.str();
   }
   
   struct blob_to_hex
   {
      std::string operator() (const char* blob, unsigned size)
      {
         std::string buf;
         buf.reserve(size*2);
         std::ostringstream s(buf);
         s.seekp(0);
         char hex[3];
         std::memset(hex,0,3);
         for ( unsigned pos=0; pos < size; ++pos)
         {  
            std::sprintf (hex, "%02X", int(blob[pos]) & 0xff);
            s << hex;
         }
         return s.str();
      }
   };

   bool valid_envelope(mapnik::Envelope<double> const& e)
   {
      return (e.minx() <= e.maxx() && e.miny() <= e.maxy()) ;
   }
   
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
   
   
   template <typename Connection>
   void pgsql2sqlite(Connection conn, 
                     std::string const& query, 
                     std::string const& output_table_name, 
                     std::string const& output_filename)
   {   
      namespace sqlite = mapnik::sqlite;
      sqlite::database db(output_filename);
      
      boost::shared_ptr<ResultSet> rs = conn->executeQuery("select * from (" + query + ") as query limit 0;");
      int count = rs->getNumFields();
      
      std::ostringstream select_sql;
      
      select_sql << "select ";
      
      for (int i=0; i<count; ++i)
      {
         if (i!=0) select_sql << ",";
         select_sql << "\"" <<  rs->getFieldName(i) << "\"";
      }
   
      select_sql << " from (" << query << ") as query";
   
      std::string table_name = table_from_sql(query);
      
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
      
      std::ostringstream geom_col_sql;
      geom_col_sql << "select f_geometry_column,srid,type from geometry_columns ";
      geom_col_sql << "where f_table_name='" << table_name << "'";
      if (schema_name.length() > 0)
      {
         geom_col_sql <<" and f_table_schema='"<< schema_name <<"'";
      }
      
      rs = conn->executeQuery(geom_col_sql.str());
   
      int srid = -1;
      std::string geom_col = "UNKNOWN";
      std::string geom_type = "UNKNOWN";
      
      if ( rs->next())
      {
         try 
         {
            srid = boost::lexical_cast<int>(rs->getValue("srid"));
         }
         catch (boost::bad_lexical_cast &ex)
         {
            std::clog << ex.what() << std::endl;
         }
         geom_col = rs->getValue("f_geometry_column");
         geom_type = rs->getValue("type");
      }
   
      // add AsBinary(<geometry_column>) modifier
      std::string select_sql_str = select_sql.str();
      boost::algorithm::replace_all(select_sql_str, "\"" + geom_col + "\"","AsBinary(" + geom_col+") as " + geom_col);

#ifdef MAPNIK_DEBUG
      std::cout << select_sql_str << "\n";
#endif
      
      std::ostringstream cursor_sql;
      std::string cursor_name("my_cursor");
   
      cursor_sql << "DECLARE " << cursor_name << " BINARY INSENSITIVE NO SCROLL CURSOR WITH HOLD FOR " << select_sql_str << " FOR READ ONLY";
      conn->execute(cursor_sql.str());
   
      boost::shared_ptr<CursorResultSet> cursor(new CursorResultSet(conn,cursor_name,10000));
   
      unsigned num_fields = cursor->getNumFields();

      if (num_fields == 0) return;
      
      std::string feature_id =  "fid";
   
      std::ostringstream create_sql;
      create_sql << "create table if not exists " << output_table_name << " (" << feature_id << " INTEGER PRIMARY KEY AUTOINCREMENT,";
      
      int geometry_oid = -1;

      std::string output_table_insert_sql = "insert into " + output_table_name + " values (?";
      
      for ( unsigned pos = 0; pos < num_fields ; ++pos)
      {
	if (pos > 0) 
	{
	  create_sql << ",";
	}
	output_table_insert_sql +=",?";
	int oid = cursor->getTypeOID(pos);
	if (geom_col == cursor->getFieldName(pos))
	{
           geometry_oid = oid;
           create_sql << "'" << cursor->getFieldName(pos) << "' BLOB";
	}
        else
        {
           create_sql << "'" << cursor->getFieldName(pos);
           switch (oid)
           {
              case 700:
              case 701:
                 create_sql << "' REAL";
                 break;
              default:
                 create_sql << "' TEXT";
                 break;
           }  
            
        }
      }
      
      create_sql << ");";
      output_table_insert_sql +=")";
      
      std::cout << "client_encoding=" << conn->client_encoding() << "\n";
      std::cout << "geometry_column=" << geom_col << "(" << geom_type 
                <<  ") srid=" << srid << " oid=" << geometry_oid << "\n";
   
  
      db.execute("begin;");
       // output table sql
      db.execute(create_sql.str());

      // spatial index sql
      std::string spatial_index_sql = "create virtual table idx_" + output_table_name 
	+ "_" + geom_col + " using rtree(pkid, xmin, xmax, ymin, ymax)";
      
      db.execute(spatial_index_sql);
      
      //blob_to_hex hex;
      int pkid = 0;
      
      std::string spatial_index_insert_sql = "insert into idx_" + output_table_name +  "_"  
         +  geom_col + " values (?,?,?,?,?)" ;
      
      sqlite::prepared_statement spatial_index(db,spatial_index_insert_sql);

#ifdef MAPNIK_DEBUG
      std::cout << output_table_insert_sql << "\n";
#endif
      
      sqlite::prepared_statement output_table(db,output_table_insert_sql);
      
      while (cursor->next())
      {
         ++pkid;
         
	 sqlite::record_type output_rec;
         output_rec.push_back(sqlite::value_type(pkid));
         bool empty_geom = true;
         const char * buf = 0;
         for (unsigned pos=0 ; pos < num_fields; ++pos)
         {
            if (! cursor->isNull(pos))
            {
               int size=cursor->getFieldLength(pos);
               int oid = cursor->getTypeOID(pos);
               buf=cursor->getValue(pos);
            
               switch (oid)
               {
                  case 25:
                  case 1042:
                  case 1043:
                  {
                     std::string text(buf);
                     boost::algorithm::replace_all(text,"'","''");
		     output_rec.push_back(sqlite::value_type(text));
                     break;
                  }
                  case 23:
                     output_rec.push_back(sqlite::value_type(int4net(buf)));
                     break;
                  case 21:
                     output_rec.push_back(sqlite::value_type(int2net(buf)));
                     break;
                  case 700:
                  {
                     float val;
                     float4net(val,buf);
                     output_rec.push_back(sqlite::value_type(val));
                     break;
                  }
                  case 701:
                  {
                     double val;
                     float8net(val,buf);
                     output_rec.push_back(sqlite::value_type(val));
                     break;
                  }
                  case 1700:
                  {
                     std::string str = numeric2string(buf);
                     try 
                     {
                        double val = boost::lexical_cast<double>(str);
                        output_rec.push_back(sqlite::value_type(val));
                     }
                     catch (boost::bad_lexical_cast & ex)
                     {
                        std::clog << ex.what() << "\n"; 
                     }
                     break;
                  }
                  
                  default:  
                  {
                     if (oid == geometry_oid)
                     {
                        mapnik::Feature feat(pkid);
                        geometry_utils::from_wkb(feat,buf,size,false,wkbGeneric);
                        if (feat.num_geometries() > 0)
                        {
                           geometry2d const& geom=feat.get_geometry(0);
                           Envelope<double> bbox = geom.envelope();
                           if (valid_envelope(bbox))
                           {
			     sqlite::record_type rec;
			     
			     rec.push_back(sqlite::value_type(pkid));
			     rec.push_back(sqlite::value_type(bbox.minx()));
			     rec.push_back(sqlite::value_type(bbox.maxx()));
			     rec.push_back(sqlite::value_type(bbox.miny()));
			     rec.push_back(sqlite::value_type(bbox.maxy()));
			     
			     spatial_index.insert_record(rec);
			     empty_geom = false;
                           }
                        }
                        
			//output_rec.push_back(sqlite::value_type("X'" + hex(buf,size) + "'"));
                        output_rec.push_back(sqlite::blob(buf,size));
                     }
                     else 
		     {
		       output_rec.push_back(sqlite::null_type());
                     }
                     break;
                  }
               }    
            }
            else 
            {
	      output_rec.push_back(sqlite::null_type());
            } 
         }
   
	 if (!empty_geom) output_table.insert_record(output_rec);
	 
         if (pkid % 1000 == 0)
         {
            std::cout << "\r processing " << pkid << " features";
            std::cout.flush();
         }
         
	 if (pkid % 100000 == 0)
         {
	   db.execute("commit;begin;");	   
         }
      }
      // commit
      db.execute("commit;");
      std::cout << "\r processed " << pkid << " features";
      std::cout << "\n Done!" << std::endl;
   }
}
