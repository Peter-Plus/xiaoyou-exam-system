#include "course.h"

Course::Course()
    : courseId(0), teacherId(0)
{
}

Course::Course(int courseId, int teacherId, const QString &college, const QString &courseName)
    : courseId(courseId), teacherId(teacherId), college(college), courseName(courseName)
{
}

QString Course::getDisplayName() const
{
    if (!teacherName.isEmpty()) {
        return QString("%1 (%2)").arg(courseName, teacherName);
    }
    return courseName;
}

bool Course::isValid() const
{
    return courseId > 0 &&
           teacherId > 0 &&
           !college.isEmpty() &&
           !courseName.isEmpty();
}
