

#if !defined CEHUIDATAINFO_HPP
#define CEHUIDATAINFO_HPP

#include <string>

struct cehuidataInfo
{
    /* data */
    std::string ID="";
    std::string NAME="";
    std::string Type="";
    std::string DIRECTION="";   //单向 双向
    std::string WIDTH = "10"
    std::string LEVEL="3";   //道路等级，1县道，2省道，3国道，4高速
    std::string geometryWkt="";

    cehuidataInfo()
    {
        ID="";
        NAME="";
        Type="";
        DIRECTION="";   //单向 双向
        WIDTH = "10"
        LEVEL="3";   //道路等级，1县道，2省道，3国道，4高速
        geometryWkt="";
    }
};





#endif // CEHUIDATAINFO_HPP
