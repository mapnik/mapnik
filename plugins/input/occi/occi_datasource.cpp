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
#include <mapnik/value_types.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

// stl
#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include <iomanip>

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

const double occi_datasource::FMAX = std::numeric_limits<double>::max();
const std::string occi_datasource::METADATA_TABLE = "USER_SDO_GEOM_METADATA";

DATASOURCE_PLUGIN(occi_datasource)

occi_datasource::occi_datasource(parameters const& params)
    : datasource (params),
      type_(datasource::Vector),
      fields_(*params.get<std::string>("fields", "*")),
      geometry_field_(*params.get<std::string>("geometry_field", "")),
      srid_initialized_(false),
      extent_initialized_(false),
      bbox_token_("!bbox!"),
      scale_denom_token_("!scale_denominator!"),
      pixel_width_token_("!pixel_width!"),
      pixel_height_token_("!pixel_height!"),
      desc_(occi_datasource::name(), *params.get<std::string>("encoding", "utf-8")),
      use_wkb_(*params.get<mapnik::boolean_type>("use_wkb", false)),
      row_limit_(*params.get<mapnik::value_integer>("row_limit", 0)),
      row_prefetch_(*params.get<int>("row_prefetch", 100)),
      pool_(0),
      conn_(0)
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "occi_datasource::init");
#endif

    if (! params.get<std::string>("user")) throw datasource_exception("OCCI Plugin: no <user> specified");
    if (! params.get<std::string>("password")) throw datasource_exception("OCCI Plugin: no <password> specified");
    if (! params.get<std::string>("host")) throw datasource_exception("OCCI Plugin: no <host> string specified");

    boost::optional<std::string> table = params.get<std::string>("table");
    if (! table)
    {
        throw datasource_exception("OCCI Plugin: no <table> parameter specified");
    }
    else
    {
        table_ = *table;
    }
    estimate_extent_ = *params.get<mapnik::boolean_type>("estimate_extent",false);
    use_spatial_index_ = *params.get<mapnik::boolean_type>("use_spatial_index",true);
    use_connection_pool_ = *params.get<mapnik::boolean_type>("use_connection_pool",true);

    boost::optional<std::string> ext = params.get<std::string>("extent");
    if (ext) extent_initialized_ = extent_.from_string(*ext);

    boost::optional<int> srid = params.get<int>("srid");
    if (srid)
    {
        srid_ = *srid;
        srid_initialized_ = true;
    }

    // connect to environment
    if (use_connection_pool_)
    {
        try
        {
            pool_ = occi_environment::instance().create_pool(
                *params.get<std::string>("user"),
                *params.get<std::string>("password"),
                *params.get<std::string>("host"),
                *params.get<int>("max_size", 5),
                *params.get<int>("initial_size", 1),
                1);
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
            conn_ = occi_environment::instance().create_connection(
                *params.get<std::string>("user"),
                *params.get<std::string>("password"),
                *params.get<std::string>("host"));
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
        s << "SELECT " << fields_ << " FROM (" << table_name_ << ") WHERE ROWNUM < 1";

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
                        desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Boolean));
                        break;
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
                    case oracle::occi::OCCI_SQLT_LNG:
                    case oracle::occi::OCCI_SQLT_LVC:
                    case oracle::occi::OCCI_SQLT_STR:
                    case oracle::occi::OCCI_SQLT_VCS:
                    case oracle::occi::OCCI_SQLT_VNU:
                    case oracle::occi::OCCI_SQLT_VBI:
                    case oracle::occi::OCCI_SQLT_VST:
                    case oracle::occi::OCCIROWID:
                    case oracle::occi::OCCI_SQLT_RDD:
                    case oracle::occi::OCCI_SQLT_RID:
                    case oracle::occi::OCCIDATE:
                    case oracle::occi::OCCI_SQLT_DAT:
                    case oracle::occi::OCCI_SQLT_DATE:
                    case oracle::occi::OCCI_SQLT_TIME:
                    case oracle::occi::OCCI_SQLT_TIME_TZ:
                    case oracle::occi::OCCITIMESTAMP:
                    case oracle::occi::OCCI_SQLT_TIMESTAMP:
                    case oracle::occi::OCCI_SQLT_TIMESTAMP_LTZ:
                    case oracle::occi::OCCI_SQLT_TIMESTAMP_TZ:
                        desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
                        break;
                    case oracle::occi::OCCIINTERVALDS:
                    case oracle::occi::OCCIINTERVALYM:
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
}

occi_datasource::~occi_datasource()
{
    if (use_connection_pool_)
    {
        if (pool_ != 0)
        {
            occi_environment::instance().destroy_pool(pool_);
        }
    }
    else
    {
        if (conn_ != 0)
        {
            occi_environment::instance().destroy_connection(conn_);
        }
    }
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

    double lox = 0.0, loy = 0.0, hix = 0.0, hiy = 0.0;


    if (estimate_extent_)
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
                lox = rs->getDouble(1);
                loy = rs->getDouble(2);
                hix = rs->getDouble(3);
                hiy = rs->getDouble(4);
                extent_.init(lox, loy, hix, hiy);
                extent_initialized_ = true;
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
                    lox = rs->getDouble(1);
                    hix = rs->getDouble(2);
                }

                if (rs->next())
                {
                    loy = rs->getDouble(1);
                    hiy = rs->getDouble(2);
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
    return boost::optional<mapnik::datasource::geometry_t>();
}

layer_descriptor occi_datasource::get_descriptor() const
{
    return desc_;
}

std::string occi_datasource::sql_bbox(box2d<double> const& env) const
{
    std::ostringstream b;
    b << std::setprecision(16);
    b << "MDSYS.SDO_GEOMETRY(" << SDO_GTYPE_2DPOLYGON << "," << srid_ << ",NULL,";
    b << " MDSYS.SDO_ELEM_INFO_ARRAY(1," << SDO_ETYPE_POLYGON << "," << SDO_INTERPRETATION_RECTANGLE << "),";
    b << " MDSYS.SDO_ORDINATE_ARRAY(";
    b << env.minx() << "," << env.miny() << ", ";
    b << env.maxx() << "," << env.maxy() << "))";
    return b.str();
}

std::string occi_datasource::populate_tokens(std::string const& sql, double scale_denom, box2d<double> const& env, double pixel_width, double pixel_height) const
{
    std::string populated_sql = sql;

    if (boost::algorithm::icontains(populated_sql, scale_denom_token_))
    {
        std::ostringstream ss;
        ss << scale_denom;
        boost::algorithm::replace_all(populated_sql, scale_denom_token_, ss.str());
    }

    if (boost::algorithm::icontains(sql, pixel_width_token_))
    {
        std::ostringstream ss;
        ss << pixel_width;
        boost::algorithm::replace_all(populated_sql, pixel_width_token_, ss.str());
    }

    if (boost::algorithm::icontains(sql, pixel_height_token_))
    {
        std::ostringstream ss;
        ss << pixel_height;
        boost::algorithm::replace_all(populated_sql, pixel_height_token_, ss.str());
    }

    if (boost::algorithm::icontains(populated_sql, bbox_token_))
    {
        boost::algorithm::replace_all(populated_sql, bbox_token_, sql_bbox(env));
    }

    return populated_sql;
}

featureset_ptr occi_datasource::features(query const& q) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "occi_datasource::features");
#endif

    box2d<double> const& box = q.get_bbox();
    const double px_gw = 1.0 / std::get<0>(q.resolution());
    const double px_gh = 1.0 / std::get<1>(q.resolution());
    const double scale_denom = q.scale_denominator();

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
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    for (; pos != end; ++pos)
    {
        s << ", " << *pos;
        ctx->push(*pos);
    }

    std::string query = populate_tokens(table_, scale_denom, box, px_gw, px_gh);

    if (use_spatial_index_)
    {
        std::ostringstream spatial_sql;
        spatial_sql << " WHERE SDO_FILTER(";
        spatial_sql << geometry_field_ << "," << sql_bbox(box);
        spatial_sql << ", 'querytype = WINDOW') = 'TRUE'";

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

    s << " FROM " << query;

    if (row_limit_ > 0)
    {
        s << " WHERE ROWNUM < " << row_limit_;
    }

    MAPNIK_LOG_DEBUG(occi) << "occi_datasource: " << s.str();

    return std::make_shared<occi_featureset>(pool_,
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
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    while (itr != end)
    {
        s << ", " << itr->get_name();
        ctx->push(itr->get_name());
        ++itr;
    }

    box2d<double> box(pt.x - tol, pt.y - tol, pt.x + tol, pt.y + tol);
    std::string query = populate_tokens(table_, FMAX, box, 0, 0);

    if (use_spatial_index_)
    {
        std::ostringstream spatial_sql;
        spatial_sql << " WHERE SDO_FILTER(";
        spatial_sql << geometry_field_ << "," << sql_bbox(box);
        spatial_sql << ", 'querytype = WINDOW') = 'TRUE'";

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

    s << " FROM " << query;

    if (row_limit_ > 0)
    {
        s << " WHERE ROWNUM < " << row_limit_;
    }

    MAPNIK_LOG_DEBUG(occi) << "occi_datasource: " << s.str();

    return std::make_shared<occi_featureset>(pool_,
                                                conn_,
                                                ctx,
                                                s.str(),
                                                desc_.get_encoding(),
                                                use_connection_pool_,
                                                use_wkb_,
                                                row_prefetch_);
}
