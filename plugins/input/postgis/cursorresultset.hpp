#ifndef CURSORRESULTSET_HPP
#define CURSORRESULTSET_HPP

#include "connection.hpp"
#include "resultset.hpp"

class CursorResultSet : public IResultSet
{
private:
    boost::shared_ptr<Connection> conn_;
    std::string cursorName_;
    boost::shared_ptr<ResultSet> rs_;
    int fetch_size_;
    bool is_closed_;
    int *refCount_;
    
    void getNextResultSet() {
        std::ostringstream s;
        s << "FETCH FORWARD " << fetch_size_ << " FROM " << cursorName_;
#ifdef MAPNIK_DEBUG
        std::clog << s.str() << "\n";
#endif
        rs_ = conn_->executeQuery(s.str());
        is_closed_ = false;
#ifdef MAPNIK_DEBUG
        std::clog << "FETCH result (" << cursorName_ << "): " << rs_->size() << " rows\n";
#endif
    }
    
public:
    CursorResultSet(boost::shared_ptr<Connection> const &conn, std::string cursorName, int fetch_count)
    :conn_(conn),cursorName_(cursorName),fetch_size_(fetch_count),is_closed_(false),refCount_(new int(1))
    {
        getNextResultSet();
    }

    CursorResultSet(const CursorResultSet& rhs)
    :conn_(rhs.conn_),cursorName_(rhs.cursorName_),rs_(rhs.rs_),fetch_size_(rhs.fetch_size_),is_closed_(rhs.is_closed_),
    refCount_(rhs.refCount_)
    {
        (*refCount_)++;
    }

    CursorResultSet& operator=(const CursorResultSet& rhs)
    {
        if (this==&rhs) return *this;
        if (--(refCount_)==0)
        {
            close();
            delete refCount_,refCount_=0;
        }
        conn_=rhs.conn_;
        cursorName_=rhs.cursorName_;
        rs_=rhs.rs_;
        refCount_=rhs.refCount_;
        fetch_size_=rhs.fetch_size_;
        is_closed_ = false;
        (*refCount_)++;
        return *this;
    }

    virtual void close()
    {
        if (!is_closed_) {
            rs_.reset();
            std::ostringstream s;
            s << "CLOSE " << cursorName_;
#ifdef MAPNIK_DEBUG
            std::clog << s.str() << "\n";
#endif
            conn_->execute(s.str());
            is_closed_ = true;
        }
    }

    virtual ~CursorResultSet()
    {
        if (--(*refCount_)==0)
        {
            close();
            delete refCount_,refCount_=0;
        }
    }

    virtual int getNumFields() const
    {
        return rs_->getNumFields();
    }

    virtual bool next()
    {
        if (rs_->next()) {
            return true;
        } else if (rs_->size() == 0) {
            return false;
        } else {
            getNextResultSet();
            return rs_->next();
        }
    }

    virtual const char* getFieldName(int index) const
    {
        return rs_->getFieldName(index);
    }

    virtual int getFieldLength(int index) const
    {
        return rs_->getFieldLength(index);
    }

    virtual int getFieldLength(const char* name) const
    {
        return rs_->getFieldLength(name);
    }

    virtual int getTypeOID(int index) const
    {
        return rs_->getTypeOID(index);
    }

    virtual int getTypeOID(const char* name) const
    {
        return rs_->getTypeOID(name);
    }

    virtual bool isNull(int index) const
    {
        return rs_->isNull(index);
    }
    
    virtual const char* getValue(int index) const
    {
        return rs_->getValue(index);
    }

    virtual const char* getValue(const char* name) const
    {
        return rs_->getValue(name);
    }
};

#endif //CURSORRESULTSET_HPP
