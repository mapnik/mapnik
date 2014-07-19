#include <iostream>
#include "singtest.hpp"

namespace utils {
  class MAPNIK_DECL Helper {
  public:
     Helper();

  private:
     static Helper myHelper __attribute__((shared));
  };

  class Testclass {};
}
