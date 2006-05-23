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

#include "postgis.hpp"
#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include "connection_manager.hpp"

DATASOURCE_PLUGIN(postgis_datasource)

const std::string postgis_datasource::GEOMETRY_COLUMNS="geometry_columns";
const std::string postgis_datasource::SPATIAL_REF_SYS="spatial_ref_system";

using std::clog;
using std::clog;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::shared_ptr;

postgis_datasource::postgis_datasource(const parameters& params)
    : datasource (params),
      table_(params.get("table")),
      type_(datasource::Vector), 
      desc_(params.get("name")),
      creator_(params.get("host"),
	       params.get("dbname"),
	       params.get("user"),
	       params.get("password"))
      
{     
    ConnectionManager *mgr=ConnectionManager::instance();   
    mgr->registerPool(creator_,10,20);

    shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
    if (pool)
    {
	const shared_ptr<Connection>& conn = pool->borrowObject();
	if (conn && conn->isOK())
	{
	    PoolGuard<shared_ptr<Connection>,shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);

	    std::string table_name=table_from_sql(table_);
	    
	    std::ostringstream s;
	    s << "select f_geometry_column,srid,type from ";
	    s << GEOMETRY_COLUMNS <<" where f_table_name='"<<table_name<<"'";
	   
	    shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
	    
	    if (rs->next())
	    {
		try 
		{
		    srid_ = lexical_cast<int>(rs->getValue("srid"));
		    desc_.set_srid(srid_);
		}
		catch (bad_lexical_cast &ex)
		{
		    clog << ex.what() << endl;
		}
		geometryColumn_=rs->getValue("f_geometry_column");
		std::string postgisType=rs->getValue("type");
	    }
	    rs->close();
	    s.str("");
	    s << "select xmin(ext),ymin(ext),xmax(ext),ymax(ext)";
	    s << " from (select estimated_extent('"<<table_name<<"','"<<geometryColumn_<<"') as ext) as tmp";

	    rs=conn->executeQuery(s.str());
	    if (rs->next())
	    {
		try 
		{
		    double lox=lexical_cast<double>(rs->getValue(0));
		    double loy=lexical_cast<double>(rs->getValue(1));
		    double hix=lexical_cast<double>(rs->getValue(2));
		    double hiy=lexical_cast<double>(rs->getValue(3));		    
		    extent_.init(lox,loy,hix,hiy);
		}
		catch (bad_lexical_cast &ex)
		{
		    clog << ex.what() << endl;
		}
	    }
	    rs->close();

	    // collect attribute desc
	    s.str("");
	    s << "select * from "<<table_<<" limit 1";
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
		    case 17285: // geometry
			desc_.add_descriptor(attribute_descriptor(fld_name,Geometry));
			break;
		    case 21:    // int2
		    case 23:    // int4
			desc_.add_descriptor(attribute_descriptor(fld_name,Integer,false,length));
 			break;
                    case 701:  // float8
 			desc_.add_descriptor(attribute_descriptor(fld_name,Double,false,length));
		    case 1042:  // bpchar
		    case 1043:  // varchar
			desc_.add_descriptor(attribute_descriptor(fld_name,String));
			break;
		    default: // shouldn't get here
			clog << "unknown type_oid="<<type_oid<<endl;
			desc_.add_descriptor(attribute_descriptor(fld_name,String));
			break;
		    }	  
		}
	    }
	}
    }
}

std::string postgis_datasource::name_="postgis";

std::string postgis_datasource::name()
{
    return name_;
}

int postgis_datasource::type() const
{
    return type_;
}

layer_descriptor const& postgis_datasource::get_descriptor() const
{
    return desc_;
}

std::string postgis_datasource::table_from_sql(const std::string& sql)
{
    std::string table_name(sql);
    transform(table_name.begin(),table_name.end(),table_name.begin(),tolower);
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
    Featureset *fs=0;
    Envelope<double> const& box=q.get_bbox();
    ConnectionManager *mgr=ConnectionManager::instance();
    shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
    if (pool)
    {
	const shared_ptr<Connection>& conn = pool->borrowObject();
	if (conn && conn->isOK())
	{       
	    PoolGuard<shared_ptr<Connection>,shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
	    std::ostringstream s;
	    // can we rely on 'gid' name???
	    s << "select ogc_fid,asbinary("<<geometryColumn_<<") as geom";
	    std::set<std::string> const& props=q.property_names();
	    std::set<std::string>::const_iterator pos=props.begin();
	    while (pos!=props.end())
	    {
		s <<",\""<<*pos<<"\"";
		++pos;
	    }	
    
	    s << " from " << table_<<" where "<<geometryColumn_<<" && setSRID('BOX3D(";
	    s << box.minx() << " " << box.miny() << ",";
	    s << box.maxx() << " " << box.maxy() << ")'::box3d,"<<srid_<<")";
	    clog << s.str() << endl;
	    shared_ptr<ResultSet> rs=conn->executeQuery(s.str(),1);
	    fs=new postgis_featureset(rs,props.size());
	}
    }
    return featureset_ptr(fs);
}

const Envelope<double>& postgis_datasource::envelope() const
{
    return extent_;
}

postgis_datasource::~postgis_datasource() {}
