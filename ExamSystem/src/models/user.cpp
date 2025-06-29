#include "user.h"

User::User():id(0),name(""),college(""){}

User::User(int id,const QString& name,const QString& college):id(id),name(name),college(college){}

int User::getId() const
{
    return id;
}

QString User::getName() const
{
    return name;
}

QString User::getCollege() const
{
    return college;
}

void User::setId(int id)
{
    this->id=id;
}

void User::setName(const QString& name)
{
    this->name=name;
}
void User::setCollege(const QString& college)
{
    this->college=college;
}
