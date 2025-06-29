#ifndef USER_H
#define USER_H

#include <QString>

class User
{
public:
    User();
    User(int id,const QString& name,const QString& college);

    int getId() const;
    QString getName() const;
    QString getCollege() const;

    void setId(int id);
    void setName(const QString& name);
    void setCollege(const QString& college);

protected:
    int id;
    QString name;
    QString college;

};






































#endif // USER_H
