#ifndef SPATIAL_CLASSESH_ORACLE
# define SPATIAL_CLASSESH_ORACLE

#ifndef OCCI_ORACLE
# include <occi.h>
#endif

class SDOPointType;
class SDOGeometry;

/************************************************************/
//  generated declarations for the SDO_POINT_TYPE object type.
/************************************************************/

class SDOPointType : public oracle::occi::PObject {

private:

   oracle::occi::Number X;
   oracle::occi::Number Y;
   oracle::occi::Number Z;

public:

   oracle::occi::Number getX() const;

   void setX(const oracle::occi::Number &value);

   oracle::occi::Number getY() const;

   void setY(const oracle::occi::Number &value);

   oracle::occi::Number getZ() const;

   void setZ(const oracle::occi::Number &value);

   void *operator new(size_t size);

   void *operator new(size_t size, const oracle::occi::Connection * sess,
      const OCCI_STD_NAMESPACE::string& table);

   void *operator new(size_t, void *ctxOCCI_);

   void *operator new(size_t size, const oracle::occi::Connection *sess,
      const OCCI_STD_NAMESPACE::string &tableName, 
      const OCCI_STD_NAMESPACE::string &typeName,
      const OCCI_STD_NAMESPACE::string &tableSchema, 
      const OCCI_STD_NAMESPACE::string &typeSchema);

   OCCI_STD_NAMESPACE::string getSQLTypeName() const;

   void getSQLTypeName(oracle::occi::Environment *env, void **schemaName,
      unsigned int &schemaNameLen, void **typeName,
      unsigned int &typeNameLen) const;

   SDOPointType();

   SDOPointType(void *ctxOCCI_) : oracle::occi::PObject (ctxOCCI_) { };

   static void *readSQL(void *ctxOCCI_);

   virtual void readSQL(oracle::occi::AnyData& streamOCCI_);

   static void writeSQL(void *objOCCI_, void *ctxOCCI_);

   virtual void writeSQL(oracle::occi::AnyData& streamOCCI_);

   ~SDOPointType();

};

/************************************************************/
//  generated declarations for the SDO_GEOMETRY object type.
/************************************************************/

class SDOGeometry : public oracle::occi::PObject {

private:

   oracle::occi::Number SDO_GTYPE;
   oracle::occi::Number SDO_SRID;
   SDOPointType * SDO_POINT;
   OCCI_STD_NAMESPACE::vector< oracle::occi::Number > SDO_ELEM_INFO;
   OCCI_STD_NAMESPACE::vector< oracle::occi::Number > SDO_ORDINATES;

public:

   oracle::occi::Number getSdo_gtype() const;

   void setSdo_gtype(const oracle::occi::Number &value);

   oracle::occi::Number getSdo_srid() const;

   void setSdo_srid(const oracle::occi::Number &value);

   SDOPointType * getSdo_point() const;

   void setSdo_point(SDOPointType * value);

   OCCI_STD_NAMESPACE::vector< oracle::occi::Number >& getSdo_elem_info();

   const OCCI_STD_NAMESPACE::vector< oracle::occi::Number >& getSdo_elem_info() const;

   void setSdo_elem_info(const OCCI_STD_NAMESPACE::vector< oracle::occi::Number > &value);

   OCCI_STD_NAMESPACE::vector< oracle::occi::Number >& getSdo_ordinates();

   const OCCI_STD_NAMESPACE::vector< oracle::occi::Number >& getSdo_ordinates() const;

   void setSdo_ordinates(const OCCI_STD_NAMESPACE::vector< oracle::occi::Number > &value);

   void *operator new(size_t size);

   void *operator new(size_t size, const oracle::occi::Connection * sess,
      const OCCI_STD_NAMESPACE::string& table);

   void *operator new(size_t, void *ctxOCCI_);

   void *operator new(size_t size, const oracle::occi::Connection *sess,
      const OCCI_STD_NAMESPACE::string &tableName, 
      const OCCI_STD_NAMESPACE::string &typeName,
      const OCCI_STD_NAMESPACE::string &tableSchema, 
      const OCCI_STD_NAMESPACE::string &typeSchema);

   OCCI_STD_NAMESPACE::string getSQLTypeName() const;

   void getSQLTypeName(oracle::occi::Environment *env, void **schemaName,
      unsigned int &schemaNameLen, void **typeName,
      unsigned int &typeNameLen) const;

   SDOGeometry();

   SDOGeometry(void *ctxOCCI_) : oracle::occi::PObject (ctxOCCI_) { };

   static void *readSQL(void *ctxOCCI_);

   virtual void readSQL(oracle::occi::AnyData& streamOCCI_);

   static void writeSQL(void *objOCCI_, void *ctxOCCI_);

   virtual void writeSQL(oracle::occi::AnyData& streamOCCI_);

   ~SDOGeometry();

};

#endif
