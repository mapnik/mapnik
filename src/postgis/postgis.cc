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

#include "postgis.hh"
#include <netinet/in.h>
#include <string>
#include <algorithm>

#include <sstream>
#include "connection_manager.hh"

DATASOURCE_PLUGIN(PostgisDatasource);

const std::string PostgisDatasource::GEOMETRY_COLUMNS="geometry_columns";
const std::string PostgisDatasource::SPATIAL_REF_SYS="spatial_ref_system";

PostgisDatasource::PostgisDatasource(const Parameters& params)
    : table_(params.get("table")),
      creator_(params.get("host"),
	       params.get("dbname"),
	       params.get("user"),
	       params.get("pass")),
      type_(0)
    
{
 
    ConnectionManager *mgr=ConnectionManager::instance();
   
    mgr->registerPool(creator_,10,20);
    std::cout<<" pool id ="<<creator_.id()<<std::endl;
    ref_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
    if (pool)
    {
	const ref_ptr<Connection>& conn = pool->borrowObject();
	if (conn && conn->isOK())
	{
	    PoolGuard<ref_ptr<Connection>,ref_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);

	    std::string table_name=table_from_sql(table_);
	    std::cout<<"TABLE NAME="<<table_name<<std::endl;
	    
	    std::ostringstream s;
	    s << "select f_geometry_column,srid,type from ";
	    s << GEOMETRY_COLUMNS <<" where f_table_name='"<<table_name<<"'";
	   
	    ref_ptr<ResultSet> rs=conn->executeQuery(s.str());
	    
	    if (rs->next())
	    {
		srid_=atoi(rs->getValue("srid"));
		geometryColumn_=rs->getValue("f_geometry_column");
		std::string postgisType=rs->getValue("type");
		
		if (postgisType=="POINT" || postgisType=="POINTM" || postgisType=="MULTIPOINT" )
		    type_=datasource::Point;
		else if (postgisType=="LINESTRING" || postgisType=="MULTILINESTRING")
		    type_=datasource::Line;
		else if (postgisType=="POLYGON" || postgisType=="MULTIPOLYGON")
		    type_=datasource::Polygon;

	    }
	    rs->close();
	    s.str("");
	    s << "select xmin(ext),ymin(ext),xmax(ext),ymax(ext)";
	    s << " from (select estimated_extent('"<<table_name<<"','"<<geometryColumn_<<"') as ext) as tmp";
	    std::cout<<s.str()<<"\n";
	    rs=conn->executeQuery(s.str());
	    if (rs->next())
	    {
		double lox,loy,hix,hiy;
		fromString(rs->getValue(0),lox);
		fromString(rs->getValue(1),loy);
		fromString(rs->getValue(2),hix);
		fromString(rs->getValue(3),hiy);
		extent_.init(lox,loy,hix,hiy);
		std::cout<<extent_<<"\n";
	    }
	    rs->close();
	}
    }
}

std::string PostgisDatasource::name()
{
    return "postgis";
}


int PostgisDatasource::type() const
{
    return type_;
}


std::string PostgisDatasource::table_from_sql(const std::string& sql)
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


FeaturesetPtr PostgisDatasource::featuresAll(const CoordTransform& t) const
{
    return FeaturesetPtr(0);
}


FeaturesetPtr PostgisDatasource::featuresInBox(const CoordTransform& t,
					       const mapnik::Envelope<double>& box) const
{
    Featureset *fs=0;
    ConnectionManager *mgr=ConnectionManager::instance();
    ref_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
    if (pool)
    {
	const ref_ptr<Connection>& conn = pool->borrowObject();
	if (conn && conn->isOK())
	{
	    PoolGuard<ref_ptr<Connection>,ref_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
	    std::ostringstream s;
	    s << "select gid,asbinary("<<geometryColumn_<<") as geom from ";
	    s << table_<<" where "<<geometryColumn_<<"&& setSRID('BOX3D(";
	    s << box.minx() << " " << box.miny() << ",";
	    s << box.maxx() << " " << box.maxy() << ")'::box3d,"<<srid_<<")";
	    std::cout << s.str()<<std::endl;
	    ref_ptr<ResultSet> rs=conn->executeQuery(s.str(),1);
	    fs=new PostgisFeatureset(rs,t);
	}
    }
    return FeaturesetPtr(fs);
}


FeaturesetPtr PostgisDatasource::featuresAtPoint(const CoordTransform& t,
						 const mapnik::coord2d& pt) const
{
    Featureset *fs=0;
    ConnectionManager *mgr=ConnectionManager::instance();
    ref_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
    if (pool)
    {
	const ref_ptr<Connection>& conn = pool->borrowObject();
	if (conn && conn->isOK())
	{
	    PoolGuard<ref_ptr<Connection>,ref_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
	
	    std::ostringstream s;	    
	    s << "select gid,asbinary("<<geometryColumn_<<") as geom from ";
	    s << table_<<" where setSRID('BOX3D(";
	    s << pt.x << " " << pt.y << ",";
	    s << pt.x << " " << pt.y << ")'::box3d,"<<srid_<<") && "<<geometryColumn_;
	    std::cout << s.str()<<std::endl;
	    ref_ptr<ResultSet> rs=conn->executeQuery(s.str(),1);
	    fs=new PostgisFeatureset(rs,t);
	}
    }
    return FeaturesetPtr(fs);
}


const Envelope<double>& PostgisDatasource::envelope() const
{
    return extent_;
}


PostgisDatasource::~PostgisDatasource()
{
}
