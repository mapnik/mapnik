#include <iostream>

#ifdef HELPER_EXPORT
#  define HELPER_DECL __attribute__ ((dllexport))
#else
#  define HELPER_DECL __attribute__ ((dllimport))
#endif

namespace utils {
  class HELPER_DECL HelperB {
  public:
	  HelperB();

  private:
     static HelperB myHelper __attribute__((shared));
  };
}
