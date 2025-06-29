#ifndef STUDENT_H
#define STUDENT_H

#include "user.h"

class Student:public User
{
public:
    Student();
    Student(int studentId,const QString& name,const QString& grade,const QString& college);
    QString getGrade() const;
    void setGrade(const QString& grade);

private:
    QString grade;
};


#endif // STUDENT_H
