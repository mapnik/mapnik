#include <iostream>
#include "singleton_helper.hpp"
#include "singtest.hpp"

using namespace std;
using namespace utils;
using namespace mapnik;


bool registered2 = singtest::instance().createStatement();

Helper Helper::myHelper;

Helper::Helper() {
  std::cout << "Helper() begin" << std::endl;
  singtest::instance().createStatement();
  std::cout << "Helper() end" << std::endl;
}
