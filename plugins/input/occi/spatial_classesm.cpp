
#ifndef SPATIAL_CLASSESM_ORACLE
# include "spatial_classesm.h"
#endif

void RegisterClasses(oracle::occi::Environment* envOCCI_)
{
    oracle::occi::Map *mapOCCI_ = envOCCI_->getMap();
    mapOCCI_->put("MDSYS.SDO_POINT_TYPE", &SDOPointType::readSQL, &SDOPointType::writeSQL);
    mapOCCI_->put("MDSYS.SDO_GEOMETRY", &SDOGeometry::readSQL, &SDOGeometry::writeSQL);
}
