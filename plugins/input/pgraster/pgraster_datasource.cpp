/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
 *****************************************************************************
 *
 * Initially developed by Sandro Santilli <strk@keybit.net> for CartoDB
 *
 *****************************************************************************/

#include "../postgis/connection_manager.hpp"
#include "../postgis/asyncresultset.hpp"
#include "pgraster_datasource.hpp"
#include "pgraster_featureset.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/global.hpp> // for byte
#include <mapnik/boolean.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/timer.hpp>
#include <mapnik/value/types.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <cfloat> // FLT_MAX
#include <string>
#include <algorithm>
#include <set>
#include <sstream>

DATASOURCE_PLUGIN_IMPL(pgraster_datasource_plugin, pgraster_datasource);
DATASOURCE_PLUGIN_EXPORT(pgraster_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(pgraster_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(pgraster_datasource_plugin);


const std::string pgraster_datasource::RASTER_COLUMNS = "raster_columns";
const std::string pgraster_datasource::RASTER_OVERVIEWS = "raster_overviews";
const std::string pgraster_datasource::SPATIAL_REF_SYS = "spatial_ref_system";

using mapnik::attribute_descriptor;
using mapnik::value_integer;
using mapnik::sql_utils::identifier;
using mapnik::sql_utils::literal;
using std::shared_ptr;

pgraster_datasource::pgraster_datasource(parameters const& params)
    : datasource(params)
    , table_(*params.get<std::string>("table", ""))
    , raster_table_(*params.get<std::string>("raster_table", ""))
    , raster_field_(*params.get<std::string>("raster_field", ""))
    , key_field_(*params.get<std::string>("key_field", ""))
    , cursor_fetch_size_(*params.get<mapnik::value_integer>("cursor_size", 0))
    , row_limit_(*params.get<value_integer>("row_limit", 0))
    , type_(datasource::Raster)
    , srid_(*params.get<value_integer>("srid", 0))
    , band_(*params.get<value_integer>("band", 0))
    , extent_initialized_(false)
    , prescale_rasters_(*params.get<mapnik::boolean_type>("prescale_rasters", false))
    , use_overviews_(*params.get<mapnik::boolean_type>("use_overviews", false))
    , clip_rasters_(*params.get<mapnik::boolean_type>("clip_rasters", false))
    , desc_(*params.get<std::string>("type"), "utf-8")
    , creator_(params)
    , re_tokens_("!(@?\\w+)!")
    , // matches  !mapnik_var!  or  !@user_var!
    pool_max_size_(*params_.get<value_integer>("max_size", 10))
    , persist_connection_(*params.get<mapnik::boolean_type>("persist_connection", true))
    , extent_from_subquery_(*params.get<mapnik::boolean_type>("extent_from_subquery", false))
    , estimate_extent_(*params.get<mapnik::boolean_type>("estimate_extent", false))
    , max_async_connections_(*params_.get<value_integer>("max_async_connection", 1))
    , asynchronous_request_(false)
    ,
    // params below are for testing purposes only and may be removed at any time
    intersect_min_scale_(*params.get<value_integer>("intersect_min_scale", 0))
    , intersect_max_scale_(*params.get<value_integer>("intersect_max_scale", 0))
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "pgraster_datasource::init");
#endif
    if (table_.empty())
    {
        throw mapnik::datasource_exception("Pgraster Plugin: missing <table> parameter");
    }

    boost::optional<std::string> ext = params.get<std::string>("extent");
    if (ext && !ext->empty())
    {
        extent_initialized_ = extent_.from_string(*ext);
    }

    // NOTE: In multithread environment, pool_max_size_ should be
    // max_async_connections_ * num_threads
    if (max_async_connections_ > 1)
    {
        if (max_async_connections_ > pool_max_size_)
        {
            std::ostringstream err;
            err << "PostGIS Plugin: Error: 'max_async_connections (" << max_async_connections_
                << ") must be <= max_size(" << pool_max_size_ << ")";
            throw mapnik::datasource_exception(err.str());
        }
        asynchronous_request_ = true;
    }

    boost::optional<value_integer> initial_size = params.get<value_integer>("initial_size", 1);
    boost::optional<mapnik::boolean_type> autodetect_key_field =
      params.get<mapnik::boolean_type>("autodetect_key_field", false);

    ConnectionManager::instance().registerPool(creator_, *initial_size, pool_max_size_);
    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
    if (!pool)
        return;

    shared_ptr<Connection> conn = pool->borrowObject();
    if (!conn)
        return;

    if (conn->isOK())
    {
        desc_.set_encoding(conn->client_encoding());

        if (raster_table_.empty())
        {
            mapnik::sql_utils::table_from_sql(table_, parsed_schema_, parsed_table_);

            // non-trivial subqueries (having no FROM) make it
            // impossible to use overviews
            // TODO: improve "table_from_sql" ?
            auto nsp = table_.find_first_not_of(" \t\r\n");
            if (nsp != std::string::npos && table_[nsp] == '(')
            {
                if (use_overviews_)
                {
                    std::ostringstream err;
                    err << "Pgraster Plugin: overviews cannot be used "
                           "with non-trivial subqueries";
                    MAPNIK_LOG_WARN(pgraster) << err.str();
                    use_overviews_ = false;
                }
                if (!extent_from_subquery_)
                {
                    std::ostringstream err;
                    err << "Pgraster Plugin: extent can only be computed "
                           "from subquery as we could not found table source";
                    MAPNIK_LOG_WARN(pgraster) << err.str();
                    extent_from_subquery_ = true;
                }
            }
        }
        else
        {
            mapnik::sql_utils::table_from_sql(raster_table_, parsed_schema_, parsed_table_);
        }

        // If we do not know either the raster_field or the srid or we
        // want to use overviews but do not know about schema, or
        // no extent was specified, then attempt to fetch the missing
        // information from a raster_columns entry.
        //
        // This will return no records if we are querying a bogus table returned
        // from the simplistic table parsing in table_from_sql() or if
        // the table parameter references a table, view, or subselect not
        // registered in the raster_columns.
        //
        geometryColumn_ = mapnik::sql_utils::unquote_copy('"', raster_field_);
        if (!parsed_table_.empty() && (geometryColumn_.empty() || srid_ == 0 ||
                                       (parsed_schema_.empty() && use_overviews_) || !extent_initialized_))
        {
#ifdef MAPNIK_STATS
            mapnik::progress_timer __stats2__(std::clog, "pgraster_datasource::init(get_srid_and_geometry_column)");
#endif
            std::ostringstream s;

            try
            {
                s << "SELECT r_raster_column col, srid, r_table_schema";
                if (!extent_initialized_)
                {
                    s << ", st_xmin(extent) xmin, st_ymin(extent) ymin"
                      << ", st_xmax(extent) xmax, st_ymax(extent) ymax";
                }
                s << " FROM " << RASTER_COLUMNS << " WHERE r_table_name=" << literal(parsed_table_);
                if (!parsed_schema_.empty())
                {
                    s << " AND r_table_schema=" << literal(parsed_schema_);
                }
                if (!geometryColumn_.empty())
                {
                    s << " AND r_raster_column=" << literal(geometryColumn_);
                }
                MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: running query " << s.str();
                shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                if (rs->next())
                {
                    geometryColumn_ = rs->getValue("col");
                    if (!extent_initialized_)
                    {
                        double lox, loy, hix, hiy;
                        if (mapnik::util::string2double(rs->getValue("xmin"), lox) &&
                            mapnik::util::string2double(rs->getValue("ymin"), loy) &&
                            mapnik::util::string2double(rs->getValue("xmax"), hix) &&
                            mapnik::util::string2double(rs->getValue("ymax"), hiy))
                        {
                            extent_.init(lox, loy, hix, hiy);
                            extent_initialized_ = true;
                            MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: Layer extent=" << extent_;
                        }
                        else
                        {
                            MAPNIK_LOG_DEBUG(pgraster)
                              << "pgraster_datasource: Could not determine extent from query: " << s.str();
                        }
                    }
                    if (srid_ == 0)
                    {
                        const char* srid_c = rs->getValue("srid");
                        if (srid_c != nullptr)
                        {
                            int result = 0;
                            const char* end = srid_c + std::strlen(srid_c);
                            if (mapnik::util::string2int(srid_c, end, result))
                            {
                                srid_ = result;
                            }
                        }
                    }
                    if (parsed_schema_.empty())
                    {
                        parsed_schema_ = rs->getValue("r_table_schema");
                    }
                }
                else
                {
                    MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: no response from metadata query " << s.str();
                }
                rs->close();
            } catch (mapnik::datasource_exception const& ex)
            {
                // let this pass on query error and use the fallback below
                MAPNIK_LOG_WARN(pgraster) << "pgraster_datasource: metadata query failed: " << ex.what();
            }

            // If we still do not know the srid then we can try to fetch
            // it from the 'table_' parameter, which should work even if it is
            // a subselect as long as we know the raster_field to query
            if (!geometryColumn_.empty() && srid_ <= 0)
            {
                s.str("");

                s << "SELECT ST_SRID(" << identifier(geometryColumn_) << ") AS srid FROM " << populate_tokens(table_)
                  << " WHERE " << identifier(geometryColumn_) << " IS NOT NULL LIMIT 1";

                shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                if (rs->next())
                {
                    const char* srid_c = rs->getValue("srid");
                    if (srid_c != nullptr)
                    {
                        int result = 0;
                        const char* end = srid_c + std::strlen(srid_c);
                        if (mapnik::util::string2int(srid_c, end, result))
                        {
                            srid_ = result;
                        }
                    }
                }
                rs->close();
            }
        }

        // If overviews were requested, take note of the max scale
        // of each available overview, sorted by scale descending
        if (use_overviews_)
        {
            std::ostringstream err;
            if (parsed_schema_.empty())
            {
                err << "Pgraster Plugin: unable to lookup available table"
                    << " overviews due to unknown schema";
                throw mapnik::datasource_exception(err.str());
            }
            if (geometryColumn_.empty())
            {
                err << "Pgraster Plugin: unable to lookup available table"
                    << " overviews due to unknown column name";
                throw mapnik::datasource_exception(err.str());
            }

            std::ostringstream s;
            s << "select "
                 "r.r_table_schema sch, "
                 "r.r_table_name tab, "
                 "r.r_raster_column col, "
                 "greatest(abs(r.scale_x), abs(r.scale_y)) scl "
                 "from"
                 " raster_overviews o,"
                 " raster_columns r "
                 "where"
                 " o.r_table_schema = "
              << literal(parsed_schema_) << " and o.r_table_name = " << literal(parsed_table_)
              << " and o.r_raster_column = " << literal(geometryColumn_)
              << " and r.r_table_schema = o.o_table_schema"
                 " and r.r_table_name = o.o_table_name"
                 " and r.r_raster_column = o.o_raster_column"
                 " ORDER BY scl ASC";
            MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: running query " << s.str();
            shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
            while (rs->next())
            {
                pgraster_overview ov = pgraster_overview();

                ov.schema = rs->getValue("sch");
                ov.table = rs->getValue("tab");
                ov.column = rs->getValue("col");
                ov.scale = atof(rs->getValue("scl"));

                if (ov.scale == 0.0f)
                {
                    MAPNIK_LOG_WARN(pgraster) << "pgraster_datasource: found invalid overview " << ov.schema << "."
                                              << ov.table << "." << ov.column << " with scale " << ov.scale;
                    continue;
                }

                overviews_.push_back(ov);

                MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: found overview " << ov.schema << "." << ov.table
                                           << "." << ov.column << " with scale " << ov.scale;
            }
            rs->close();
            if (overviews_.empty())
            {
                MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: no overview found for " << parsed_schema_ << "."
                                           << parsed_table_ << "." << geometryColumn_;
            }
        }

        // detect primary key
        if (*autodetect_key_field && key_field_.empty())
        {
#ifdef MAPNIK_STATS
            mapnik::progress_timer __stats2__(std::clog, "pgraster_datasource::bind(get_primary_key)");
#endif

            std::ostringstream s;
            s << "SELECT a.attname, a.attnum, t.typname, t.typname in ('int2','int4','int8') "
                 "AS is_int FROM pg_class c, pg_attribute a, pg_type t, pg_namespace n, pg_index i "
                 "WHERE a.attnum > 0 AND a.attrelid = c.oid "
                 "AND a.atttypid = t.oid AND c.relnamespace = n.oid "
                 "AND c.oid = i.indrelid AND i.indisprimary = 't' "
                 "AND t.typname !~ '^geom' AND c.relname = "
              << literal(parsed_table_)
              << " "
              //"AND a.attnum = ANY (i.indkey) " // postgres >= 8.1
              << "AND (i.indkey[0]=a.attnum OR i.indkey[1]=a.attnum OR i.indkey[2]=a.attnum "
                 "OR i.indkey[3]=a.attnum OR i.indkey[4]=a.attnum OR i.indkey[5]=a.attnum "
                 "OR i.indkey[6]=a.attnum OR i.indkey[7]=a.attnum OR i.indkey[8]=a.attnum "
                 "OR i.indkey[9]=a.attnum) ";
            if (!parsed_schema_.empty())
            {
                s << "AND n.nspname=" << literal(parsed_schema_) << ' ';
            }
            s << "ORDER BY a.attnum";

            shared_ptr<ResultSet> rs_key = conn->executeQuery(s.str());
            if (rs_key->next())
            {
                unsigned int result_rows = rs_key->size();
                if (result_rows == 1)
                {
                    bool is_int = (std::string(rs_key->getValue(3)) == "t");
                    if (is_int)
                    {
                        const char* key_field_string = rs_key->getValue(0);
                        if (key_field_string)
                        {
                            key_field_ = std::string(key_field_string);

                            MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: auto-detected key field of '"
                                                       << key_field_ << "' on table '" << parsed_table_ << "'";
                        }
                    }
                    else
                    {
                        // throw for cases like a numeric primary key, which is invalid
                        // as it should be floating point (int numerics are useless)
                        std::ostringstream err;
                        err << "PostGIS Plugin: Error: '" << rs_key->getValue(0) << "' on table '" << parsed_table_
                            << "' is not a valid integer primary key field\n";
                        throw mapnik::datasource_exception(err.str());
                    }
                }
                else if (result_rows > 1)
                {
                    std::ostringstream err;
                    err << "PostGIS Plugin: Error: '"
                        << "multi column primary key detected but is not supported";
                    throw mapnik::datasource_exception(err.str());
                }
            }
            rs_key->close();
        }

        // if a globally unique key field/primary key is required
        // but still not known at this point, then throw
        if (*autodetect_key_field && key_field_.empty())
        {
            throw mapnik::datasource_exception("PostGIS Plugin: Error: primary key required"
                                               " but could not be detected for table '" +
                                               parsed_table_ +
                                               "', please supply 'key_field'"
                                               " option to specify field to use for primary key");
        }

        if (srid_ == 0)
        {
            srid_ = -1;

            MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: Table " << table_ << " is using SRID=" << srid_;
        }

        // At this point the raster_field may still not be known
        // but we'll catch that where more useful...
        MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: Using SRID=" << srid_;
        MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: Using geometry_column=" << geometryColumn_;

        // collect attribute desc
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats2__(std::clog, "pgraster_datasource::bind(get_column_description)");
#endif

        std::ostringstream s;
        s << "SELECT * FROM " << populate_tokens(table_) << " LIMIT 0";

        shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
        int count = rs->getNumFields();
        bool found_key_field = false;
        for (int i = 0; i < count; ++i)
        {
            std::string fld_name = rs->getFieldName(i);
            int type_oid = rs->getTypeOID(i);

            // validate type of key_field
            if (!found_key_field && !key_field_.empty() && fld_name == key_field_)
            {
                if (type_oid == 20 || type_oid == 21 || type_oid == 23)
                {
                    found_key_field = true;
                    desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Integer));
                }
                else
                {
                    std::ostringstream error_s;
                    error_s << "invalid type '";

                    std::ostringstream type_s;
                    type_s << "SELECT oid, typname FROM pg_type WHERE oid = " << type_oid;

                    shared_ptr<ResultSet> rs_oid = conn->executeQuery(type_s.str());
                    if (rs_oid->next())
                    {
                        error_s << rs_oid->getValue("typname") << "' (oid:" << rs_oid->getValue("oid") << ")";
                    }
                    else
                    {
                        error_s << "oid:" << type_oid << "'";
                    }

                    rs_oid->close();
                    error_s << " for key_field '" << fld_name << "' - "
                            << "must be an integer primary key";

                    rs->close();
                    throw mapnik::datasource_exception(error_s.str());
                }
            }
            else
            {
                switch (type_oid)
                {
                    case 16: // bool
                        desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Boolean));
                        break;
                    case 20: // int8
                    case 21: // int2
                    case 23: // int4
                        desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Integer));
                        break;
                    case 700:  // float4
                    case 701:  // float8
                    case 1700: // numeric
                        desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Double));
                        break;
                    case 1042: // bpchar
                    case 1043: // varchar
                    case 25:   // text
                    case 705:  // literal
                        desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::String));
                        break;
                    default: // should not get here
#ifdef MAPNIK_LOG
                        s.str("");
                        s << "SELECT oid, typname FROM pg_type WHERE oid = " << type_oid;

                        shared_ptr<ResultSet> rs_oid = conn->executeQuery(s.str());
                        if (rs_oid->next())
                        {
                            std::string typname(rs_oid->getValue("typname"));
                            if (typname != "geometry" && typname != "raster")
                            {
                                MAPNIK_LOG_WARN(pgraster) << "pgraster_datasource: Unknown type=" << typname
                                                          << " (oid:" << rs_oid->getValue("oid") << ")";
                            }
                        }
                        else
                        {
                            MAPNIK_LOG_WARN(pgraster) << "pgraster_datasource: Unknown type_oid=" << type_oid;
                        }
                        rs_oid->close();
#endif
                        break;
                }
            }
        }

        rs->close();
    }

    // Close explicitly the connection so we can 'fork()' without sharing open connections
    conn->close();
}

pgraster_datasource::~pgraster_datasource()
{
    if (!persist_connection_)
    {
        CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
        if (pool)
        {
            try
            {
                shared_ptr<Connection> conn = pool->borrowObject();
                if (conn)
                {
                    conn->close();
                }
            } catch (mapnik::datasource_exception const& ex)
            {
                // happens when borrowObject tries to
                // create a new connection and fails.
                // In turn, new connection would be needed
                // when our broke and was thus no good to
                // be borrowed
                // See https://github.com/mapnik/mapnik/issues/2191
            }
        }
    }
}

const char* pgraster_datasource::name()
{
    return "pgraster";
}

mapnik::datasource::datasource_t pgraster_datasource::type() const
{
    return type_;
}

layer_descriptor pgraster_datasource::get_descriptor() const
{
    return desc_;
}

std::string pgraster_datasource::sql_bbox(box2d<double> const& env) const
{
    std::ostringstream b;
    b.precision(16);
    b << "ST_MakeEnvelope(";
    b << env.minx() << "," << env.miny() << ",";
    b << env.maxx() << "," << env.maxy() << ",";
    b << std::max(srid_, 0) << ")";
    return b.str();
}

std::string pgraster_datasource::populate_tokens(std::string const& sql) const
{
    return populate_tokens(sql,
                           FLT_MAX,
                           box2d<double>(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX),
                           0,
                           0,
                           mapnik::attributes{},
                           false);
}

std::string pgraster_datasource::populate_tokens(std::string const& sql,
                                                 double scale_denom,
                                                 box2d<double> const& env,
                                                 double pixel_width,
                                                 double pixel_height,
                                                 mapnik::attributes const& vars,
                                                 bool intersect) const
{
    std::ostringstream populated_sql;
    std::cmatch m;
    char const* start = sql.data();
    char const* end = start + sql.size();

    populated_sql.precision(16);
    populated_sql << std::showpoint;

    while (std::regex_search(start, end, m, re_tokens_))
    {
        populated_sql.write(start, m[0].first - start);
        start = m[0].second;

        auto m1 = boost::make_iterator_range(m[1].first, m[1].second);
        if (m1.front() == '@')
        {
            std::string var_name(m1.begin() + 1, m1.end());
            auto itr = vars.find(var_name);
            if (itr != vars.end())
            {
                auto var_value = itr->second.to_string();
                populated_sql << literal(var_value);
            }
            else
            {
                populated_sql << "NULL"; // undefined @variable
            }
        }
        else if (boost::algorithm::equals(m1, "bbox"))
        {
            populated_sql << sql_bbox(env);
            intersect = false;
        }
        else if (boost::algorithm::equals(m1, "pixel_height"))
        {
            populated_sql << pixel_height;
        }
        else if (boost::algorithm::equals(m1, "pixel_width"))
        {
            populated_sql << pixel_width;
        }
        else if (boost::algorithm::equals(m1, "scale_denominator"))
        {
            populated_sql << scale_denom;
        }
        else
        {
            populated_sql << "NULL"; // unrecognized !token!
        }
    }

    populated_sql.write(start, end - start);

    if (intersect)
    {
        if (intersect_min_scale_ > 0 && (scale_denom <= intersect_min_scale_))
        {
            populated_sql << " WHERE ST_Intersects(" << identifier(geometryColumn_) << ", " << sql_bbox(env) << ")";
        }
        else if (intersect_max_scale_ > 0 && (scale_denom >= intersect_max_scale_))
        {
            // do no bbox restriction
        }
        else
        {
            populated_sql << " WHERE " << identifier(geometryColumn_) << " && " << sql_bbox(env);
        }
    }

    return populated_sql.str();
}

std::shared_ptr<IResultSet> pgraster_datasource::get_resultset(std::shared_ptr<Connection>& conn,
                                                               std::string const& sql,
                                                               CnxPool_ptr const& pool,
                                                               processor_context_ptr ctx) const
{
    if (!ctx)
    {
        // ! asynchronous_request_
        if (cursor_fetch_size_ > 0)
        {
            // cursor
            std::ostringstream csql;
            std::string cursor_name = conn->new_cursor_name();

            csql << "DECLARE " << cursor_name << " BINARY INSENSITIVE NO SCROLL CURSOR WITH HOLD FOR " << sql
                 << " FOR READ ONLY";

            if (!conn->execute(csql.str()))
            {
                // TODO - better error
                throw mapnik::datasource_exception("Pgraster Plugin: error creating cursor for data select.");
            }

            return std::make_shared<CursorResultSet>(conn, cursor_name, cursor_fetch_size_);
        }
        else
        {
            // no cursor
            return conn->executeQuery(sql, 1);
        }
    }
    else
    { // asynchronous requests

        std::shared_ptr<postgis_processor_context> pgis_ctxt = std::static_pointer_cast<postgis_processor_context>(ctx);
        if (conn)
        {
            // lauch async req & create asyncresult with conn
            conn->executeAsyncQuery(sql, 1);
            return std::make_shared<AsyncResultSet>(pgis_ctxt, pool, conn, sql);
        }
        else
        {
            // create asyncresult  with  null connection
            std::shared_ptr<AsyncResultSet> res = std::make_shared<AsyncResultSet>(pgis_ctxt, pool, conn, sql);
            pgis_ctxt->add_request(res);
            return res;
        }
    }
}

processor_context_ptr pgraster_datasource::get_context(feature_style_context_map& ctx) const
{
    if (!asynchronous_request_)
    {
        return processor_context_ptr();
    }

    std::string ds_name(name());
    feature_style_context_map::const_iterator itr = ctx.find(ds_name);
    if (itr != ctx.end())
    {
        return itr->second;
    }
    else
    {
        return ctx.insert(std::make_pair(ds_name, std::make_shared<postgis_processor_context>())).first->second;
    }
}

featureset_ptr pgraster_datasource::features(query const& q) const
{
    // if the driver is in asynchronous mode, return the appropriate fetaures
    if (asynchronous_request_)
    {
        return features_with_context(q, std::make_shared<postgis_processor_context>());
    }
    else
    {
        return features_with_context(q, processor_context_ptr());
    }
}

featureset_ptr pgraster_datasource::features_with_context(query const& q, processor_context_ptr proc_ctx) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "pgraster_datasource::features_with_context");
#endif

    box2d<double> const& box = q.get_bbox();
    double scale_denom = q.scale_denominator();

    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());

    if (pool)
    {
        shared_ptr<Connection> conn;

        if (asynchronous_request_)
        {
            // limit use to num_async_request_ => if reached don't borrow the last connexion object
            std::shared_ptr<postgis_processor_context> pgis_ctxt =
              std::static_pointer_cast<postgis_processor_context>(proc_ctx);
            if (pgis_ctxt->num_async_requests_ < max_async_connections_)
            {
                conn = pool->borrowObject();
                pgis_ctxt->num_async_requests_++;
            }
        }
        else
        {
            // Always get a connection in synchronous mode
            conn = pool->borrowObject();
            if (!conn)
            {
                throw mapnik::datasource_exception("Pgraster Plugin: Null connection");
            }
        }

        if (geometryColumn_.empty())
        {
            std::ostringstream s_error;
            s_error << "PostGIS: geometry name lookup failed for table '";

            if (!parsed_schema_.empty())
            {
                s_error << parsed_schema_ << ".";
            }
            s_error << parsed_table_ << "'. Please manually provide the 'raster_field' parameter or add an entry "
                    << "in the geometry_columns for '";

            if (!parsed_schema_.empty())
            {
                s_error << parsed_schema_ << ".";
            }
            s_error << parsed_table_ << "'.";

            throw mapnik::datasource_exception(s_error.str());
        }

        const double px_gw = 1.0 / std::get<0>(q.resolution());
        const double px_gh = 1.0 / std::get<1>(q.resolution());

        MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: px_gw=" << px_gw << " px_gh=" << px_gh;

        std::string table_with_bbox;
        std::string col = geometryColumn_;
        table_with_bbox = table_; // possibly a subquery

        if (use_overviews_ && !overviews_.empty())
        {
            std::string sch = overviews_[0].schema;
            std::string tab = overviews_[0].table;
            col = overviews_[0].column;
            const double scale = std::min(px_gw, px_gh);
            std::vector<pgraster_overview>::const_reverse_iterator i;
            for (i = overviews_.rbegin(); i != overviews_.rend(); ++i)
            {
                const pgraster_overview& o = *i;
                if (o.scale < scale)
                {
                    sch = o.schema;
                    tab = o.table;
                    col = o.column;
                    MAPNIK_LOG_DEBUG(pgraster)
                      << "pgraster_datasource: using overview " << o.schema << "." << o.table << "." << o.column
                      << " with scale=" << o.scale << " for min out scale=" << scale;
                    break;
                }
                else
                {
                    MAPNIK_LOG_DEBUG(pgraster)
                      << "pgraster_datasource: overview " << o.schema << "." << o.table << "." << o.column
                      << " with scale=" << o.scale << " not good for min out scale " << scale;
                }
            }
            boost::algorithm::replace_all(table_with_bbox, parsed_table_, tab);
            boost::algorithm::replace_all(table_with_bbox, parsed_schema_, sch);
            boost::algorithm::replace_all(table_with_bbox, geometryColumn_, col);
        }
        table_with_bbox = populate_tokens(table_with_bbox, scale_denom, box, px_gw, px_gh, q.variables());

        std::ostringstream s;

        s << "SELECT ST_AsBinary(";

        if (band_)
            s << "ST_Band(";

        if (prescale_rasters_)
            s << "ST_Resize(";

        if (clip_rasters_)
            s << "ST_Clip(";

        s << identifier(col);

        if (clip_rasters_)
        {
            s << ", ST_Expand(" << sql_bbox(box) << ", greatest(abs(ST_ScaleX(" << identifier(col)
              << ")), abs(ST_ScaleY(" << identifier(col) << ")))))";
        }

        if (prescale_rasters_)
        {
            const double scale = std::min(px_gw, px_gh);
            s << ", least(1.0, abs(ST_ScaleX(" << identifier(col) << "))::float8/" << scale
              << "), least(1.0, abs(ST_ScaleY(" << identifier(col) << "))::float8/" << scale << "))";
            // TODO: if band_ is given, we'll interpret as indexed so
            //       the rescaling must NOT ruin it (use algorithm mode!)
        }

        if (band_)
            s << ", " << band_ << ")";

        s << ") AS geom";

        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        std::set<std::string> const& props = q.property_names();
        std::set<std::string>::const_iterator pos = props.begin();
        std::set<std::string>::const_iterator end = props.end();

        if (!key_field_.empty())
        {
            s << ',' << identifier(key_field_);
            ctx->push(key_field_);

            for (; pos != end; ++pos)
            {
                if (*pos != key_field_)
                {
                    s << ',' << identifier(*pos);
                    ctx->push(*pos);
                }
            }
        }
        else
        {
            for (; pos != end; ++pos)
            {
                s << ',' << identifier(*pos);
                ctx->push(*pos);
            }
        }

        s << " FROM " << table_with_bbox;

        if (row_limit_ > 0)
        {
            s << " LIMIT " << row_limit_;
        }

        MAPNIK_LOG_DEBUG(pgraster) << "pgraster_datasource: "
                                      "features query: "
                                   << s.str();

        std::shared_ptr<IResultSet> rs = get_resultset(conn, s.str(), pool, proc_ctx);
        return std::make_shared<pgraster_featureset>(rs,
                                                     ctx,
                                                     desc_.get_encoding(),
                                                     !key_field_.empty(),
                                                     band_ ? 1 : 0 // whatever band number is given we'd have
                                                                   // extracted with ST_Band above so it becomes
                                                                   // band number 1
        );
    }

    return mapnik::make_invalid_featureset();
}

featureset_ptr pgraster_datasource::features_at_point(coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "pgraster_datasource::features_at_point");
#endif
    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (!conn)
            return mapnik::make_invalid_featureset();

        if (conn->isOK())
        {
            if (geometryColumn_.empty())
            {
                std::ostringstream s_error;
                s_error << "PostGIS: geometry name lookup failed for table '";

                if (!parsed_schema_.empty())
                {
                    s_error << parsed_schema_ << ".";
                }
                s_error << parsed_table_ << "'. Please manually provide the 'raster_field' parameter or add an entry "
                        << "in the geometry_columns for '";

                if (!parsed_schema_.empty())
                {
                    s_error << parsed_schema_ << ".";
                }
                s_error << parsed_table_ << "'.";

                throw mapnik::datasource_exception(s_error.str());
            }

            std::ostringstream s;
            s << "SELECT ST_AsBinary(" << identifier(geometryColumn_) << ") AS geom";

            mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
            auto const& desc = desc_.get_descriptors();

            if (!key_field_.empty())
            {
                s << ',' << identifier(key_field_);
                ctx->push(key_field_);
                for (auto const& attr_info : desc)
                {
                    std::string const& name = attr_info.get_name();
                    if (name != key_field_)
                    {
                        s << ',' << identifier(name);
                        ctx->push(name);
                    }
                }
            }
            else
            {
                for (auto const& attr_info : desc)
                {
                    std::string const& name = attr_info.get_name();
                    s << ',' << identifier(name);
                    ctx->push(name);
                }
            }

            box2d<double> box(pt.x - tol, pt.y - tol, pt.x + tol, pt.y + tol);
            std::string table_with_bbox = populate_tokens(table_, FLT_MAX, box, 0, 0, mapnik::attributes{});

            s << " FROM " << table_with_bbox;

            if (row_limit_ > 0)
            {
                s << " LIMIT " << row_limit_;
            }

            std::shared_ptr<IResultSet> rs = get_resultset(conn, s.str(), pool);
            return std::make_shared<pgraster_featureset>(rs, ctx, desc_.get_encoding(), !key_field_.empty());
        }
    }

    return mapnik::make_invalid_featureset();
}

box2d<double> pgraster_datasource::envelope() const
{
    if (extent_initialized_)
    {
        return extent_;
    }

    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (!conn)
            return extent_;
        if (conn->isOK())
        {
            std::ostringstream s;

            std::string col = geometryColumn_;
            std::string sch = parsed_schema_;
            std::string tab = parsed_table_;

            if (!overviews_.empty())
            {
                // query from highest-factor overview instead
                const pgraster_overview& o = overviews_.back();
                sch = o.schema;
                tab = o.table;
                col = o.column;
            }

            if (col.empty())
            {
                std::ostringstream s_error;
                s_error << "PostGIS: unable to query the layer extent of table '";

                if (!sch.empty())
                {
                    s_error << sch << ".";
                }
                s_error << parsed_table_ << "' because we cannot determine the raster field name."
                        << "\nPlease provide either an 'extent' parameter to skip this query, "
                        << "a 'raster_field' and/or 'raster_table' parameter, or add "
                        << "standard constraints to your raster table.";

                throw mapnik::datasource_exception("Pgraster Plugin: " + s_error.str());
            }

            if (estimate_extent_)
            {
                if (tab.empty())
                {
                    std::ostringstream s_error;
                    s_error << "PostGIS: unable to query the layer extent as "
                            << "we couldn't determine the raster table name.\n"
                            << "Please provide either an 'extent' parameter to skip this query, "
                            << "a 'raster_table' parameter, or do not set 'estimate_extent'";
                    throw mapnik::datasource_exception("Pgraster Plugin: " + s_error.str());
                }
                s << "SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)"
                  << " FROM (SELECT ST_EstimatedExtent(";

                if (!sch.empty())
                {
                    s << literal(sch) << ',';
                }

                s << literal(tab) << ',' << literal(col) << ") as ext) as tmp";
            }
            else
            {
                s << "SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)"
                  << " FROM (SELECT ST_Extent(" << identifier(col) << "::geometry) as ext from ";

                if (extent_from_subquery_)
                {
                    // if a subselect limits records then calculating the extent upon the
                    // subquery will be faster and the bounds will be more accurate
                    s << populate_tokens(table_) << ") as tmpx";
                }
                else
                {
                    if (!sch.empty())
                    {
                        s << identifier(sch) << ".";
                    }

                    // but if the subquery does not limit records then querying the
                    // actual table will be faster as indexes can be used
                    s << identifier(tab) << ") as tmp";
                }
            }

            shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
            if (rs->next() && !rs->isNull(0))
            {
                double lox, loy, hix, hiy;
                if (mapnik::util::string2double(rs->getValue(0), lox) &&
                    mapnik::util::string2double(rs->getValue(1), loy) &&
                    mapnik::util::string2double(rs->getValue(2), hix) &&
                    mapnik::util::string2double(rs->getValue(3), hiy))
                {
                    extent_.init(lox, loy, hix, hiy);
                    extent_initialized_ = true;
                }
                else
                {
                    MAPNIK_LOG_DEBUG(pgraster)
                      << "pgraster_datasource: Could not determine extent from query: " << s.str();
                }
            }
            rs->close();
        }
    }

    return extent_;
}

boost::optional<mapnik::datasource_geometry_t> pgraster_datasource::get_geometry_type() const
{
    return boost::optional<mapnik::datasource_geometry_t>();
}
