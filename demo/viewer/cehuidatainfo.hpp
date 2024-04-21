

#if !defined CEHUIDATAINFO_HPP
#define CEHUIDATAINFO_HPP

#include <string>

struct cehuidataInfo
{
    /* data */
    std::string ID;
    std::string newID;
    std::string KIND_NUM;
    std::string KIND;
    std::string WIDTH;
    std::string DIRECTION;
    std::string LENGTH;
    std::string PATHNAME;
    int OSMID;
    std::string geometryWkt;
};





#endif // CEHUIDATAINFO_HPP
