#if !defined GROUPINFO_HPP
#define GROUPINFO_HPP
#include <QString>
#include <QList>


struct GroupInfo
{
    QString id;
    QString name;
    QList<int> versions;
};

Q_DECLARE_METATYPE(GroupInfo)

#endif  //GROUPINFO_HPP





