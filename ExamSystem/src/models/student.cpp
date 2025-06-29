#include "student.h"

Student::Student():User(),grade(""){}

Student::Student(int studentId,const QString& name,const QString& grade,const QString& college):User(studentId,name,college),grade(grade){}

QString Student::getGrade() const
{
    return grade;
}

void Student::setGrade(const QString& grade)
{
    this->grade=grade;
}
