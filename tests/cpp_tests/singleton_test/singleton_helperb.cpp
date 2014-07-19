#include <iostream>
#include "Singleton_Helper.hpp"
#include "Singleton_Helperb.hpp"
#include "singtest.hpp"

using namespace std;
using namespace utils;

HelperB  HelperB::myHelper;

HelperB::HelperB() {
  std::cout << "HelperB() begin " << std::endl;
  mapnik::singtest::instance().createStatement();
  std::cout << "HelperB() end" << std::endl;
}
