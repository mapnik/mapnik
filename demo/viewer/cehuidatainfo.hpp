

#if !defined CEHUIDATAINFO_HPP
#define CEHUIDATAINFO_HPP

#include<string>

struct cehuidataInfo
{
    /* data */
    std::string ID;
    int KIND_NUM;
    std::string KIND;
    double WIDTH;
    int DIRECTION;
    double LENGTH;
    std::string PATHNAME;
    int OSMID;

};





#endif // CEHUIDATAINFO_HPP