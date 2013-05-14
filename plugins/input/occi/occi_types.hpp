/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software, you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library, if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef OCCI_TYPES_HPP
#define OCCI_TYPES_HPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/pool.hpp>
#include <mapnik/utils.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>

#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

// stl
#include <string>
#include <sstream>

// occi
#include <occi.h>

// ott generated SDOGeometry classes
#include "spatial_classesh.h"
#include "spatial_classesm.h"

// check for oracle support
#if OCCI_MAJOR_VERSION >= 10 && OCCI_MINOR_VERSION >= 1
//     We have at least ORACLE 10g >= 10.2.0.X
#else
#error Only ORACLE 10g >= 10.2.0.X is supported !
#endif

using mapnik::Pool;
using mapnik::singleton;
using mapnik::CreateStatic;

// geometry types definitions
enum
{
    SDO_GTYPE_UNKNOWN                   = 0,
    SDO_GTYPE_POINT                     = 1,
    SDO_GTYPE_LINE                      = 2,
    SDO_GTYPE_POLYGON                   = 3,
    SDO_GTYPE_COLLECTION                = 4,
    SDO_GTYPE_MULTIPOINT                = 5,
    SDO_GTYPE_MULTILINE                 = 6,
    SDO_GTYPE_MULTIPOLYGON              = 7,

    SDO_GTYPE_2DPOINT                   = 2001,
    SDO_GTYPE_2DLINE                    = 2002,
    SDO_GTYPE_2DPOLYGON                 = 2003,
    SDO_GTYPE_2DMULTIPOINT              = 2005,
    SDO_GTYPE_2DMULTILINE               = 2006,
    SDO_GTYPE_2DMULTIPOLYGON            = 2007,

    SDO_ELEM_INFO_SIZE                  = 3,

    SDO_ETYPE_UNKNOWN                   = 0,
    SDO_ETYPE_POINT                     = 1,
    SDO_ETYPE_LINESTRING                = 2,
    SDO_ETYPE_POLYGON                   = 1003,
    SDO_ETYPE_POLYGON_INTERIOR          = 2003,
    SDO_ETYPE_COMPOUND_LINESTRING       = 4,
    SDO_ETYPE_COMPOUND_POLYGON          = 1005,
    SDO_ETYPE_COMPOUND_POLYGON_INTERIOR = 2005,

    SDO_INTERPRETATION_POINT            = 1,
    SDO_INTERPRETATION_RECTANGLE        = 3,
    SDO_INTERPRETATION_CIRCLE           = 4,
    SDO_INTERPRETATION_STRAIGHT         = 1,
    SDO_INTERPRETATION_CIRCULAR         = 2
};

class Environment : public mapnik::singleton<Environment, mapnik::CreateStatic>
{
    friend class mapnik::CreateStatic<Environment>;

public:

    oracle::occi::Environment* get_environment()
    {
        return env_;
    }

    std::string resolve_gtype(int gtype)
    {
        switch (gtype)
        {
        case SDO_GTYPE_UNKNOWN:        return "SDO_GTYPE_UNKNOWN";
        case SDO_GTYPE_POINT:          return "SDO_GTYPE_POINT";
        case SDO_GTYPE_LINE:           return "SDO_GTYPE_LINE";
        case SDO_GTYPE_POLYGON:        return "SDO_GTYPE_POLYGON";
        case SDO_GTYPE_MULTIPOINT:     return "SDO_GTYPE_MULTIPOINT";
        case SDO_GTYPE_MULTILINE:      return "SDO_GTYPE_MULTILINE";
        case SDO_GTYPE_MULTIPOLYGON:   return "SDO_GTYPE_MULTIPOLYGON";
        case SDO_GTYPE_COLLECTION:     return "SDO_GTYPE_COLLECTION";
        default:                       return "<unknown SDO_GTYPE>";
        }
    }

    std::string resolve_etype(int etype)
    {
        switch (etype)
        {
        case SDO_ETYPE_UNKNOWN:                   return "SDO_ETYPE_UNKNOWN";
        case SDO_ETYPE_POINT:                     return "SDO_ETYPE_POINT";
        case SDO_ETYPE_LINESTRING:                return "SDO_ETYPE_LINESTRING";
        case SDO_ETYPE_POLYGON:                   return "SDO_ETYPE_POLYGON";
        case SDO_ETYPE_POLYGON_INTERIOR:          return "SDO_ETYPE_POLYGON_INTERIOR";
        case SDO_ETYPE_COMPOUND_LINESTRING:       return "SDO_ETYPE_COMPOUND_LINESTRING";
        case SDO_ETYPE_COMPOUND_POLYGON:          return "SDO_ETYPE_COMPOUND_POLYGON";
        case SDO_ETYPE_COMPOUND_POLYGON_INTERIOR: return "SDO_ETYPE_COMPOUND_POLYGON_INTERIOR";
        default:                                  return "<unknown SDO_ETYPE>";
        }
    }

    std::string resolve_datatype(int type_id)
    {
        switch (type_id)
        {
        case oracle::occi::OCCIINT:                 return "OCCIINT";
        case oracle::occi::OCCIUNSIGNED_INT:        return "OCCIUNSIGNED_INT";
        case oracle::occi::OCCIFLOAT:               return "OCCIFLOAT";
        case oracle::occi::OCCIBFLOAT:              return "OCCIBFLOAT";
        case oracle::occi::OCCIDOUBLE:              return "OCCIDOUBLE";
        case oracle::occi::OCCIBDOUBLE:             return "OCCIBDOUBLE";
        case oracle::occi::OCCINUMBER:              return "OCCINUMBER";
        case oracle::occi::OCCI_SQLT_NUM:           return "OCCI_SQLT_NUM";
        case oracle::occi::OCCICHAR:                return "OCCICHAR";
        case oracle::occi::OCCISTRING:              return "OCCISTRING";
        case oracle::occi::OCCI_SQLT_AFC:           return "OCCI_SQLT_AFC";
        case oracle::occi::OCCI_SQLT_AVC:           return "OCCI_SQLT_AVC";
        case oracle::occi::OCCI_SQLT_CHR:           return "OCCI_SQLT_CHR";
        case oracle::occi::OCCI_SQLT_LVC:           return "OCCI_SQLT_LVC";
        case oracle::occi::OCCI_SQLT_LNG:           return "OCCI_SQLT_LNG";
        case oracle::occi::OCCI_SQLT_STR:           return "OCCI_SQLT_STR";
        case oracle::occi::OCCI_SQLT_VCS:           return "OCCI_SQLT_VCS";
        case oracle::occi::OCCI_SQLT_VNU:           return "OCCI_SQLT_VNU";
        case oracle::occi::OCCI_SQLT_VBI:           return "OCCI_SQLT_VBI";
        case oracle::occi::OCCI_SQLT_VST:           return "OCCI_SQLT_VST";
        case oracle::occi::OCCI_SQLT_RDD:           return "OCCI_SQLT_RDD";
        case oracle::occi::OCCIDATE:                return "OCCIDATE";
        case oracle::occi::OCCITIMESTAMP:           return "OCCITIMESTAMP";
        case oracle::occi::OCCI_SQLT_DAT:           return "OCCI_SQLT_DAT";
        case oracle::occi::OCCI_SQLT_TIMESTAMP:     return "OCCI_SQLT_TIMESTAMP";
        case oracle::occi::OCCI_SQLT_TIMESTAMP_LTZ: return "OCCI_SQLT_TIMESTAMP_LTZ";
        case oracle::occi::OCCI_SQLT_TIMESTAMP_TZ:  return "OCCI_SQLT_TIMESTAMP_TZ";
        case oracle::occi::OCCIPOBJECT:             return "OCCIPOBJECT";
        default:                                    return "<unknown ATTR_DATA_TYPE>";
        }
    }

private:

    Environment()
        : env_(0)
    {
        MAPNIK_LOG_DEBUG(occi) << "occi_environment: constructor";

        env_ = oracle::occi::Environment::createEnvironment(
            (oracle::occi::Environment::Mode)(oracle::occi::Environment::OBJECT
                                              | oracle::occi::Environment::THREADED_MUTEXED));
        RegisterClasses(env_);
    }

    ~Environment()
    {
        MAPNIK_LOG_DEBUG(occi) << "occi_environment: destructor";

        oracle::occi::Environment::terminateEnvironment(env_);
        env_ = 0;
    }

    oracle::occi::Environment* env_;
};

class ResultSet
{
public:
    ResultSet(oracle::occi::Connection* conn,
              std::string const& s,
              const unsigned prefetch = 0)
        : conn_(conn),
          stmt_(NULL),
          rs_(NULL),
          metadata_queried_(false)
    {
        stmt_ = conn_->createStatement(s);

        if (prefetch > 0)
        {
            stmt_->setPrefetchMemorySize(0);
            stmt_->setPrefetchRowCount(prefetch);
        }

        rs_ = stmt_->executeQuery();
    }

    virtual ~ResultSet()
    {
        close();
    }

    inline void close()
    {
        if (stmt_)
        {
            if (rs_)
            {
                stmt_->closeResultSet(rs_);
                rs_ = NULL;
            }

            conn_->terminateStatement(stmt_);
            stmt_ = NULL;
        }
    }

    inline bool next()
    {
        return rs_ && rs_->next() == oracle::occi::ResultSet::DATA_AVAILABLE;
    }

    inline int getNumFields()
    {
        query_metadata();

        return metadata_.size();
    }

    inline const char* getFieldName(int index)
    {
        query_metadata();

        if (index >= 0 && (size_t)index < metadata_.size())
        {
            return metadata_[index].getString(oracle::occi::MetaData::ATTR_NAME).c_str();
        }

        return NULL;
    }

    inline int getTypeOID(int index)
    {
        query_metadata();

        if (index >= 0 && (size_t)index < metadata_.size())
        {
            return metadata_[index].getInt(oracle::occi::MetaData::ATTR_DATA_TYPE);
        }

        return 0;
    }

    inline bool isNull(int index) const
    {
        return rs_->isNull(index);
    }

    inline int getInt(int index)
    {
        return rs_->getInt(index);
    }

    inline float getFloat(int index)
    {
        return rs_->getFloat(index);
    }

    inline double getDouble(int index)
    {
        return rs_->getDouble(index);
    }

    inline oracle::occi::PObject* getObject(int index)
    {
        return rs_->getObject(index);
    }

    inline const char* getString(int index)
    {
        return rs_->getString(index).c_str();
    }

    inline unsigned int getBlob(int index, std::vector<char>& buffer)
    {
        if (! rs_->isNull(index))
        {
            oracle::occi::Blob blob = rs_->getBlob(1);
            blob.open(oracle::occi::OCCI_LOB_READONLY);

            unsigned int size = blob.length();
            if (buffer.size() < size)
            {
                buffer.resize(size);
            }

            oracle::occi::Stream* instream = blob.getStream(1, 0);
            instream->readBuffer(buffer.data(), size);
            blob.closeStream(instream);
            blob.close();

            return size;
        }

        return 0;
    }

private:

    inline void query_metadata()
    {
        if (! metadata_queried_ && rs_)
        {
            metadata_ = rs_->getColumnListMetaData();
            metadata_queried_ = true;
        }
    }

    oracle::occi::Connection* conn_;
    oracle::occi::Statement* stmt_;
    oracle::occi::ResultSet* rs_;

    std::vector<oracle::occi::MetaData> metadata_;
    bool metadata_queried_;

    ResultSet(const ResultSet&);
    ResultSet& operator=(const ResultSet);
};

class Connection
{
public:
    explicit Connection(std::string const& user,
                        std::string const& pass,
                        std::string const& host)
        : conn_(NULL)
    {
        MAPNIK_LOG_DEBUG(occi) << "occi_environment: create_connection";

        conn_ = Environment::instance().get_environment()->createConnection(user, pass, host);
    }

    ~Connection()
    {
        close();
    }

    boost::shared_ptr<ResultSet> execute_query(std::string const& s, const unsigned prefetch = 0)
    {
        MAPNIK_LOG_DEBUG(occi) << "occi_connection_ptr: " << s;

        return boost::make_shared<ResultSet>(conn_, s, prefetch);
    }

    bool isOK()
    {
        return (conn_ != NULL);
    }

    void close()
    {
        if (conn_)
        {
            MAPNIK_LOG_DEBUG(occi) << "occi_environment: destroy_connection";

            Environment::instance().get_environment()->terminateConnection(conn_);

            conn_ = NULL;
        }
    }

private:

    oracle::occi::Connection* conn_;
    oracle::occi::Statement* stmt_;
    oracle::occi::ResultSet* rs_;

    Connection(const Connection&);
    Connection& operator=(const Connection);
};

template <typename T>
class ConnectionCreator
{

public:
    ConnectionCreator(boost::optional<std::string> const& user,
                      boost::optional<std::string> const& pass,
                      boost::optional<std::string> const& host)
        : user_(user),
          pass_(pass),
          host_(host)
    {}

    T* operator()() const
    {
        return new T(*user_, *pass_, *host_);
    }

    inline std::string id() const
    {
        return connection_string();
    }

    inline std::string connection_string() const
    {
        std::string connect_str = connection_string_safe();
        if (pass_   && !pass_->empty()) connect_str += " password=" + *pass_;
        return connect_str;
    }

    inline std::string connection_string_safe() const
    {
        std::string connect_str;
        if (host_   && !host_->empty()) connect_str += "host=" + *host_;
        if (user_   && !user_->empty()) connect_str += " user=" + *user_;
        return connect_str;
    }

private:
    boost::optional<std::string> user_;
    boost::optional<std::string> pass_;
    boost::optional<std::string> host_;
};

class ConnectionManager : public singleton <ConnectionManager,CreateStatic>
{

    friend class CreateStatic<ConnectionManager>;
    typedef Pool<Connection,ConnectionCreator> PoolType;
    typedef std::map<std::string,boost::shared_ptr<PoolType> > ContType;
    typedef boost::shared_ptr<Connection> HolderType;
    ContType pools_;

public:

    bool registerPool(const ConnectionCreator<Connection>& creator,unsigned initialSize,unsigned maxSize)
    {
        ContType::const_iterator itr = pools_.find(creator.id());

        if (itr != pools_.end())
        {
            itr->second->set_initial_size(initialSize);
            itr->second->set_max_size(maxSize);
        }
        else
        {
            return pools_.insert(
                std::make_pair(creator.id(),
                               boost::make_shared<PoolType>(creator,initialSize,maxSize))).second;
        }
        return false;

    }

    boost::shared_ptr<PoolType> getPool(std::string const& key)
    {
        ContType::const_iterator itr=pools_.find(key);
        if (itr!=pools_.end())
        {
            return itr->second;
        }
        static const boost::shared_ptr<PoolType> emptyPool;
        return emptyPool;
    }

    ConnectionManager() {}
private:
    ConnectionManager(const ConnectionManager&);
    ConnectionManager& operator=(const ConnectionManager);
};


#endif // OCCI_TYPES_HPP
