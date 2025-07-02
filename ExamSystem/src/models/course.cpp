#include "course.h"

Course::Course()
    : courseId(0)
    , teacherId(0)
    , credits(3)
    , courseHours(48)
    , semester("2024-2025学年第一学期")
    , maxStudents(50)
    , status("开放选课")
{
}

Course::Course(int courseId, int teacherId, const QString &college, const QString &courseName,
               const QString &description, int credits, int courseHours,
               const QString &semester, int maxStudents, const QString &status)
    : courseId(courseId)
    , teacherId(teacherId)
    , college(college)
    , courseName(courseName)
    , description(description)
    , credits(credits)
    , courseHours(courseHours)
    , semester(semester)
    , maxStudents(maxStudents)
    , status(status)
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

QString Course::getDescription() const { return description; }
void    Course::setDescription(const QString &desc) { description = desc; }

int     Course::getCredits() const { return credits; }
void    Course::setCredits(int c) { credits = c; }

int     Course::getCourseHours() const { return courseHours; }
void    Course::setCourseHours(int h) { courseHours = h; }

QString Course::getSemester() const { return semester; }
void    Course::setSemester(const QString &sem) { semester = sem; }

int     Course::getMaxStudents() const { return maxStudents; }
void    Course::setMaxStudents(int m) { maxStudents = m; }

QString Course::getStatus() const { return status; }
void    Course::setStatus(const QString &s) { status = s; }

void Course::setStudentCount(int c) { studentCount = c; }
