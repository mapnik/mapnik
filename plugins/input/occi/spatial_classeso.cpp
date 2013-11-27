#ifndef SPATIAL_CLASSESH_ORACLE
# include "spatial_classesh.h"
#endif


/*****************************************************************/
//  generated method implementations for the SDO_POINT_TYPE object type.
/*****************************************************************/

oracle::occi::Number SDOPointType::getX() const
{
    return X;
}

void SDOPointType::setX(const oracle::occi::Number &value)
{
    X = value;
}

oracle::occi::Number SDOPointType::getY() const
{
    return Y;
}

void SDOPointType::setY(const oracle::occi::Number &value)
{
    Y = value;
}

oracle::occi::Number SDOPointType::getZ() const
{
    return Z;
}

void SDOPointType::setZ(const oracle::occi::Number &value)
{
    Z = value;
}

void *SDOPointType::operator new(size_t size)
{
    return oracle::occi::PObject::operator new(size);
}

void *SDOPointType::operator new(size_t size, const oracle::occi::Connection * sess,
                                 const OCCI_STD_NAMESPACE::string& table)
{
    return oracle::occi::PObject::operator new(size, sess, table,
                                               (char *) "MDSYS.SDO_POINT_TYPE");
}

void *SDOPointType::operator new(size_t size, void *ctxOCCI_)
{
    return oracle::occi::PObject::operator new(size, ctxOCCI_);
}

void *SDOPointType::operator new(size_t size,
                                 const oracle::occi::Connection *sess,
                                 const OCCI_STD_NAMESPACE::string &tableName,
                                 const OCCI_STD_NAMESPACE::string &typeName,
                                 const OCCI_STD_NAMESPACE::string &tableSchema,
                                 const OCCI_STD_NAMESPACE::string &typeSchema)
{
    return oracle::occi::PObject::operator new(size, sess, tableName,
                                               typeName, tableSchema, typeSchema);
}

OCCI_STD_NAMESPACE::string SDOPointType::getSQLTypeName() const
{
    return OCCI_STD_NAMESPACE::string("MDSYS.SDO_POINT_TYPE");
}

void SDOPointType::getSQLTypeName(oracle::occi::Environment *env, void **schemaName,
                                  unsigned int &schemaNameLen, void **typeName, unsigned int &typeNameLen) const
{
    PObject::getSQLTypeName(env, &SDOPointType::readSQL, schemaName,
                            schemaNameLen, typeName, typeNameLen);
}

SDOPointType::SDOPointType()
{
}

void *SDOPointType::readSQL(void *ctxOCCI_)
{
    SDOPointType *objOCCI_ = new(ctxOCCI_) SDOPointType(ctxOCCI_);
    oracle::occi::AnyData streamOCCI_(ctxOCCI_);

    try
    {
        if (streamOCCI_.isNull())
            objOCCI_->setNull();
        else
            objOCCI_->readSQL(streamOCCI_);
    }
    catch (oracle::occi::SQLException& excep)
    {
        delete objOCCI_;
        excep.setErrorCtx(ctxOCCI_);
        return (void *)nullptr;
    }
    return (void *)objOCCI_;
}

void SDOPointType::readSQL(oracle::occi::AnyData& streamOCCI_)
{
    X = streamOCCI_.getNumber();
    Y = streamOCCI_.getNumber();
    Z = streamOCCI_.getNumber();
}

void SDOPointType::writeSQL(void *objectOCCI_, void *ctxOCCI_)
{
    SDOPointType *objOCCI_ = (SDOPointType *) objectOCCI_;
    oracle::occi::AnyData streamOCCI_(ctxOCCI_);

    try
    {
        if (objOCCI_->isNull())
            streamOCCI_.setNull();
        else
            objOCCI_->writeSQL(streamOCCI_);
    }
    catch (oracle::occi::SQLException& excep)
    {
        excep.setErrorCtx(ctxOCCI_);
    }
    return;
}

void SDOPointType::writeSQL(oracle::occi::AnyData& streamOCCI_)
{
    streamOCCI_.setNumber(X);
    streamOCCI_.setNumber(Y);
    streamOCCI_.setNumber(Z);
}

SDOPointType::~SDOPointType()
{
}

/*****************************************************************/
//  generated method implementations for the SDO_GEOMETRY object type.
/*****************************************************************/

oracle::occi::Number SDOGeometry::getSdo_gtype() const
{
    return SDO_GTYPE;
}

void SDOGeometry::setSdo_gtype(const oracle::occi::Number &value)
{
    SDO_GTYPE = value;
}

oracle::occi::Number SDOGeometry::getSdo_srid() const
{
    return SDO_SRID;
}

void SDOGeometry::setSdo_srid(const oracle::occi::Number &value)
{
    SDO_SRID = value;
}

SDOPointType * SDOGeometry::getSdo_point() const
{
    return SDO_POINT;
}

void SDOGeometry::setSdo_point(SDOPointType * value)
{
    SDO_POINT = value;
}

OCCI_STD_NAMESPACE::vector< oracle::occi::Number >& SDOGeometry::getSdo_elem_info()
{
    return SDO_ELEM_INFO;
}

const OCCI_STD_NAMESPACE::vector< oracle::occi::Number >& SDOGeometry::getSdo_elem_info() const
{
    return SDO_ELEM_INFO;
}

void SDOGeometry::setSdo_elem_info(const OCCI_STD_NAMESPACE::vector< oracle::occi::Number > &value)
{
    SDO_ELEM_INFO = value;
}

OCCI_STD_NAMESPACE::vector< oracle::occi::Number >& SDOGeometry::getSdo_ordinates()
{
    return SDO_ORDINATES;
}

const OCCI_STD_NAMESPACE::vector< oracle::occi::Number >& SDOGeometry::getSdo_ordinates() const
{
    return SDO_ORDINATES;
}

void SDOGeometry::setSdo_ordinates(const OCCI_STD_NAMESPACE::vector< oracle::occi::Number > &value)
{
    SDO_ORDINATES = value;
}

void *SDOGeometry::operator new(size_t size)
{
    return oracle::occi::PObject::operator new(size);
}

void *SDOGeometry::operator new(size_t size, const oracle::occi::Connection * sess,
                                const OCCI_STD_NAMESPACE::string& table)
{
    return oracle::occi::PObject::operator new(size, sess, table,
                                               (char *) "MDSYS.SDO_GEOMETRY");
}

void *SDOGeometry::operator new(size_t size, void *ctxOCCI_)
{
    return oracle::occi::PObject::operator new(size, ctxOCCI_);
}

void *SDOGeometry::operator new(size_t size,
                                const oracle::occi::Connection *sess,
                                const OCCI_STD_NAMESPACE::string &tableName,
                                const OCCI_STD_NAMESPACE::string &typeName,
                                const OCCI_STD_NAMESPACE::string &tableSchema,
                                const OCCI_STD_NAMESPACE::string &typeSchema)
{
    return oracle::occi::PObject::operator new(size, sess, tableName,
                                               typeName, tableSchema, typeSchema);
}

OCCI_STD_NAMESPACE::string SDOGeometry::getSQLTypeName() const
{
    return OCCI_STD_NAMESPACE::string("MDSYS.SDO_GEOMETRY");
}

void SDOGeometry::getSQLTypeName(oracle::occi::Environment *env, void **schemaName,
                                 unsigned int &schemaNameLen, void **typeName, unsigned int &typeNameLen) const
{
    PObject::getSQLTypeName(env, &SDOGeometry::readSQL, schemaName,
                            schemaNameLen, typeName, typeNameLen);
}

SDOGeometry::SDOGeometry()
{
    SDO_POINT = (SDOPointType *) 0;
}

void *SDOGeometry::readSQL(void *ctxOCCI_)
{
    SDOGeometry *objOCCI_ = new(ctxOCCI_) SDOGeometry(ctxOCCI_);
    oracle::occi::AnyData streamOCCI_(ctxOCCI_);

    try
    {
        if (streamOCCI_.isNull())
            objOCCI_->setNull();
        else
            objOCCI_->readSQL(streamOCCI_);
    }
    catch (oracle::occi::SQLException& excep)
    {
        delete objOCCI_;
        excep.setErrorCtx(ctxOCCI_);
        return (void *)nullptr;
    }
    return (void *)objOCCI_;
}

void SDOGeometry::readSQL(oracle::occi::AnyData& streamOCCI_)
{
    SDO_GTYPE = streamOCCI_.getNumber();
    SDO_SRID = streamOCCI_.getNumber();
    SDO_POINT = (SDOPointType *) streamOCCI_.getObject(&SDOPointType::readSQL);
    oracle::occi::getVector(streamOCCI_, SDO_ELEM_INFO);
    oracle::occi::getVector(streamOCCI_, SDO_ORDINATES);
}

void SDOGeometry::writeSQL(void *objectOCCI_, void *ctxOCCI_)
{
    SDOGeometry *objOCCI_ = (SDOGeometry *) objectOCCI_;
    oracle::occi::AnyData streamOCCI_(ctxOCCI_);

    try
    {
        if (objOCCI_->isNull())
            streamOCCI_.setNull();
        else
            objOCCI_->writeSQL(streamOCCI_);
    }
    catch (oracle::occi::SQLException& excep)
    {
        excep.setErrorCtx(ctxOCCI_);
    }
    return;
}

void SDOGeometry::writeSQL(oracle::occi::AnyData& streamOCCI_)
{
    streamOCCI_.setNumber(SDO_GTYPE);
    streamOCCI_.setNumber(SDO_SRID);
    streamOCCI_.setObject(SDO_POINT);
    oracle::occi::setVector(streamOCCI_, SDO_ELEM_INFO);
    oracle::occi::setVector(streamOCCI_, SDO_ORDINATES);
}

SDOGeometry::~SDOGeometry()
{
    delete SDO_POINT;
}
