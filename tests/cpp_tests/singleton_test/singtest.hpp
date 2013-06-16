#ifndef MAPNIK_SINGTEST_HPP
#define MAPNIK_SINGTEST_HPP

#include <iostream>
#include "mapnik/utils.hpp"
#include "mapnik/noncopyable.hpp"

namespace mapnik {

class singtest;

//Any of these work. The compiler will basically interperet all the same, dllimport.
//extern template class mapnik::singleton<mapnik::singtest, mapnik::CreateStatic>;
//extern template class __declspec(dllexport) mapnik::singleton<mapnik::singtest, mapnik::CreateStatic>;
//template class __declspec(dllexport) mapnik::singleton<mapnik::singtest, mapnik::CreateStatic>;

#if defined MAPNIK_EXPORTS
template class __declspec(dllexport) mapnik::singleton<mapnik::singtest, mapnik::CreateStatic>;
#else
template class __declspec(dllimport) mapnik::singleton<mapnik::singtest, mapnik::CreateStatic>;
#endif

class MAPNIK_DECL singtest
    : public singleton<singtest, CreateStatic>, private mapnik::noncopyable
{
    friend class CreateStatic<singtest>;
public:
    bool createStatement();
private:
    singtest();
    ~singtest();
};

}

#endif