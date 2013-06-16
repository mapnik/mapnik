#include <iostream>

namespace utils {
  class HelperB {
  public:
	  HelperB();

  private:
     static HelperB myHelper __attribute__((shared));
  };
}
