#ifndef COURSE_H
#define COURSE_H

#include <QString>

class Course
{
private:
    int courseId;
    int teacherId;
    QString college;
    QString courseName;
    QString teacherName; // 扩展字段

public:
    // 构造函数
    Course();
    Course(int courseId, int teacherId, const QString &college, const QString &courseName);

    // Getter方法
    int getCourseId() const { return courseId; }
    int getTeacherId() const { return teacherId; }
    QString getCollege() const { return college; }
    QString getCourseName() const { return courseName; }
    QString getTeacherName() const { return teacherName; }

    // Setter方法
    void setCourseId(int id) { courseId = id; }
    void setTeacherId(int id) { teacherId = id; }
    void setCollege(const QString &col) { college = col; }
    void setCourseName(const QString &name) { courseName = name; }
    void setTeacherName(const QString &name) { teacherName = name; }

    // 实用方法
    QString getDisplayName() const;
    bool isValid() const;
};

#endif // COURSE_H
