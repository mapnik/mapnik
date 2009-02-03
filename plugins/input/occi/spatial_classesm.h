#ifndef SPATIAL_CLASSESM_ORACLE
# define SPATIAL_CLASSESM_ORACLE

#ifndef OCCI_ORACLE
# include <occi.h>
#endif

#ifndef SPATIAL_CLASSESH_ORACLE
# include "spatial_classesh.h"
#endif

void RegisterClasses(oracle::occi::Environment* envOCCI_);

#endif
