#include <singleton_helper.hpp>
#include <singleton_helperb.hpp>
#include <iostream>
#include "singtest.hpp"

using namespace std;
using namespace utils;

int main( int argc, const char* argv[] ) {
    cout << "Main Begin." << endl;

    mapnik::singtest::instance().createStatement();

    Helper helper;

    HelperB helperb;

    cout << "Main End." << endl;
}
