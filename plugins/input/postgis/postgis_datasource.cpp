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

#include "connection_manager.hpp"
#include "postgis_datasource.hpp"
#include "postgis_featureset.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/global.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/timer.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>

// stl
#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include <iomanip>

DATASOURCE_PLUGIN(postgis_datasource)

const double postgis_datasource::FMAX = std::numeric_limits<double>::max();
const std::string postgis_datasource::GEOMETRY_COLUMNS = "geometry_columns";
const std::string postgis_datasource::SPATIAL_REF_SYS = "spatial_ref_system";

using boost::shared_ptr;
using mapnik::PoolGuard;
using mapnik::attribute_descriptor;

postgis_datasource::postgis_datasource(parameters const& params, bool bind)
    : datasource(params),
      table_(*params_.get<std::string>("table", "")),
      schema_(""),
      geometry_table_(*params_.get<std::string>("geometry_table", "")),
      geometry_field_(*params_.get<std::string>("geometry_field", "")),
      key_field_(*params_.get<std::string>("key_field", "")),
      cursor_fetch_size_(*params_.get<int>("cursor_size", 0)),
      row_limit_(*params_.get<int>("row_limit", 0)),
      type_(datasource::Vector),
      srid_(*params_.get<int>("srid", 0)),
      extent_initialized_(false),
      simplify_geometries_(false),
      desc_(*params_.get<std::string>("type"), "utf-8"),
      creator_(params.get<std::string>("host"),
               params.get<std::string>("port"),
               params.get<std::string>("dbname"),
               params.get<std::string>("user"),
               params.get<std::string>("password"),
               params.get<std::string>("connect_timeout", "4")),
      bbox_token_("!bbox!"),
      scale_denom_token_("!scale_denominator!"),
      pixel_width_token_("!pixel_width!"),
      pixel_height_token_("!pixel_height!"),
      persist_connection_(*params_.get<mapnik::boolean>("persist_connection", true)),
      extent_from_subquery_(*params_.get<mapnik::boolean>("extent_from_subquery", false)),
      // params below are for testing purposes only (will likely be removed at any time)
      intersect_min_scale_(*params_.get<int>("intersect_min_scale", 0)),
      intersect_max_scale_(*params_.get<int>("intersect_max_scale", 0))
{
    if (table_.empty())
    {
        throw mapnik::datasource_exception("Postgis Plugin: missing <table> parameter");
    }

    boost::optional<std::string> ext = params_.get<std::string>("extent");
    if (ext && !ext->empty())
    {
        extent_initialized_ = extent_.from_string(*ext);
    }

    if (bind)
    {
        this->bind();
    }
}

void postgis_datasource::bind() const
{
    if (is_bound_)
    {
        return;
    }

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "postgis_datasource::bind");
#endif

    boost::optional<int> initial_size = params_.get<int>("initial_size", 1);
    boost::optional<int> max_size = params_.get<int>("max_size", 10);
    boost::optional<mapnik::boolean> autodetect_key_field = params_.get<mapnik::boolean>("autodetect_key_field", false);

    boost::optional<mapnik::boolean> simplify_opt = params_.get<mapnik::boolean>("simplify_geometries", false);
    simplify_geometries_ = simplify_opt && *simplify_opt;

    ConnectionManager::instance().registerPool(creator_, *initial_size, *max_size);
    shared_ptr< Pool<Connection,ConnectionCreator> > pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (conn && conn->isOK())
        {
            PoolGuard<shared_ptr<Connection>,
                shared_ptr< Pool<Connection,ConnectionCreator> > > guard(conn, pool);

            desc_.set_encoding(conn->client_encoding());

            if (geometry_table_.empty())
            {
                geometry_table_ = mapnik::sql_utils::table_from_sql(table_);
            }

            std::string::size_type idx = geometry_table_.find_last_of('.');
            if (idx != std::string::npos)
            {
                schema_ = geometry_table_.substr(0, idx);
                geometry_table_ = geometry_table_.substr(idx + 1);
            }
            else
            {
                geometry_table_ = geometry_table_.substr(0);
            }

            // If we do not know both the geometry_field and the srid
            // then first attempt to fetch the geometry name from a geometry_columns entry.
            // This will return no records if we are querying a bogus table returned
            // from the simplistic table parsing in table_from_sql() or if
            // the table parameter references a table, view, or subselect not
            // registered in the geometry columns.
            geometryColumn_ = geometry_field_;
            if (geometryColumn_.empty() || srid_ == 0)
            {
#ifdef MAPNIK_STATS
                mapnik::progress_timer __stats2__(std::clog, "postgis_datasource::bind(get_srid_and_geometry_column)");
#endif

                std::ostringstream s;
                s << "SELECT f_geometry_column, srid FROM "
                  << GEOMETRY_COLUMNS <<" WHERE f_table_name='"
                  << mapnik::sql_utils::unquote_double(geometry_table_)
                  << "'";

                if (! schema_.empty())
                {
                    s << " AND f_table_schema='"
                      << mapnik::sql_utils::unquote_double(schema_)
                      << "'";
                }

                if (! geometry_field_.empty())
                {
                    s << " AND f_geometry_column='"
                      << mapnik::sql_utils::unquote_double(geometry_field_)
                      << "'";
                }

                shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                if (rs->next())
                {
                    geometryColumn_ = rs->getValue("f_geometry_column");

                    if (srid_ == 0)
                    {
                        const char* srid_c = rs->getValue("srid");
                        if (srid_c != NULL)
                        {
                            int result = 0;
                            if (mapnik::util::string2int(srid_c, result))
                            {
                                srid_ = result;
                            }
                        }
                    }
                }
                rs->close();

                // If we still do not know the srid then we can try to fetch
                // it from the 'table_' parameter, which should work even if it is
                // a subselect as long as we know the geometry_field to query
                if (! geometryColumn_.empty() && srid_ <= 0)
                {
                    s.str("");

                    s << "SELECT ST_SRID(\"" << geometryColumn_ << "\") AS srid FROM "
                      << populate_tokens(table_) << " WHERE \"" << geometryColumn_ << "\" IS NOT NULL LIMIT 1;";

                    shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                    if (rs->next())
                    {
                        const char* srid_c = rs->getValue("srid");
                        if (srid_c != NULL)
                        {
                            int result = 0;
                            if (mapnik::util::string2int(srid_c, result))
                            {
                                srid_ = result;
                            }
                        }
                    }
                    rs->close();
                }
            }

            // detect primary key
            if (*autodetect_key_field && key_field_.empty())
            {
#ifdef MAPNIK_STATS
                mapnik::progress_timer __stats2__(std::clog, "postgis_datasource::bind(get_primary_key)");
#endif

                std::ostringstream s;
                s << "SELECT a.attname, a.attnum, t.typname, t.typname in ('int2','int4','int8') "
                    "AS is_int FROM pg_class c, pg_attribute a, pg_type t, pg_namespace n, pg_index i "
                    "WHERE a.attnum > 0 AND a.attrelid = c.oid "
                    "AND a.atttypid = t.oid AND c.relnamespace = n.oid "
                    "AND c.oid = i.indrelid AND i.indisprimary = 't' "
                    "AND t.typname !~ '^geom' AND c.relname ="
                  << " '" << mapnik::sql_utils::unquote_double(geometry_table_) << "' "
                    //"AND a.attnum = ANY (i.indkey) " // postgres >= 8.1
                  << "AND (i.indkey[0]=a.attnum OR i.indkey[1]=a.attnum OR i.indkey[2]=a.attnum "
                    "OR i.indkey[3]=a.attnum OR i.indkey[4]=a.attnum OR i.indkey[5]=a.attnum "
                    "OR i.indkey[6]=a.attnum OR i.indkey[7]=a.attnum OR i.indkey[8]=a.attnum "
                    "OR i.indkey[9]=a.attnum) ";
                if (! schema_.empty())
                {
                    s << "AND n.nspname='"
                      << mapnik::sql_utils::unquote_double(schema_)
                      << "' ";
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

                                MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: auto-detected key field of '"
                                                          << key_field_ << "' on table '" << geometry_table_ << "'";
                            }
                        }
                        else
                        {
                            // throw for cases like a numeric primary key, which is invalid
                            // as it should be floating point (int numerics are useless)
                            std::ostringstream err;
                            err << "PostGIS Plugin: Error: '"
                                << rs_key->getValue(0)
                                << "' on table '"
                                << geometry_table_
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
                throw mapnik::datasource_exception(std::string("PostGIS Plugin: Error: primary key required")
                                                   + " but could not be detected for table '" +
                                                   geometry_table_ + "', please supply 'key_field' option to specify field to use for primary key");
            }

            if (srid_ == 0)
            {
                srid_ = -1;

                MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: Table " << table_ << " is using SRID=" << srid_;
            }

            // At this point the geometry_field may still not be known
            // but we'll catch that where more useful...
            MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: Using SRID=" << srid_;
            MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: Using geometry_column=" << geometryColumn_;

            // collect attribute desc
#ifdef MAPNIK_STATS
            mapnik::progress_timer __stats2__(std::clog, "postgis_datasource::bind(get_column_description)");
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
                if (! found_key_field && ! key_field_.empty() && fld_name == key_field_)
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
                            error_s << rs_oid->getValue("typname")
                                    << "' (oid:" << rs_oid->getValue("oid") << ")";
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
                    case 16:    // bool
                        desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Boolean));
                        break;
                    case 20:    // int8
                    case 21:    // int2
                    case 23:    // int4
                        desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Integer));
                        break;
                    case 700:   // float4
                    case 701:   // float8
                    case 1700:  // numeric
                        desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Double));
                        break;
                    case 1042:  // bpchar
                    case 1043:  // varchar
                    case 25:    // text
                    case 705:   // literal
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
                            if (typname != "geometry")
                            {
                                MAPNIK_LOG_WARN(postgis) << "postgis_datasource: Unknown type=" << typname
                                                         << " (oid:" << rs_oid->getValue("oid") << ")";
                            }
                        }
                        else
                        {
                            MAPNIK_LOG_WARN(postgis) << "postgis_datasource: Unknown type_oid=" << type_oid;
                        }
                        rs_oid->close();
#endif
                        break;
                    }
                }
            }

            rs->close();

            is_bound_ = true;
        }
    }
}

postgis_datasource::~postgis_datasource()
{
    if (is_bound_ && ! persist_connection_)
    {
        shared_ptr< Pool<Connection,ConnectionCreator> > pool = ConnectionManager::instance().getPool(creator_.id());
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

const char * postgis_datasource::name()
{
    return "postgis";
}

mapnik::datasource::datasource_t postgis_datasource::type() const
{
    return type_;
}

layer_descriptor postgis_datasource::get_descriptor() const
{
    if (! is_bound_)
    {
        bind();
    }

    return desc_;
}

std::string postgis_datasource::sql_bbox(box2d<double> const& env) const
{
    std::ostringstream b;

    if (srid_ > 0)
    {
        b << "ST_SetSRID(";
    }

    b << "'BOX3D(";
    b << std::setprecision(16);
    b << env.minx() << " " << env.miny() << ",";
    b << env.maxx() << " " << env.maxy() << ")'::box3d";

    if (srid_ > 0)
    {
        b << ", " << srid_ << ")";
    }

    return b.str();
}

std::string postgis_datasource::populate_tokens(std::string const& sql) const
{
    std::string populated_sql = sql;

    if (boost::algorithm::icontains(sql, bbox_token_))
    {
        box2d<double> max_env(-1.0 * FMAX, -1.0 * FMAX, FMAX, FMAX);
        const std::string max_box = sql_bbox(max_env);
        boost::algorithm::replace_all(populated_sql, bbox_token_, max_box);
    }

    if (boost::algorithm::icontains(sql, scale_denom_token_))
    {
        std::ostringstream ss;
        ss << FMAX;
        boost::algorithm::replace_all(populated_sql, scale_denom_token_, ss.str());
    }

    if (boost::algorithm::icontains(sql, pixel_width_token_))
    {
        std::ostringstream ss;
        ss << 0;
        boost::algorithm::replace_all(populated_sql, pixel_width_token_, ss.str());
    }

    if (boost::algorithm::icontains(sql, pixel_height_token_))
    {
        std::ostringstream ss;
        ss << 0;
        boost::algorithm::replace_all(populated_sql, pixel_height_token_, ss.str());
    }

    return populated_sql;
}

std::string postgis_datasource::populate_tokens(std::string const& sql, double scale_denom, box2d<double> const& env, double pixel_width, double pixel_height) const
{
    std::string populated_sql = sql;
    std::string box = sql_bbox(env);

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
        boost::algorithm::replace_all(populated_sql, bbox_token_, box);
        return populated_sql;
    }
    else
    {
        std::ostringstream s;

        if (intersect_min_scale_ > 0 && (scale_denom <= intersect_min_scale_))
        {
            s << " WHERE ST_Intersects(\"" << geometryColumn_ << "\"," << box << ")";
        }
        else if (intersect_max_scale_ > 0 && (scale_denom >= intersect_max_scale_))
        {
            // do no bbox restriction
        }
        else
        {
            s << " WHERE \"" << geometryColumn_ << "\" && " << box;
        }

        return populated_sql + s.str();
    }
}


boost::shared_ptr<IResultSet> postgis_datasource::get_resultset(boost::shared_ptr<Connection> const &conn, std::string const& sql) const
{
    if (cursor_fetch_size_ > 0)
    {
        // cursor
        std::ostringstream csql;
        std::string cursor_name = conn->new_cursor_name();

        csql << "DECLARE " << cursor_name << " BINARY INSENSITIVE NO SCROLL CURSOR WITH HOLD FOR " << sql << " FOR READ ONLY";

        if (! conn->execute(csql.str()))
        {
            // TODO - better error
            throw mapnik::datasource_exception("Postgis Plugin: error creating cursor for data select." );
        }

        return boost::make_shared<CursorResultSet>(conn, cursor_name, cursor_fetch_size_);

    }
    else
    {
        // no cursor
        return conn->executeQuery(sql, 1);
    }
}

featureset_ptr postgis_datasource::features(const query& q) const
{
    if (! is_bound_)
    {
        bind();
    }

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "postgis_datasource::features");
#endif

    box2d<double> const& box = q.get_bbox();
    double scale_denom = q.scale_denominator();

    shared_ptr< Pool<Connection,ConnectionCreator> > pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (conn && conn->isOK())
        {
            PoolGuard<shared_ptr<Connection>, shared_ptr< Pool<Connection,ConnectionCreator> > > guard(conn ,pool);

            if (geometryColumn_.empty())
            {
                std::ostringstream s_error;
                s_error << "PostGIS: geometry name lookup failed for table '";

                if (! schema_.empty())
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_
                        << "'. Please manually provide the 'geometry_field' parameter or add an entry "
                        << "in the geometry_columns for '";

                if (! schema_.empty())
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_ << "'.";

                throw mapnik::datasource_exception(s_error.str());
            }

            std::ostringstream s;

            const double px_gw = 1.0 / boost::get<0>(q.resolution());
            const double px_gh = 1.0 / boost::get<1>(q.resolution());

            s << "SELECT ST_AsBinary(";

            if (simplify_geometries_) {
              s << "ST_Simplify(";
            }

            s << "\"" << geometryColumn_ << "\"";

            if (simplify_geometries_) {
              const double tolerance = std::min(px_gw, px_gh) / 2.0;
              s << ", " << tolerance << ")";
            }

            s << ") AS geom";

            mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();
            std::set<std::string> const& props = q.property_names();
            std::set<std::string>::const_iterator pos = props.begin();
            std::set<std::string>::const_iterator end = props.end();

            if (! key_field_.empty())
            {
                mapnik::sql_utils::quote_attr(s, key_field_);
                ctx->push(key_field_);

                for (; pos != end; ++pos)
                {
                    if (*pos != key_field_)
                    {
                        mapnik::sql_utils::quote_attr(s, *pos);
                        ctx->push(*pos);
                    }
                }
            }
            else
            {
                for (; pos != end; ++pos)
                {
                    mapnik::sql_utils::quote_attr(s, *pos);
                    ctx->push(*pos);
                }
            }

            std::string table_with_bbox = populate_tokens(table_, scale_denom, box, px_gw, px_gh);

            s << " FROM " << table_with_bbox;

            if (row_limit_ > 0)
            {
                s << " LIMIT " << row_limit_;
            }

            boost::shared_ptr<IResultSet> rs = get_resultset(conn, s.str());
            return boost::make_shared<postgis_featureset>(rs, ctx, desc_.get_encoding(), !key_field_.empty());
        }
        else
        {
            throw mapnik::datasource_exception("Postgis Plugin: bad connection");
        }
    }

    return featureset_ptr();
}

featureset_ptr postgis_datasource::features_at_point(coord2d const& pt, double tol) const
{
    if (! is_bound_)
    {
        bind();
    }

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "postgis_datasource::features_at_point");
#endif
    shared_ptr< Pool<Connection,ConnectionCreator> > pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (conn && conn->isOK())
        {
            PoolGuard<shared_ptr<Connection>, shared_ptr< Pool<Connection,ConnectionCreator> > > guard(conn, pool);

            if (geometryColumn_.empty())
            {
                std::ostringstream s_error;
                s_error << "PostGIS: geometry name lookup failed for table '";

                if (! schema_.empty())
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_
                        << "'. Please manually provide the 'geometry_field' parameter or add an entry "
                        << "in the geometry_columns for '";

                if (! schema_.empty())
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_ << "'.";

                throw mapnik::datasource_exception(s_error.str());
            }

            std::ostringstream s;
            s << "SELECT ST_AsBinary(\"" << geometryColumn_ << "\") AS geom";

            mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();
            std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
            std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();

            if (! key_field_.empty())
            {
                mapnik::sql_utils::quote_attr(s, key_field_);
                ctx->push(key_field_);
                for (; itr != end; ++itr)
                {
                    if (itr->get_name() != key_field_)
                    {
                        mapnik::sql_utils::quote_attr(s, itr->get_name());
                        ctx->push(itr->get_name());
                    }
                }
            }
            else
            {
                for (; itr != end; ++itr)
                {
                    mapnik::sql_utils::quote_attr(s, itr->get_name());
                    ctx->push(itr->get_name());
                }
            }

            box2d<double> box(pt.x - tol, pt.y - tol, pt.x + tol, pt.y + tol);
            std::string table_with_bbox = populate_tokens(table_, FMAX, box, 0, 0);

            s << " FROM " << table_with_bbox;

            if (row_limit_ > 0)
            {
                s << " LIMIT " << row_limit_;
            }

            boost::shared_ptr<IResultSet> rs = get_resultset(conn, s.str());
            return boost::make_shared<postgis_featureset>(rs, ctx, desc_.get_encoding(), !key_field_.empty());
        }
    }

    return featureset_ptr();
}

box2d<double> postgis_datasource::envelope() const
{
    if (extent_initialized_)
    {
        return extent_;
    }

    if (! is_bound_)
    {
        bind();
    }

    shared_ptr< Pool<Connection,ConnectionCreator> > pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (conn && conn->isOK())
        {
            PoolGuard<shared_ptr<Connection>, shared_ptr< Pool<Connection,ConnectionCreator> > > guard(conn, pool);

            std::ostringstream s;

            boost::optional<mapnik::boolean> estimate_extent =
                params_.get<mapnik::boolean>("estimate_extent", false);

            if (geometryColumn_.empty())
            {
                std::ostringstream s_error;
                s_error << "PostGIS: unable to query the layer extent of table '";

                if (! schema_.empty())
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_ << "' because we cannot determine the geometry field name."
                        << "\nPlease provide either an 'extent' parameter to skip this query, "
                        << "a 'geometry_field' and/or 'geometry_table' parameter, or add a "
                        << "record to the 'geometry_columns' for your table.";

                throw mapnik::datasource_exception("Postgis Plugin: " + s_error.str());
            }

            if (estimate_extent && *estimate_extent)
            {
                s << "SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)"
                  << " FROM (SELECT ST_Estimated_Extent('";

                if (! schema_.empty())
                {
                    s << mapnik::sql_utils::unquote_double(schema_) << "','";
                }

                s << mapnik::sql_utils::unquote_double(geometry_table_) << "','"
                  << mapnik::sql_utils::unquote_double(geometryColumn_) << "') as ext) as tmp";
            }
            else
            {
                s << "SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)"
                  << " FROM (SELECT ST_Extent(" <<geometryColumn_<< ") as ext from ";

                if (extent_from_subquery_)
                {
                    // if a subselect limits records then calculating the extent upon the
                    // subquery will be faster and the bounds will be more accurate
                    s << populate_tokens(table_) << ") as tmp";
                }
                else
                {
                    if (! schema_.empty())
                    {
                        s << schema_ << ".";
                    }

                    // but if the subquery does not limit records then querying the
                    // actual table will be faster as indexes can be used
                    s << geometry_table_ << ") as tmp";
                }
            }

            shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
            if (rs->next() && ! rs->isNull(0))
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
                    MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: Could not determine extent from query: " << s.str();
                }
            }
            rs->close();
        }
    }

    return extent_;
}

boost::optional<mapnik::datasource::geometry_t> postgis_datasource::get_geometry_type() const
{
    if (! is_bound_)
    {
        bind();
    }

    boost::optional<mapnik::datasource::geometry_t> result;

    shared_ptr< Pool<Connection,ConnectionCreator> > pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (conn && conn->isOK())
        {
            PoolGuard<shared_ptr<Connection>, shared_ptr< Pool<Connection,ConnectionCreator> > > guard(conn, pool);

            std::ostringstream s;
            std::string g_type;

            s << "SELECT lower(type) as type FROM "
              << GEOMETRY_COLUMNS <<" WHERE f_table_name='"
              << mapnik::sql_utils::unquote_double(geometry_table_)
              << "'";

            if (! schema_.empty())
            {
                s << " AND f_table_schema='"
                  << mapnik::sql_utils::unquote_double(schema_)
                  << "'";
            }

            if (! geometry_field_.empty())
            {
                s << " AND f_geometry_column='"
                  << mapnik::sql_utils::unquote_double(geometry_field_)
                  << "'";
            }

            shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
            if (rs->next())
            {
                g_type = rs->getValue("type");
                if (boost::algorithm::contains(g_type, "line"))
                {
                    result.reset(mapnik::datasource::LineString);
                    return result;
                }
                else if (boost::algorithm::contains(g_type, "point"))
                {
                    result.reset(mapnik::datasource::Point);
                    return result;
                }
                else if (boost::algorithm::contains(g_type, "polygon"))
                {
                    result.reset(mapnik::datasource::Polygon);
                    return result;
                }
                else // geometry
                {
                    result.reset(mapnik::datasource::Collection);
                    return result;
                }
            }

            // fallback to querying first several features
            if (g_type.empty() && ! geometryColumn_.empty())
            {
                s.str("");

                std::string prev_type("");

                s << "SELECT ST_GeometryType(\""
                  << geometryColumn_ << "\") AS geom"
                  << " FROM " << populate_tokens(table_);

                if (row_limit_ > 0 && row_limit_ < 5)
                {
                    s << " LIMIT " << row_limit_;
                }
                else
                {
                    s << " LIMIT 5";
                }

                shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                while (rs->next() && ! rs->isNull(0))
                {
                    const char* data = rs->getValue(0);
                    if (boost::algorithm::icontains(data, "line"))
                    {
                        g_type = "linestring";
                        result.reset(mapnik::datasource::LineString);
                    }
                    else if (boost::algorithm::icontains(data, "point"))
                    {
                        g_type = "point";
                        result.reset(mapnik::datasource::Point);
                    }
                    else if (boost::algorithm::icontains(data, "polygon"))
                    {
                        g_type = "polygon";
                        result.reset(mapnik::datasource::Polygon);
                    }
                    else // geometry
                    {
                        result.reset(mapnik::datasource::Collection);
                        return result;
                    }
                    if (! prev_type.empty() && g_type != prev_type)
                    {
                        result.reset(mapnik::datasource::Collection);
                        return result;
                    }
                    prev_type = g_type;
                }
            }
        }
    }

    return result;
}
