#ifndef TEACHER_H
#define TEACHER_H

#include "user.h"

class Teacher:public User
{
public:
    Teacher();
    Teacher(int teacherId,const QString& name,const QString& college);

    int getTeacherId() const { return getId(); }
    void setTeacherId(int id) { setId(id); }
};

#endif // TEACHER_H
