/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#include "occi_datasource.hpp"
#include "occi_featureset.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/timer.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/make_shared.hpp>

// stl
#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include <iomanip>

using boost::lexical_cast;
using boost::bad_lexical_cast;

using mapnik::datasource;
using mapnik::parameters;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;
using mapnik::box2d;
using mapnik::coord2d;

using oracle::occi::Environment;
using oracle::occi::Connection;
using oracle::occi::Statement;
using oracle::occi::ResultSet;
using oracle::occi::MetaData;
using oracle::occi::SQLException;
using oracle::occi::Type;
using oracle::occi::StatelessConnectionPool;

const std::string occi_datasource::METADATA_TABLE = "USER_SDO_GEOM_METADATA";

DATASOURCE_PLUGIN(occi_datasource)

occi_datasource::occi_datasource(parameters const& params, bool bind)
    : datasource (params),
      type_(datasource::Vector),
      fields_(*params_.get<std::string>("fields", "*")),
      geometry_field_(*params_.get<std::string>("geometry_field", "")),
      srid_initialized_(false),
      extent_initialized_(false),
      desc_(*params_.get<std::string>("type"), *params_.get<std::string>("encoding", "utf-8")),
      use_wkb_(*params_.get<mapnik::boolean>("use_wkb", false)),
      row_limit_(*params_.get<int>("row_limit", 0)),
      row_prefetch_(*params_.get<int>("row_prefetch", 100)),
      pool_(0),
      conn_(0)
{
    if (! params_.get<std::string>("user")) throw datasource_exception("OCCI Plugin: no <user> specified");
    if (! params_.get<std::string>("password")) throw datasource_exception("OCCI Plugin: no <password> specified");
    if (! params_.get<std::string>("host")) throw datasource_exception("OCCI Plugin: no <host> string specified");

    boost::optional<std::string> table = params_.get<std::string>("table");
    if (! table)
    {
        throw datasource_exception("OCCI Plugin: no <table> parameter specified");
    }
    else
    {
        table_ = *table;
    }

    use_spatial_index_ = *params_.get<mapnik::boolean>("use_spatial_index",true);
    use_connection_pool_ = *params_.get<mapnik::boolean>("use_connection_pool",true);

    boost::optional<std::string> ext = params_.get<std::string>("extent");
    if (ext) extent_initialized_ = extent_.from_string(*ext);

    boost::optional<int> srid = params_.get<int>("srid");
    if (srid)
    {
        srid_ = *srid;
        srid_initialized_ = true;
    }

    if (bind)
    {
        this->bind();
    }
}

occi_datasource::~occi_datasource()
{
    if (is_bound_)
    {
        Environment* env = occi_environment::get_environment();

        if (use_connection_pool_)
        {
            if (pool_ != 0)
            {
                env->terminateStatelessConnectionPool(pool_, StatelessConnectionPool::SPD_FORCE);
            }
        }
        else
        {
            if (conn_ != 0)
            {
                env->terminateConnection(conn_);
            }
        }
    }
}

void occi_datasource::bind() const
{
    if (is_bound_) return;

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "occi_datasource::bind");
#endif

    // connect to environment
    if (use_connection_pool_)
    {
        try
        {
            Environment* env = occi_environment::get_environment();

            pool_ = env->createStatelessConnectionPool(
                *params_.get<std::string>("user"),
                *params_.get<std::string>("password"),
                *params_.get<std::string>("host"),
                *params_.get<int>("max_size", 5),
                *params_.get<int>("initial_size", 1),
                1,
                StatelessConnectionPool::HOMOGENEOUS);
        }
        catch (SQLException& ex)
        {
            throw datasource_exception("OCCI Plugin: " + ex.getMessage());
        }
    }
    else
    {
        try
        {
            Environment* env = occi_environment::get_environment();

            conn_ = env->createConnection(
                *params_.get<std::string>("user"),
                *params_.get<std::string>("password"),
                *params_.get<std::string>("host"));
        }
        catch (SQLException& ex)
        {
            throw datasource_exception("OCCI Plugin: " + ex.getMessage());
        }
    }

    // extract real table name
    table_name_ = mapnik::sql_utils::table_from_sql(table_);

    // get SRID and/or GEOMETRY_FIELD from metadata table only if we need to
    if (! srid_initialized_ || geometry_field_ == "")
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, "occi_datasource::get_srid_and_geometry_field");
#endif

        std::ostringstream s;
        s << "SELECT srid, column_name FROM " << METADATA_TABLE << " WHERE";
        s << " LOWER(table_name) = LOWER('" << table_name_ << "')";

        if (geometry_field_ != "")
        {
            s << " AND LOWER(column_name) = LOWER('" << geometry_field_ << "')";
        }

        MAPNIK_LOG_DEBUG(occi) << "occi_datasource: " << s.str();

        try
        {
            occi_connection_ptr conn;
            if (use_connection_pool_) conn.set_pool(pool_);
            else                      conn.set_connection(conn_, false);

            ResultSet* rs = conn.execute_query(s.str());
            if (rs && rs->next ())
            {
                if (! srid_initialized_)
                {
                    srid_ = rs->getInt(1);
                    srid_initialized_ = true;
                }

                if (geometry_field_ == "")
                {
                    geometry_field_ = rs->getString(2);
                }
            }
        }
        catch (SQLException& ex)
        {
            throw datasource_exception("OCCI Plugin: " + ex.getMessage());
        }
    }

    // get columns description
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, "occi_datasource::get_column_description");
#endif

        std::ostringstream s;
        s << "SELECT " << fields_ << " FROM (" << table_name_ << ") WHERE rownum < 1";

        MAPNIK_LOG_DEBUG(occi) << "occi_datasource: " << s.str();

        try
        {
            occi_connection_ptr conn;
            if (use_connection_pool_) conn.set_pool(pool_);
            else                      conn.set_connection(conn_, false);

            ResultSet* rs = conn.execute_query(s.str());
            if (rs)
            {
                std::vector<MetaData> listOfColumns = rs->getColumnListMetaData();

                for (unsigned int i = 0; i < listOfColumns.size(); ++i)
                {
                    MetaData columnObj = listOfColumns[i];

                    std::string fld_name = columnObj.getString(MetaData::ATTR_NAME);
                    int type_oid = columnObj.getInt(MetaData::ATTR_DATA_TYPE);

                    /*
                      int type_code = columnObj.getInt(MetaData::ATTR_TYPECODE);
                      if (type_code == OCCI_TYPECODE_OBJECT)
                      {
                      desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Object));
                      continue;
                      }
                    */

                    switch (type_oid)
                    {
                    case oracle::occi::OCCIBOOL:
                    case oracle::occi::OCCIINT:
                    case oracle::occi::OCCIUNSIGNED_INT:
                    case oracle::occi::OCCIROWID:
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
                    case oracle::occi::OCCI_SQLT_RDD:
                    case oracle::occi::OCCI_SQLT_STR:
                    case oracle::occi::OCCI_SQLT_VCS:
                    case oracle::occi::OCCI_SQLT_VNU:
                    case oracle::occi::OCCI_SQLT_VBI:
                    case oracle::occi::OCCI_SQLT_VST:
                        desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
                        break;
                    case oracle::occi::OCCIDATE:
                    case oracle::occi::OCCITIMESTAMP:
                    case oracle::occi::OCCIINTERVALDS:
                    case oracle::occi::OCCIINTERVALYM:
                    case oracle::occi::OCCI_SQLT_DAT:
                    case oracle::occi::OCCI_SQLT_DATE:
                    case oracle::occi::OCCI_SQLT_TIME:
                    case oracle::occi::OCCI_SQLT_TIME_TZ:
                    case oracle::occi::OCCI_SQLT_TIMESTAMP:
                    case oracle::occi::OCCI_SQLT_TIMESTAMP_LTZ:
                    case oracle::occi::OCCI_SQLT_TIMESTAMP_TZ:
                    case oracle::occi::OCCI_SQLT_INTERVAL_YM:
                    case oracle::occi::OCCI_SQLT_INTERVAL_DS:
                    case oracle::occi::OCCIANYDATA:
                    case oracle::occi::OCCIBLOB:
                    case oracle::occi::OCCIBFILE:
                    case oracle::occi::OCCIBYTES:
                    case oracle::occi::OCCICLOB:
                    case oracle::occi::OCCIVECTOR:
                    case oracle::occi::OCCIMETADATA:
                    case oracle::occi::OCCIPOBJECT:
                    case oracle::occi::OCCIREF:
                    case oracle::occi::OCCIREFANY:
                    case oracle::occi::OCCISTREAM:
                    case oracle::occi::OCCICURSOR:
                    case oracle::occi::OCCI_SQLT_FILE:
                    case oracle::occi::OCCI_SQLT_CFILE:
                    case oracle::occi::OCCI_SQLT_REF:
                    case oracle::occi::OCCI_SQLT_CLOB:
                    case oracle::occi::OCCI_SQLT_BLOB:
                    case oracle::occi::OCCI_SQLT_RSET:
                        MAPNIK_LOG_WARN(occi) << "occi_datasource: Unsupported datatype "
                                              << occi_enums::resolve_datatype(type_oid)
                                              << " (type_oid=" << type_oid << ")";
                        break;
                    default:
                        MAPNIK_LOG_WARN(occi) << "occi_datasource: Unknown datatype "
                                              << "(type_oid=" << type_oid << ")";
                        break;
                    }
                }
            }
        }
        catch (SQLException& ex)
        {
            throw datasource_exception(ex.getMessage());
        }
    }

    is_bound_ = true;
}

const char * occi_datasource::name()
{
    return "occi";
}

mapnik::datasource::datasource_t occi_datasource::type() const
{
    return type_;
}

box2d<double> occi_datasource::envelope() const
{
    if (extent_initialized_) return extent_;
    if (! is_bound_) bind();

    double lox = 0.0, loy = 0.0, hix = 0.0, hiy = 0.0;

    boost::optional<mapnik::boolean> estimate_extent =
        params_.get<mapnik::boolean>("estimate_extent",false);

    if (estimate_extent && *estimate_extent)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, "occi_datasource::envelope(estimate_extent)");
#endif

        std::ostringstream s;
        s << "SELECT MIN(c.x), MIN(c.y), MAX(c.x), MAX(c.y) FROM ";
        s << " (SELECT SDO_AGGR_MBR(" << geometry_field_ << ") shape FROM " << table_ << ") a, ";
        s << " TABLE(SDO_UTIL.GETVERTICES(a.shape)) c";

        MAPNIK_LOG_DEBUG(occi) << "occi_datasource: " << s.str();

        try
        {
            occi_connection_ptr conn;
            if (use_connection_pool_) conn.set_pool(pool_);
            else                      conn.set_connection(conn_, false);

            ResultSet* rs = conn.execute_query(s.str());
            if (rs && rs->next())
            {
                try
                {
                    lox = lexical_cast<double>(rs->getDouble(1));
                    loy = lexical_cast<double>(rs->getDouble(2));
                    hix = lexical_cast<double>(rs->getDouble(3));
                    hiy = lexical_cast<double>(rs->getDouble(4));
                    extent_.init(lox, loy, hix, hiy);
                    extent_initialized_ = true;
                }
                catch (bad_lexical_cast& ex)
                {
                    MAPNIK_LOG_WARN(occi) << "OCCI Plugin: " << ex.what();
                }
            }
        }
        catch (SQLException& ex)
        {
            throw datasource_exception("OCCI Plugin: " + ex.getMessage());
        }
    }
    else if (use_spatial_index_)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, "occi_datasource::envelope(use_spatial_index)");
#endif

        std::ostringstream s;
        s << "SELECT dim.sdo_lb, dim.sdo_ub FROM ";
        s << METADATA_TABLE << " m, TABLE(m.diminfo) dim ";
        s << " WHERE LOWER(m.table_name) = LOWER('" << table_name_ << "') AND dim.sdo_dimname = 'X'";
        s << " UNION ";
        s << "SELECT dim.sdo_lb, dim.sdo_ub FROM ";
        s << METADATA_TABLE << " m, TABLE(m.diminfo) dim ";
        s << " WHERE LOWER(m.table_name) = LOWER('" << table_name_ << "') AND dim.sdo_dimname = 'Y'";

        MAPNIK_LOG_DEBUG(occi) << "occi_datasource: " << s.str();

        try
        {
            occi_connection_ptr conn;
            if (use_connection_pool_) conn.set_pool(pool_);
            else                      conn.set_connection(conn_, false);

            ResultSet* rs = conn.execute_query(s.str());
            if (rs)
            {
                if (rs->next())
                {
                    try
                    {
                        lox = lexical_cast<double>(rs->getDouble(1));
                        hix = lexical_cast<double>(rs->getDouble(2));
                    }
                    catch (bad_lexical_cast& ex)
                    {
                        MAPNIK_LOG_WARN(occi) << "OCCI Plugin: " << ex.what();
                    }
                }

                if (rs->next())
                {
                    try
                    {
                        loy = lexical_cast<double>(rs->getDouble(1));
                        hiy = lexical_cast<double>(rs->getDouble(2));
                    }
                    catch (bad_lexical_cast& ex)
                    {
                        MAPNIK_LOG_WARN(occi) << "OCCI Plugin: " << ex.what();
                    }
                }

                extent_.init(lox, loy, hix, hiy);
                extent_initialized_ = true;
            }
        }
        catch (SQLException& ex)
        {
            throw datasource_exception("OCCI Plugin: " + ex.getMessage());
        }
    }

    if (! extent_initialized_)
    {
        throw datasource_exception("OCCI Plugin: unable to determine the extent of a <occi> table");
    }

    return extent_;
}


boost::optional<mapnik::datasource::geometry_t> occi_datasource::get_geometry_type() const
{
    // FIXME
    //if (! is_bound_) bind();
    return boost::optional<mapnik::datasource::geometry_t>();
}

layer_descriptor occi_datasource::get_descriptor() const
{
    if (! is_bound_) bind();

    return desc_;
}

featureset_ptr occi_datasource::features(query const& q) const
{
    if (! is_bound_) bind();

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "occi_datasource::features");
#endif

    box2d<double> const& box = q.get_bbox();

    std::ostringstream s;
    s << "SELECT ";
    if (use_wkb_)
    {
        s << "SDO_UTIL.TO_WKBGEOMETRY(" << geometry_field_ << ")";
    }
    else
    {
        s << geometry_field_;
    }
    std::set<std::string> const& props = q.property_names();
    std::set<std::string>::const_iterator pos = props.begin();
    std::set<std::string>::const_iterator end = props.end();
    mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();
    for (; pos != end; ++pos)
    {
        s << ", " << *pos;
        ctx->push(*pos);
    }

    s << " FROM ";

    std::string query(table_);

    if (use_spatial_index_)
    {
        std::ostringstream spatial_sql;
        spatial_sql << std::setprecision(16);
        spatial_sql << " WHERE SDO_FILTER(" << geometry_field_ << ",";
        spatial_sql << "  MDSYS.SDO_GEOMETRY(" << SDO_GTYPE_2DPOLYGON << "," << srid_ << ",NULL,";
        spatial_sql << "  MDSYS.SDO_ELEM_INFO_ARRAY(1," << SDO_ETYPE_POLYGON << "," << SDO_INTERPRETATION_RECTANGLE << "),";
        spatial_sql << "  MDSYS.SDO_ORDINATE_ARRAY(";
        spatial_sql << box.minx() << "," << box.miny() << ", ";
        spatial_sql << box.maxx() << "," << box.maxy() << ")), 'querytype=WINDOW') = 'TRUE'";

        if (boost::algorithm::ifind_first(query, "WHERE"))
        {
            boost::algorithm::ireplace_first(query, "WHERE", spatial_sql.str() + " AND ");
        }
        else if (boost::algorithm::ifind_first(query, table_name_))
        {
            boost::algorithm::ireplace_first(query, table_name_, table_name_ + " " + spatial_sql.str());
        }
        else
        {
            MAPNIK_LOG_WARN(occi) << "occi_datasource: cannot determine where to add the spatial filter declaration";
        }
    }

    if (row_limit_ > 0)
    {
        std::ostringstream row_limit_string;
        row_limit_string << "rownum < " << row_limit_;
        if (boost::algorithm::ifind_first(query, "WHERE"))
        {
            boost::algorithm::ireplace_first(query, "WHERE", row_limit_string.str() + " AND ");
        }
        else if (boost::algorithm::ifind_first(query, table_name_))
        {
            boost::algorithm::ireplace_first(query, table_name_, table_name_ + " " + row_limit_string.str());
        }
        else
        {
            MAPNIK_LOG_WARN(occi) << "occi_datasource: Cannot determine where to add the row limit declaration";
        }
    }

    s << query;

    MAPNIK_LOG_DEBUG(occi) << "occi_datasource: " << s.str();

    return boost::make_shared<occi_featureset>(pool_,
                                               conn_,
                                               ctx,
                                               s.str(),
                                               desc_.get_encoding(),
                                               use_connection_pool_,
                                               use_wkb_,
                                               row_prefetch_);
}

featureset_ptr occi_datasource::features_at_point(coord2d const& pt, double tol) const
{
    if (! is_bound_) bind();

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "occi_datasource::features_at_point");
#endif

    std::ostringstream s;
    s << "SELECT ";
    if (use_wkb_)
    {
        s << "SDO_UTIL.TO_WKBGEOMETRY(" << geometry_field_ << ")";
    }
    else
    {
        s << geometry_field_;
    }
    std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
    std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
    mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();
    while (itr != end)
    {
        s << ", " << itr->get_name();
        ctx->push(itr->get_name());
        ++itr;
    }

    s << " FROM ";

    std::string query(table_);

    if (use_spatial_index_)
    {
        std::ostringstream spatial_sql;
        spatial_sql << std::setprecision(16);
        spatial_sql << " WHERE SDO_FILTER(" << geometry_field_ << ",";
        spatial_sql << "  MDSYS.SDO_GEOMETRY(" << SDO_GTYPE_2DPOINT << "," << srid_ << ",NULL,";
        spatial_sql << "  MDSYS.SDO_ELEM_INFO_ARRAY(1," << SDO_ETYPE_POINT << "," << SDO_INTERPRETATION_POINT << "),";
        spatial_sql << "  MDSYS.SDO_ORDINATE_ARRAY(";
        spatial_sql << pt.x << "," << pt.y << ")), 'querytype=WINDOW') = 'TRUE'";

        if (boost::algorithm::ifind_first(query, "WHERE"))
        {
            boost::algorithm::ireplace_first(query, "WHERE", spatial_sql.str() + " AND ");
        }
        else if (boost::algorithm::ifind_first(query, table_name_))
        {
            boost::algorithm::ireplace_first(query, table_name_, table_name_ + " " + spatial_sql.str());
        }
        else
        {
            MAPNIK_LOG_WARN(occi) << "occi_datasource: Cannot determine where to add the spatial filter declaration";
        }
    }

    if (row_limit_ > 0)
    {
        std::ostringstream row_limit_string;
        row_limit_string << "rownum < " << row_limit_;
        if (boost::algorithm::ifind_first(query, "WHERE"))
        {
            boost::algorithm::ireplace_first(query, "WHERE", row_limit_string.str() + " AND ");
        }
        else if (boost::algorithm::ifind_first(query, table_name_))
        {
            boost::algorithm::ireplace_first(query, table_name_, table_name_ + " " + row_limit_string.str());
        }
        else
        {
            MAPNIK_LOG_WARN(occi) << "occi_datasource: Cannot determine where to add the row limit declaration";
        }
    }

    s << query;

    MAPNIK_LOG_DEBUG(occi) << "occi_datasource: " << s.str();

    return boost::make_shared<occi_featureset>(pool_,
                                               conn_,
                                               ctx,
                                               s.str(),
                                               desc_.get_encoding(),
                                               use_connection_pool_,
                                               use_wkb_,
                                               row_prefetch_);
}
